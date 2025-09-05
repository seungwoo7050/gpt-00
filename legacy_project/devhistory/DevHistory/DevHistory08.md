# DevHistory 08: LogCaster-CPP MVP4 - C++ Persistence Layer

## 1. 개요 (Overview)

C++ 구현의 네 번째 단계(MVP4)는 C++의 현대적인 기능들을 사용하여 C 버전 MVP4와 동일한 영속성(Persistence) 계층을 구현합니다. 이 단계에서는 C++ 표준 라이브러리가 파일 시스템, 파일 I/O, 스레딩을 얼마나 안전하고 우아하게 처리할 수 있는지 보여주는 데 중점을 둡니다.

**주요 기능 추가 및 개선:**
- **비동기 로깅:** C 버전과 마찬가지로 별도의 Writer 스레드를 두어 파일 I/O 작업을 비동기적으로 처리합니다. `std::thread`, `std::mutex`, `std::condition_variable`, `std::queue` 등 C++ 표준 라이브러리 요소들을 사용하여 이를 구현합니다.
- **`std::filesystem` 활용:** 로그 디렉토리 생성, 파일 경로 조합, 로그 파일 이름 변경(로테이션) 등 모든 파일 시스템 관련 작업을 C++17 표준인 `<filesystem>` 라이브러리를 사용하여 처리합니다. 이를 통해 플랫폼 종속적인 코드를 피하고 코드의 이식성과 가독성을 높입니다.
- **`std::fstream` 활용:** 파일 읽기/쓰기는 C의 `FILE*` 포인터 대신 C++의 `<fstream>` 라이브러리(구체적으로 `std::ofstream`)를 사용합니다. 이는 RAII(Resource Acquisition Is Initialization) 패턴을 따르므로, 파일 핸들을 수동으로 닫아주지 않아도 소멸자에서 자동으로 리소스가 해제되어 리소스 누수 가능성을 줄여줍니다.
- **RAII 기반 리소스 관리:** `PersistenceManager`의 소멸자에서 Writer 스레드를 안전하게 종료하고 `join()`하는 로직을 구현하여, 프로그램 종료 시 모든 보류 중인 로그가 디스크에 기록되도록 보장합니다.

**아키텍처 변경:**
- **`PersistenceManager` 클래스 도입:** 영속성 관련 모든 책임을 캡슐화하는 클래스입니다. 설정, 쓰기 큐, Writer 스레드, 파일 핸들러 등을 멤버 변수로 관리합니다.
- **`LogServer`와의 연동:** `LogServer`는 `std::unique_ptr<PersistenceManager>`를 소유하며, `main` 함수에서 커맨드 라인 인자에 따라 생성된 `PersistenceManager` 객체를 주입(inject)받습니다. `LogServer`의 클라이언트 처리 로직은 영속성 관리자가 존재할 경우 `persistence->write()`를 호출하여 로그 기록을 요청합니다.

이 MVP를 통해 C++ 버전은 C 버전의 영속성 기능을 모두 갖추면서, C++의 타입 안전성, 리소스 관리 용이성, 플랫폼 독립성 등의 장점을 활용하여 훨씬 더 견고하고 유지보수하기 쉬운 코드를 완성합니다.

## 2. 빌드 시스템 (Build System)

`Persistence.cpp`가 소스 목록에 추가되고, 구형 컴파일러에서 `<filesystem>`을 지원하기 위한 조건부 라이브러리 링크 설정이 추가됩니다.

```cmake
# [SEQUENCE: MVP4-1]
# ... (project, CMAKE_CXX_STANDARD 등은 MVP6와 동일) ...

# [SEQUENCE: MVP4-2]
# MVP4에 필요한 소스 파일 목록 (Persistence.cpp 추가)
set(SOURCES
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/LogBuffer.cpp
    src/QueryHandler.cpp
    src/QueryParser.cpp
    src/Persistence.cpp
)

add_executable(logcaster-cpp ${SOURCES})

target_link_libraries(logcaster-cpp PRIVATE Threads::Threads)

# [SEQUENCE: MVP4-3]
# 구형 GCC/G++ 컴파일러를 위한 stdc++fs 라이브러리 링크
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(logcaster-cpp PRIVATE stdc++fs)
endif()
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. `include/Persistence.h` (신규 파일)

영속성 관리자를 위한 설정 구조체(`PersistenceConfig`)와 메인 클래스(`PersistenceManager`)를 정의합니다.

```cpp
// [SEQUENCE: MVP4-4]
#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>
#include <filesystem>

// [SEQUENCE: MVP4-5]
// 영속성 설정을 위한 구조체
struct PersistenceConfig {
    bool enabled = false;
    std::filesystem::path log_directory = "./logs";
    size_t max_file_size = 10 * 1024 * 1024; // 10MB
    std::chrono::milliseconds flush_interval = std::chrono::milliseconds(1000);
};

// [SEQUENCE: MVP4-6]
// 영속성 관리자 클래스
class PersistenceManager {
public:
    explicit PersistenceManager(const PersistenceConfig& config);
    ~PersistenceManager();

    PersistenceManager(const PersistenceManager&) = delete;
    PersistenceManager& operator=(const PersistenceManager&) = delete;

    void write(const std::string& message);

private:
    void writerThread();
    void rotateFile();

    PersistenceConfig config_;
    std::ofstream log_file_;
    std::filesystem::path current_filepath_;
    size_t current_file_size_ = 0;

    std::queue<std::string> write_queue_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::thread writer_thread_;
    std::atomic<bool> running_;
};

#endif // PERSISTENCE_H
```

### 3.2. `src/Persistence.cpp` (신규 파일)

`PersistenceManager`의 핵심 로직을 구현합니다. 생성자에서 스레드를 시작하고, 소멸자에서 스레드를 안전하게 종료하며, 비동기 쓰기 및 파일 로테이션을 처리합니다.

```cpp
// [SEQUENCE: MVP4-7]
#include "Persistence.h"
#include <iostream>
#include <iomanip>

// [SEQUENCE: MVP4-8]
// 생성자: 디렉토리 생성, 파일 열기, Writer 스레드 시작
PersistenceManager::PersistenceManager(const PersistenceConfig& config)
    : config_(config), running_(true) {
    if (!config_.enabled) return;

    try {
        std::filesystem::create_directories(config_.log_directory);
        current_filepath_ = config_.log_directory / "current.log";
        log_file_.open(current_filepath_, std::ios::app);
        if (!log_file_.is_open()) {
            throw std::runtime_error("Failed to open log file: " + current_filepath_.string());
        }
        current_file_size_ = std::filesystem::exists(current_filepath_) ? std::filesystem::file_size(current_filepath_) : 0;

        writer_thread_ = std::thread(&PersistenceManager::writerThread, this);
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error("Filesystem error: " + std::string(e.what()));
    }
}

// [SEQUENCE: MVP4-9]
// 소멸자: Writer 스레드의 안전한 종료 보장 (RAII)
PersistenceManager::~PersistenceManager() {
    if (!config_.enabled || !running_) return;

    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        running_ = false;
    }
    condition_.notify_one();
    if (writer_thread_.joinable()) {
        writer_thread_.join();
    }
}

// [SEQUENCE: MVP4-10]
// 외부에서 로그 쓰기를 요청하는 API
void PersistenceManager::write(const std::string& message) {
    if (!config_.enabled) return;
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        write_queue_.push(message);
    }
    condition_.notify_one();
}

// [SEQUENCE: MVP4-11]
// Writer 스레드의 메인 루프
void PersistenceManager::writerThread() {
    while (running_ || !write_queue_.empty()) {
        std::queue<std::string> local_queue;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait_for(lock, config_.flush_interval, [this] { return !running_ || !write_queue_.empty(); });
            
            if (!write_queue_.empty()) {
                local_queue.swap(write_queue_);
            }
        }

        while (!local_queue.empty()) {
            const std::string& message = local_queue.front();
            if (log_file_.is_open()) {
                log_file_ << message << std::endl;
                current_file_size_ += message.length() + 1;
            }
            local_queue.pop();
        }

        if (log_file_.is_open() && current_file_size_ >= config_.max_file_size) {
            rotateFile();
        }
    }
}

// [SEQUENCE: MVP4-12]
// 로그 파일 로테이션
void PersistenceManager::rotateFile() {
    log_file_.close();
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream new_filename;
    new_filename << "log-" << std::put_time(std::localtime(&time_t_now), "%Y%m%d-%H%M%S") << ".log";
    
    try {
        std::filesystem::rename(current_filepath_, config_.log_directory / new_filename.str());
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to rotate log file: " << e.what() << std::endl;
    }

    log_file_.open(current_filepath_, std::ios::app);
    current_file_size_ = 0;
}
```

### 3.3. `include/LogServer.h` (MVP4 업데이트)

`PersistenceManager`를 소유하고 관리하기 위한 포인터와 설정 메소드를 추가합니다.

```cpp
// [SEQUENCE: MVP4-13]
#ifndef LOGSERVER_H
#define LOGSERVER_H

// ... (기존 include)
#include "Persistence.h" // Persistence.h 추가

class LogServer {
public:
    // ... (기존 public 메소드)

    // [SEQUENCE: MVP4-14]
    // PersistenceManager를 주입하기 위한 메소드
    void setPersistenceManager(std::unique_ptr<PersistenceManager> persistence);

private:
    // ... (기존 private 메소드)

    // ... (기존 멤버 변수)
    std::unique_ptr<QueryHandler> queryHandler_;

    // [SEQUENCE: MVP4-15]
    // MVP4 추가 사항
    std::unique_ptr<PersistenceManager> persistence_;
};

#endif // LOGSERVER_H
```

### 3.4. `src/LogServer.cpp` (MVP4 업데이트)

클라이언트 처리 로직에서 `PersistenceManager`의 `write` 메소드를 호출하도록 수정합니다.

```cpp
// [SEQUENCE: MVP4-16]
// ... (include 및 생성자 등은 동일)

// [SEQUENCE: MVP4-17]
// 클라이언트 작업 핸들러 (MVP4 버전)
void LogServer::handleClientTask(int client_fd) {
    char buffer[4096];
    while (true) {
        ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (nbytes <= 0) break;
        buffer[nbytes] = '\0';
        std::string log_message(buffer);

        // 1. 인메모리 버퍼에 저장
        logBuffer_->push(log_message);

        // [SEQUENCE: MVP4-18]
        // 2. 영속성 관리자에게 쓰기 요청 (활성화된 경우)
        if (persistence_) {
            persistence_->write(log_message);
        }
    }
    close(client_fd);
}

// [SEQUENCE: MVP4-19]
// PersistenceManager 설정 메소드 구현
void LogServer::setPersistenceManager(std::unique_ptr<PersistenceManager> persistence) {
    persistence_ = std::move(persistence);
}

// ... (나머지 함수는 MVP3와 동일)
```

### 3.5. `src/main.cpp` (MVP4 업데이트)

`getopt`를 사용하여 영속성 관련 커맨드 라인 인자를 파싱하고, `PersistenceManager`를 생성하여 `LogServer`에 주입하는 로직이 추가됩니다.

```cpp
// [SEQUENCE: MVP4-20]
#include "LogServer.h"
#include "Persistence.h"
#include <iostream>
#include <getopt.h>

int main(int argc, char* argv[]) {
    int port = 9999;
    PersistenceConfig persist_config;

    // [SEQUENCE: MVP4-21]
    // 커맨드 라인 인자 파싱
    int opt;
    while ((opt = getopt(argc, argv, "p:d:s:Ph")) != -1) {
        switch (opt) {
            case 'p': port = std::stoi(optarg); break;
            case 'P': persist_config.enabled = true; break;
            case 'd': persist_config.log_directory = optarg; break;
            case 's': persist_config.max_file_size = std::stoul(optarg) * 1024 * 1024; break;
            case 'h':
                std::cout << "Usage: " << argv[0] << " [-p port] [-P] [-d dir] [-s size_mb] [-h]" << std::endl;
                return 0;
        }
    }

    try {
        LogServer server(port);

        // [SEQUENCE: MVP4-22]
        // 영속성 관리자 생성 및 주입
        if (persist_config.enabled) {
            auto persistence = std::make_unique<PersistenceManager>(persist_config);
            server.setPersistenceManager(std::move(persistence));
            std::cout << "Persistence enabled. Dir: " << persist_config.log_directory 
                      << ", Max Size: " << persist_config.max_file_size / (1024*1024) << " MB" << std::endl;
        }

        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## 4. 테스트 코드 (Test Code)

C++ MVP4 서버는 C MVP4와 동일한 커맨드 라인 인자와 프로토콜을 사용하므로, `DevHistory07.md`에 포함된 `tests/test_persistence.py` 스크립트를 사용하여 기능을 검증할 수 있습니다.

*(테스트 코드 내용은 DevHistory07.md와 동일하여 생략)*
