# DevHistory 02: LogCaster-CPP MVP1 - C++ OOP Design

## 1. 개요 (Overview)

C로 구현된 첫 번째 MVP의 성공적인 개발 이후, 프로젝트의 두 번째 단계는 동일한 기능을 C++로 재구현하는 데 초점을 맞춥니다. 이 단계의 목표는 C++의 객체 지향 프로그래밍(OOP)과 최신 기능(Modern C++)을 활용하여 더 안전하고, 유지보수하기 쉬우며, 확장성 있는 코드를 작성하는 것입니다.

**주요 철학적 변화:**
- **C (절차적 프로그래밍) → C++ (객체 지향 프로그래밍):** 수동 리소스 관리, 함수 포인터, 전역 상태 관리에서 벗어나, 클래스 기반의 캡슐화, RAII(Resource Acquisition Is Initialization), 가상 함수, STL 컨테이너 등을 적극적으로 활용합니다.
- **오류 처리:** C 스타일의 반환 코드 확인 방식에서 C++의 예외 처리(Exception Handling) 방식으로 전환하여 오류 처리를 더 명확하고 견고하게 만듭니다.

**핵심 아키텍처:**
- **`LogServer` 클래스:** 서버의 모든 리소스(소켓, 스레드 등)와 생명주기를 관리합니다. RAII 패턴을 따라 생성자에서 리소스를 획득하고 소멸자에서 자동으로 해제합니다.
- **`Logger` 추상 클래스:** 로깅 동작에 대한 인터페이스를 정의합니다. 이를 통해 향후 파일 로거나 네트워크 로거 등 다양한 로깅 방식을 쉽게 추가할 수 있습니다.
- **`ConsoleLogger` 구상 클래스:** `Logger` 인터페이스를 구현하여 콘솔(stdout)에 로그를 출력하는 구체적인 클래스입니다.
- **동시성 모델:** C 버전의 `select` 모델 대신, 각 클라이언트 연결마다 별도의 `std::thread`를 생성하는 **'클라이언트당 스레드(Thread-per-Client)'** 모델을 채택하여 구현을 단순화합니다.

## 2. 빌드 시스템 (Build System)

C++ 프로젝트에서는 `make` 대신 `CMake`를 빌드 시스템으로 채택했습니다. `CMake`는 더 나은 의존성 관리, 크로스 플랫폼 지원, 최신 C++ 기능 탐지 등 대규모 프로젝트에 더 적합한 장점을 가집니다.

```cmake
# [SEQUENCE: MVP2-1]
# CMake 최소 요구 버전 설정
cmake_minimum_required(VERSION 3.10)

# [SEQUENCE: MVP2-2]
# 프로젝트 이름 및 버전 설정
project(LogCaster-CPP VERSION 1.0)

# [SEQUENCE: MVP2-3]
# C++ 표준 설정 (C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# [SEQUENCE: MVP2-4]
# 실행 파일 생성
add_executable(logcaster-cpp
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
)

# [SEQUENCE: MVP2-5]
# 헤더 파일 경로 추가
target_include_directories(logcaster-cpp PUBLIC include)

# [SEQUENCE: MVP2-6]
# 스레드 라이브러리 링크
find_package(Threads REQUIRED)
target_link_libraries(logcaster-cpp PRIVATE Threads::Threads)

# [SEQUENCE: MVP2-7]
# 설치 경로 설정 (선택 사항)
install(TARGETS logcaster-cpp DESTINATION bin)
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. `include/Logger.h`

로깅을 위한 추상 인터페이스(`Logger`)와 콘솔 출력을 위한 구상 클래스(`ConsoleLogger`)를 정의합니다.

```cpp
// [SEQUENCE: MVP2-8]
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>

// [SEQUENCE: MVP2-9]
// 로깅을 위한 추상 기본 클래스
class Logger {
public:
    virtual ~Logger() = default; // 가상 소멸자
    virtual void log(const std::string& message) = 0; // 순수 가상 함수
};

// [SEQUENCE: MVP2-10]
// 콘솔에 로그를 출력하는 구상 클래스
class ConsoleLogger : public Logger {
public:
    void log(const std::string& message) override;
};

#endif // LOGGER_H
```

### 3.2. `src/Logger.cpp`

`ConsoleLogger`의 `log` 함수를 구현합니다. 현재 시간을 로그 메시지 앞에 타임스탬프로 추가하여 출력합니다.

```cpp
// [SEQUENCE: MVP2-11]
#include "Logger.h"

void ConsoleLogger::log(const std::string& message) {
    // [SEQUENCE: MVP2-12]
    // 현재 시간 타임스탬프 생성
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    // [SEQUENCE: MVP2-13]
    // 타임스탬프와 메시지를 콘솔에 출력
    std::cout << "[" << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S") << "] "
              << message << std::endl;
}
```

### 3.3. `include/LogServer.h`

서버의 핵심 로직을 캡슐화하는 `LogServer` 클래스를 정의합니다. RAII 패턴을 사용하여 리소스 관리를 자동화합니다.

```cpp
// [SEQUENCE: MVP2-14]
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

// [SEQUENCE: MVP2-15]
class LogServer {
public:
    explicit LogServer(int port = DEFAULT_PORT);
    ~LogServer();

    // 복사 및 이동 생성자/대입 연산자 삭제
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

### 3.4. `src/LogServer.cpp`

`LogServer` 클래스의 멤버 함수들을 구현합니다. 생성자에서 소켓을 초기화하고, 소멸자에서 모든 리소스를 안전하게 해제합니다.

```cpp
// [SEQUENCE: MVP2-16]
#include "LogServer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <iostream>

// [SEQUENCE: MVP2-17]
// 생성자: 리소스 획득 (소켓 생성, 바인딩, 리스닝)
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

// [SEQUENCE: MVP2-18]
// 소멸자: 리소스 해제 (스레드 정리, 소켓 닫기)
LogServer::~LogServer() {
    running_ = false;
    if (listen_fd_ >= 0) {
        close(listen_fd_);
    }
    
    // 모든 클라이언트 스레드가 종료될 때까지 대기
    for (auto& t : client_threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
    logger_->log("Server shut down gracefully.");
}

// [SEQUENCE: MVP2-19]
// 서버 실행
void LogServer::run() {
    running_ = true;
    logger_->log("Server is running and waiting for connections...");

    // [SEQUENCE: MVP2-20]
    // 연결 수락을 위한 별도의 스레드 시작
    std::thread acceptor_thread(&LogServer::accept_connections, this);
    acceptor_thread.join(); // 메인 스레드는 accept 스레드가 끝날 때까지 대기
}

// [SEQUENCE: MVP2-21]
// 클라이언트 연결을 수락하는 루프
void LogServer::accept_connections() {
    while (running_) {
        int client_socket = accept(listen_fd_, nullptr, nullptr);
        if (client_socket < 0) {
            if (!running_) break; // 서버가 종료 중이면 루프 탈출
            logger_->log("Failed to accept connection");
            continue;
        }

        logger_->log("Accepted new connection on socket " + std::to_string(client_socket));

        // [SEQUENCE: MVP2-22]
        // 각 클라이언트를 위한 새 스레드 생성 및 관리
        std::lock_guard<std::mutex> lock(client_mutex_);
        client_threads_.emplace_back(&LogServer::handle_client, this, client_socket);
    }
}

// [SEQUENCE: MVP2-23]
// 개별 클라이언트의 데이터를 처리하는 함수
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
            break; // 루프 탈출하여 스레드 종료
        }
        
        // [SEQUENCE: MVP2-24]
        // 수신된 데이터를 로거를 통해 출력
        std::string message(buffer, nbytes);
        logger_->log("[LOG] from socket " + std::to_string(client_socket) + ": " + message);
    }
    close(client_socket);
}
```

### 3.5. `src/main.cpp`

C++ 프로그램의 진입점입니다. `LogServer` 객체를 생성하고 실행하며, 예외 처리를 통해 서버 초기화 실패에 대응합니다.

```cpp
// [SEQUENCE: MVP2-25]
#include "LogServer.h"
#include <iostream>
#include <csignal>

std::unique_ptr<LogServer> server_ptr;

void signal_handler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    // LogServer의 소멸자가 호출되도록 server_ptr을 리셋
    server_ptr.reset(); 
    exit(signum);
}

int main(int argc, char* argv[]) {
    // [SEQUENCE: MVP2-26]
    // 시그널 핸들러 등록
    signal(SIGINT, signal_handler);

    try {
        // [SEQUENCE: MVP2-27]
        // LogServer 객체 생성 및 실행
        int port = (argc > 1) ? std::stoi(argv[1]) : DEFAULT_PORT;
        server_ptr = std::make_unique<LogServer>(port);
        server_ptr->run();
    } catch (const std::exception& e) {
        // [SEQUENCE: MVP2-28]
        // 예외 처리
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## 4. 테스트 코드 (Test Code)

C++ 버전 서버의 기능은 C 버전과 동일하므로, MVP1에서 사용했던 Python 테스트 클라이언트(`tests/test_client.py`)를 그대로 사용하여 검증할 수 있습니다. 이는 두 버전이 동일한 프로토콜을 준수하고 있음을 보여줍니다.

*(테스트 코드 내용은 DevHistory01.md와 동일하여 생략)*

## 5. 개선 제안 (Improvement Notes)

- **클라이언트당 스레드 모델의 한계:** 이 모델은 구현이 간단하지만, 클라이언트 수가 수백, 수천 개로 늘어날 경우 스레드 생성 및 컨텍스트 스위칭 오버헤드로 인해 시스템 성능이 심각하게 저하됩니다. 이는 'C10k' 문제를 해결하기에 적합하지 않은 모델입니다. 다음 MVP에서는 C 버전과 마찬가지로 스레드 풀 아키텍처를 도입하여 이 문제를 해결해야 합니다.
- **자원 관리:** 현재는 `client_threads_` 벡터가 계속 커지기만 합니다. 종료된 스레드를 벡터에서 제거하는 로직이 필요합니다. 이를 구현하지 않으면 서버가 오래 실행될수록 메모리 사용량이 불필요하게 증가합니다.
- **로거 확장성:** `LogServer` 생성자에서 `ConsoleLogger`를 하드코딩하여 생성하고 있습니다. 향후에는 외부에서 `Logger` 구현체를 주입(Dependency Injection)받는 방식으로 변경하여 유연성을 높일 수 있습니다.
