# DevHistory CPP-MVP-1: C++ OOP Design Reconstruction

## 1. Introduction

This document details the reconstruction of the C++ version of Minimum Viable Product 1 (MVP1). Following the successful implementation of the C-based server, this phase focuses on re-implementing the same core functionality using C++.

The primary objective is to leverage C++'s object-oriented programming (OOP) features and modern idioms to create a safer, more maintainable, and more extensible codebase. This MVP moves away from the procedural C approach, embracing classes, RAII (Resource Acquisition Is Initialization), and exception handling to manage complexity and resources.

**Core Architectural Changes:**
- **OOP Design:** The logic is encapsulated within classes (`LogServer`, `Logger`, `ConsoleLogger`).
- **RAII for Resource Management:** Resources like sockets are managed by the `LogServer` class, which automatically handles their acquisition in the constructor and release in the destructor.
- **Exception Handling:** C-style return code checks are replaced with C++ exceptions for more robust and clearer error handling.
- **Concurrency Model:** This MVP uses a simple **"thread-per-client"** model, where each incoming connection is handled by a new `std::thread`. This contrasts with the C version's `select()`-based model and is chosen for its implementation simplicity in this initial C++ version.

## 2. In-depth Analysis for Technical Interviews

### Architectural Choices
- **Why a Class-Based, OOP Approach?**
  - **Choice:** Instead of C-style functions operating on a struct, we've encapsulated all server logic and state within the `LogServer` class.
  - **Reasoning:** Encapsulation bundles data (like the listening socket) with the methods that operate on it, reducing the risk of inconsistent state. It makes the code easier to understand and maintain. The use of an abstract `Logger` class provides a clear interface for extension, allowing for different logging strategies (e.g., file, network) to be added later without changing the `LogServer` code (Dependency Injection).
- **Why Thread-per-Client?**
  - **Choice:** Each client connection is managed in its own `std::thread`.
  - **Reasoning:** This is one of the simplest concurrency models to implement and reason about. The logic for handling a single client is self-contained in the `handle_client` function, and the OS handles the scheduling of threads. This was a deliberate choice for MVP1 to get a C++ version running quickly and to contrast with the C version's I/O multiplexing.
  - **Trade-offs:** This model is not scalable. It does not solve the "C10k problem." Creating a thread for every client is resource-intensive (memory and CPU context-switching). For a large number of connections, this would lead to performance degradation and resource exhaustion. This limitation is a key driver for implementing a more advanced thread pool in the next MVP.

### Technical Trade-offs
- **RAII and Exceptions vs. Manual Checks**
  - **Trade-off:** The constructor of `LogServer` immediately throws a `std::runtime_error` if any part of the setup (socket, bind, listen) fails. This is much cleaner than the C version, which required checking the return code of every function and performing manual cleanup.
  - **Benefit:** This makes the code significantly safer. It's impossible to have a partially constructed `LogServer` object. If the constructor completes, the object is valid and ready. The destructor guarantees that resources are released, even if an error occurs later. This eliminates entire classes of resource-leak bugs.

### Challenges Faced & Design Decisions
- **Signal Handling with Objects**
  - **Challenge:** How does a C-style signal handler (`signal(SIGINT, ...)`), which is a global function, gracefully shut down a C++ object?
  - **Decision:** A global `std::unique_ptr<LogServer> server_ptr` is used. The signal handler calls `server_ptr.reset()`, which safely invokes the `LogServer` destructor. This triggers the RAII cleanup (closing sockets, joining threads). While still relying on a global pointer, it's a C++-idiomatic way to bridge the gap between C-style signals and object-oriented resource management.
- **Managing Thread Lifetime**
  - **Challenge:** The `client_threads_` vector grows indefinitely as new clients connect. The threads are never removed from the vector after they exit.
  - **Decision:** For MVP1, this is accepted as a known limitation. A production-ready server would need a mechanism to periodically iterate through the vector, `join()` threads that have finished, and remove them from the collection to prevent unbounded memory growth.

### Production Considerations & Future Improvements
- **Concurrency Model:** As noted, the thread-per-client model is unsuitable for production. The immediate next step is to replace it with a thread pool architecture to reuse a fixed number of worker threads, dramatically improving scalability and performance.
- **Dependency Injection:** The `LogServer` constructor currently hard-codes the creation of a `ConsoleLogger`. A more flexible design would use dependency injection, allowing the caller to pass in any object that conforms to the `Logger` interface. This makes the `LogServer` more reusable and easier to test.
- **Graceful Shutdown:** The current shutdown mechanism is abrupt (`exit(signum)` in the signal handler). A more graceful shutdown would involve stopping the `accept_connections` loop, waiting for all client threads to finish their work, and then exiting.

## 3. Sequence List

This section contains the complete source code and build scripts for the C++-MVP-1, broken down by their logical implementation sequence.

### [SEQUENCE: CPP-MVP1-1] `CMakeLists.txt` - CMake Version and Project
```cmake
# CMake minimum required version
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(LogCaster-CPP VERSION 1.0)
```

### [SEQUENCE: CPP-MVP1-2] `CMakeLists.txt` - C++ Standard
```cmake
# Set C++ standard (C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
```

### [SEQUENCE: CPP-MVP1-3] `CMakeLists.txt` - Executable and Sources
```cmake
# Add executable
add_executable(logcaster-cpp
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
)
```

### [SEQUENCE: CPP-MVP1-4] `CMakeLists.txt` - Include Directories
```cmake
# Add include directories
target_include_directories(logcaster-cpp PUBLIC include)
```

### [SEQUENCE: CPP-MVP1-5] `CMakeLists.txt` - Link Libraries
```cmake
# Link thread library
find_package(Threads REQUIRED)
target_link_libraries(logcaster-cpp PRIVATE Threads::Threads)
```

### [SEQUENCE: CPP-MVP1-6] `include/Logger.h` - Abstract Logger Class
```cpp
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>

// Abstract base class for logging
class Logger {
public:
    virtual ~Logger() = default; // Virtual destructor
    virtual void log(const std::string& message) = 0; // Pure virtual function
};
```

### [SEQUENCE: CPP-MVP1-7] `include/Logger.h` - Concrete ConsoleLogger Class
```cpp
// Concrete class for logging to the console
class ConsoleLogger : public Logger {
public:
    void log(const std::string& message) override;
};

#endif // LOGGER_H
```

### [SEQUENCE: CPP-MVP1-8] `src/Logger.cpp` - Implementation
```cpp
#include "Logger.h"

void ConsoleLogger::log(const std::string& message) {
    // Create a timestamp for the current time
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    // Print the timestamp and message to the console
    std::cout << "[" << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S") << "] "
              << message << std::endl;
}
```

### [SEQUENCE: CPP-MVP1-9] `include/LogServer.h` - Class Definition
```cpp
#ifndef LOG_SERVER_H
#define LOG_SERVER_H

#include <vector>
#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include "Logger.h"

#define DEFAULT_PORT 9999
#define BUFFER_SIZE 4096

class LogServer {
public:
    explicit LogServer(int port = DEFAULT_PORT);
    ~LogServer();

    // Delete copy/move constructors and assignment operators
    LogServer(const LogServer&) = delete;
    LogServer& operator=(const LogServer&) = delete;

    void run();

private:
    void accept_connections();
    void handle_client(int client_socket);

    int port_;
    int listen_fd_;
    std::atomic<bool> running_;
    std::unique_ptr<Logger> logger_;
    
    std::vector<std::thread> client_threads_;
    std::mutex client_mutex_;
};

#endif // LOG_SERVER_H
```

### [SEQUENCE: CPP-MVP1-10] `src/LogServer.cpp` - Constructor
```cpp
#include "LogServer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <iostream>

// Constructor: acquire resources (socket, bind, listen)
LogServer::LogServer(int port)
    : port_(port), listen_fd_(-1), running_(false) {
    
    logger_ = std::make_unique<ConsoleLogger>();

    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("Failed to set socket options");
    }

    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(listen_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(listen_fd_, 10) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }

    logger_->log("LogCaster-CPP server starting on port " + std::to_string(port_));
}
```

### [SEQUENCE: CPP-MVP1-11] `src/LogServer.cpp` - Destructor
```cpp
// Destructor: release resources (join threads, close socket)
LogServer::~LogServer() {
    running_ = false;
    if (listen_fd_ >= 0) {
        close(listen_fd_); // This will unblock the accept() call
    }
    
    // Wait for all client threads to finish
    for (auto& t : client_threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
    logger_->log("Server shut down gracefully.");
}
```

### [SEQUENCE: CPP-MVP1-12] `src/LogServer.cpp` - `run()` and `accept_connections()`
```cpp
void LogServer::run() {
    running_ = true;
    logger_->log("Server is running and waiting for connections...");

    // Start a separate thread for accepting connections
    std::thread acceptor_thread(&LogServer::accept_connections, this);
    acceptor_thread.join(); // Main thread waits for the acceptor to finish
}

void LogServer::accept_connections() {
    while (running_) {
        int client_socket = accept(listen_fd_, nullptr, nullptr);
        if (client_socket < 0) {
            if (!running_) break; // Exit loop if server is shutting down
            logger_->log("Failed to accept connection");
            continue;
        }

        logger_->log("Accepted new connection on socket " + std::to_string(client_socket));

        // Create and manage a new thread for each client
        std::lock_guard<std::mutex> lock(client_mutex_);
        client_threads_.emplace_back(&LogServer::handle_client, this, client_socket);
    }
}
```

### [SEQUENCE: CPP-MVP1-13] `src/LogServer.cpp` - `handle_client()`
```cpp
// Handles data processing for an individual client
void LogServer::handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    while (running_) {
        std::memset(buffer, 0, BUFFER_SIZE);
        int nbytes = read(client_socket, buffer, sizeof(buffer) - 1);

        if (nbytes <= 0) {
            if (nbytes < 0) {
                logger_->log("Read error from socket " + std::to_string(client_socket));
            }
            logger_->log("Client on socket " + std::to_string(client_socket) + " disconnected.");
            break; // Exit loop to terminate the thread
        }
        
        // Output the received data via the logger
        std::string message(buffer, nbytes);
        // Remove trailing newline for cleaner logs
        if (!message.empty() && message.back() == '\n') {
            message.pop_back();
        }
        logger_->log("[LOG] from socket " + std::to_string(client_socket) + ": " + message);
    }
    close(client_socket);
}
```

### [SEQUENCE: CPP-MVP1-14] `src/main.cpp` - Entry Point and Signal Handling
```cpp
#include "LogServer.h"
#include <iostream>
#include <csignal>

std::unique_ptr<LogServer> server_ptr;

void signal_handler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    // Resetting the unique_ptr triggers the LogServer destructor
    server_ptr.reset(); 
    exit(signum);
}

int main(int argc, char* argv[]) {
    // Register signal handler
    signal(SIGINT, signal_handler);

    try {
        // Create and run the LogServer object
        int port = (argc > 1) ? std::stoi(argv[1]) : DEFAULT_PORT;
        server_ptr = std::make_unique<LogServer>(port);
        server_ptr->run();
    } catch (const std::exception& e) {
        // Exception handling
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## 4. Build Verification

The project was built and tested successfully. The C++ server was run on port 9998 to avoid conflicts.

### Build Log
```
$ mkdir -p build && cd build && cmake .. && make
-- The CXX compiler identification is GNU 11.4.0
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Found Threads: TRUE  
-- Configuring done
-- Generating done
-- Build files have been written to: /home/user/gpt-00/log-caster-cpp-reconstructed/build
[ 25%] Building CXX object CMakeFiles/logcaster-cpp.dir/src/main.cpp.o
[ 50%] Building CXX object CMakeFiles/logcaster-cpp.dir/src/LogServer.cpp.o
[ 75%] Building CXX object CMakeFiles/logcaster-cpp.dir/src/Logger.cpp.o
[100%] Linking CXX executable logcaster-cpp
[100%] Built target logcaster-cpp
```

### Test Execution Log

**Server Output:**
```
$ ./log-caster-cpp-reconstructed/build/logcaster-cpp 9998
[2025-09-05 10:57:39] LogCaster-CPP server starting on port 9998
[2025-09-05 10:57:39] Server is running and waiting for connections...
[2025-09-05 10:57:45] Accepted new connection on socket 5
[2025-09-05 10:57:46] [LOG] from socket 5: Client 1, message 1
... 
[2025-09-05 10:57:47] Client on socket 5 disconnected.
```

**Test Client Output:**
```
$ python3 ./log-caster-cpp-reconstructed/tests/test_client.py single && ...
--- Starting test with 1 clients ---

Test with 1 clients finished in 1.00 seconds.
--- Starting test with 5 clients ---

Test with 5 clients finished in 1.20 seconds.
--- Starting test with 50 clients ---

Test with 50 clients finished in 2.97 seconds.
```
