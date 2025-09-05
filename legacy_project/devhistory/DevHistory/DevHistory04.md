# DevHistory 04: LogCaster-CPP MVP2 - Modern C++ Concurrency

## 1. 개요 (Overview)

C++ 구현의 두 번째 단계(MVP2)는 C++의 최신 기능들을 본격적으로 활용하여 C 버전 MVP2와 동일한 기능(스레드 풀, 인메모리 버퍼, 쿼리 인터페이스)을 더 안전하고 효율적으로 구현하는 데 중점을 둡니다. C++ MVP1의 '클라이언트당 스레드' 모델의 한계를 극복하고, 고성능 서버의 기반을 다지는 단계입니다.

**주요 기능 추가 및 개선:**
- **C++11 스레드 풀:** `pthreads` 대신 `<thread>`, `<mutex>`, `<condition_variable>` 등 C++ 표준 라이브러리를 사용하여 스레드 풀을 구현합니다. `std::function`을 통해 어떤 종류의 호출 가능한(callable) 작업이든 작업 큐에 저장할 수 있어 유연성이 크게 향상됩니다.
- **STL 기반 로그 버퍼:** 직접 인덱스를 관리하는 C의 원형 배열 대신, `std::deque`를 사용하여 스레드 안전 로그 버퍼를 구현합니다. `std::deque`는 양쪽 끝에서의 삽입/삭제가 O(1) 시간 복잡도를 가져 원형 버퍼 구현에 이상적입니다.
- **객체 지향 쿼리 처리:** 쿼리 처리 로직을 별도의 `QueryHandler` 클래스로 캡슐화하여 서버의 다른 기능들과 명확하게 분리합니다.
- **RAII 및 스마트 포인터 활용:** 서버의 모든 주요 구성 요소(`ThreadPool`, `LogBuffer`, `QueryHandler` 등)를 `std::unique_ptr` 또는 `std::shared_ptr`로 관리하여, 리소스 누수 가능성을 원천적으로 차단하고 코드의 안정성을 높입니다.

**아키텍처 변경:**
- **이벤트 루프 유지:** C++ MVP1의 '클라이언트당 스레드' 모델을 폐기하고, C 버전 MVP2와 유사하게 `select` 기반의 메인 이벤트 루프를 다시 도입합니다. 이는 소수의 스레드로 다수의 연결을 효율적으로 관리하기 위함입니다.
- **작업 위임:** 메인 스레드는 `select`를 통해 I/O 이벤트를 감지하고, 실제 데이터 처리 작업은 `ThreadPool`에 비동기적으로 위임합니다. 이를 통해 메인 스레드는 항상 새로운 연결을 수락할 준비 상태를 유지할 수 있습니다.

이 MVP를 통해 C++ 버전은 C 버전의 기능적 동등성을 확보하면서도, C++ 언어의 장점을 최대한 활용하여 더 높은 수준의 코드 안전성, 가독성, 유지보수성을 달성합니다.

## 2. 빌드 시스템 (Build System)

MVP2에서는 `ThreadPool`, `LogBuffer`, `QueryHandler` 클래스가 추가됨에 따라, `CMakeLists.txt`의 소스 파일 목록이 업데이트됩니다.

```cmake
# [SEQUENCE: MVP2-1]
# CMake 최소 요구 버전 설정
cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름 및 C++ 언어 지정
project(LogCasterCPP VERSION 1.0.0 LANGUAGES CXX)

# C++ 표준 설정 (C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 컴파일러 플래그 설정
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic")

# 헤더 파일 경로 추가
include_directories(include)

# [SEQUENCE: MVP2-2]
# MVP2에 필요한 소스 파일 목록
set(SOURCES
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/LogBuffer.cpp
    src/QueryHandler.cpp
)

# 실행 파일 생성
add_executable(logcaster-cpp ${SOURCES})

# [SEQUENCE: MVP2-3]
# 스레드 라이브러리 링크
find_package(Threads REQUIRED)
target_link_libraries(logcaster-cpp PRIVATE Threads::Threads)
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. `include/ThreadPool.h`

C++11 표준 라이브러리를 사용하여 구현된 헤더 전용(header-only) 스레드 풀입니다. 템플릿을 사용하여 어떤 형태의 함수든 인자와 함께 작업 큐에 추가하고, `std::future`를 통해 결과값을 비동기적으로 받을 수 있습니다.

```cpp
// [SEQUENCE: MVP2-4]
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>

// [SEQUENCE: MVP2-5]
class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // [SEQUENCE: MVP2-6]
    // 작업을 큐에 추가하는 템플릿 함수
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    void workerThread();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

// [SEQUENCE: MVP2-7]
// enqueue 멤버 함수의 템플릿 구현
template<typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    // [SEQUENCE: MVP2-8]
    // 작업을 std::packaged_task로 래핑하여 future를 얻음
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks_.emplace([task]() { (*task)(); });
    }
    condition_.notify_one();
    return res;
}

#endif // THREADPOOL_H
```

### 3.2. `src/ThreadPool.cpp`

스레드 풀의 생성자, 소멸자 및 작업자 스레드의 실행 로직을 구현합니다.

```cpp
// [SEQUENCE: MVP2-9]
#include "ThreadPool.h"

// [SEQUENCE: MVP2-10]
// 생성자: 작업자 스레드를 생성하고 실행 시작
ThreadPool::ThreadPool(size_t numThreads) : stop_(false) {
    workers_.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back([this] { workerThread(); });
    }
}

// [SEQUENCE: MVP2-11]
// 소멸자: 모든 스레드가 안전하게 종료되도록 보장
ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all(); // 모든 대기 중인 스레드를 깨움
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

// [SEQUENCE: MVP2-12]
// 작업자 스레드의 메인 루프
void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            // [SEQUENCE: MVP2-13]
            // 작업이 있거나, 종료 신호가 올 때까지 대기
            condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
            if (stop_ && tasks_.empty()) {
                return; // 종료
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task(); // 작업 실행
    }
}
```

### 3.3. `include/LogBuffer.h`

`std::deque`를 내부 컨테이너로 사용하는 스레드 안전 로그 버퍼를 정의합니다.

```cpp
// [SEQUENCE: MVP2-14]
#ifndef LOGBUFFER_H
#define LOGBUFFER_H

#include <deque>
#include <string>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>

// [SEQUENCE: MVP2-15]
// 로그 항목 구조체 (MVP2 버전)
struct LogEntry {
    std::string message;
    std::chrono::system_clock::time_point timestamp;

    LogEntry(std::string msg)
        : message(std::move(msg)), timestamp(std::chrono::system_clock::now()) {}
};

// [SEQUENCE: MVP2-16]
// 로그 버퍼 클래스
class LogBuffer {
public:
    explicit LogBuffer(size_t capacity = 10000);
    ~LogBuffer() = default;

    void push(std::string message);
    std::vector<std::string> search(const std::string& keyword) const;

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
};

#endif // LOGBUFFER_H
```

### 3.4. `src/LogBuffer.cpp`

로그 버퍼의 `push`, `search` 등 주요 기능을 구현합니다. `std::mutex`를 사용하여 동시 접근을 제어합니다.

```cpp
// [SEQUENCE: MVP2-17]
#include "LogBuffer.h"
#include <sstream>
#include <iomanip>

LogBuffer::LogBuffer(size_t capacity) : capacity_(capacity) {}

// [SEQUENCE: MVP2-18]
// 로그를 버퍼에 추가
void LogBuffer::push(std::string message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (buffer_.size() >= capacity_) {
        dropOldest_();
    }
    buffer_.emplace_back(std::move(message));
    totalLogs_++;
}

// [SEQUENCE: MVP2-19]
// 가장 오래된 로그를 삭제 (내부 헬퍼 함수)
void LogBuffer::dropOldest_() {
    if (!buffer_.empty()) {
        buffer_.pop_front();
        droppedLogs_++;
    }
}

// [SEQUENCE: MVP2-20]
// 키워드를 포함하는 로그 검색
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

// [SEQUENCE: MVP2-21]
// 현재 버퍼 크기 반환
size_t LogBuffer::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_.size();
}

// [SEQUENCE: MVP2-22]
// 통계 정보 반환
LogBuffer::StatsSnapshot LogBuffer::getStats() const {
    return { totalLogs_.load(), droppedLogs_.load() };
}
```

### 3.5. `include/QueryHandler.h`

쿼리 처리를 캡슐화한 `QueryHandler` 클래스를 정의합니다.

```cpp
// [SEQUENCE: MVP2-23]
#ifndef QUERYHANDLER_H
#define QUERYHANDLER_H

#include <string>
#include <memory>
#include "LogBuffer.h"

class QueryHandler {
public:
    explicit QueryHandler(std::shared_ptr<LogBuffer> buffer);
    std::string processQuery(const std::string& query);

private:
    std::string handleSearch(const std::string& query);
    std::string handleStats();
    std::string handleCount();
    std::string handleHelp();

    std::shared_ptr<LogBuffer> buffer_;
};

#endif // QUERYHANDLER_H
```

### 3.6. `src/QueryHandler.cpp`

`QueryHandler`의 멤버 함수를 구현합니다. MVP2에서는 간단한 문자열 파싱으로 명령을 처리합니다.

```cpp
// [SEQUENCE: MVP2-24]
#include "QueryHandler.h"
#include <sstream>

QueryHandler::QueryHandler(std::shared_ptr<LogBuffer> buffer) : buffer_(buffer) {}

// [SEQUENCE: MVP2-25]
// 쿼리 문자열을 파싱하고 적절한 핸들러 호출
std::string QueryHandler::processQuery(const std::string& query) {
    if (query.find("QUERY keyword=") == 0) {
        return handleSearch(query);
    } else if (query == "STATS") {
        return handleStats();
    } else if (query == "COUNT") {
        return handleCount();
    } else if (query == "HELP") {
        return handleHelp();
    }
    return "ERROR: Unknown command. Use HELP for usage.\n";
}

// [SEQUENCE: MVP2-26]
// 검색 쿼리 처리
std::string QueryHandler::handleSearch(const std::string& query) {
    std::string keyword = query.substr(14); // "QUERY keyword="
    auto results = buffer_->search(keyword);
    std::stringstream ss;
    ss << "FOUND: " << results.size() << " matches\n";
    for (const auto& res : results) {
        ss << res << "\n";
    }
    return ss.str();
}

// [SEQUENCE: MVP2-27]
// 통계 쿼리 처리
std::string QueryHandler::handleStats() {
    auto stats = buffer_->getStats();
    std::stringstream ss;
    ss << "STATS: Total=" << stats.totalLogs << ", Dropped=" << stats.droppedLogs 
       << ", Current=" << buffer_->size() << "\n";
    return ss.str();
}

// [SEQUENCE: MVP2-28]
// 카운트 쿼리 처리
std::string QueryHandler::handleCount() {
    std::stringstream ss;
    ss << "COUNT: " << buffer_->size() << "\n";
    return ss.str();
}

// [SEQUENCE: MVP2-29]
// 도움말 쿼리 처리
std::string QueryHandler::handleHelp() {
    return "Available commands:\n"
           "  STATS - Show buffer statistics\n"
           "  COUNT - Show number of logs in buffer\n"
           "  HELP  - Show this help message\n"
           "  QUERY keyword=<word> - Search for a keyword\n";
}
```

### 3.7. `include/LogServer.h` (MVP2 업데이트)

`LogServer`가 `ThreadPool`과 `LogBuffer` 등 새로운 구성 요소들을 소유하고 관리하도록 수정됩니다.

```cpp
// [SEQUENCE: MVP2-30]
#ifndef LOGSERVER_H
#define LOGSERVER_H

#include <memory>
#include <atomic>
#include <string>
#include "Logger.h"
#include "ThreadPool.h"
#include "LogBuffer.h"
#include "QueryHandler.h"

class LogServer {
public:
    explicit LogServer(int port = 9999);
    ~LogServer();

    LogServer(const LogServer&) = delete;
    LogServer& operator=(const LogServer&) = delete;

    void start();
    void stop();

private:
    void initialize();
    void runEventLoop();
    void handleNewConnection(int listener_fd, bool is_query_port);
    void handleClientTask(int client_fd);
    void handleQueryTask(int client_fd);

    int port_;
    int queryPort_;
    int listenFd_;
    int queryFd_;
    fd_set masterSet_;
    std::atomic<bool> running_;
    
    std::unique_ptr<Logger> logger_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::shared_ptr<LogBuffer> logBuffer_;
    std::unique_ptr<QueryHandler> queryHandler_;
};

#endif // LOGSERVER_H
```

### 3.8. `src/LogServer.cpp` (MVP2 업데이트)

`select` 기반의 이벤트 루프를 사용하여 두 개의 포트(로그, 쿼리)를 동시에 감시하고, 들어온 요청을 스레드 풀에 위임하는 로직으로 재구성됩니다.

```cpp
// [SEQUENCE: MVP2-31]
#include "LogServer.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <signal.h>

static LogServer* g_server_ptr = nullptr;
void signal_handler_cpp(int signum) {
    if (g_server_ptr) g_server_ptr->stop();
}

// [SEQUENCE: MVP2-32]
// 생성자: 모든 멤버 변수 초기화
LogServer::LogServer(int port)
    : port_(port), queryPort_(9998), listenFd_(-1), queryFd_(-1), running_(false) {
    logger_ = std::make_unique<ConsoleLogger>();
    threadPool_ = std::make_unique<ThreadPool>();
    logBuffer_ = std::make_shared<LogBuffer>();
    queryHandler_ = std::make_unique<QueryHandler>(logBuffer_);
    FD_ZERO(&masterSet_);
}

// [SEQUENCE: MVP2-33]
// 소멸자: 서버 중지
LogServer::~LogServer() {
    stop();
}

// [SEQUENCE: MVP2-34]
// 서버 시작
void LogServer::start() {
    if (running_) return;
    initialize();
    running_ = true;
    logger_->log("Server started.");
    runEventLoop();
}

// [SEQUENCE: MVP2-35]
// 서버 중지
void LogServer::stop() {
    if (!running_) return;
    running_ = false;
    if (listenFd_ != -1) close(listenFd_);
    if (queryFd_ != -1) close(queryFd_);
    logger_->log("Server stopped.");
}

// [SEQUENCE: MVP2-36]
// 리스너 소켓 생성 및 초기화
void LogServer::initialize() {
    auto create_listener = [&](int port) -> int {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) throw std::runtime_error("Socket creation failed");
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
    FD_SET(listenFd_, &masterSet_);
    FD_SET(queryFd_, &masterSet_);
    g_server_ptr = this;
    signal(SIGINT, signal_handler_cpp);
}

// [SEQUENCE: MVP2-37]
// 메인 이벤트 루프
void LogServer::runEventLoop() {
    while (running_) {
        fd_set read_fds = masterSet_;
        int max_fd = std::max(listenFd_, queryFd_);
        timeval tv = {1, 0};
        int result = select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);
        if (result < 0 && errno != EINTR) {
            logger_->log("Select error");
            break;
        }
        if (result == 0) continue;

        if (FD_ISSET(listenFd_, &read_fds)) {
            handleNewConnection(listenFd_, false);
        }
        if (FD_ISSET(queryFd_, &read_fds)) {
            handleNewConnection(queryFd_, true);
        }
    }
}

// [SEQUENCE: MVP2-38]
// 새 연결 처리
void LogServer::handleNewConnection(int listener_fd, bool is_query_port) {
    int client_fd = accept(listener_fd, nullptr, nullptr);
    if (client_fd < 0) return;

    if (is_query_port) {
        threadPool_->enqueue(&LogServer::handleQueryTask, this, client_fd);
    } else {
        threadPool_->enqueue(&LogServer::handleClientTask, this, client_fd);
    }
}

// [SEQUENCE: MVP2-39]
// 로그 클라이언트 작업
void LogServer::handleClientTask(int client_fd) {
    char buffer[4096];
    while (true) {
        ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (nbytes <= 0) break;
        buffer[nbytes] = '\0';
        logBuffer_->push(std::string(buffer));
    }
    close(client_fd);
}

// [SEQUENCE: MVP2-40]
// 쿼리 클라이언트 작업
void LogServer::handleQueryTask(int client_fd) {
    char buffer[4096];
    ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (nbytes > 0) {
        buffer[nbytes] = '\0';
        std::string query(buffer);
        // Remove trailing newline
        query.erase(query.find_last_not_of("\r\n") + 1);
        std::string response = queryHandler_->processQuery(query);
        send(client_fd, response.c_str(), response.length(), 0);
    }
    close(client_fd);
}
```

### 3.9. `src/main.cpp` (MVP2 업데이트)

`main` 함수는 `LogServer`를 생성하고 `start()`를 호출하는 역할만 합니다. 모든 복잡성은 `LogServer` 클래스 내부에서 처리됩니다.

```cpp
// [SEQUENCE: MVP2-41]
#include "LogServer.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        // [SEQUENCE: MVP2-42]
        int port = (argc > 1) ? std::stoi(argv[1]) : 9999;
        LogServer server(port);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## 4. 테스트 코드 (Test Code)

C++ MVP2 서버는 C MVP2와 동일한 포트 및 프로토콜을 사용하므로, `DevHistory03.md`에 포함된 `tests/test_mvp2.py` 스크립트를 사용하여 기능을 검증할 수 있습니다.

*(테스트 코드 내용은 DevHistory03.md와 동일하여 생략)*

```
