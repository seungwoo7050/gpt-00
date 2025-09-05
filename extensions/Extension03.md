# Extension Guide 03: Securing the Server with TLS and Authentication

## 1. Objective

This guide addresses the critical security shortcomings of the project by implementing TLS/SSL encryption and a basic authentication mechanism. All network communication is currently in plaintext, making it vulnerable to eavesdropping and tampering.

Our goals are:
1.  **Implement TLS/SSL Encryption:** Use the OpenSSL library to encrypt all traffic for both the `LogServer` and the `IRCServer`.
2.  **Add IRC Password Authentication:** Implement support for the standard `PASS` command to require a password for clients connecting to the `IRCServer`.

## 2. Current Limitation (Legacy Code)

The server uses standard, unencrypted TCP sockets for all communication.

**Legacy Code (`src/LogServer.cpp`):**
```cpp
// [SEQUENCE: CPP-MVP2-36]
// 리스너 소켓 생성 및 초기화
void LogServer::initialize() {
    auto create_listener = [&](int port) -> int {
        // This creates a standard AF_INET, SOCK_STREAM socket with no encryption.
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) throw std::runtime_error("Socket creation failed");
        // ... bind, listen ...
        return fd;
    };
    listenFd_ = create_listener(port_);
    queryFd_ = create_listener(queryPort_);
    // ...
}
```

**Legacy Code (`src/IRCServer.cpp`):**
```cpp
// [SEQUENCE: CPP-MVP6-7]
void IRCServer::handleClient(int clientFd, const std::string& clientAddr) {
    char buffer[4096];
    while (running_.load()) {
        // All data is read and sent in plaintext using recv() and send().
        ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
        // ...
    }
}
```

Furthermore, the IRC server allows any client to connect and register without any form of authentication, which is a major security risk.

## 3. Proposed Solution: OpenSSL and `PASS` Command

We will leverage the OpenSSL library to wrap our raw socket file descriptors in a TLS/SSL layer.

-   **TLS/SSL Integration:**
    1.  An `SSL_CTX` (SSL Context) will be created at server startup, configured with a server certificate and private key.
    2.  When a new client connects, `accept()` will still return a standard file descriptor.
    3.  This descriptor will be used to create a new `SSL` object.
    4.  The TLS handshake will be performed using `SSL_accept()`.
    5.  All subsequent I/O will be performed using `SSL_read()` and `SSL_write()` instead of `recv()` and `send()`.

-   **IRC Authentication:**
    1.  The server will be configured with a master password.
    2.  We will add a handler for the `PASS` IRC command.
    3.  Clients must send the correct password via the `PASS` command *before* sending `NICK` and `USER` to register. Clients failing to do so will be disconnected.

## 4. Implementation Guidance

#### **Step 1: Generate a Self-Signed Certificate for Testing**

For development, you can generate a key and a self-signed certificate using the `openssl` command line tool. Create a `certs` directory in your project root.

```bash
mkdir -p certs
openssl req -x509 -newkey rsa:4096 -keyout certs/server.key -out certs/server.crt -sha256 -days 365 -nodes -subj "/C=XX/ST=State/L=City/O=Organization/OU=Unit/CN=localhost"
```

#### **Step 2: Modify `CMakeLists.txt` to Link OpenSSL**

Update your CMake configuration to find and link the OpenSSL libraries.

**Modified Code (`CMakeLists.txt`):**
```cmake
# ... after find_package(Threads REQUIRED)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Link OpenSSL to the main executable
# Note: This should be added to the existing target_link_libraries call
target_link_libraries(logcaster-cpp PRIVATE Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

# Also link to the test executable if tests need SSL
target_link_libraries(run_tests PRIVATE logcaster-cpp GTest::gtest_main OpenSSL::SSL OpenSSL::Crypto)
```

#### **Step 3: Create an SSL Utility Class**

To avoid code duplication, create a simple wrapper for initializing OpenSSL and the `SSL_CTX`.

**New File (`include/SslManager.h`):**
```cpp
#pragma once

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#include <stdexcept>

class SslManager {
public:
    SslManager(const std::string& cert_path, const std::string& key_path) {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();

        ctx_ = SSL_CTX_new(TLS_server_method());
        if (!ctx_) {
            throw std::runtime_error("Unable to create SSL context");
        }

        if (SSL_CTX_use_certificate_file(ctx_, cert_path.c_str(), SSL_FILETYPE_PEM) <= 0) {
            ERR_print_errors_fp(stderr);
            throw std::runtime_error("Failed to load certificate file");
        }

        if (SSL_CTX_use_PrivateKey_file(ctx_, key_path.c_str(), SSL_FILETYPE_PEM) <= 0 ) {
            ERR_print_errors_fp(stderr);
            throw std::runtime_error("Failed to load private key file");
        }
    }

    ~SslManager() {
        if (ctx_) SSL_CTX_free(ctx_);
    }

    SSL_CTX* getContext() const {
        return ctx_;
    }

private:
    SSL_CTX* ctx_ = nullptr;
};
```

#### **Step 4: Integrate TLS into `IRCServer`**

We need to modify `IRCClient` to hold an `SSL*` pointer and update the server to use `SSL_read`/`SSL_write`.

**Modified Code (`include/IRCClient.h`):**
```cpp
#include <openssl/ssl.h>

class IRCClient {
    // ...
private:
    int fd_;
    SSL* ssl_ = nullptr; // Add SSL pointer
    bool authenticated_ = false; // For PASS command
    // ...
public:
    void setSsl(SSL* ssl) { ssl_ = ssl; }
    SSL* getSsl() const { return ssl_; }
    // ... add getters/setters for authenticated_
};
```

**Modified Code (`src/IRCServer.cpp`):**
```cpp
// At the top
#include "SslManager.h"

// In IRCServer class definition (header)
std::unique_ptr<SslManager> sslManager_;

// In IRCServer constructor
IRCServer::IRCServer(...) {
    // ...
    // Initialize SslManager
    sslManager_ = std::make_unique<SslManager>("certs/server.crt", "certs/server.key");
}

// In acceptClients (or epoll equivalent)
void IRCServer::acceptClients() {
    // ... after accept() call ...
    int clientFd = accept(...);
    if (clientFd >= 0) {
        SSL* ssl = SSL_new(sslManager_->getContext());
        SSL_set_fd(ssl, clientFd);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(clientFd);
            continue;
        }

        auto client = clientManager_->addClient(clientFd, clientAddress);
        client->setSsl(ssl);
        // ... enqueue task
    }
}

// In handleClient (or readIrcClientData)
void IRCServer::handleClient(int clientFd, ...) {
    auto client = clientManager_->getClientByFd(clientFd);
    SSL* ssl = client->getSsl();
    char buffer[4096];

    while (running_.load()) {
        // Replace recv() with SSL_read()
        ssize_t bytesRead = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        
        if (bytesRead > 0) {
            // ... process data ...
        } else {
            // Handle SSL-specific errors (e.g., for non-blocking sockets)
            int err = SSL_get_error(ssl, bytesRead);
            if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
                // For non-blocking, just continue
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            break; // Connection closed or real error
        }
    }
    // ... cleanup: SSL_shutdown(ssl), SSL_free(ssl), close(clientFd)
}

// You must also replace all `send()` calls with `SSL_write()`.
// For example, in IRCChannel.cpp:
void IRCChannel::broadcast(...) {
    // ...
    // send(client->getFd(), ...);
    SSL_write(client->getSsl(), message.c_str(), message.length());
    // ...
}
```

#### **Step 5: Implement `PASS` Authentication in `IRCCommandHandler`**

First, add a password member to `IRCServer` and pass it during construction. Then, update the command handler.

**Modified Code (`src/IRCCommandHandler.cpp`):**
```cpp
// Add a new handler for the PASS command
void IRCCommandHandler::handlePass(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& command) {
    if (client->isRegistered()) {
        sendNumericReply(client, ERR_ALREADYREGISTRED, ":You may not reregister");
        return;
    }
    if (command.params.empty()) {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
        return;
    }

    // server_->getServerPassword() is a new method you need to add to IRCServer
    if (command.params[0] == server_->getServerPassword()) {
        client->setAuthenticated(true);
    } else {
        sendNumericReply(client, ERR_PASSWDMISMATCH, ":Password incorrect");
        // Optionally disconnect the client here
    }
}

// Modify NICK and USER handlers to check for authentication
void IRCCommandHandler::handleNick(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& command) {
    if (!client->isAuthenticated()) {
        sendNumericReply(client, ERR_PASSWDMISMATCH, ":Password required");
        // Disconnect the client for security
        server_->getClientManager()->removeClient(client->getFd());
        return;
    }
    // ... rest of NICK logic
}

// In handleCommand, route the "PASS" command
if (cmd.command == "PASS") {
    handlePass(client, cmd);
}
```

## 5. Verification

1.  **Start the server.** It should now load the certificate and listen for secure connections. You may want to use a standard TLS port like 6697 for the IRC server.

2.  **Test Log Server with `openssl s_client`:**
    Assuming your log server is on port 9998:
    ```bash
    # Connect to the server
    openssl s_client -connect localhost:9998
    
    # Once connected, you can type a log message and press Enter
    This is a secure log message.
    ```
    The server should receive the message. The `s_client` command will also print the server's certificate details.

3.  **Test IRC Server with a TLS-aware client (e.g., irssi):**
    -   Start `irssi`.
    -   Add the server with the correct port and password:
        `/server add -network MySecureNet -password <your_password> -ssl localhost 6697`
    -   Connect to the server:
        `/connect MySecureNet`

    If you try to connect without the `-ssl` flag or with the wrong password, the server should reject your connection. If you connect correctly, all IRC communication will be encrypted.
