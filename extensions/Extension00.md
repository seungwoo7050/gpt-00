# Extension Guide 00: Implementing a Scalable Asynchronous I/O Model

## 1. Objective

This guide details the process of refactoring the networking layer of both `LogServer` and `IRCServer` from their current models to a modern, highly scalable asynchronous I/O model using `epoll` on Linux. This change addresses the C10K problem, enabling the server to handle tens of thousands of concurrent connections efficiently.

- **`LogServer`**: Replace the `select()`-based event loop with an `epoll`-based one.
- **`IRCServer`**: Replace the "thread-per-client" model with the same `epoll`-based event loop to manage all client connections within a single thread.

## 2. Current Limitation (Legacy Code)

The existing server implementations have significant scalability bottlenecks.

### 2.1. `LogServer`: `select()`-based Polling

The `LogServer` uses `select()`, which has a well-known limitation: it requires scanning through all file descriptors in the `fd_set` on every iteration. This becomes highly inefficient as the number of connections grows.

**Legacy Code (`src/LogServer.cpp`):**
```cpp
// [SEQUENCE: CPP-MVP1-12]
// [SEQUENCE: CPP-MVP2-37]
// 메인 이벤트 루프
void LogServer::runEventLoop() {
    while (running_) {
        fd_set read_fds = masterSet_;
        int max_fd = std::max(listenFd_, queryFd_);
        // The select() call is the bottleneck. It has O(N) complexity.
        int result = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
        if (result < 0 && errno != EINTR) {
            logger_->log("Select error");
            break;
        }

        if (FD_ISSET(listenFd_, &read_fds)) {
            handleNewConnection(listenFd_, false);
        }
        if (FD_ISSET(queryFd_, &read_fds)) {
            handleNewConnection(queryFd_, true);
        }
    }
}
```

### 2.2. `IRCServer`: Thread-per-Client Model

The `IRCServer` dedicates a thread from a thread pool to each connected client for its entire lifecycle. While better than creating a new thread for every connection, this model still ties up thread resources for I/O-bound tasks and does not scale well beyond a few hundred clients due to context-switching overhead and memory consumption.

**Legacy Code (`src/IRCServer.cpp`):**
```cpp
// [SEQUENCE: CPP-MVP6-7]
void IRCServer::acceptClients() {
    while (running_.load()) {
        // ... accept logic ...
        int clientFd = accept(listenFd_, (struct sockaddr*)&clientAddr, &addrLen);
        
        if (clientFd >= 0) {
            // ... client setup ...

            // This is the main issue: a thread is dispatched to block on one client.
            threadPool_->enqueue([this, clientFd, clientAddress]() {
                handleClient(clientFd, clientAddress);
            });
        }
    }
}

void IRCServer::handleClient(int clientFd, const std::string& clientAddr) {
    char buffer[4096];
    // This loop blocks on recv() for a single client.
    while (running_.load()) {
        ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
        // ... processing logic ...
    }
    // ... cleanup ...
}
```

## 3. Proposed Solution: `epoll`

`epoll` is a Linux-specific I/O event notification facility. Unlike `select`, it operates in O(1) time complexity. The kernel monitors file descriptors for you and provides a list of only the ones that are ready for I/O, eliminating the need to iterate over every single descriptor.

We will create a unified event loop that:
1.  Creates an `epoll` instance.
2.  Adds the listening sockets (`LogServer` and `IRCServer`) to the `epoll` instance.
3.  When a new client connects, its socket is also added to the `epoll` instance in non-blocking mode.
4.  The main loop waits on `epoll_wait()`, which blocks until one or more registered sockets have I/O events.
5.  It processes events only for the ready sockets, whether it's accepting a new connection or reading data from an existing client.

## 4. Implementation Guidance

We will refactor `LogServer` first, then apply a similar pattern to `IRCServer`.

### 4.1. Refactoring `LogServer` with `epoll`

#### **Step 1: Add `epoll` headers and member variables**

Modify `LogServer.h` to include the `epoll` file descriptor.

**New Code (`include/LogServer.h`):**
```cpp
#include <sys/epoll.h> // Add this header

class LogServer {
// ... existing members
private:
    // ...
    int listenFd_;
    int queryFd_;
    int epollFd_; // Add this for epoll instance
    fd_set masterSet_; // This will be removed
    // ...
};
```

#### **Step 2: Update `initialize` to create an `epoll` instance**

In `LogServer.cpp`, modify `initialize()` to create the `epoll` instance and register the listening sockets.

**Modified Code (`src/LogServer.cpp`):**
```cpp
#include <sys/epoll.h> // Ensure header is included

void LogServer::initialize() {
    auto create_listener = [&](int port) -> int {
        // ... (same as before, but also set non-blocking)
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) throw std::runtime_error("Socket creation failed");
        
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) throw std::runtime_error("Bind failed");
        if (listen(fd, 128) < 0) throw std::runtime_error("Listen failed");
        return fd;
    };
    listenFd_ = create_listener(port_);
    queryFd_ = create_listener(queryPort_);
    
    // Create epoll instance
    epollFd_ = epoll_create1(0);
    if (epollFd_ == -1) {
        throw std::runtime_error("Failed to create epoll instance");
    }

    auto add_to_epoll = [&](int fd) {
        epoll_event event;
        event.data.fd = fd;
        event.events = EPOLLIN | EPOLLET; // Use Edge-Triggered mode
        if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) == -1) {
            throw std::runtime_error("Failed to add listener to epoll");
        }
    };

    add_to_epoll(listenFd_);
    add_to_epoll(queryFd_);
}
```

#### **Step 3: Replace `runEventLoop` with the `epoll_wait` loop**

This is the core change. The `select` loop is replaced entirely.

**Modified Code (`src/LogServer.cpp`):**
```cpp
void LogServer::runEventLoop() {
    std::vector<epoll_event> events(128);

    while (running_) {
        int n_events = epoll_wait(epollFd_, events.data(), events.size(), -1);
        if (n_events < 0 && errno != EINTR) {
            logger_->log("epoll_wait error");
            break;
        }

        for (int i = 0; i < n_events; ++i) {
            int fd = events[i].data.fd;
            if (fd == listenFd_ || fd == queryFd_) {
                // New connection
                handleNewConnection(fd, fd == queryFd_);
            } else {
                // Data from an existing client. Read and process.
                if (events[i].events & EPOLLIN) {
                    handleClientTask(fd);
                }
            }
        }
    }
}
```

#### **Step 4: Adapt `handleNewConnection` and `handleClientTask`**

The connection handler now accepts all pending connections and adds them to epoll. The client handler reads all available data non-blockingly.

**Modified Code (`src/LogServer.cpp`):**
```cpp
void LogServer::handleNewConnection(int listener_fd, bool is_query_port) {
    while (true) {
        int client_fd = accept(listener_fd, nullptr, nullptr);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break; // Done
            return;
        }

        int flags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

        if (client_count_ >= 1024) {
            close(client_fd);
            continue;
        }

        epoll_event event;
        event.data.fd = client_fd;
        event.events = EPOLLIN | EPOLLET; // Edge-triggered read
        if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, client_fd, &event) == -1) {
            close(client_fd);
        } else {
            client_count_++;
            // Associate fd with query type if needed, e.g., using a map
        }
    }
}

// This function is now non-blocking and called from the event loop.
void LogServer::handleClientTask(int client_fd) {
    // Note: The original code distinguished between log and query tasks.
    // A robust implementation would use a map to check if client_fd is a query client.
    // For this example, we assume all readable clients are log clients for simplicity.
    char buffer[4096];
    while (true) {
        ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (nbytes > 0) {
            buffer[nbytes] = '\0';
            std::string log_message(buffer, nbytes);
            logBuffer_->push(log_message, "info", "unknown");
            if (persistence_) {
                persistence_->write(log_message);
            }
        } else if (nbytes == 0) {
            // Client disconnected
            epoll_ctl(epollFd_, EPOLL_CTL_DEL, client_fd, nullptr);
            close(client_fd);
            client_count_--;
            break;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break; // No more data
            // Real error
            epoll_ctl(epollFd_, EPOLL_CTL_DEL, client_fd, nullptr);
            close(client_fd);
            client_count_--;
            break;
        }
    }
}
```

### 4.2. Refactoring `IRCServer` with `epoll`

The same principles apply to `IRCServer`. The goal is to eliminate the `acceptClients` thread and the `handleClient` blocking calls, managing all I/O in a single event loop.

#### **Step 1: Add `epoll` members and create a central event loop**

You would add an `epollFd_` and an event loop thread to `IRCServer`. The `start()` method would create this thread.

**Modified Code (`src/IRCServer.cpp`):**
```cpp
void IRCServer::start() {
    if (running_.load()) return;
    
    try {
        setupSocket(); // Creates listenFd_ and sets it to non-blocking
        
        epollFd_ = epoll_create1(0);
        if (epollFd_ == -1) throw std::runtime_error("epoll_create1 failed");

        epoll_event event;
        event.data.fd = listenFd_;
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, listenFd_, &event) == -1) {
            throw std::runtime_error("epoll_ctl failed for listen socket");
        }

        // ... other initializations ...
        
        running_ = true;
        
        // Start the single event loop thread, replacing acceptThread_
        eventLoopThread_ = std::thread(&IRCServer::runEventLoop, this);
        
        std::cout << "IRC server started on port " << port_ << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to start IRC server: " << e.what() << std::endl;
        throw;
    }
}
```

#### **Step 2: Implement the `epoll` event loop**

This new `runEventLoop` method will be the heart of the `IRCServer`.

**New Code (`src/IRCServer.cpp`):**
```cpp
void IRCServer::runEventLoop() {
    std::vector<epoll_event> events(MAX_CLIENTS);

    while(running_.load()) {
        int n_events = epoll_wait(epollFd_, events.data(), events.size(), -1);

        for (int i = 0; i < n_events; ++i) {
            int fd = events[i].data.fd;

            if (fd == listenFd_) {
                acceptNewIrcClient();
            } else if (events[i].events & EPOLLIN) {
                // Data is available to read from a client.
                // Business logic can be dispatched to the thread pool to avoid blocking the event loop.
                auto client = clientManager_->getClientByFd(fd);
                if (client) {
                    threadPool_->enqueue([this, client]() {
                        readIrcClientData(client);
                    });
                }
            } else {
                // Handle other events like EPOLLOUT, EPOLLERR, etc.
                handleClientDisconnection(fd);
            }
        }
    }
}
```

#### **Step 3: Create non-blocking helper functions**

**New Code (`src/IRCServer.cpp`):**
```cpp
void IRCServer::acceptNewIrcClient() {
    while(true) {
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        int clientFd = accept(listenFd_, (struct sockaddr*)&clientAddr, &addrLen);

        if (clientFd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            return;
        }

        // ... (logic from old acceptClients to add client to clientManager_) ...
        auto client = clientManager_->addClient(clientFd, clientAddress);
        if (!client) { close(clientFd); continue; }

        int flags = fcntl(clientFd, F_GETFL, 0);
        fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

        epoll_event event;
        event.data.fd = clientFd;
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, clientFd, &event) == -1) {
            clientManager_->removeClient(clientFd);
            close(clientFd);
        }
    }
}

// Note: This function now takes a shared_ptr<IRCClient> to ensure thread safety.
void IRCServer::readIrcClientData(std::shared_ptr<IRCClient> client) {
    int clientFd = client->getFd();
    char buffer[4096];
    
    // A per-client buffer is needed for robust parsing.
    // We'll use a temporary string here for simplicity. 
    std::string& incomplete_buffer = client->getReadBuffer(); 

    while (true) {
        ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead > 0) {
            incomplete_buffer.append(buffer, bytesRead);
        } else if (bytesRead == 0) {
            handleClientDisconnection(clientFd);
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break; // No more data
            handleClientDisconnection(clientFd);
            return;
        }
    }

    // Process all complete messages in the buffer
    size_t pos = 0;
    while ((pos = incomplete_buffer.find("\r\n")) != std::string::npos) {
        std::string line = incomplete_buffer.substr(0, pos);
        incomplete_buffer.erase(0, pos + 2);
        if (!line.empty()) {
            processClientData(clientFd, line);
        }
    }
}

void IRCServer::handleClientDisconnection(int clientFd) {
    epoll_ctl(epollFd_, EPOLL_CTL_DEL, clientFd, nullptr);
    auto client = clientManager_->getClientByFd(clientFd);
    if (client) {
        IRCCommandParser::IRCCommand quitCmd;
        quitCmd.command = "QUIT";
        quitCmd.trailing = "Connection closed";
        commandHandler_->handleCommand(client, quitCmd);
        clientManager_->removeClient(clientFd);
    }
    close(clientFd);
}
```
*Note: To make `readIrcClientData` truly robust, you would need to add a `std::string readBuffer_` to the `IRCClient` class to handle partial messages received across multiple `recv` calls.*

## 5. Verification

After refactoring, you can verify the functionality using the same test scripts as before.
1.  Start the `logcaster-cpp` executable.
2.  Run `tests/test_client.py` to check the `LogServer`.
3.  Run `tests/test_irc.py` to check the `IRCServer`.

The external behavior should be identical, but the server will now be capable of handling a much larger number of connections with significantly lower resource usage. You can use tools like `netcat` or custom scripts to open thousands of connections to test the new scalability.
