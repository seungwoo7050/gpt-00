# 21. C++ 예외 처리 완벽 가이드 ⚡

*LogCaster 프로젝트를 위한 사전지식 문서*

---

## 📋 문서 정보
- **난이도**: 🟡 중급 → 🔴 고급
- **예상 학습 시간**: 8-10시간
- **전제 조건**: 
  - C++ 기본 문법 숙지
  - 포인터와 메모리 관리 이해
  - 클래스와 상속 개념 이해
- **실습 환경**: C++11 이상, GCC/G++, 디버거

## 🎯 이 문서에서 배울 내용

**"안전하고 견고한 C++ 프로그램을 만들기 위한 핵심 기술! ⚡"**

예외 처리는 **프로그램의 안전성과 견고성**을 보장하는 핵심 메커니즘입니다. 마치 **건물의 안전장치**처럼, 예상치 못한 상황에서도 프로그램이 우아하게 복구되거나 종료될 수 있도록 합니다.

---

## 🚨 1. 예외 처리 기초

### 예외란 무엇인가?

**예외(Exception)**는 프로그램 실행 중 발생하는 예상치 못한 상황을 나타내는 객체입니다.

```cpp
// 기본 예외 처리 구조
#include <iostream>
#include <stdexcept>

void demonstrateBasicException() {
    try {
        // 예외가 발생할 수 있는 코드
        int* ptr = nullptr;
        if (!ptr) {
            throw std::runtime_error("널 포인터 접근!");
        }
        *ptr = 42;  // 이 코드는 실행되지 않음
    }
    catch (const std::runtime_error& e) {
        // 예외 처리 코드
        std::cout << "예외 발생: " << e.what() << std::endl;
    }
    catch (...) {
        // 모든 다른 예외 처리
        std::cout << "알 수 없는 예외 발생!" << std::endl;
    }
}

// LogCaster에서의 실제 사용 예시
class LogServer {
public:
    void startServer(int port) {
        try {
            if (port < 1024 || port > 65535) {
                throw std::invalid_argument("포트 범위가 잘못되었습니다: " + std::to_string(port));
            }
            
            if (!createSocket(port)) {
                throw std::runtime_error("소켓 생성 실패");
            }
            
            std::cout << "서버가 포트 " << port << "에서 시작되었습니다." << std::endl;
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "설정 오류: " << e.what() << std::endl;
            throw;  // 예외 재던지기
        }
        catch (const std::runtime_error& e) {
            std::cerr << "런타임 오류: " << e.what() << std::endl;
            cleanup();  // 정리 작업
            throw;
        }
    }

private:
    bool createSocket(int port) { 
        // 소켓 생성 로직
        return true; 
    }
    
    void cleanup() {
        // 리소스 정리
        std::cout << "서버 리소스 정리 중..." << std::endl;
    }
};
```

### 예외의 장점과 단점

**장점:**
- 오류 코드보다 명확하고 읽기 쉬움
- 자동 스택 해제 (Stack Unwinding)
- 타입 안전성
- 처리 로직과 오류 처리 로직 분리

**단점:**
- 성능 오버헤드 (예외 발생 시)
- 코드 복잡성 증가 가능
- 디버깅이 어려울 수 있음

---

## 🎯 2. 표준 예외 클래스 계층

```cpp
#include <stdexcept>
#include <iostream>
#include <memory>

// 표준 예외 클래스 사용 예시
class LogStorage {
private:
    std::unique_ptr<char[]> buffer_;
    size_t capacity_;
    size_t size_;

public:
    LogStorage(size_t capacity) : capacity_(capacity), size_(0) {
        if (capacity == 0) {
            throw std::invalid_argument("용량은 0보다 커야 합니다");
        }
        
        try {
            buffer_ = std::make_unique<char[]>(capacity_);
        }
        catch (const std::bad_alloc& e) {
            throw std::runtime_error("메모리 할당 실패: " + std::string(e.what()));
        }
    }
    
    void addLog(const std::string& message) {
        if (size_ >= capacity_) {
            throw std::overflow_error("로그 스토리지가 가득참");
        }
        
        if (message.empty()) {
            throw std::invalid_argument("빈 로그 메시지는 추가할 수 없습니다");
        }
        
        // 로그 추가 로직...
        size_++;
    }
    
    std::string getLog(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("인덱스가 범위를 벗어남: " + std::to_string(index));
        }
        
        // 로그 반환 로직...
        return "Log entry " + std::to_string(index);
    }
};

// 표준 예외 사용 예시
void demonstrateStandardExceptions() {
    try {
        LogStorage storage(100);
        
        // 정상 작업
        storage.addLog("서버 시작됨");
        std::cout << storage.getLog(0) << std::endl;
        
        // 예외 발생 상황들
        storage.addLog("");  // invalid_argument
        // storage.getLog(1000);  // out_of_range
        
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "잘못된 인자: " << e.what() << std::endl;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "범위 오류: " << e.what() << std::endl;
    }
    catch (const std::overflow_error& e) {
        std::cerr << "오버플로우 오류: " << e.what() << std::endl;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "런타임 오류: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "표준 예외: " << e.what() << std::endl;
    }
}
```

### 주요 표준 예외 클래스

```cpp
// 표준 예외 계층구조 이해
namespace ExceptionHierarchy {
    void demonstrateHierarchy() {
        // std::exception (최상위)
        //   ├── std::logic_error
        //   │   ├── std::invalid_argument
        //   │   ├── std::domain_error  
        //   │   ├── std::length_error
        //   │   └── std::out_of_range
        //   ├── std::runtime_error
        //   │   ├── std::range_error
        //   │   ├── std::overflow_error
        //   │   └── std::underflow_error
        //   ├── std::bad_alloc
        //   ├── std::bad_cast
        //   └── std::bad_typeid
        
        std::cout << "표준 예외 계층구조:\n";
        std::cout << "1. logic_error: 프로그램 로직 오류\n";
        std::cout << "2. runtime_error: 실행 시간 오류\n";
        std::cout << "3. bad_alloc: 메모리 할당 실패\n";
        std::cout << "4. bad_cast: 타입 변환 실패\n";
    }
}
```

---

## 🏗️ 3. 커스텀 예외 클래스

### 기본 커스텀 예외

```cpp
#include <exception>
#include <string>

// LogCaster 전용 예외 클래스들
namespace LogCaster {
    
    // 기본 LogCaster 예외
    class LogCasterException : public std::exception {
    protected:
        std::string message_;
        int error_code_;
        
    public:
        LogCasterException(const std::string& message, int code = 0) 
            : message_(message), error_code_(code) {}
            
        const char* what() const noexcept override {
            return message_.c_str();
        }
        
        int getErrorCode() const noexcept { return error_code_; }
        
        virtual std::string getErrorType() const { return "LogCasterException"; }
    };
    
    // 네트워크 관련 예외
    class NetworkException : public LogCasterException {
    public:
        NetworkException(const std::string& message, int code = 0) 
            : LogCasterException("Network Error: " + message, code) {}
            
        std::string getErrorType() const override { return "NetworkException"; }
    };
    
    // 스토리지 관련 예외
    class StorageException : public LogCasterException {
    private:
        size_t attempted_size_;
        size_t available_space_;
        
    public:
        StorageException(const std::string& message, size_t attempted, size_t available) 
            : LogCasterException("Storage Error: " + message), 
              attempted_size_(attempted), available_space_(available) {}
              
        size_t getAttemptedSize() const { return attempted_size_; }
        size_t getAvailableSpace() const { return available_space_; }
        
        std::string getErrorType() const override { return "StorageException"; }
        
        std::string getDetailedMessage() const {
            return message_ + " (시도: " + std::to_string(attempted_size_) + 
                   ", 사용가능: " + std::to_string(available_space_) + ")";
        }
    };
    
    // 설정 관련 예외
    class ConfigurationException : public LogCasterException {
    private:
        std::string config_key_;
        std::string config_value_;
        
    public:
        ConfigurationException(const std::string& key, const std::string& value, 
                             const std::string& reason) 
            : LogCasterException("Configuration Error for '" + key + "': " + reason),
              config_key_(key), config_value_(value) {}
              
        const std::string& getConfigKey() const { return config_key_; }
        const std::string& getConfigValue() const { return config_value_; }
        
        std::string getErrorType() const override { return "ConfigurationException"; }
    };
}

// 사용 예시
class LogServer {
private:
    int port_;
    bool running_;
    
public:
    void configure(int port, const std::string& log_level) {
        if (port < 1024 || port > 65535) {
            throw LogCaster::ConfigurationException(
                "port", std::to_string(port), 
                "포트는 1024-65535 범위여야 합니다"
            );
        }
        
        if (log_level != "DEBUG" && log_level != "INFO" && 
            log_level != "WARN" && log_level != "ERROR") {
            throw LogCaster::ConfigurationException(
                "log_level", log_level, 
                "유효한 로그 레벨이 아닙니다"
            );
        }
        
        port_ = port;
    }
    
    void start() {
        try {
            if (!createSocket()) {
                throw LogCaster::NetworkException("소켓 생성 실패", errno);
            }
            
            if (!bindSocket()) {
                throw LogCaster::NetworkException("소켓 바인딩 실패", errno);  
            }
            
            running_ = true;
            std::cout << "서버가 포트 " << port_ << "에서 시작됨" << std::endl;
        }
        catch (const LogCaster::NetworkException& e) {
            std::cerr << "네트워크 오류 [" << e.getErrorCode() << "]: " 
                      << e.what() << std::endl;
            throw;
        }
    }
    
private:
    bool createSocket() { return true; }  // 실제 구현
    bool bindSocket() { return true; }    // 실제 구현
};
```

### 고급 커스텀 예외 기법

```cpp
// 예외 체이닝과 컨텍스트 정보
namespace LogCaster {
    
    class DetailedException : public std::exception {
    private:
        std::string message_;
        std::string file_;
        int line_;
        std::string function_;
        std::exception_ptr nested_exception_;
        
    public:
        DetailedException(const std::string& message, const char* file, 
                         int line, const char* function) 
            : message_(message), file_(file), line_(line), function_(function) {
            // 현재 예외가 있다면 중첩 예외로 저장
            nested_exception_ = std::current_exception();
        }
        
        const char* what() const noexcept override {
            return message_.c_str();
        }
        
        std::string getFullInfo() const {
            std::string info = message_ + "\n";
            info += "File: " + file_ + ":" + std::to_string(line_) + "\n";
            info += "Function: " + function_ + "\n";
            
            if (nested_exception_) {
                try {
                    std::rethrow_exception(nested_exception_);
                }
                catch (const std::exception& e) {
                    info += "Nested: " + std::string(e.what()) + "\n";
                }
                catch (...) {
                    info += "Nested: Unknown exception\n";
                }
            }
            
            return info;
        }
    };
    
    // 매크로를 통한 편리한 사용
    #define THROW_DETAILED(message) \
        throw LogCaster::DetailedException(message, __FILE__, __LINE__, __FUNCTION__)
        
    // 사용 예시
    void processLogFile(const std::string& filename) {
        try {
            if (filename.empty()) {
                THROW_DETAILED("파일 이름이 비어있습니다");
            }
            
            // 파일 처리 로직...
            throw std::runtime_error("파일 읽기 실패");  // 시뮬레이션
        }
        catch (const std::runtime_error& e) {
            // 기존 예외를 감싸서 더 자세한 정보 제공
            THROW_DETAILED("로그 파일 처리 중 오류 발생: " + filename);
        }
    }
}
```

---

## 🔒 4. RAII와 예외 안전성

### 예외 안전성 수준

```cpp
#include <memory>
#include <vector>
#include <fstream>

// 예외 안전성 수준:
// 1. 기본 보장 (Basic Guarantee): 리소스 누수 없음
// 2. 강한 보장 (Strong Guarantee): 실패 시 원래 상태 유지  
// 3. 예외 없음 보장 (No-throw Guarantee): 예외 발생 안함

class LogBuffer {
private:
    std::vector<std::string> logs_;
    std::unique_ptr<std::ofstream> file_;
    
public:
    // 강한 예외 안전성 보장 - 실패 시 원래 상태 유지
    void addLog(const std::string& message) {
        if (message.empty()) {
            throw std::invalid_argument("빈 메시지는 추가할 수 없습니다");
        }
        
        // 임시 벡터를 사용하여 강한 예외 안전성 보장
        auto temp_logs = logs_;  // 복사
        temp_logs.push_back(message);  // 여기서 예외 발생 가능
        
        // 성공했을 때만 실제 데이터 업데이트
        logs_ = std::move(temp_logs);  // noexcept 이동
    }
    
    // 예외 없음 보장 - noexcept 명시
    size_t size() const noexcept {
        return logs_.size();
    }
    
    bool empty() const noexcept {
        return logs_.empty();
    }
    
    // 기본 예외 안전성 보장 - 리소스 누수 없음
    void saveToFile(const std::string& filename) {
        // RAII를 사용한 파일 핸들링
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("파일을 열 수 없습니다: " + filename);
        }
        
        // 파일 쓰기 중 예외 발생해도 file 객체는 자동으로 정리됨
        for (const auto& log : logs_) {
            file << log << '\n';
            if (file.fail()) {
                throw std::runtime_error("파일 쓰기 실패");
            }
        }
        
        // file은 여기서 자동으로 닫힘 (RAII)
    }
    
    // 복사 연산 - 강한 예외 안전성
    LogBuffer& operator=(const LogBuffer& other) {
        if (this == &other) return *this;
        
        // 임시 객체 생성 (여기서 예외 발생 가능)
        LogBuffer temp(other);
        
        // 성공했을 때만 스왑 (noexcept)
        swap(temp);
        
        return *this;
    }
    
private:
    void swap(LogBuffer& other) noexcept {
        logs_.swap(other.logs_);
        file_.swap(other.file_);
    }
};

// RAII 클래스 예시 - 리소스 자동 관리
class SocketRAII {
private:
    int socket_fd_;
    
public:
    explicit SocketRAII(int port) : socket_fd_(-1) {
        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd_ < 0) {
            throw std::runtime_error("소켓 생성 실패");
        }
        
        // 바인딩 시도
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        
        if (bind(socket_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            close(socket_fd_);  // 생성자에서 실패 시 정리
            throw std::runtime_error("소켓 바인딩 실패");
        }
    }
    
    // 소멸자에서 자동 정리
    ~SocketRAII() noexcept {
        if (socket_fd_ >= 0) {
            close(socket_fd_);
        }
    }
    
    // 복사 금지
    SocketRAII(const SocketRAII&) = delete;
    SocketRAII& operator=(const SocketRAII&) = delete;
    
    // 이동 허용
    SocketRAII(SocketRAII&& other) noexcept : socket_fd_(other.socket_fd_) {
        other.socket_fd_ = -1;
    }
    
    SocketRAII& operator=(SocketRAII&& other) noexcept {
        if (this != &other) {
            if (socket_fd_ >= 0) {
                close(socket_fd_);
            }
            socket_fd_ = other.socket_fd_;
            other.socket_fd_ = -1;
        }
        return *this;
    }
    
    int get() const noexcept { return socket_fd_; }
};
```

### 스마트 포인터와 예외 안전성

```cpp
#include <memory>
#include <iostream>

// 스마트 포인터를 활용한 예외 안전한 코드
class LogProcessor {
private:
    std::unique_ptr<LogBuffer> buffer_;
    std::shared_ptr<std::ofstream> output_file_;
    
public:
    LogProcessor(const std::string& output_filename) {
        try {
            // unique_ptr 사용으로 예외 시 자동 정리
            buffer_ = std::make_unique<LogBuffer>();
            
            // shared_ptr와 커스텀 deleter
            output_file_ = std::shared_ptr<std::ofstream>(
                new std::ofstream(output_filename),
                [](std::ofstream* file) {
                    if (file && file->is_open()) {
                        std::cout << "파일 닫는 중..." << std::endl;
                        file->close();
                    }
                    delete file;
                }
            );
            
            if (!output_file_->is_open()) {
                throw std::runtime_error("출력 파일을 열 수 없습니다");
            }
        }
        catch (...) {
            // 생성자에서 예외 발생 시 이미 생성된 자원들은 자동으로 정리됨
            std::cerr << "LogProcessor 생성 실패" << std::endl;
            throw;
        }
    }
    
    void processLog(const std::string& message) {
        if (!buffer_ || !output_file_) {
            throw std::logic_error("LogProcessor가 올바르게 초기화되지 않았습니다");
        }
        
        try {
            buffer_->addLog(message);
            *output_file_ << message << '\n';
            
            if (output_file_->fail()) {
                throw std::runtime_error("파일 쓰기 실패");
            }
        }
        catch (const std::exception& e) {
            std::cerr << "로그 처리 중 오류: " << e.what() << std::endl;
            throw;  // 예외 재던지기
        }
    }
    
    // shared_ptr을 반환하여 안전한 공유
    std::shared_ptr<std::ofstream> getOutputFile() const {
        return output_file_;
    }
};

// weak_ptr을 사용한 순환 참조 방지
class LogNode {
private:
    std::string data_;
    std::shared_ptr<LogNode> next_;
    std::weak_ptr<LogNode> parent_;  // 순환 참조 방지
    
public:
    LogNode(const std::string& data) : data_(data) {}
    
    void setNext(std::shared_ptr<LogNode> next) {
        next_ = next;
        if (next) {
            next->parent_ = shared_from_this();
        }
    }
    
    std::shared_ptr<LogNode> getNext() const { return next_; }
    
    std::shared_ptr<LogNode> getParent() const { 
        return parent_.lock();  // weak_ptr을 shared_ptr로 변환
    }
    
    const std::string& getData() const { return data_; }
};
```

---

## 🎮 5. 고급 예외 처리 기법

### 예외 명세와 noexcept

```cpp
// C++11 이후 예외 명세
class ModernLogServer {
public:
    // 예외를 던지지 않는 함수
    int getPort() const noexcept { return port_; }
    bool isRunning() const noexcept { return running_; }
    
    // 조건부 noexcept - 템플릿에서 유용
    template<typename T>
    void addLog(T&& log) noexcept(std::is_nothrow_constructible_v<std::string, T>) {
        try {
            logs_.emplace_back(std::forward<T>(log));
        }
        catch (...) {
            // noexcept(false)인 경우에만 실행
            if constexpr (!std::is_nothrow_constructible_v<std::string, T>) {
                std::terminate();  // 예외를 던질 수 없으므로 프로그램 종료
            }
        }
    }
    
    // 이동 생성자는 보통 noexcept
    ModernLogServer(ModernLogServer&& other) noexcept 
        : port_(other.port_), running_(other.running_), logs_(std::move(other.logs_)) {
        other.running_ = false;
    }
    
    // 소멸자는 항상 noexcept (기본값)
    ~ModernLogServer() noexcept {
        shutdown();
    }
    
private:
    int port_ = 0;
    bool running_ = false;
    std::vector<std::string> logs_;
    
    void shutdown() noexcept {
        // 소멸자에서는 예외를 던지면 안 됨
        try {
            running_ = false;
            // 정리 작업...
        }
        catch (...) {
            // 소멸자에서 예외 발생 시 무시하거나 로그만 남김
            std::cerr << "셧다운 중 오류 발생" << std::endl;
        }
    }
};
```

### 함수 try 블록

```cpp
// 생성자와 소멸자에서의 특별한 예외 처리
class ResourceManager {
private:
    std::unique_ptr<int[]> data_;
    std::unique_ptr<std::ofstream> file_;
    
public:
    // 함수 try 블록 - 초기화 리스트에서 발생하는 예외 처리
    ResourceManager(size_t size, const std::string& filename)
    try : data_(std::make_unique<int[]>(size)), 
          file_(std::make_unique<std::ofstream>(filename))
    {
        // 생성자 본문
        if (!file_->is_open()) {
            throw std::runtime_error("파일 열기 실패");
        }
    }
    catch (const std::bad_alloc& e) {
        // data_ 할당 실패 시
        std::cerr << "메모리 할당 실패: " << e.what() << std::endl;
        throw;  // 반드시 재던지기 (생성자에서)
    }
    catch (const std::exception& e) {
        // 기타 예외
        std::cerr << "ResourceManager 생성 실패: " << e.what() << std::endl;
        throw;  // 반드시 재던지기
    }
    
    // 함수 try 블록은 일반 함수에서도 사용 가능
    void processData(const std::vector<int>& input)
    try {
        if (!data_ || !file_) {
            throw std::logic_error("리소스가 초기화되지 않음");
        }
        
        for (size_t i = 0; i < input.size(); ++i) {
            data_[i] = input[i] * 2;
            *file_ << data_[i] << " ";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "데이터 처리 중 오류: " << e.what() << std::endl;
        throw;  // 선택적 재던지기
    }
};
```

### std::exception_ptr과 예외 전송

```cpp
#include <future>
#include <thread>
#include <exception>

// 스레드 간 예외 전송
class AsyncLogProcessor {
private:
    std::vector<std::future<void>> workers_;
    std::exception_ptr stored_exception_;
    
public:
    void startAsyncProcessing(const std::vector<std::string>& logs) {
        for (size_t i = 0; i < logs.size(); ++i) {
            auto future = std::async(std::launch::async, [this, i, &logs]() {
                try {
                    processLogAsync(logs[i]);
                }
                catch (...) {
                    // 예외를 저장
                    std::lock_guard<std::mutex> lock(exception_mutex_);
                    if (!stored_exception_) {
                        stored_exception_ = std::current_exception();
                    }
                }
            });
            
            workers_.push_back(std::move(future));
        }
    }
    
    void waitForCompletion() {
        // 모든 작업 완료 대기
        for (auto& worker : workers_) {
            worker.wait();
        }
        
        // 저장된 예외가 있다면 재던지기
        if (stored_exception_) {
            std::rethrow_exception(stored_exception_);
        }
    }
    
private:
    std::mutex exception_mutex_;
    
    void processLogAsync(const std::string& log) {
        // 로그 처리 로직
        if (log.empty()) {
            throw std::invalid_argument("빈 로그");
        }
        
        // 시뮬레이션: 10% 확률로 예외 발생
        if (std::rand() % 10 == 0) {
            throw std::runtime_error("랜덤 처리 오류");
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
};

// 사용 예시
void demonstrateAsyncException() {
    try {
        AsyncLogProcessor processor;
        std::vector<std::string> logs = {"log1", "log2", "", "log4", "log5"};
        
        processor.startAsyncProcessing(logs);
        processor.waitForCompletion();
        
        std::cout << "모든 로그 처리 완료" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "비동기 처리 중 오류: " << e.what() << std::endl;
    }
}
```

---

## 🎯 6. LogCaster 프로젝트 예외 처리 전략

### 계층별 예외 처리

```cpp
// LogCaster 전용 예외 처리 전략
namespace LogCaster {
    
    // 1. 네트워크 계층 예외 처리
    class NetworkLayer {
    public:
        void acceptConnection(int server_fd) {
            try {
                int client_fd = accept(server_fd, nullptr, nullptr);
                if (client_fd < 0) {
                    throw NetworkException("클라이언트 연결 수락 실패", errno);
                }
                
                handleClient(client_fd);
            }
            catch (const NetworkException& e) {
                // 네트워크 예외는 로깅하고 계속 진행
                logError("Network", e.what());
                // 다른 연결은 계속 처리
            }
            catch (const std::exception& e) {
                // 예상치 못한 예외는 상위로 전파
                logError("Unexpected", e.what());
                throw;
            }
        }
        
    private:
        void handleClient(int client_fd) {
            try {
                // 클라이언트 처리 로직
                char buffer[1024];
                ssize_t bytes = recv(client_fd, buffer, sizeof(buffer), 0);
                
                if (bytes < 0) {
                    throw NetworkException("데이터 수신 실패", errno);
                }
                
                processMessage(std::string(buffer, bytes));
            }
            catch (...) {
                close(client_fd);  // 리소스 정리
                throw;  // 예외 재던지기
            }
        }
        
        void processMessage(const std::string& message);
        void logError(const std::string& category, const std::string& message);
    };
    
    // 2. 스토리지 계층 예외 처리
    class StorageLayer {
    private:
        std::unique_ptr<LogBuffer> buffer_;
        std::mutex buffer_mutex_;
        
    public:
        void storeLog(const std::string& message) {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            
            try {
                if (!buffer_) {
                    throw StorageException("스토리지가 초기화되지 않음", 0, 0);
                }
                
                if (buffer_->size() >= buffer_->capacity()) {
                    // 버퍼가 가득 참 - 오래된 로그 삭제 후 재시도
                    buffer_->removeOldest();
                }
                
                buffer_->add(message);
            }
            catch (const StorageException& e) {
                // 스토리지 예외는 복구 시도
                if (attemptRecovery()) {
                    buffer_->add(message);  // 재시도
                } else {
                    throw;  // 복구 실패 시 예외 전파
                }
            }
        }
        
    private:
        bool attemptRecovery() {
            try {
                // 복구 로직 (예: 임시 파일에 저장)
                return true;
            }
            catch (...) {
                return false;
            }
        }
    };
    
    // 3. 애플리케이션 계층 예외 처리
    class LogCasterApp {
    private:
        std::unique_ptr<NetworkLayer> network_;
        std::unique_ptr<StorageLayer> storage_;
        std::atomic<bool> shutdown_requested_{false};
        
    public:
        void run() {
            try {
                initialize();
                mainLoop();
            }
            catch (const ConfigurationException& e) {
                // 설정 오류는 치명적 - 프로그램 종료
                std::cerr << "치명적 설정 오류: " << e.what() << std::endl;
                std::exit(1);
            }
            catch (const LogCasterException& e) {
                // LogCaster 예외는 로깅 후 우아한 종료
                std::cerr << "LogCaster 오류 (" << e.getErrorType() << "): " 
                          << e.what() << std::endl;
                gracefulShutdown();
            }
            catch (const std::exception& e) {
                // 표준 예외는 예기치 못한 오류
                std::cerr << "예기치 못한 오류: " << e.what() << std::endl;
                emergencyShutdown();
            }
            catch (...) {
                // 알 수 없는 예외
                std::cerr << "알 수 없는 치명적 오류 발생" << std::endl;
                std::terminate();
            }
        }
        
    private:
        void initialize() {
            network_ = std::make_unique<NetworkLayer>();
            storage_ = std::make_unique<StorageLayer>();
            
            // 초기화 실패는 ConfigurationException으로 감싸기
            try {
                network_->initialize();
                storage_->initialize();
            }
            catch (const std::exception& e) {
                throw ConfigurationException("initialization", "", 
                    "초기화 실패: " + std::string(e.what()));
            }
        }
        
        void mainLoop() {
            while (!shutdown_requested_.load()) {
                try {
                    network_->processConnections();
                }
                catch (const NetworkException& e) {
                    // 네트워크 예외는 로깅만 하고 계속
                    std::cerr << "네트워크 오류: " << e.what() << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
        }
        
        void gracefulShutdown() noexcept {
            try {
                shutdown_requested_.store(true);
                // 진행 중인 작업 완료 대기
                // 리소스 정리
            }
            catch (...) {
                // 셧다운 중 예외는 무시
            }
        }
        
        void emergencyShutdown() noexcept {
            // 긴급 종료 - 최소한의 정리만
            shutdown_requested_.store(true);
        }
    };
}
```

### 예외 로깅과 모니터링

```cpp
// 예외 로깅 시스템
class ExceptionLogger {
private:
    std::ofstream log_file_;
    std::mutex log_mutex_;
    
public:
    ExceptionLogger(const std::string& filename) : log_file_(filename, std::ios::app) {
        if (!log_file_.is_open()) {
            throw std::runtime_error("예외 로그 파일을 열 수 없음: " + filename);
        }
    }
    
    void logException(const std::exception& e, const std::string& context = "") {
        std::lock_guard<std::mutex> lock(log_mutex_);
        
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        log_file_ << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
        log_file_ << "EXCEPTION: " << typeid(e).name() << " - " << e.what();
        
        if (!context.empty()) {
            log_file_ << " (Context: " << context << ")";
        }
        
        log_file_ << std::endl;
        log_file_.flush();
    }
    
    void logUnknownException(const std::string& context = "") {
        std::lock_guard<std::mutex> lock(log_mutex_);
        
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        log_file_ << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
        log_file_ << "UNKNOWN EXCEPTION";
        
        if (!context.empty()) {
            log_file_ << " (Context: " << context << ")";
        }
        
        log_file_ << std::endl;
        log_file_.flush();
    }
};

// 전역 예외 핸들러
void setupGlobalExceptionHandler() {
    static ExceptionLogger logger("logcaster_exceptions.log");
    
    // std::terminate 핸들러 설정
    std::set_terminate([]() {
        try {
            logger.logUnknownException("std::terminate called");
            std::rethrow_exception(std::current_exception());
        }
        catch (const std::exception& e) {
            logger.logException(e, "terminate handler");
        }
        catch (...) {
            logger.logUnknownException("terminate handler");
        }
        
        std::abort();
    });
}
```

---

## 📊 7. 성능과 베스트 프랙티스

### 예외 처리 성능 최적화

```cpp
// 예외 처리 성능 고려사항
class PerformantLogProcessor {
private:
    static constexpr size_t MAX_LOGS = 10000;
    std::array<std::string, MAX_LOGS> logs_;
    size_t log_count_ = 0;
    
public:
    // GOOD: 예외를 사용하지 않는 빠른 경로
    bool tryAddLog(const std::string& message) noexcept {
        if (log_count_ >= MAX_LOGS || message.empty()) {
            return false;  // 예외 대신 bool 반환
        }
        
        logs_[log_count_++] = message;
        return true;
    }
    
    // 예외를 던지는 버전 (필요한 경우에만)
    void addLogWithException(const std::string& message) {
        if (log_count_ >= MAX_LOGS) {
            throw std::overflow_error("로그 버퍼가 가득참");
        }
        
        if (message.empty()) {
            throw std::invalid_argument("빈 메시지");
        }
        
        logs_[log_count_++] = message;
    }
    
    // 성능 중요한 루프에서는 예외 최소화
    size_t processBatch(const std::vector<std::string>& messages) noexcept {
        size_t processed = 0;
        
        for (const auto& msg : messages) {
            if (tryAddLog(msg)) {  // 예외 없는 버전 사용
                ++processed;
            } else {
                break;  // 실패 시 중단
            }
        }
        
        return processed;
    }
};

// 예외 대신 std::optional 사용
#include <optional>

class SafeLogRetriever {
private:
    std::vector<std::string> logs_;
    
public:
    // 예외 대신 optional 사용 - 성능상 이점
    std::optional<std::string> getLog(size_t index) const noexcept {
        if (index >= logs_.size()) {
            return std::nullopt;  // 예외 대신 빈 값
        }
        
        return logs_[index];
    }
    
    // 예외 기반 버전 (API 호환성을 위해 필요한 경우)
    const std::string& getLogUnsafe(size_t index) const {
        if (index >= logs_.size()) {
            throw std::out_of_range("인덱스 초과: " + std::to_string(index));
        }
        
        return logs_[index];
    }
};
```

### 예외 처리 베스트 프랙티스

```cpp
// 예외 처리 베스트 프랙티스 모음
namespace BestPractices {
    
    // 1. 예외는 예외적인 상황에만 사용
    class LogValidator {
    public:
        // GOOD: 일반적인 검증은 bool 반환
        static bool isValidLogLevel(const std::string& level) noexcept {
            return level == "DEBUG" || level == "INFO" || 
                   level == "WARN" || level == "ERROR";
        }
        
        // 예외는 진짜 오류 상황에만
        static void validateCriticalConfig(const std::string& config_path) {
            std::ifstream file(config_path);
            if (!file.is_open()) {
                throw std::runtime_error("중요 설정 파일 읽기 실패: " + config_path);
            }
        }
    };
    
    // 2. 값으로 예외 던지기, 참조로 받기
    void demonstrateExceptionHandling() {
        try {
            // GOOD: 값으로 던지기
            throw std::runtime_error("오류 메시지");
        }
        catch (const std::exception& e) {  // GOOD: const 참조로 받기
            std::cout << e.what() << std::endl;
        }
        // BAD: catch (std::exception e) - 불필요한 복사
    }
    
    // 3. 리소스 관리는 RAII 사용
    class FileProcessor {
    private:
        std::unique_ptr<std::ifstream> input_;
        std::unique_ptr<std::ofstream> output_;
        
    public:
        FileProcessor(const std::string& input_file, const std::string& output_file) 
            : input_(std::make_unique<std::ifstream>(input_file)),
              output_(std::make_unique<std::ofstream>(output_file)) {
            
            if (!input_->is_open()) {
                throw std::runtime_error("입력 파일 열기 실패");
            }
            
            if (!output_->is_open()) {
                throw std::runtime_error("출력 파일 열기 실패");
            }
        }
        
        // 소멸자에서 자동 정리 - try/catch 불필요
        ~FileProcessor() = default;  // unique_ptr이 알아서 정리
        
        void process() {
            std::string line;
            while (std::getline(*input_, line)) {
                // 처리 중 예외 발생해도 파일은 자동으로 닫힘
                processLine(line);
                *output_ << line << '\n';
            }
        }
        
    private:
        void processLine(const std::string& line) {
            if (line.empty()) {
                throw std::invalid_argument("빈 라인 처리 불가");
            }
            // 처리 로직...
        }
    };
    
    // 4. 적절한 예외 타입 선택
    class ConfigManager {
    public:
        void setPort(int port) {
            // 논리적 오류 - logic_error 계열
            if (port < 1 || port > 65535) {
                throw std::invalid_argument("포트 범위 오류: " + std::to_string(port));
            }
        }
        
        void loadFromFile(const std::string& filename) {
            std::ifstream file(filename);
            
            // 런타임 오류 - runtime_error 계열
            if (!file.is_open()) {
                throw std::runtime_error("설정 파일 읽기 실패: " + filename);
            }
        }
    };
    
    // 5. 예외 체이닝과 컨텍스트 보존
    class HighLevelProcessor {
    public:
        void processUserRequest(const std::string& request) {
            try {
                parseRequest(request);
            }
            catch (const std::exception& e) {
                // 하위 수준 예외를 상위 수준 컨텍스트로 래핑
                throw std::runtime_error(
                    "사용자 요청 처리 실패 '" + request + "': " + e.what()
                );
            }
        }
        
    private:
        void parseRequest(const std::string& request) {
            if (request.empty()) {
                throw std::invalid_argument("빈 요청");
            }
            // 파싱 로직...
        }
    };
}
```

---

## 🧪 8. 예외 처리 테스트

### 예외 상황 테스트

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// 예외 처리 테스트 예시
class LogStorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        storage = std::make_unique<LogStorage>(100);
    }
    
    std::unique_ptr<LogStorage> storage;
};

// 예외 발생 테스트
TEST_F(LogStorageTest, ThrowsOnInvalidInput) {
    // 빈 메시지로 예외 발생 확인
    EXPECT_THROW(storage->addLog(""), std::invalid_argument);
    
    // 특정 예외 메시지 확인
    EXPECT_THROW({
        try {
            storage->addLog("");
        }
        catch (const std::invalid_argument& e) {
            EXPECT_STREQ("빈 로그 메시지는 추가할 수 없습니다", e.what());
            throw;
        }
    }, std::invalid_argument);
}

// 예외 안전성 테스트
TEST_F(LogStorageTest, ExceptionSafety) {
    // 원래 상태
    storage->addLog("정상 로그");
    size_t original_size = storage->size();
    
    // 예외 발생 상황
    EXPECT_THROW(storage->addLog(""), std::invalid_argument);
    
    // 원래 상태 유지 확인 (강한 예외 안전성)
    EXPECT_EQ(original_size, storage->size());
    
    // 새로운 정상 로그 추가 가능 확인
    EXPECT_NO_THROW(storage->addLog("새로운 정상 로그"));
}

// 리소스 누수 테스트
TEST_F(LogStorageTest, NoResourceLeakOnException) {
    // Mock 객체를 사용한 리소스 관리 테스트
    auto mock_file = std::make_shared<MockFile>();
    
    EXPECT_CALL(*mock_file, open())
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(*mock_file, write(::testing::_))
        .WillOnce(::testing::Throw(std::runtime_error("쓰기 실패")));
    EXPECT_CALL(*mock_file, close())
        .Times(1);  // 예외 발생해도 반드시 호출되어야 함
    
    LogFileWriter writer(mock_file);
    EXPECT_THROW(writer.writeLog("테스트"), std::runtime_error);
}

// 멀티스레드 예외 처리 테스트
TEST(AsyncLogProcessorTest, HandlesExceptionsInMultipleThreads) {
    AsyncLogProcessor processor;
    std::vector<std::string> logs_with_errors = {
        "정상 로그 1", "", "정상 로그 2", "", "정상 로그 3"
    };
    
    processor.startAsyncProcessing(logs_with_errors);
    
    // 예외 발생 확인
    EXPECT_THROW(processor.waitForCompletion(), std::invalid_argument);
}
```

### 성능 테스트

```cpp
#include <chrono>

// 예외 처리 성능 테스트
class ExceptionPerformanceTest : public ::testing::Test {
protected:
    static constexpr int ITERATIONS = 100000;
    
    void measureTime(const std::string& test_name, 
                    std::function<void()> test_func) {
        auto start = std::chrono::high_resolution_clock::now();
        test_func();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << test_name << ": " << duration.count() << " μs" << std::endl;
    }
};

TEST_F(ExceptionPerformanceTest, CompareErrorHandlingMethods) {
    LogProcessor processor;
    
    // 1. 예외 없는 경우 (정상 경로)
    measureTime("Normal path", [&]() {
        for (int i = 0; i < ITERATIONS; ++i) {
            processor.tryAddLog("정상 로그 " + std::to_string(i));
        }
    });
    
    // 2. 예외 발생하지 않는 검증
    measureTime("Validation without exceptions", [&]() {
        for (int i = 0; i < ITERATIONS; ++i) {
            if (processor.isValidLog("정상 로그")) {
                processor.addLogUnsafe("정상 로그");
            }
        }
    });
    
    // 3. 예외 발생 경우 (느림)
    measureTime("Exception path", [&]() {
        for (int i = 0; i < 1000; ++i) {  // 적은 반복으로 측정
            try {
                processor.addLogWithException("");  // 예외 발생
            }
            catch (const std::exception&) {
                // 예외 처리
            }
        }
    });
}
```

---

## 📚 9. 실전 예제: LogCaster 예외 처리 완성

### 완전한 예외 안전한 LogServer

```cpp
// LogCaster의 완전한 예외 안전 구현
namespace LogCaster {
    
class SafeLogServer {
private:
    int port_;
    int server_fd_;
    std::atomic<bool> running_{false};
    std::unique_ptr<LogStorage> storage_;
    std::vector<std::thread> worker_threads_;
    std::exception_ptr server_exception_;
    mutable std::mutex exception_mutex_;
    
public:
    explicit SafeLogServer(int port) : port_(port), server_fd_(-1) {
        if (port < 1024 || port > 65535) {
            throw ConfigurationException("port", std::to_string(port), 
                                       "포트는 1024-65535 범위여야 합니다");
        }
        
        try {
            storage_ = std::make_unique<LogStorage>(10000);
        }
        catch (const std::bad_alloc& e) {
            throw std::runtime_error("LogStorage 메모리 할당 실패: " + std::string(e.what()));
        }
    }
    
    ~SafeLogServer() noexcept {
        stop();
    }
    
    // 복사 금지, 이동 허용
    SafeLogServer(const SafeLogServer&) = delete;
    SafeLogServer& operator=(const SafeLogServer&) = delete;
    SafeLogServer(SafeLogServer&&) = default;
    SafeLogServer& operator=(SafeLogServer&&) = default;
    
    void start() {
        if (running_.exchange(true)) {
            throw std::logic_error("서버가 이미 실행 중입니다");
        }
        
        try {
            createSocket();
            bindSocket();
            listenSocket();
            startWorkerThreads();
            
            std::cout << "LogServer가 포트 " << port_ << "에서 시작되었습니다." << std::endl;
        }
        catch (...) {
            // 시작 실패 시 정리
            running_ = false;
            cleanup();
            throw;
        }
    }
    
    void stop() noexcept {
        if (!running_.exchange(false)) {
            return;  // 이미 중지됨
        }
        
        try {
            // 워커 스레드 종료
            for (auto& thread : worker_threads_) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
            worker_threads_.clear();
            
            cleanup();
            
            std::cout << "LogServer가 안전하게 종료되었습니다." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "서버 종료 중 오류: " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "서버 종료 중 알 수 없는 오류 발생" << std::endl;
        }
    }
    
    bool isRunning() const noexcept {
        return running_.load();
    }
    
    void waitForShutdown() {
        // 예외 발생 시까지 또는 정상 종료까지 대기
        while (running_.load()) {
            {
                std::lock_guard<std::mutex> lock(exception_mutex_);
                if (server_exception_) {
                    std::rethrow_exception(server_exception_);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
private:
    void createSocket() {
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) {
            throw NetworkException("소켓 생성 실패", errno);
        }
        
        // SO_REUSEADDR 설정
        int opt = 1;
        if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            close(server_fd_);
            server_fd_ = -1;
            throw NetworkException("소켓 옵션 설정 실패", errno);
        }
    }
    
    void bindSocket() {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port_);
        
        if (bind(server_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            throw NetworkException("소켓 바인딩 실패", errno);
        }
    }
    
    void listenSocket() {
        if (listen(server_fd_, SOMAXCONN) < 0) {
            throw NetworkException("소켓 리스닝 실패", errno);
        }
    }
    
    void startWorkerThreads() {
        size_t thread_count = std::thread::hardware_concurrency();
        if (thread_count == 0) thread_count = 4;  // 기본값
        
        worker_threads_.reserve(thread_count);
        
        for (size_t i = 0; i < thread_count; ++i) {
            worker_threads_.emplace_back([this]() {
                workerThreadMain();
            });
        }
    }
    
    void workerThreadMain() noexcept {
        try {
            while (running_.load()) {
                try {
                    acceptAndHandleClient();
                }
                catch (const NetworkException& e) {
                    // 네트워크 예외는 로깅만 하고 계속
                    std::cerr << "네트워크 오류: " << e.what() << std::endl;
                }
                catch (const std::exception& e) {
                    // 기타 예외는 심각한 오류로 간주
                    std::lock_guard<std::mutex> lock(exception_mutex_);
                    if (!server_exception_) {
                        server_exception_ = std::current_exception();
                    }
                    break;
                }
            }
        }
        catch (...) {
            // 최후의 안전장치
            std::lock_guard<std::mutex> lock(exception_mutex_);
            if (!server_exception_) {
                server_exception_ = std::current_exception();
            }
        }
    }
    
    void acceptAndHandleClient() {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd_, 
                             reinterpret_cast<sockaddr*>(&client_addr), 
                             &client_len);
        
        if (client_fd < 0) {
            if (errno == EBADF || errno == EINVAL) {
                // 서버 소켓이 닫힘 - 정상 종료
                return;
            }
            throw NetworkException("클라이언트 연결 수락 실패", errno);
        }
        
        // RAII로 클라이언트 소켓 관리
        SocketRAII client_socket(client_fd);
        handleClient(client_socket.get(), client_addr);
    }
    
    void handleClient(int client_fd, const sockaddr_in& client_addr) {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        std::cout << "클라이언트 연결: " << client_ip << std::endl;
        
        try {
            char buffer[1024];
            while (running_.load()) {
                ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                
                if (bytes <= 0) {
                    if (bytes < 0 && errno != ECONNRESET) {
                        throw NetworkException("데이터 수신 실패", errno);
                    }
                    break;  // 연결 종료
                }
                
                buffer[bytes] = '\0';
                std::string message(buffer, bytes);
                
                // 로그 저장
                storage_->addLog(message, "INFO", client_ip);
                
                // 응답 전송
                const char* ack = "OK\n";
                if (send(client_fd, ack, strlen(ack), 0) < 0) {
                    throw NetworkException("응답 전송 실패", errno);
                }
            }
        }
        catch (const StorageException& e) {
            // 스토리지 오류는 클라이언트에게 알림
            const char* error_msg = "STORAGE_ERROR\n";
            send(client_fd, error_msg, strlen(error_msg), 0);
            throw;  // 재던지기
        }
        
        std::cout << "클라이언트 연결 해제: " << client_ip << std::endl;
    }
    
    void cleanup() noexcept {
        if (server_fd_ >= 0) {
            close(server_fd_);
            server_fd_ = -1;
        }
    }
    
    // RAII 소켓 래퍼
    class SocketRAII {
    private:
        int fd_;
    public:
        explicit SocketRAII(int fd) : fd_(fd) {}
        ~SocketRAII() noexcept {
            if (fd_ >= 0) {
                close(fd_);
            }
        }
        
        SocketRAII(const SocketRAII&) = delete;
        SocketRAII& operator=(const SocketRAII&) = delete;
        
        int get() const noexcept { return fd_; }
    };
};

} // namespace LogCaster

// 사용 예시
int main() {
    try {
        LogCaster::SafeLogServer server(9999);
        
        server.start();
        
        // 종료 시그널 처리
        std::signal(SIGINT, [](int) {
            std::cout << "\n종료 신호 받음..." << std::endl;
            std::exit(0);
        });
        
        server.waitForShutdown();
        
    }
    catch (const LogCaster::LogCasterException& e) {
        std::cerr << "LogCaster 오류: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "오류: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "알 수 없는 오류 발생" << std::endl;
        return 2;
    }
    
    return 0;
}
```

---

## 📋 10. 체크리스트와 정리

### 예외 처리 설계 체크리스트

- [ ] **예외 계층 구조 설계**
  - [ ] 프로젝트 전용 기본 예외 클래스
  - [ ] 기능별 예외 클래스 (Network, Storage 등)
  - [ ] 적절한 표준 예외 상속

- [ ] **예외 안전성 보장**
  - [ ] 기본 보장: 리소스 누수 없음
  - [ ] 강한 보장: 실패 시 원래 상태 유지
  - [ ] noexcept 함수 명시

- [ ] **RAII 패턴 적용**
  - [ ] 스마트 포인터 사용
  - [ ] 커스텀 RAII 클래스
  - [ ] 자동 리소스 정리

- [ ] **성능 고려사항**
  - [ ] 핫패스에서 예외 최소화
  - [ ] std::optional 등 대안 고려
  - [ ] 예외 발생 비용 측정

- [ ] **테스트 작성**
  - [ ] 예외 발생 테스트
  - [ ] 예외 안전성 테스트
  - [ ] 리소스 누수 테스트

### 핵심 포인트 정리

1. **예외는 예외적인 상황에만 사용**
2. **RAII로 리소스 자동 관리**
3. **적절한 예외 타입 선택**
4. **예외 안전성 3단계 이해**
5. **성능을 고려한 설계**

---

*🎯 예외 처리는 견고한 C++ 프로그램의 핵심입니다. LogCaster 프로젝트에서 이러한 기법들을 실제로 적용해보면서 예외 처리 전문가가 되어보세요!*