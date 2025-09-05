# DevHistory CPP-MVP-6: IRC Server Integration

## 1. Introduction

This document details the reconstruction of the sixth and final Minimum Viable Product (MVP6) for the C++ version of LogCaster. This phase integrates a full-featured IRC server into the existing LogCaster, allowing users to monitor logs in real-time using standard IRC clients.

**Key Features Added:**
- **IRC Server:** A complete IRC server is added to the project, running on a separate port.
- **Log Streaming:** The IRC server is integrated with the LogBuffer, allowing real-time streaming of logs to IRC channels.
- **IRC Commands:** The server supports a subset of standard IRC commands, including `NICK`, `USER`, `JOIN`, `PART`, `PRIVMSG`, `QUIT`, `PING`, `LIST`, and `NAMES`.

## 2. In-depth Analysis for Technical Interviews

### Architectural Choices
- **Modular IRC Component:** The IRC server is designed as a modular component, with its own set of classes for managing clients, channels, and commands. This separation of concerns makes the code easier to maintain and extend.
- **Asynchronous I/O:** The IRC server uses non-blocking sockets and a thread pool to handle multiple clients concurrently. This allows the server to handle a large number of connections without blocking the main thread.
- **Callback-based Log Streaming:** The IRC server uses a callback mechanism to receive log entries from the LogBuffer. This decouples the IRC server from the LogBuffer, allowing them to be developed and tested independently.

### Technical Trade-offs
- **Simplified IRC Feature Set:** The IRC server implements a subset of the full IRC protocol. This was a conscious decision to reduce complexity and focus on the core functionality of log streaming. Features like server-to-server communication and advanced channel modes were not implemented.
- **Basic Log Filtering:** The log filtering mechanism is basic, allowing users to filter logs by level. More advanced filtering, such as by keyword or regular expression, could be added in the future.

### Challenges Faced & Design Decisions
- **Build System Complexity:** Integrating the new IRC module into the existing CMake build system was a challenge. The `CMakeLists.txt` file had to be updated to include the new source files and to link against the necessary libraries.
- **Circular Dependencies:** There were several circular dependencies between the header files of the IRC module. This was resolved by using forward declarations and by carefully ordering the include statements.
- **Authentication Logic:** The IRC authentication logic was tricky to get right. The server needs to handle the `NICK` and `USER` commands in the correct order to properly authenticate a client. This was resolved by carefully managing the client's state and by using a `checkAuthentication` function to verify that the client has sent both commands.

## 3. Sequence List

This section contains the complete source code for the C++-MVP-6. Existing sequences are preserved, and new MVP6 sequences are added.

### `log-caster-cpp-reconstructed/CMakeLists.txt`
```cmake
# [SEQUENCE: CPP-MVP1-1]
# CMake 최소 요구 버전 설정
cmake_minimum_required(VERSION 3.10)

# [SEQUENCE: CPP-MVP1-2]
# 프로젝트 이름 및 버전 설정
project(LogCaster-CPP VERSION 1.0)

# [SEQUENCE: CPP-MVP1-3]
# C++ 표준 설정 (C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# [SEQUENCE: CPP-MVP2-2]
# MVP3에 필요한 소스 파일 목록 (QueryParser.cpp 추가)
set(SOURCES
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/LogBuffer.cpp
    src/QueryHandler.cpp
    src/QueryParser.cpp
    src/Persistence.cpp
    # [SEQUENCE: CPP-MVP6-1]
    src/IRCServer.cpp
    src/IRCClient.cpp
    src/IRCChannel.cpp
    src/IRCChannelManager.cpp
    src/IRCClientManager.cpp
    src/IRCCommandParser.cpp
    src/IRCCommandHandler.cpp
)

# [SEQUENCE: CPP-MVP1-4]
# 실행 파일 생성
add_executable(logcaster-cpp ${SOURCES})


# [SEQUENCE: CPP-MVP1-5]
# 헤더 파일 경로 추가
target_include_directories(logcaster-cpp PUBLIC include)

# [SEQUENCE: CPP-MVP1-6]
# [SEQUENCE: CPP-MVP2-3]
# 스레드 라이브러리 링크
find_package(Threads REQUIRED)
target_link_libraries(logcaster-cpp PRIVATE Threads::Threads)

# [SEQUENCE: CPP-MVP4-3]
# 구형 GCC/G++ 컴파일러를 위한 stdc++fs 라이브러리 링크
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(logcaster-cpp PRIVATE stdc++fs)
endif()

# [SEQUENCE: CPP-MVP1-7]
# 설치 경로 설정 (선택 사항)
install(TARGETS logcaster-cpp DESTINATION bin)
```

### `log-caster-cpp-reconstructed/include/LogServer.h`
```cpp
// [SEQUENCE: CPP-MVP1-9]
#ifndef LOGSERVER_H
#define LOGSERVER_H

#include <memory>
#include <atomic>
#include <string>
// [SEQUENCE: CPP-MVP2-30]
#include "Logger.h"
#include "ThreadPool.h"
#include "LogBuffer.h"
#include "QueryHandler.h"
#include "Persistence.h"

// [SEQUENCE: CPP-MVP1-9]
class LogServer {
public:
    explicit LogServer(int port = 9999, int queryPort = 9998);
    ~LogServer();

    LogServer(const LogServer&) = delete;
    LogServer& operator=(const LogServer&) = delete;

    // [SEQUENCE: CPP-MVP2-30]
    void start();
    void stop();

    // [SEQUENCE: CPP-MVP4-14]
    void setPersistenceManager(std::unique_ptr<PersistenceManager> persistence);

    // [SEQUENCE: CPP-MVP6-9]
    std::shared_ptr<LogBuffer> getLogBuffer() const { return logBuffer_; }

private:
    // [SEQUENCE: CPP-MVP2-30]
    void initialize();
    void runEventLoop();
    void handleNewConnection(int listener_fd, bool is_query_port);
    void handleClientTask(int client_fd);
    void handleQueryTask(int client_fd);

    int port_;
    // [SEQUENCE: CPP-MVP2-30]
    int queryPort_;
    int listenFd_;
    fd_set masterSet_;
    std::atomic<bool> running_;
    
    std::unique_ptr<Logger> logger_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::shared_ptr<LogBuffer> logBuffer_;
    std::unique_ptr<QueryHandler> queryHandler_;

    // [SEQUENCE: CPP-MVP4-15]
    std::unique_ptr<PersistenceManager> persistence_;

    // [SEQUENCE: CPP-MVP5-1]
    std::atomic<int> client_count_{0};
};

#endif // LOGSERVER_H
```

### `log-caster-cpp-reconstructed/src/main.cpp`
```cpp
// [SEQUENCE: CPP-MVP1-14]
#include "LogServer.h"
#include "Persistence.h"
#include "IRCServer.h"
#include <iostream>
#include <getopt.h>
#include <thread>
#include <csignal>

// [SEQUENCE: CPP-MVP6-11]
static std::unique_ptr<LogServer> g_logServer;
static std::unique_ptr<IRCServer> g_ircServer;

void signal_handler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down...\n";
    if (g_ircServer) g_ircServer->stop();
    if (g_logServer) g_logServer->stop();
}

int main(int argc, char* argv[]) {
    int port = 9999;
    PersistenceConfig persist_config;
    // [SEQUENCE: CPP-MVP6-12]
    bool irc_enabled = false;
    int irc_port = 6667;

    // [SEQUENCE: CPP-MVP4-21]
    // 커맨드 라인 인자 파싱
    int opt;
    while ((opt = getopt(argc, argv, "p:d:s:iI:Ph")) != -1) {
        switch (opt) {
            case 'p': port = std::stoi(optarg); break;
            case 'P': persist_config.enabled = true; break;
            case 'd': persist_config.log_directory = optarg; break;
            case 's': persist_config.max_file_size = std::stoul(optarg) * 1024 * 1024; break;
            // [SEQUENCE: CPP-MVP6-13]
            case 'i': irc_enabled = true; break;
            case 'I': 
                irc_enabled = true;
                irc_port = std::stoi(optarg);
                break;
            case 'h':
                std::cout << "Usage: " << argv[0] << " [-p port] [-P] [-d dir] [-s size_mb] [-i] [-I irc_port] [-h]" << std::endl;
                return 0;
        }
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    try {
        g_logServer = std::make_unique<LogServer>(port);

        // [SEQUENCE: CPP-MVP4-22]
        // 영속성 관리자 생성 및 주입
        if (persist_config.enabled) {
            auto persistence = std::make_unique<PersistenceManager>(persist_config);
            g_logServer->setPersistenceManager(std::move(persistence));
            std::cout << "Persistence enabled. Dir: " << persist_config.log_directory 
                      << ", Max Size: " << persist_config.max_file_size / (1024*1024) << " MB" << std::endl;
        }

        // [SEQUENCE: CPP-MVP6-14]
        // IRC 서버 생성 및 별도 스레드에서 실행
        if (irc_enabled) {
            g_ircServer = std::make_unique<IRCServer>(irc_port, g_logServer->getLogBuffer());
            std::thread irc_thread([] { g_ircServer->start(); });
            irc_thread.detach();
            std::cout << "IRC Server enabled on port " << irc_port << std::endl;
        }

        g_logServer->start(); // This will block until the server is stopped

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

### `log-caster-cpp-reconstructed/include/LogBuffer.h`
```cpp
// [SEQUENCE: C-MVP2-14]
#ifndef LOGBUFFER_H
#define LOGBUFFER_H

#include <deque>
#include <string>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>
#include <functional>
#include <map>

// [SEQUENCE: C-MVP3-11]
// Forward declaration
class ParsedQuery;

// [SEQUENCE: CPP-MVP6-2]
// LogEntry 확장 및 콜백 타입 정의
struct LogEntry {
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    std::string level;
    std::string source;

    LogEntry(std::string msg, std::string lvl, std::string src)
        : message(std::move(msg)), timestamp(std::chrono::system_clock::now()), level(std::move(lvl)), source(std::move(src)) {}
};

// [SEQUENCE: CPP-MVP6-3]
typedef std::function<void(const LogEntry&)> LogCallback;

// [SEQUENCE: C-MVP2-16]
// 로그 버퍼 클래스
class LogBuffer {
public:
    explicit LogBuffer(size_t capacity = 10000);
    ~LogBuffer() = default;

    void push(std::string message, const std::string& level, const std::string& source);
    std::vector<std::string> search(const std::string& keyword) const;

    // [SEQUENCE: C-MVP3-12]
    // MVP3에 추가된 고급 검색 메소드
    std::vector<std::string> searchEnhanced(const ParsedQuery& query) const;

    // [SEQUENCE: CPP-MVP6-4]
    void registerCallback(const std::string& channel, LogCallback callback);

    struct StatsSnapshot {
        uint64_t totalLogs;
        uint64_t droppedLogs;
    };
    StatsSnapshot getStats() const;
    size_t size() const;

private:
    void dropOldest_();

    mutable std::mutex mutex_;
    std::deque<LogEntry> buffer_;
    size_t capacity_;

    std::atomic<uint64_t> totalLogs_{0};
    std::atomic<uint64_t> droppedLogs_{0};

    // [SEQUENCE: CPP-MVP6-5]
    std::map<std::string, std::vector<LogCallback>> callbacks_;
};

#endif // LOGBUFFER_H
```

### `log-caster-cpp-reconstructed/src/LogBuffer.cpp`
```cpp
// [SEQUENCE: CPP-MVP2-17]
#include "LogBuffer.h"
#include "QueryParser.h"
#include <sstream>
#include <iomanip>

LogBuffer::LogBuffer(size_t capacity) : capacity_(capacity) {}

// [SEQUENCE: CPP-MVP6-6]
void LogBuffer::push(std::string message, const std::string& level, const std::string& source) {
    LogEntry entry(std::move(message), level, source);
    std::lock_guard<std::mutex> lock(mutex_);
    if (buffer_.size() >= capacity_) {
        dropOldest_();
    }
    buffer_.push_back(entry);
    totalLogs_++;

    // Notify callbacks
    for (auto const& [channel, callbacks] : callbacks_) {
        // Simple matching for now
        if (channel == "#logs-all" || (channel == "#logs-error" && level == "ERROR")) {
            for (const auto& callback : callbacks) {
                callback(entry);
            }
        }
    }
}

void LogBuffer::dropOldest_() {
    if (!buffer_.empty()) {
        buffer_.pop_front();
        droppedLogs_++;
    }
}

std::vector<std::string> LogBuffer::search(const std::string& keyword) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> results;
    for (const auto& entry : buffer_) {
        if (entry.message.find(keyword) != std::string::npos) {
            auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
            std::stringstream ss;
            ss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
            ss << entry.message;
            results.push_back(ss.str());
        }
    }
    return results;
}

std::vector<std::string> LogBuffer::searchEnhanced(const ParsedQuery& query) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> results;
    for (const auto& entry : buffer_) {
        if (query.matches(entry.message, entry.timestamp)) {
            auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
            std::stringstream ss;
            ss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
            ss << entry.message;
            results.push_back(ss.str());
        }
    }
    return results;
}

// [SEQUENCE: CPP-MVP6-8]
void LogBuffer::registerCallback(const std::string& channel, LogCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_[channel].push_back(callback);
}

size_t LogBuffer::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_.size();
}

LogBuffer::StatsSnapshot LogBuffer::getStats() const {
    return { totalLogs_.load(), droppedLogs_.load() };
}
```

### `log-caster-cpp-reconstructed/include/IRCServer.h`
```cpp
// [SEQUENCE: CPP-MVP6-2]
#ifndef IRCSERVER_H
#define IRCSERVER_H

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>

class LogBuffer;
class ThreadPool;
class IRCChannelManager;
class IRCClientManager;
class IRCCommandParser;
class IRCCommandHandler;

class IRCServer {
public:
    static constexpr int DEFAULT_IRC_PORT = 6667;
    static constexpr int MAX_CLIENTS = 1000;
    static constexpr int THREAD_POOL_SIZE = 8;
    
    explicit IRCServer(int port = DEFAULT_IRC_PORT, std::shared_ptr<LogBuffer> logBuffer = nullptr);
    ~IRCServer();
    
    void start();
    void stop();
    bool isRunning() const { return running_.load(); }
    
    void setLogBuffer(std::shared_ptr<LogBuffer> buffer);
    std::shared_ptr<LogBuffer> getLogBuffer() const { return logBuffer_; }
    
    std::shared_ptr<IRCChannelManager> getChannelManager() const { return channelManager_; }
    std::shared_ptr<IRCClientManager> getClientManager() const { return clientManager_; }
    
    std::string getServerName() const { return "logcaster-irc"; }
    std::string getServerVersion() const { return "1.0"; }
    std::string getServerCreated() const { return serverCreated_; }
    
private:
    void setupSocket();
    void acceptClients();
    void handleClient(int clientFd, const std::string& clientAddr);
    void processClientData(int clientFd, const std::string& data);
    
    int port_;
    int listenFd_;
    std::atomic<bool> running_;
    std::thread acceptThread_;
    std::string serverCreated_;
    
    std::shared_ptr<LogBuffer> logBuffer_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::shared_ptr<IRCChannelManager> channelManager_;
    std::shared_ptr<IRCClientManager> clientManager_;
    std::unique_ptr<IRCCommandParser> commandParser_;
    std::unique_ptr<IRCCommandHandler> commandHandler_;
    
    mutable std::mutex mutex_;
};

#endif // IRCSERVER_H
```

### `log-caster-cpp-reconstructed/src/IRCServer.cpp`
```cpp
// [SEQUENCE: CPP-MVP6-7]
#include "IRCServer.h"
#include "IRCChannelManager.h"
#include "IRCClientManager.h"
#include "IRCCommandParser.h"
#include "IRCCommandHandler.h"
#include "IRCClient.h"
#include "ThreadPool.h"
#include "LogBuffer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>

IRCServer::IRCServer(int port, std::shared_ptr<LogBuffer> logBuffer)
    : port_(port)
    , listenFd_(-1)
    , running_(false)
    , logBuffer_(logBuffer) {
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    serverCreated_ = std::ctime(&time_t);
    serverCreated_.pop_back(); // Remove newline
    
    threadPool_ = std::make_unique<ThreadPool>(THREAD_POOL_SIZE);
    channelManager_ = std::make_shared<IRCChannelManager>();
    clientManager_ = std::make_shared<IRCClientManager>();
    commandParser_ = std::make_unique<IRCCommandParser>();
    commandHandler_ = std::make_unique<IRCCommandHandler>(this);
}

IRCServer::~IRCServer() {
    stop();
}

void IRCServer::start() {
    if (running_.load()) {
        return;
    }
    
    try {
        setupSocket();
        
        channelManager_->initializeLogChannels();
        
        if (logBuffer_) {
            logBuffer_->registerCallback("#logs-all", 
                [this](const LogEntry& entry) {
                    channelManager_->distributeLogEntry(entry);
                });
            logBuffer_->registerCallback("#logs-error", 
                [this](const LogEntry& entry) {
                    if (entry.level == "ERROR") {
                        channelManager_->distributeLogEntry(entry);
                    }
                });
        }
        
        running_ = true;
        
        acceptThread_ = std::thread(&IRCServer::acceptClients, this);
        
        std::cout << "IRC server started on port " << port_ << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to start IRC server: " << e.what() << std::endl;
        throw;
    }
}

void IRCServer::stop() {
    if (!running_.load()) {
        return;
    }
    
    running_ = false;
    
    if (listenFd_ != -1) {
        close(listenFd_);
        listenFd_ = -1;
    }
    
    if (acceptThread_.joinable()) {
        acceptThread_.join();
    }
    
    auto allClients = clientManager_->getAllClients();
    for (const auto& client : allClients) {
        channelManager_->partAllChannels(client, "Server shutting down");
        clientManager_->removeClient(client->getFd());
    }
    
    std::cout << "IRC server stopped" << std::endl;
}

void IRCServer::setLogBuffer(std::shared_ptr<LogBuffer> buffer) {
    std::lock_guard<std::mutex> lock(mutex_);
    logBuffer_ = buffer;
}

void IRCServer::setupSocket() {
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
    }
    
    int opt = 1;
    if (setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(listenFd_);
        throw std::runtime_error("Failed to set socket options: " + std::string(strerror(errno)));
    }
    
    int flags = fcntl(listenFd_, F_GETFL, 0);
    if (flags < 0 || fcntl(listenFd_, F_SETFL, flags | O_NONBLOCK) < 0) {
        close(listenFd_);
        throw std::runtime_error("Failed to set non-blocking mode: " + std::string(strerror(errno)));
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);
    
    if (bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(listenFd_);
        throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
    }
    
    if (listen(listenFd_, 128) < 0) {
        close(listenFd_);
        throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));
    }
}

void IRCServer::acceptClients() {
    while (running_.load()) {
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        
        int clientFd = accept(listenFd_, (struct sockaddr*)&clientAddr, &addrLen);
        
        if (clientFd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            
            if (running_.load()) {
                std::cerr << "Accept error: " << strerror(errno) << std::endl;
            }
            continue;
        }
        
        char addrStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, addrStr, sizeof(addrStr));
        std::string clientAddress = std::string(addrStr) + ":" + 
                                   std::to_string(ntohs(clientAddr.sin_port));
        
        if (clientManager_->getClientCount() >= MAX_CLIENTS) {
            std::string errorMsg = "ERROR :Server is full\r\n";
            send(clientFd, errorMsg.c_str(), errorMsg.length(), MSG_NOSIGNAL);
            close(clientFd);
            continue;
        }
        
        int flags = fcntl(clientFd, F_GETFL, 0);
        if (flags >= 0) {
            fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
        }
        
        auto client = clientManager_->addClient(clientFd, clientAddress);
        if (!client) {
            close(clientFd);
            continue;
        }
        
        std::cout << "New IRC client connected from " << clientAddress << std::endl;
        
        threadPool_->enqueue([this, clientFd, clientAddress]() {
            handleClient(clientFd, clientAddress);
        });
    }
}

void IRCServer::handleClient(int clientFd, const std::string& clientAddr) {
    char buffer[4096];
    std::string incomplete;
    
    while (running_.load()) {
        ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            
            std::string data = incomplete + std::string(buffer, bytesRead);
            incomplete.clear();
            
            size_t pos = 0;
            while ((pos = data.find("\r\n")) != std::string::npos || 
                   (pos = data.find("\n")) != std::string::npos) {
                
                std::string line = data.substr(0, pos);
                data.erase(0, pos + (data[pos] == '\r' ? 2 : 1));
                
                if (!line.empty()) {
                    processClientData(clientFd, line);
                }
            }
            
            if (!data.empty()) {
                incomplete = data;
            }
            
            clientManager_->updateClientActivity(clientFd);
            
        } else if (bytesRead == 0) {
            break;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                break;
            }
            
            if (!clientManager_->clientExists(clientFd)) {
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    auto client = clientManager_->getClientByFd(clientFd);
    if (client) {
        std::cout << "IRC client disconnected: " << client->getNickname() 
                  << " (" << clientAddr << ")" << std::endl;
        
        IRCCommandParser::IRCCommand quitCmd;
        quitCmd.command = "QUIT";
        quitCmd.trailing = "Connection closed";
        commandHandler_->handleCommand(client, quitCmd);
        
        clientManager_->removeClient(clientFd);
    }
    
    close(clientFd);
}

void IRCServer::processClientData(int clientFd, const std::string& data) {
    auto client = clientManager_->getClientByFd(clientFd);
    if (!client) {
        return;
    }
    
    auto cmd = IRCCommandParser::parse(data);
    
    if (!cmd.command.empty()) {
        commandHandler_->handleCommand(client, cmd);
    }
}
```

### `log-caster-cpp-reconstructed/include/IRCClient.h`
```cpp
// [SEQUENCE: CPP-MVP6-14]
#ifndef IRCCLIENT_H
#define IRCCLIENT_H

#include <string>
#include <set>
#include <mutex>
#include <chrono>
#include <memory>

class IRCClient {
public:
    enum class State {
        CONNECTED,
        AUTHENTICATED,
        DISCONNECTED
    };
    
    IRCClient(int fd, const std::string& address);
    ~IRCClient();
    
    void setNickname(const std::string& nick);
    void setUsername(const std::string& user);
    void setRealname(const std::string& realname);
    void setHostname(const std::string& hostname);
    
    void joinChannel(const std::string& channel);
    void partChannel(const std::string& channel);
    bool isInChannel(const std::string& channel) const;
    
    void sendMessage(const std::string& message);
    void sendNumericReply(int code, const std::string& params);
    void sendErrorReply(int code, const std::string& params);
    
    void setState(State state);
    State getState() const;
    bool isAuthenticated() const;
    void updateLastActivity();
    
    int getFd() const { return fd_; }
    const std::string& getAddress() const { return address_; }
    const std::string& getNickname() const { return nickname_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getRealname() const { return realname_; }
    const std::string& getHostname() const { return hostname_; }
    const std::set<std::string>& getChannels() const { return channels_; }
    
    std::string getFullIdentifier() const;
    
private:
    int fd_;
    std::string address_;
    std::string nickname_;
    std::string username_;
    std::string realname_;
    std::string hostname_;
    State state_;
    
    std::set<std::string> channels_;
    
    std::chrono::steady_clock::time_point lastActivity_;
    
    mutable std::mutex mutex_;
    
    void sendRawMessage(const std::string& message);
};

#endif // IRCCLIENT_H
```

### `log-caster-cpp-reconstructed/src/IRCClient.cpp`
```cpp
// [SEQUENCE: CPP-MVP6-15]
#include "IRCClient.h"
#include <sys/socket.h>
#include "IRCCommandParser.h"
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>

IRCClient::IRCClient(int fd, const std::string& address)
    : fd_(fd)
    , address_(address)
    , state_(State::CONNECTED)
    , lastActivity_(std::chrono::steady_clock::now()) {
    
    size_t colonPos = address.find(':');
    if (colonPos != std::string::npos) {
        hostname_ = address.substr(0, colonPos);
    } else {
        hostname_ = address;
    }
}

IRCClient::~IRCClient() {
    if (fd_ != -1) {
        close(fd_);
    }
}

void IRCClient::setNickname(const std::string& nick) {
    std::lock_guard<std::mutex> lock(mutex_);
    nickname_ = nick;
    updateLastActivity();
}

void IRCClient::setUsername(const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    username_ = user;
    updateLastActivity();
}

void IRCClient::setRealname(const std::string& realname) {
    std::lock_guard<std::mutex> lock(mutex_);
    realname_ = realname;
    updateLastActivity();
}

void IRCClient::setHostname(const std::string& hostname) {
    std::lock_guard<std::mutex> lock(mutex_);
    hostname_ = hostname;
    updateLastActivity();
}

void IRCClient::joinChannel(const std::string& channel) {
    std::lock_guard<std::mutex> lock(mutex_);
    channels_.insert(channel);
    updateLastActivity();
}

void IRCClient::partChannel(const std::string& channel) {
    std::lock_guard<std::mutex> lock(mutex_);
    channels_.erase(channel);
    updateLastActivity();
}

bool IRCClient::isInChannel(const std::string& channel) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return channels_.find(channel) != channels_.end();
}

void IRCClient::sendMessage(const std::string& message) {
    if (fd_ == -1) {
        return;
    }
    
    std::string fullMessage = message;
    if (fullMessage.size() < 2 || 
        fullMessage.substr(fullMessage.size() - 2) != "\r\n") {
        fullMessage += "\r\n";
    }
    
    sendRawMessage(fullMessage);
}

void IRCClient::sendNumericReply(int code, const std::string& params) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string serverName = "logcaster-irc";
    std::string nick = nickname_.empty() ? "*" : nickname_;
    
    std::string reply = IRCCommandParser::formatReply(serverName, nick, code, params);
    sendMessage(reply);
}

void IRCClient::sendErrorReply(int code, const std::string& params) {
    sendNumericReply(code, params);
}

void IRCClient::setState(State state) {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = state;
    updateLastActivity();
}

IRCClient::State IRCClient::getState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

bool IRCClient::isAuthenticated() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == State::AUTHENTICATED;
}

void IRCClient::updateLastActivity() {
    lastActivity_ = std::chrono::steady_clock::now();
}

std::string IRCClient::getFullIdentifier() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (nickname_.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    oss << nickname_ << "!" << username_ << "@" << hostname_;
    return oss.str();
}

void IRCClient::sendRawMessage(const std::string& message) {
    if (fd_ == -1) {
        return;
    }
    
    ssize_t totalSent = 0;
    ssize_t messageLen = message.length();
    
    while (totalSent < messageLen) {
        ssize_t sent = send(fd_, message.c_str() + totalSent, 
                           messageLen - totalSent, MSG_NOSIGNAL);
        
        if (sent <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            std::cerr << "Error sending to client " << nickname_ 
                     << ": " << strerror(errno) << std::endl;
            break;
        }
        
        totalSent += sent;
    }
}
```

### `log-caster-cpp-reconstructed/include/IRCChannel.h`
```cpp
// [SEQUENCE: CPP-MVP6-12]
#ifndef IRCCHANNEL_H
#define IRCCHANNEL_H

#include <string>
#include <map>
#include <memory>
#include <functional>
#include <shared_mutex>
#include <chrono>
#include <set>
#include <vector>

class IRCClient;
struct LogEntry;

class IRCChannel {
public:
    enum class Type {
        NORMAL,
        LOG_STREAM,
    };
    
    struct ChannelModes {
        bool topicProtected = true;
    };
    
    explicit IRCChannel(const std::string& name, Type type = Type::NORMAL);
    ~IRCChannel();
    
    void addClient(std::shared_ptr<IRCClient> client);
    void removeClient(const std::string& nickname);
    bool hasClient(const std::string& nickname) const;
    
    void broadcast(const std::string& message);
    void broadcastExcept(const std::string& message, const std::string& exceptNick);
    
    void setTopic(const std::string& topic, const std::string& setBy);
    const std::string& getTopic() const { return topic_; }
    
    const ChannelModes& getModes() const { return modes_; }
    
    bool isOperator(const std::string& nickname) const;
    
    void setLogFilter(const std::function<bool(const LogEntry&)>& filter);
    void enableLogStreaming(bool enable);
    bool isLogStreamingEnabled() const { return streamingEnabled_; }
    void processLogEntry(const LogEntry& entry);
    
    const std::string& getName() const { return name_; }
    Type getType() const { return type_; }
    size_t getClientCount() const;
    std::vector<std::shared_ptr<IRCClient>> getClients() const;
    
    static std::function<bool(const LogEntry&)> createLevelFilter(const std::string& level);
    
private:
    std::string name_;
    Type type_;
    std::string topic_;
    std::string topicSetBy_;
    std::chrono::system_clock::time_point topicSetTime_;
    
    ChannelModes modes_;
    
    std::map<std::string, std::shared_ptr<IRCClient>> clients_;
    std::set<std::string> operators_;
    
    bool streamingEnabled_;
    std::function<bool(const LogEntry&)> logFilter_;
    
    mutable std::shared_mutex mutex_;
    
    std::string formatLogEntry(const LogEntry& entry) const;
};

#endif // IRCCHANNEL_H
```

### `log-caster-cpp-reconstructed/src/IRCChannel.cpp`
```cpp
// [SEQUENCE: CPP-MVP6-13]
#include "IRCChannel.h"
#include "IRCClient.h"
#include "IRCCommandParser.h"
#include "LogBuffer.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

IRCChannel::IRCChannel(const std::string& name, Type type) 
    : name_(name)
    , type_(type)
    , topic_("")
    , topicSetBy_("")
    , streamingEnabled_(false) {
}

IRCChannel::~IRCChannel() = default;

void IRCChannel::addClient(std::shared_ptr<IRCClient> client) {
    if (!client) return;
    
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    std::string nickname = client->getNickname();
    clients_[nickname] = client;
    
    if (clients_.size() == 1) {
        operators_.insert(nickname);
    }
}

void IRCChannel::removeClient(const std::string& nickname) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    clients_.erase(nickname);
    operators_.erase(nickname);
}

bool IRCChannel::hasClient(const std::string& nickname) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return clients_.find(nickname) != clients_.end();
}

void IRCChannel::broadcast(const std::string& message) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& [nick, client] : clients_) {
        client->sendMessage(message);
    }
}

void IRCChannel::broadcastExcept(const std::string& message, const std::string& exceptNick) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& [nick, client] : clients_) {
        if (nick != exceptNick) {
            client->sendMessage(message);
        }
    }
}

void IRCChannel::setTopic(const std::string& topic, const std::string& setBy) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    topic_ = topic;
    topicSetBy_ = setBy;
    topicSetTime_ = std::chrono::system_clock::now();
}

bool IRCChannel::isOperator(const std::string& nickname) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return operators_.find(nickname) != operators_.end();
}

void IRCChannel::setLogFilter(const std::function<bool(const LogEntry&)>& filter) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    logFilter_ = filter;
}

void IRCChannel::enableLogStreaming(bool enable) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    streamingEnabled_ = enable;
}

void IRCChannel::processLogEntry(const LogEntry& entry) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    if (!streamingEnabled_ || clients_.empty()) {
        return;
    }
    
    if (logFilter_ && !logFilter_(entry)) {
        return;
    }
    
    std::string formattedLog = formatLogEntry(entry);
    std::string message = IRCCommandParser::formatUserMessage(
        "LogBot", "log", "system", "PRIVMSG", name_, formattedLog
    );
    
    lock.unlock();
    broadcast(message);
}

size_t IRCChannel::getClientCount() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return clients_.size();
}

std::vector<std::shared_ptr<IRCClient>> IRCChannel::getClients() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::vector<std::shared_ptr<IRCClient>> clients;
    clients.reserve(clients_.size());
    for (const auto& [nick, client] : clients_) {
        clients.push_back(client);
    }
    return clients;
}

std::function<bool(const LogEntry&)> IRCChannel::createLevelFilter(const std::string& level) {
    return [level](const LogEntry& entry) {
        return entry.level == level;
    };
}

std::string IRCChannel::formatLogEntry(const LogEntry& entry) const {
    std::ostringstream oss;
    
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    struct tm* tm = localtime(&time_t);
    oss << "[" << std::put_time(tm, "%Y-%m-%d %H:%M:%S") << "] ";
    
    if (!entry.level.empty()) {
        oss << entry.level << ": ";
    }
    
    if (!entry.source.empty()) {
        oss << "[" << entry.source << "] ";
    }
    
    oss << entry.message;
    
    return oss.str();
}
```

### `log-caster-cpp-reconstructed/include/IRCChannelManager.h`
```cpp
// [SEQUENCE: CPP-MVP6-3]
#ifndef IRCCHANNELMANAGER_H
#define IRCCHANNELMANAGER_H

#include <string>
#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <functional>

#include "IRCChannel.h"
class IRCClient;
struct LogEntry;

class IRCChannelManager {
public:
    IRCChannelManager();
    ~IRCChannelManager();
    
    std::shared_ptr<IRCChannel> createChannel(const std::string& name, IRCChannel::Type type = IRCChannel::Type::NORMAL);
    void removeChannel(const std::string& name);
    bool channelExists(const std::string& name) const;
    std::shared_ptr<IRCChannel> getChannel(const std::string& name) const;
    
    bool joinChannel(std::shared_ptr<IRCClient> client, const std::string& channelName, const std::string& key = "");
    bool partChannel(std::shared_ptr<IRCClient> client, const std::string& channelName, const std::string& reason = "");
    void partAllChannels(std::shared_ptr<IRCClient> client, const std::string& reason = "");
    
    std::vector<std::string> getChannelList() const;
    
    void initializeLogChannels();
    void distributeLogEntry(const LogEntry& entry);
    
private:
    std::unordered_map<std::string, std::shared_ptr<IRCChannel>> channels_;
    
    mutable std::shared_mutex mutex_;
    
    bool isValidChannelName(const std::string& name) const;
    std::string normalizeChannelName(const std::string& name) const;
    void sendJoinMessages(std::shared_ptr<IRCChannel> channel, std::shared_ptr<IRCClient> client);
    void sendPartMessages(std::shared_ptr<IRCChannel> channel, std::shared_ptr<IRCClient> client, const std::string& reason);
    
    struct LogChannelConfig {
        std::string name;
        std::string level;
        std::string description;
    };
    static const std::vector<LogChannelConfig> defaultLogChannels_;
};

#endif // IRCCHANNELMANAGER_H
```

### `log-caster-cpp-reconstructed/src/IRCChannelManager.cpp`
```cpp
// [SEQUENCE: CPP-MVP6-8]
#include "IRCChannelManager.h"
#include "IRCChannel.h"
#include "IRCClient.h"
#include "LogBuffer.h"
#include <algorithm>
#include <iostream>

const std::vector<IRCChannelManager::LogChannelConfig> IRCChannelManager::defaultLogChannels_ = {
    {"#logs-all", "*", "All log messages"},
    {"#logs-error", "ERROR", "Error level logs only"},
};

IRCChannelManager::IRCChannelManager() {}

IRCChannelManager::~IRCChannelManager() = default;

std::shared_ptr<IRCChannel> IRCChannelManager::createChannel(const std::string& name, 
                                                             IRCChannel::Type type) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    std::string normalizedName = normalizeChannelName(name);
    
    if (channels_.find(normalizedName) != channels_.end()) {
        return channels_[normalizedName];
    }
    
    auto channel = std::make_shared<IRCChannel>(normalizedName, type);
    channels_[normalizedName] = channel;
    
    return channel;
}

void IRCChannelManager::removeChannel(const std::string& name) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    std::string normalizedName = normalizeChannelName(name);
    
    if (normalizedName.find("#logs-") == 0) {
        return;
    }
    
    channels_.erase(normalizedName);
}

bool IRCChannelManager::channelExists(const std::string& name) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::string normalizedName = normalizeChannelName(name);
    return channels_.find(normalizedName) != channels_.end();
}

std::shared_ptr<IRCChannel> IRCChannelManager::getChannel(const std::string& name) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::string normalizedName = normalizeChannelName(name);
    auto it = channels_.find(normalizedName);
    
    if (it != channels_.end()) {
        return it->second;
    }
    
    return nullptr;
}

bool IRCChannelManager::joinChannel(std::shared_ptr<IRCClient> client, 
                                   const std::string& channelName, 
                                   const std::string& key) {
    if (!client || !client->isAuthenticated()) {
        return false;
    }
    
    std::shared_ptr<IRCChannel> channel;
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        std::string normalizedName = normalizeChannelName(channelName);
        auto it = channels_.find(normalizedName);
        
        if (it == channels_.end()) {
            if (normalizedName.find("#logs-") != 0) {
                channel = std::make_shared<IRCChannel>(normalizedName, IRCChannel::Type::NORMAL);
                channels_[normalizedName] = channel;
            } else {
                return false; 
            }
        } else {
            channel = it->second;
        }
    }
    
    channel->addClient(client);
    client->joinChannel(channelName);
    
    sendJoinMessages(channel, client);
    
    return true;
}

bool IRCChannelManager::partChannel(std::shared_ptr<IRCClient> client, 
                                   const std::string& channelName, 
                                   const std::string& reason) {
    if (!client) {
        return false;
    }
    
    auto channel = getChannel(channelName);
    if (!channel || !channel->hasClient(client->getNickname())) {
        return false;
    }
    
    sendPartMessages(channel, client, reason);
    
    channel->removeClient(client->getNickname());
    client->partChannel(channelName);
    
    if (channel->getClientCount() == 0 && channelName.find("#logs-") != 0) {
        removeChannel(channelName);
    }
    
    return true;
}

void IRCChannelManager::partAllChannels(std::shared_ptr<IRCClient> client, 
                                       const std::string& reason) {
    if (!client) {
        return;
    }
    
    auto clientChannels = client->getChannels();
    for (const auto& channelName : clientChannels) {
        partChannel(client, channelName, reason);
    }
}

std::vector<std::string> IRCChannelManager::getChannelList() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<std::string> channelNames;
    channelNames.reserve(channels_.size());
    
    for (const auto& [name, channel] : channels_) {
        channelNames.push_back(name);
    }
    
    return channelNames;
}

void IRCChannelManager::initializeLogChannels() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& config : defaultLogChannels_) {
        auto channel = std::make_shared<IRCChannel>(config.name, IRCChannel::Type::LOG_STREAM);
        channel->setTopic(config.description, "LogCaster");
        channel->enableLogStreaming(true);
        
        if (config.level != "*") {
            channel->setLogFilter(IRCChannel::createLevelFilter(config.level));
        }
        
        channels_[config.name] = channel;
    }
}

void IRCChannelManager::distributeLogEntry(const LogEntry& entry) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& [name, channel] : channels_) {
        if (channel->getType() == IRCChannel::Type::LOG_STREAM && 
            channel->isLogStreamingEnabled()) {
            channel->processLogEntry(entry);
        }
    }
}

bool IRCChannelManager::isValidChannelName(const std::string& name) const {
    if (name.empty() || name.length() > 50) {
        return false;
    }
    
    if (name[0] != '#' && name[0] != '&') {
        return false;
    }
    
    for (char c : name) {
        if (c == ' ' || c == ',' || c == '\x07' || c < 32) {
            return false;
        }
    }
    
    return true;
}

std::string IRCChannelManager::normalizeChannelName(const std::string& name) const {
    return name;
}

void IRCChannelManager::sendJoinMessages(std::shared_ptr<IRCChannel> channel, 
                                         std::shared_ptr<IRCClient> client) {
    std::string joinMsg = ":" + client->getFullIdentifier() + " JOIN :" + channel->getName();
    channel->broadcast(joinMsg);
}

void IRCChannelManager::sendPartMessages(std::shared_ptr<IRCChannel> channel, 
                                        std::shared_ptr<IRCClient> client, 
                                        const std::string& reason) {
    std::string partMsg = ":" + client->getFullIdentifier() + " PART " + channel->getName();
    if (!reason.empty()) {
        partMsg += " :" + reason;
    }
    channel->broadcast(partMsg);
}
```

### `log-caster-cpp-reconstructed/include/IRCClientManager.h`
```cpp
// [SEQUENCE: CPP-MVP6-4]
#ifndef IRCCLIENTMANAGER_H
#define IRCCLIENTMANAGER_H

#include <string>
#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <chrono>
#include <set>

class IRCClient;

class IRCClientManager {
public:
    IRCClientManager();
    ~IRCClientManager();
    
    std::shared_ptr<IRCClient> addClient(int fd, const std::string& address);
    void removeClient(int fd);
    bool clientExists(int fd) const;
    
    std::shared_ptr<IRCClient> getClientByFd(int fd) const;
    std::shared_ptr<IRCClient> getClientByNickname(const std::string& nickname) const;
    std::vector<std::shared_ptr<IRCClient>> getAllClients() const;
    
    bool isNicknameAvailable(const std::string& nickname) const;
    void registerNickname(int fd, const std::string& nickname);
    
    void updateClientActivity(int fd);
    
    size_t getClientCount() const;
    
private:
    std::unordered_map<int, std::shared_ptr<IRCClient>> clientsByFd_;
    std::unordered_map<std::string, std::shared_ptr<IRCClient>> clientsByNick_;
    
    mutable std::shared_mutex mutex_;
    
    std::string normalizeNickname(const std::string& nick) const;
};

#endif // IRCCLIENTMANAGER_H
```

### `log-caster-cpp-reconstructed/src/IRCClientManager.cpp`
```cpp
// [SEQUENCE: CPP-MVP6-9]
#include "IRCClientManager.h"
#include "IRCClient.h"
#include <algorithm>
#include <iostream>

IRCClientManager::IRCClientManager() {}

IRCClientManager::~IRCClientManager() = default;

std::shared_ptr<IRCClient> IRCClientManager::addClient(int fd, const std::string& address) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto client = std::make_shared<IRCClient>(fd, address);
    clientsByFd_[fd] = client;
    
    return client;
}

void IRCClientManager::removeClient(int fd) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto it = clientsByFd_.find(fd);
    if (it == clientsByFd_.end()) {
        return;
    }
    
    auto client = it->second;
    std::string nick = client->getNickname();
    if (!nick.empty()) {
        clientsByNick_.erase(normalizeNickname(nick));
    }

    clientsByFd_.erase(it);
}

bool IRCClientManager::clientExists(int fd) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return clientsByFd_.find(fd) != clientsByFd_.end();
}

std::shared_ptr<IRCClient> IRCClientManager::getClientByFd(int fd) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = clientsByFd_.find(fd);
    if (it != clientsByFd_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<IRCClient> IRCClientManager::getClientByNickname(const std::string& nickname) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = clientsByNick_.find(normalizeNickname(nickname));
    if (it != clientsByNick_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<IRCClient>> IRCClientManager::getAllClients() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<std::shared_ptr<IRCClient>> clients;
    clients.reserve(clientsByFd_.size());
    
    for (const auto& [fd, client] : clientsByFd_) {
        clients.push_back(client);
    }
    
    return clients;
}

bool IRCClientManager::isNicknameAvailable(const std::string& nickname) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return clientsByNick_.find(normalizeNickname(nickname)) == clientsByNick_.end();
}

void IRCClientManager::registerNickname(int fd, const std::string& nickname) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto it = clientsByFd_.find(fd);
    if (it == clientsByFd_.end()) {
        return;
    }
    
    std::string normalized = normalizeNickname(nickname);
    clientsByNick_[normalized] = it->second;
}

void IRCClientManager::updateClientActivity(int fd) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = clientsByFd_.find(fd);
    if (it != clientsByFd_.end()) {
        it->second->updateLastActivity();
    }
}

size_t IRCClientManager::getClientCount() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return clientsByFd_.size();
}

std::string IRCClientManager::normalizeNickname(const std::string& nick) const {
    std::string normalized = nick;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    return normalized;
}
```

### `log-caster-cpp-reconstructed/include/IRCCommandParser.h`
```cpp
// [SEQUENCE: CPP-MVP6-5]
#ifndef IRCCOMMANDPARSER_H
#define IRCCOMMANDPARSER_H

#include <string>
#include <vector>
#include <sstream>
#include <map>

class IRCCommandParser {
public:
    struct IRCCommand {
        std::string prefix;
        std::string command;
        std::vector<std::string> params;
        std::string trailing;
        
        std::string getParam(size_t index) const {
            return (index < params.size()) ? params[index] : "";
        }
    };
    
    enum ReplyCode {
        RPL_WELCOME = 1,
        RPL_YOURHOST = 2,
        RPL_CREATED = 3,
        RPL_MYINFO = 4,
        RPL_NAMREPLY = 353,
        RPL_ENDOFNAMES = 366,
        ERR_NOSUCHNICK = 401,
        ERR_NOSUCHCHANNEL = 403,
        ERR_CANNOTSENDTOCHAN = 404,
        ERR_NORECIPIENT = 411,
        ERR_NOTEXTTOSEND = 412,
        ERR_UNKNOWNCOMMAND = 421,
        ERR_NONICKNAMEGIVEN = 431,
        ERR_NICKNAMEINUSE = 433,
        ERR_NOTONCHANNEL = 442,
        ERR_NOTREGISTERED = 451,
        ERR_NEEDMOREPARAMS = 461,
        ERR_ALREADYREGISTRED = 462,
    };
    
    static IRCCommand parse(const std::string& line);
    
    static std::string formatReply(const std::string& serverName, 
                                   const std::string& nick,
                                   int code, 
                                   const std::string& params);

    static std::string formatUserMessage(const std::string& nick,
                                         const std::string& user,
                                         const std::string& host,
                                         const std::string& command,
                                         const std::string& target,
                                         const std::string& message);

    static std::string toUpper(const std::string& str);
    static std::vector<std::string> splitChannels(const std::string& channels);

private:
    static std::string extractPrefix(std::string& line);
    static std::string extractCommand(std::string& line);
    static std::vector<std::string> extractParams(std::string& line);
};

#endif // IRCCOMMANDPARSER_H
```

### `log-caster-cpp-reconstructed/src/IRCCommandParser.cpp`
```cpp
// [SEQUENCE: CPP-MVP6-10]
#include "IRCCommandParser.h"
#include <algorithm>
#include <cctype>

IRCCommandParser::IRCCommand IRCCommandParser::parse(const std::string& line) {
    IRCCommand cmd;
    
    if (line.empty()) {
        return cmd;
    }
    
    std::string workingLine = line;
    
    if (!workingLine.empty() && (workingLine.back() == '\r' || workingLine.back() == '\n')) {
        workingLine.pop_back();
    }
    
    cmd.prefix = extractPrefix(workingLine);
    cmd.command = extractCommand(workingLine);
    cmd.params = extractParams(workingLine);
    
    if (!cmd.params.empty() && !cmd.params.back().empty() && cmd.params.back()[0] == ':') {
        cmd.trailing = cmd.params.back().substr(1);
        cmd.params.pop_back();
    }
    
    return cmd;
}

std::string IRCCommandParser::extractPrefix(std::string& line) {
    if (line.empty() || line[0] != ':') {
        return "";
    }
    
    size_t spacePos = line.find(' ');
    if (spacePos == std::string::npos) {
        return "";
    }
    
    std::string prefix = line.substr(1, spacePos - 1);
    line = line.substr(spacePos + 1);
    return prefix;
}

std::string IRCCommandParser::extractCommand(std::string& line) {
    if (line.empty()) {
        return "";
    }
    
    size_t spacePos = line.find(' ');
    if (spacePos == std::string::npos) {
        std::string command = line;
        line.clear();
        return toUpper(command);
    }
    
    std::string command = line.substr(0, spacePos);
    line = line.substr(spacePos + 1);
    return toUpper(command);
}

std::vector<std::string> IRCCommandParser::extractParams(std::string& line) {
    std::vector<std::string> params;
    
    while (!line.empty()) {
        if (line[0] == ':') {
            params.push_back(line);
            break;
        }
        
        size_t spacePos = line.find(' ');
        if (spacePos == std::string::npos) {
            params.push_back(line);
            break;
        }
        
        params.push_back(line.substr(0, spacePos));
        line = line.substr(spacePos + 1);
    }
    
    return params;
}

std::string IRCCommandParser::formatReply(const std::string& serverName,
                                          const std::string& nick,
                                          int code,
                                          const std::string& params) {
    std::ostringstream oss;
    oss << ":" << serverName << " " << code << " " << nick << " " << params;
    return oss.str();
}

std::string IRCCommandParser::formatUserMessage(const std::string& nick,
                                                const std::string& user,
                                                const std::string& host,
                                                const std::string& command,
                                                const std::string& target,
                                                const std::string& message) {
    std::ostringstream oss;
    oss << ":" << nick << "!" << user << "@" << host 
        << " " << command << " " << target;
    
    if (!message.empty()) {
        oss << " :" << message;
    }
    
    return oss.str();
}

std::string IRCCommandParser::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::vector<std::string> IRCCommandParser::splitChannels(const std::string& channels) {
    std::vector<std::string> result;
    std::stringstream ss(channels);
    std::string channel;
    
    while (std::getline(ss, channel, ',')) {
        if (!channel.empty()) {
            result.push_back(channel);
        }
    }
    
    return result;
}
```

### `log-caster-cpp-reconstructed/include/IRCCommandHandler.h`
```cpp
// [SEQUENCE: CPP-MVP6-6]
#ifndef IRCCOMMANDHANDLER_H
#define IRCCOMMANDHANDLER_H

#include <memory>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include "IRCCommandParser.h"

class IRCServer;
class IRCClient;

class IRCCommandHandler {
public:
    using CommandHandler = std::function<void(std::shared_ptr<IRCClient>, 
                                              const IRCCommandParser::IRCCommand&)>;
    
    explicit IRCCommandHandler(IRCServer* server);
    ~IRCCommandHandler();
    
    void handleCommand(std::shared_ptr<IRCClient> client, 
                      const IRCCommandParser::IRCCommand& cmd);
    
private:
    void handleNICK(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handleUSER(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handleJOIN(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handlePART(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handlePRIVMSG(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handleQUIT(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handlePING(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handleLIST(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handleNAMES(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);

    void sendWelcome(std::shared_ptr<IRCClient> client);
    void checkAuthentication(std::shared_ptr<IRCClient> client);

    IRCServer* server_;
    std::map<std::string, CommandHandler> handlers_;
};

#endif // IRCCOMMANDHANDLER_H
```

### `log-caster-cpp-reconstructed/src/IRCCommandHandler.cpp`
```cpp
// [SEQUENCE: CPP-MVP6-11]
#include "IRCCommandHandler.h"
#include "IRCServer.h"
#include "IRCClient.h"
#include "IRCChannel.h"
#include "IRCChannelManager.h"
#include "IRCClientManager.h"
#include "LogBuffer.h"
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>

IRCCommandHandler::IRCCommandHandler(IRCServer* server) : server_(server) {
    handlers_["NICK"] = [this](auto client, auto cmd) { handleNICK(client, cmd); };
    handlers_["USER"] = [this](auto client, auto cmd) { handleUSER(client, cmd); };
    handlers_["JOIN"] = [this](auto client, auto cmd) { handleJOIN(client, cmd); };
    handlers_["PART"] = [this](auto client, auto cmd) { handlePART(client, cmd); };
    handlers_["PRIVMSG"] = [this](auto client, auto cmd) { handlePRIVMSG(client, cmd); };
    handlers_["QUIT"] = [this](auto client, auto cmd) { handleQUIT(client, cmd); };
    handlers_["PING"] = [this](auto client, auto cmd) { handlePING(client, cmd); };
    handlers_["LIST"] = [this](auto client, auto cmd) { handleLIST(client, cmd); };
    handlers_["NAMES"] = [this](auto client, auto cmd) { handleNAMES(client, cmd); };
}

IRCCommandHandler::~IRCCommandHandler() = default;

void IRCCommandHandler::handleCommand(std::shared_ptr<IRCClient> client, 
                                     const IRCCommandParser::IRCCommand& cmd) {
    if (!client || cmd.command.empty()) {
        return;
    }
    
    if (!client->isAuthenticated() && 
        cmd.command != "NICK" && cmd.command != "USER" && 
        cmd.command != "QUIT") {
        client->sendErrorReply(IRCCommandParser::ERR_NOTREGISTERED, 
                              ":You have not registered");
        return;
    }
    
    auto it = handlers_.find(cmd.command);
    if (it != handlers_.end()) {
        it->second(client, cmd);
    } else {
        client->sendErrorReply(IRCCommandParser::ERR_UNKNOWNCOMMAND,
                              cmd.command + " :Unknown command");
    }
}

void IRCCommandHandler::handleNICK(std::shared_ptr<IRCClient> client, 
                                   const IRCCommandParser::IRCCommand& cmd) {
    if (cmd.params.empty()) {
        client->sendErrorReply(IRCCommandParser::ERR_NONICKNAMEGIVEN,
                              ":No nickname given");
        return;
    }
    
    std::string newNick = cmd.getParam(0);
    
    if (!server_->getClientManager()->isNicknameAvailable(newNick)) {
        client->sendErrorReply(IRCCommandParser::ERR_NICKNAMEINUSE,
                              newNick + " :Nickname is already in use");
        return;
    }
    
    std::string oldNick = client->getNickname();
    auto clientManager = server_->getClientManager();
    
    if (!oldNick.empty()) {
        clientManager->removeClient(client->getFd());
    }

    client->setNickname(newNick);
    clientManager->registerNickname(client->getFd(), newNick);
    checkAuthentication(client);
}

void IRCCommandHandler::handleUSER(std::shared_ptr<IRCClient> client,
                                   const IRCCommandParser::IRCCommand& cmd) {
    if (client->isAuthenticated()) {
        client->sendErrorReply(IRCCommandParser::ERR_ALREADYREGISTRED,
                              ":You may not reregister");
        return;
    }
    
    if (cmd.params.size() < 3) {
        client->sendErrorReply(IRCCommandParser::ERR_NEEDMOREPARAMS,
                              "USER :Not enough parameters");
        return;
    }
    
    client->setUsername(cmd.getParam(0));
    client->setHostname(cmd.getParam(1));
    client->setRealname(cmd.trailing.empty() ? cmd.getParam(3) : cmd.trailing);
    
    checkAuthentication(client);
}

void IRCCommandHandler::handleJOIN(std::shared_ptr<IRCClient> client,
                                   const IRCCommandParser::IRCCommand& cmd) {
    if (cmd.params.empty()) {
        client->sendErrorReply(IRCCommandParser::ERR_NEEDMOREPARAMS,
                              "JOIN :Not enough parameters");
        return;
    }
    
    std::vector<std::string> channels = IRCCommandParser::splitChannels(cmd.getParam(0));
    
    auto channelManager = server_->getChannelManager();
    
    for (const auto& channelName : channels) {
        if (channelName.find("#logs-") == 0 && !channelManager->channelExists(channelName)) {
            client->sendErrorReply(IRCCommandParser::ERR_NOSUCHCHANNEL,
                                  channelName + " :Log channel does not exist");
            continue;
        }
        
        channelManager->joinChannel(client, channelName);
    }
}

void IRCCommandHandler::handlePART(std::shared_ptr<IRCClient> client,
                                   const IRCCommandParser::IRCCommand& cmd) {
    if (cmd.params.empty()) {
        client->sendErrorReply(IRCCommandParser::ERR_NEEDMOREPARAMS,
                              "PART :Not enough parameters");
        return;
    }
    
    std::vector<std::string> channels = IRCCommandParser::splitChannels(cmd.getParam(0));
    std::string reason = cmd.trailing.empty() ? client->getNickname() : cmd.trailing;
    
    auto channelManager = server_->getChannelManager();
    
    for (const auto& channelName : channels) {
        if (!channelManager->channelExists(channelName)) {
            client->sendErrorReply(IRCCommandParser::ERR_NOSUCHCHANNEL,
                                  channelName + " :No such channel");
            continue;
        }
        
        if (!client->isInChannel(channelName)) {
            client->sendErrorReply(IRCCommandParser::ERR_NOTONCHANNEL,
                                  channelName + " :You're not on that channel");
            continue;
        }
        
        channelManager->partChannel(client, channelName, reason);
    }
}

void IRCCommandHandler::handlePRIVMSG(std::shared_ptr<IRCClient> client,
                                      const IRCCommandParser::IRCCommand& cmd) {
    if (cmd.params.empty()) {
        client->sendErrorReply(IRCCommandParser::ERR_NORECIPIENT,
                              ":No recipient given (PRIVMSG)");
        return;
    }
    
    if (cmd.trailing.empty() && cmd.params.size() < 2) {
        client->sendErrorReply(IRCCommandParser::ERR_NOTEXTTOSEND,
                              ":No text to send");
        return;
    }
    
    std::string target = cmd.getParam(0);
    std::string message = cmd.trailing.empty() ? cmd.getParam(1) : cmd.trailing;
    
    if (target[0] == '#' || target[0] == '&') {
        auto channel = server_->getChannelManager()->getChannel(target);
        if (!channel) {
            client->sendErrorReply(IRCCommandParser::ERR_NOSUCHCHANNEL,
                                  target + " :No such channel");
            return;
        }
        
        if (!channel->hasClient(client->getNickname())) {
            client->sendErrorReply(IRCCommandParser::ERR_CANNOTSENDTOCHAN,
                                  target + " :Cannot send to channel");
            return;
        }
        
        std::string privmsg = IRCCommandParser::formatUserMessage(
            client->getNickname(), client->getUsername(), client->getHostname(),
            "PRIVMSG", target, message
        );
        channel->broadcastExcept(privmsg, client->getNickname());
    } else {
        auto targetClient = server_->getClientManager()->getClientByNickname(target);
        if (!targetClient) {
            client->sendErrorReply(IRCCommandParser::ERR_NOSUCHNICK,
                                  target + " :No such nick/channel");
            return;
        }
        
        std::string privmsg = IRCCommandParser::formatUserMessage(
            client->getNickname(), client->getUsername(), client->getHostname(),
            "PRIVMSG", target, message
        );
        targetClient->sendMessage(privmsg);
    }
}

void IRCCommandHandler::handleQUIT(std::shared_ptr<IRCClient> client,
                                   const IRCCommandParser::IRCCommand& cmd) {
    std::string quitMessage = cmd.trailing.empty() ? "Client Quit" : cmd.trailing;
    
    std::string quitNotice = ":" + client->getFullIdentifier() + "QUIT :" + quitMessage;
    for (const auto& channelName : client->getChannels()) {
        auto channel = server_->getChannelManager()->getChannel(channelName);
        if (channel) {
            channel->broadcastExcept(quitNotice, client->getNickname());
        }
    }
    
    server_->getChannelManager()->partAllChannels(client, quitMessage);
    client->setState(IRCClient::State::DISCONNECTED);
}

void IRCCommandHandler::handlePING(std::shared_ptr<IRCClient> client,
                                   const IRCCommandParser::IRCCommand& cmd) {
    std::string param = cmd.params.empty() ? server_->getServerName() : cmd.getParam(0);
    client->sendMessage(":" + server_->getServerName() + " PONG " + 
                       server_->getServerName() + " :" + param);
}

void IRCCommandHandler::handleLIST(std::shared_ptr<IRCClient> client,
                                   const IRCCommandParser::IRCCommand& cmd) {
    auto channelManager = server_->getChannelManager();
    auto channels = channelManager->getChannelList();

    for(const auto& channelName : channels) {
        auto channel = channelManager->getChannel(channelName);
        if(channel) {
            client->sendMessage(channel->getName() + " " + std::to_string(channel->getClientCount()) + " :" + channel->getTopic());
        }
    }
}

void IRCCommandHandler::handleNAMES(std::shared_ptr<IRCClient> client,
                                    const IRCCommandParser::IRCCommand& cmd) {
    if (cmd.params.empty()) {
        // Send names for all channels
        auto channelManager = server_->getChannelManager();
        auto channels = channelManager->getChannelList();
        for(const auto& channelName : channels) {
            auto channel = channelManager->getChannel(channelName);
            if(channel) {
                std::string names;
                for(const auto& member : channel->getClients()) {
                    names += member->getNickname() + " ";
                }
                client->sendNumericReply(IRCCommandParser::RPL_NAMREPLY, "= " + channelName + " :" + names);
                client->sendNumericReply(IRCCommandParser::RPL_ENDOFNAMES, channelName + " :End of /NAMES list.");
            }
        }
        return;
    }

    std::vector<std::string> channels = IRCCommandParser::splitChannels(cmd.getParam(0));
    auto channelManager = server_->getChannelManager();

    for (const auto& channelName : channels) {
        auto channel = channelManager->getChannel(channelName);
        if (channel) {
            std::string names;
            for(const auto& member : channel->getClients()) {
                names += member->getNickname() + " ";
            }
            client->sendNumericReply(IRCCommandParser::RPL_NAMREPLY, "= " + channelName + " :" + names);
            client->sendNumericReply(IRCCommandParser::RPL_ENDOFNAMES, channelName + " :End of /NAMES list.");
        }
    }
}

void IRCCommandHandler::sendWelcome(std::shared_ptr<IRCClient> client) {
    std::string nick = client->getNickname();
    std::string serverName = server_->getServerName();
    
    client->sendNumericReply(IRCCommandParser::RPL_WELCOME,
        ":Welcome to the LogCaster IRC Network " + client->getFullIdentifier());
    
    client->sendNumericReply(IRCCommandParser::RPL_YOURHOST,
        ":Your host is " + serverName + ", running version " + server_->getServerVersion());
    
    client->sendNumericReply(IRCCommandParser::RPL_CREATED,
        ":This server was created " + server_->getServerCreated());
    
    client->sendNumericReply(IRCCommandParser::RPL_MYINFO,
        serverName + " " + server_->getServerVersion() + " o o");
}

void IRCCommandHandler::checkAuthentication(std::shared_ptr<IRCClient> client) {
    if (!client->getNickname().empty() && !client->getUsername().empty()) {
        client->setState(IRCClient::State::AUTHENTICATED);
        sendWelcome(client);
    }
}
```