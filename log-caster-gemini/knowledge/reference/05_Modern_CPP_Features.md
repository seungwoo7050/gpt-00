# 22. Modern C++ Features (C++11/14/17/20/23) 완전 가이드 🚀

*LogCaster 프로젝트를 위한 사전지식 문서*

---

## 📋 문서 정보
- **난이도**: 🟡 중급 → 🔴 고급
- **예상 학습 시간**: 12-15시간
- **전제 조건**: 
  - C++ 기본 문법 숙지
  - 템플릿 기본 이해
  - 포인터와 메모리 관리
- **실습 환경**: C++11 이상 (C++17/20 권장), GCC 9+/Clang 10+

## 🎯 이 문서에서 배울 내용

**"최신 C++로 더 안전하고 표현력 있는 코드 작성하기! 🌟"**

Modern C++는 언어를 **더 안전하고, 더 표현력 있고, 더 효율적으로** 만드는 혁신적인 기능들을 제공합니다. 마치 **스마트폰이 기존 휴대폰을 대체**한 것처럼, Modern C++는 전통적인 C++ 방식을 크게 개선합니다.

---

## 🎯 1. C++11: 혁명의 시작

### Auto 키워드와 타입 추론

```cpp
#include <iostream>
#include <vector>
#include <map>
#include <string>

// LogCaster에서 auto 활용 예시
class LogEntry {
public:
    std::string message;
    std::string level;
    std::chrono::system_clock::time_point timestamp;
    
    LogEntry(const std::string& msg, const std::string& lvl) 
        : message(msg), level(lvl), timestamp(std::chrono::system_clock::now()) {}
};

class ModernLogServer {
private:
    std::vector<LogEntry> logs_;
    std::map<std::string, int> level_counts_;
    
public:
    void demonstrateAuto() {
        // BEFORE C++11: 긴 타입 선언
        std::vector<LogEntry>::iterator old_it = logs_.begin();
        std::map<std::string, int>::const_iterator old_map_it = level_counts_.cbegin();
        
        // AFTER C++11: auto로 간결하게
        auto modern_it = logs_.begin();           // 컴파일러가 타입 추론
        auto modern_map_it = level_counts_.cbegin();
        
        // 복잡한 람다 타입도 auto로
        auto logger = [this](const std::string& msg) {
            logs_.emplace_back(msg, "INFO");
            level_counts_["INFO"]++;
        };
        
        // 함수 반환 타입도 auto
        auto getCurrentTime = []() -> auto {
            return std::chrono::system_clock::now();
        };
        
        // Range-based for와 함께 사용
        for (const auto& log : logs_) {
            std::cout << log.message << " [" << log.level << "]" << std::endl;
        }
        
        // STL 알고리즘과 함께
        auto error_count = std::count_if(logs_.begin(), logs_.end(),
            [](const auto& log) { return log.level == "ERROR"; }
        );
        
        std::cout << "Error logs: " << error_count << std::endl;
    }
    
    // auto 반환 타입 (C++14부터)
    auto getLogsByLevel(const std::string& level) {
        std::vector<LogEntry> filtered;
        std::copy_if(logs_.begin(), logs_.end(), std::back_inserter(filtered),
            [&level](const auto& log) { return log.level == level; }
        );
        return filtered;  // 반환 타입을 컴파일러가 추론
    }
};
```

### Range-based For 루프

```cpp
// LogCaster에서 Range-based for 활용
class LogProcessor {
public:
    void processLogs(const std::vector<std::string>& messages) {
        // 전통적인 방식
        for (size_t i = 0; i < messages.size(); ++i) {
            processMessage(messages[i]);
        }
        
        // iterator 방식
        for (auto it = messages.begin(); it != messages.end(); ++it) {
            processMessage(*it);
        }
        
        // Modern C++11: Range-based for
        for (const auto& message : messages) {
            processMessage(message);  // 훨씬 간결하고 안전
        }
    }
    
    void demonstrateAdvancedRangeFor() {
        std::map<std::string, std::vector<LogEntry>> logs_by_level;
        
        // 중첩 컨테이너도 쉽게 순회
        for (const auto& [level, entries] : logs_by_level) {  // C++17 structured binding
            std::cout << "Level: " << level << std::endl;
            for (const auto& entry : entries) {
                std::cout << "  - " << entry.message << std::endl;
            }
        }
        
        // 수정하면서 순회 (참조 사용)
        std::vector<LogEntry> logs;
        for (auto& log : logs) {
            if (log.level == "DEBUG") {
                log.level = "TRACE";  // 직접 수정
            }
        }
        
        // 인덱스와 함께 순회 (C++20 enumerate 시뮬레이션)
        for (const auto& [index, log] : enumerate(logs)) {
            std::cout << index << ": " << log.message << std::endl;
        }
    }
    
private:
    void processMessage(const std::string& message) {
        std::cout << "Processing: " << message << std::endl;
    }
    
    // enumerate 헬퍼 함수 (C++20 스타일)
    template<typename Container>
    auto enumerate(Container&& container) {
        struct Iterator {
            size_t index;
            typename Container::const_iterator it;
            
            auto operator*() const { return std::make_pair(index, *it); }
            Iterator& operator++() { ++index; ++it; return *this; }
            bool operator!=(const Iterator& other) const { return it != other.it; }
        };
        
        struct Enumerable {
            Container& container;
            auto begin() const { return Iterator{0, container.begin()}; }
            auto end() const { return Iterator{0, container.end()}; }
        };
        
        return Enumerable{container};
    }
};
```

### Lambda 함수

```cpp
#include <algorithm>
#include <functional>

// LogCaster에서 람다 활용
class LambdaLogServer {
private:
    std::vector<LogEntry> logs_;
    
public:
    void demonstrateBasicLambdas() {
        // 1. 기본 람다
        auto simple_logger = [](const std::string& msg) {
            std::cout << "Log: " << msg << std::endl;
        };
        simple_logger("Hello Lambda!");
        
        // 2. 캡처와 함께
        std::string prefix = "[SERVER] ";
        auto prefixed_logger = [prefix](const std::string& msg) {
            std::cout << prefix << msg << std::endl;
        };
        
        // 3. 레퍼런스 캡처
        int log_count = 0;
        auto counting_logger = [&log_count](const std::string& msg) {
            ++log_count;
            std::cout << "#" << log_count << ": " << msg << std::endl;
        };
        
        // 4. 모든 것을 값으로 캡처
        auto copy_all_logger = [=](const std::string& msg) mutable {
            ++log_count;  // mutable이 있어야 수정 가능
            std::cout << prefix << "#" << log_count << ": " << msg << std::endl;
        };
        
        // 5. 모든 것을 레퍼런스로 캡처
        auto ref_all_logger = [&](const std::string& msg) {
            ++log_count;
            std::cout << prefix << "#" << log_count << ": " << msg << std::endl;
        };
    }
    
    void demonstrateAdvancedLambdas() {
        // STL 알고리즘과 람다
        std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        
        // 짝수 찾기
        auto is_even = [](int n) { return n % 2 == 0; };
        auto even_count = std::count_if(numbers.begin(), numbers.end(), is_even);
        
        // 조건부 로깅
        auto conditional_logger = [this](const std::string& msg, const std::string& level) {
            if (level == "ERROR" || level == "FATAL") {
                // 즉시 출력
                std::cout << "[URGENT] " << msg << std::endl;
            } else {
                // 버퍼에 저장
                logs_.emplace_back(msg, level);
            }
        };
        
        // 람다를 함수 객체로 저장
        std::function<void(const std::string&)> stored_logger = 
            [this](const std::string& msg) {
                logs_.emplace_back(msg, "INFO");
            };
        
        // 고차 함수 (함수를 반환하는 함수)
        auto create_level_logger = [this](const std::string& level) {
            return [this, level](const std::string& msg) {
                logs_.emplace_back(msg, level);
                std::cout << "[" << level << "] " << msg << std::endl;
            };
        };
        
        auto error_logger = create_level_logger("ERROR");
        auto info_logger = create_level_logger("INFO");
        
        error_logger("Something went wrong!");
        info_logger("System started successfully");
    }
    
    // 람다를 매개변수로 받는 함수
    template<typename FilterFunc>
    void processFilteredLogs(FilterFunc filter) {
        for (const auto& log : logs_) {
            if (filter(log)) {
                std::cout << "Filtered: " << log.message << std::endl;
            }
        }
    }
    
    void usageExample() {
        // 다양한 필터 람다 사용
        processFilteredLogs([](const LogEntry& log) {
            return log.level == "ERROR";
        });
        
        processFilteredLogs([](const LogEntry& log) {
            return log.message.find("network") != std::string::npos;
        });
        
        // 람다로 복잡한 조건 표현
        auto recent_errors = [](const LogEntry& log) {
            auto now = std::chrono::system_clock::now();
            auto five_minutes_ago = now - std::chrono::minutes(5);
            return log.level == "ERROR" && log.timestamp > five_minutes_ago;
        };
        
        processFilteredLogs(recent_errors);
    }
};
```

### Smart Pointers

```cpp
#include <memory>

// LogCaster에서 스마트 포인터 활용
class NetworkConnection {
public:
    NetworkConnection(int socket_fd) : socket_fd_(socket_fd) {
        std::cout << "네트워크 연결 생성 (fd: " << socket_fd_ << ")" << std::endl;
    }
    
    ~NetworkConnection() {
        std::cout << "네트워크 연결 해제 (fd: " << socket_fd_ << ")" << std::endl;
        // 실제로는 close(socket_fd_) 호출
    }
    
    void sendData(const std::string& data) {
        std::cout << "데이터 전송: " << data << std::endl;
    }
    
    int getSocketFd() const { return socket_fd_; }

private:
    int socket_fd_;
};

class SmartPointerLogServer {
private:
    // unique_ptr: 독점적 소유권
    std::unique_ptr<NetworkConnection> primary_connection_;
    
    // shared_ptr: 공유 소유권
    std::shared_ptr<std::ofstream> log_file_;
    
    // weak_ptr: 순환 참조 방지
    std::weak_ptr<SmartPointerLogServer> self_;
    
public:
    void demonstrateUniquePtr() {
        // unique_ptr 생성
        auto connection1 = std::make_unique<NetworkConnection>(100);
        
        // 소유권 이전 (move semantics)
        primary_connection_ = std::move(connection1);
        // connection1은 이제 nullptr
        
        if (primary_connection_) {
            primary_connection_->sendData("Hello from unique_ptr!");
        }
        
        // 자동으로 해제됨 (RAII)
    }
    
    void demonstrateSharedPtr() {
        // shared_ptr 생성
        auto shared_file = std::make_shared<std::ofstream>("logcaster.log");
        log_file_ = shared_file;  // 참조 카운트 증가 (2)
        
        {
            auto another_ref = log_file_;  // 참조 카운트 증가 (3)
            std::cout << "참조 카운트: " << another_ref.use_count() << std::endl;
            
            if (another_ref && another_ref->is_open()) {
                *another_ref << "로그 메시지" << std::endl;
            }
        }  // another_ref 소멸, 참조 카운트 감소 (2)
        
        std::cout << "참조 카운트: " << log_file_.use_count() << std::endl;
        
        // 커스텀 deleter
        auto connection_with_deleter = std::shared_ptr<NetworkConnection>(
            new NetworkConnection(200),
            [](NetworkConnection* conn) {
                std::cout << "커스텀 deleter 호출" << std::endl;
                delete conn;
            }
        );
    }
    
    void demonstrateWeakPtr() {
        auto shared_server = std::make_shared<SmartPointerLogServer>();
        
        // weak_ptr은 참조 카운트를 증가시키지 않음
        std::weak_ptr<SmartPointerLogServer> weak_server = shared_server;
        
        std::cout << "shared_ptr 참조 카운트: " << shared_server.use_count() << std::endl;
        
        // weak_ptr 사용하기
        if (auto locked_server = weak_server.lock()) {  // shared_ptr로 변환
            std::cout << "서버에 안전하게 접근" << std::endl;
            // locked_server 사용
        } else {
            std::cout << "서버가 이미 해제됨" << std::endl;
        }
        
        shared_server.reset();  // shared_ptr 해제
        
        if (weak_server.expired()) {
            std::cout << "weak_ptr이 가리키는 객체가 해제됨" << std::endl;
        }
    }
    
    // 팩토리 패턴과 스마트 포인터
    static std::unique_ptr<SmartPointerLogServer> createServer() {
        return std::make_unique<SmartPointerLogServer>();
    }
    
    // 공유 리소스 관리
    class SharedResourceManager {
    private:
        std::shared_ptr<std::vector<LogEntry>> shared_logs_;
        
    public:
        SharedResourceManager() : shared_logs_(std::make_shared<std::vector<LogEntry>>()) {}
        
        std::shared_ptr<std::vector<LogEntry>> getSharedLogs() {
            return shared_logs_;  // 공유 소유권 전달
        }
        
        void addLog(const LogEntry& entry) {
            shared_logs_->push_back(entry);
        }
    };
    
    // 약한 참조를 사용한 관찰자 패턴
    class LogObserver {
    private:
        std::weak_ptr<SmartPointerLogServer> server_;
        
    public:
        LogObserver(std::shared_ptr<SmartPointerLogServer> server) 
            : server_(server) {}
        
        void checkServer() {
            if (auto server = server_.lock()) {
                std::cout << "서버가 여전히 살아있음" << std::endl;
            } else {
                std::cout << "서버가 해제됨" << std::endl;
            }
        }
    };
};
```

### Move Semantics와 Perfect Forwarding

```cpp
#include <utility>
#include <string>

// LogCaster에서 Move Semantics 활용
class MoveLogEntry {
private:
    std::string message_;
    std::string level_;
    std::vector<char> large_data_;
    
public:
    // 생성자
    MoveLogEntry(std::string message, std::string level) 
        : message_(std::move(message)), level_(std::move(level)) {
        // 매개변수가 이미 복사본이므로 move로 효율적으로 이동
    }
    
    // 복사 생성자
    MoveLogEntry(const MoveLogEntry& other) 
        : message_(other.message_), level_(other.level_), large_data_(other.large_data_) {
        std::cout << "복사 생성자 호출" << std::endl;
    }
    
    // 이동 생성자
    MoveLogEntry(MoveLogEntry&& other) noexcept
        : message_(std::move(other.message_)), 
          level_(std::move(other.level_)),
          large_data_(std::move(other.large_data_)) {
        std::cout << "이동 생성자 호출" << std::endl;
    }
    
    // 복사 대입 연산자
    MoveLogEntry& operator=(const MoveLogEntry& other) {
        if (this != &other) {
            message_ = other.message_;
            level_ = other.level_;
            large_data_ = other.large_data_;
            std::cout << "복사 대입 연산자 호출" << std::endl;
        }
        return *this;
    }
    
    // 이동 대입 연산자
    MoveLogEntry& operator=(MoveLogEntry&& other) noexcept {
        if (this != &other) {
            message_ = std::move(other.message_);
            level_ = std::move(other.level_);
            large_data_ = std::move(other.large_data_);
            std::cout << "이동 대입 연산자 호출" << std::endl;
        }
        return *this;
    }
    
    // 게터들
    const std::string& getMessage() const& { return message_; }
    std::string getMessage() && { return std::move(message_); }  // rvalue 참조용
    
    const std::string& getLevel() const { return level_; }
    
    void setLargeData(std::vector<char> data) {
        large_data_ = std::move(data);  // 효율적인 이동
    }
};

class PerfectForwardingLogServer {
public:
    // Perfect Forwarding을 사용한 emplace 스타일 함수
    template<typename... Args>
    void emplaceLog(Args&&... args) {
        logs_.emplace_back(std::forward<Args>(args)...);
        std::cout << "로그 추가 완료" << std::endl;
    }
    
    // Universal Reference와 Perfect Forwarding
    template<typename T>
    void addLogEntry(T&& entry) {
        // T가 lvalue면 복사, rvalue면 이동
        logs_.push_back(std::forward<T>(entry));
    }
    
    void demonstrateMoveSemantics() {
        std::vector<char> large_data(1000000, 'x');  // 1MB 데이터
        
        // 전통적인 방식 (복사 발생)
        MoveLogEntry entry1("메시지1", "INFO");
        entry1.setLargeData(large_data);  // 복사
        
        // Move semantics 사용 (복사 없음)
        MoveLogEntry entry2("메시지2", "ERROR");
        entry2.setLargeData(std::move(large_data));  // 이동, large_data는 이제 비어있음
        
        // emplace로 직접 생성 (가장 효율적)
        emplaceLog("메시지3", "WARN");
        
        // 임시 객체를 효율적으로 추가
        addLogEntry(MoveLogEntry("임시 메시지", "DEBUG"));  // 이동 발생
        
        // lvalue를 추가 (복사 발생)
        MoveLogEntry permanent_entry("영구 메시지", "TRACE");
        addLogEntry(permanent_entry);  // 복사
        
        // 명시적 이동
        addLogEntry(std::move(permanent_entry));  // 이동, permanent_entry는 이제 비어있음
    }
    
    // Factory 함수와 move semantics
    static MoveLogEntry createErrorLog(std::string message) {
        return MoveLogEntry(std::move(message), "ERROR");  // NRVO 또는 move
    }
    
    // 조건부 move
    template<typename Container>
    void addLogsFrom(Container&& container) {
        for (auto&& entry : std::forward<Container>(container)) {
            logs_.push_back(std::forward<decltype(entry)>(entry));
        }
    }

private:
    std::vector<MoveLogEntry> logs_;
};
```

---

## 🌟 2. C++14: 편의성 향상

### Generic Lambdas

```cpp
// C++14 Generic Lambda 예시
class GenericLambdaExample {
public:
    void demonstrateGenericLambdas() {
        // Generic lambda (C++14)
        auto generic_printer = [](const auto& item) {
            std::cout << "Item: " << item << std::endl;
        };
        
        generic_printer(42);
        generic_printer("Hello");
        generic_printer(3.14);
        
        // 복잡한 generic lambda
        auto generic_logger = [](const auto& level, const auto& message) {
            std::cout << "[" << level << "] " << message << std::endl;
        };
        
        generic_logger("INFO", "서버 시작");
        generic_logger(1, "에러 코드");
        
        // STL과 함께 사용
        std::vector<int> numbers = {1, 2, 3, 4, 5};
        std::vector<std::string> words = {"hello", "world", "cpp"};
        
        auto print_container = [](const auto& container) {
            for (const auto& item : container) {
                std::cout << item << " ";
            }
            std::cout << std::endl;
        };
        
        print_container(numbers);
        print_container(words);
        
        // 람다에서 템플릿 매개변수 추론
        auto transform_and_print = [](auto container, auto transformer) {
            for (auto& item : container) {
                item = transformer(item);
                std::cout << item << " ";
            }
            std::cout << std::endl;
        };
        
        std::vector<int> nums = {1, 2, 3};
        transform_and_print(nums, [](auto x) { return x * 2; });
    }
};
```

### Binary Literals와 Digit Separators

```cpp
// C++14 Binary Literals와 숫자 구분자
class ModernLiterals {
public:
    void demonstrateLiterals() {
        // Binary literals (C++14)
        int binary_value = 0b1010'1100;  // 172
        int hex_value = 0xFF'FF'FF'FF;   // 4294967295
        long long large_number = 1'000'000'000LL;
        
        std::cout << "Binary: " << binary_value << std::endl;
        std::cout << "Hex: " << hex_value << std::endl;
        std::cout << "Large: " << large_number << std::endl;
        
        // 로그 레벨을 비트 플래그로 표현
        enum class LogLevel : int {
            NONE  = 0b0000'0000,
            ERROR = 0b0000'0001,
            WARN  = 0b0000'0010,
            INFO  = 0b0000'0100,
            DEBUG = 0b0000'1000,
            TRACE = 0b0001'0000,
            ALL   = 0b0001'1111
        };
        
        LogLevel current_level = static_cast<LogLevel>(
            static_cast<int>(LogLevel::ERROR) | 
            static_cast<int>(LogLevel::WARN)
        );
        
        // 비트 연산으로 로그 레벨 확인
        auto hasLevel = [](LogLevel current, LogLevel check) {
            return (static_cast<int>(current) & static_cast<int>(check)) != 0;
        };
        
        std::cout << "Has ERROR: " << hasLevel(current_level, LogLevel::ERROR) << std::endl;
        std::cout << "Has INFO: " << hasLevel(current_level, LogLevel::INFO) << std::endl;
    }
};
```

### Variable Templates

```cpp
// C++14 Variable Templates
template<typename T>
constexpr bool is_log_entry_v = std::is_same_v<T, LogEntry>;

template<typename T>
constexpr size_t type_size_v = sizeof(T);

// 로그 처리 성능 상수들
template<typename T>
constexpr size_t buffer_size_v = 1024;

template<>
constexpr size_t buffer_size_v<LogEntry> = 4096;  // 특수화

template<>
constexpr size_t buffer_size_v<std::string> = 512;

class VariableTemplateExample {
public:
    template<typename T>
    void processData(const T& data) {
        constexpr size_t buffer_size = buffer_size_v<T>;
        std::array<char, buffer_size> buffer;
        
        std::cout << "Processing " << typeid(T).name() 
                  << " with buffer size: " << buffer_size << std::endl;
    }
};
```

---

## ⭐ 3. C++17: 표현력 혁신

### Structured Bindings

```cpp
#include <tuple>
#include <map>
#include <optional>

// C++17 Structured Bindings
class StructuredBindingExample {
public:
    // 튜플을 반환하는 함수
    std::tuple<std::string, int, bool> getLogInfo() {
        return {"ERROR", 404, true};
    }
    
    // pair를 반환하는 함수
    std::pair<int, std::string> parseLogLine(const std::string& line) {
        // 간단한 파싱 시뮬레이션
        return {line.length(), line.substr(0, 10)};
    }
    
    void demonstrateStructuredBindings() {
        // 튜플 언패킹
        auto [level, code, is_critical] = getLogInfo();
        std::cout << "Level: " << level << ", Code: " << code 
                  << ", Critical: " << is_critical << std::endl;
        
        // pair 언패킹
        auto [length, preview] = parseLogLine("This is a log message");
        std::cout << "Length: " << length << ", Preview: " << preview << std::endl;
        
        // 맵 순회가 훨씬 깔끔해짐
        std::map<std::string, int> log_counts = {
            {"ERROR", 10}, {"WARN", 25}, {"INFO", 100}
        };
        
        for (const auto& [level, count] : log_counts) {
            std::cout << level << ": " << count << std::endl;
        }
        
        // 배열도 구조 분해 가능
        int coordinates[3] = {1, 2, 3};
        auto [x, y, z] = coordinates;
        std::cout << "X: " << x << ", Y: " << y << ", Z: " << z << std::endl;
        
        // 사용자 정의 타입도 가능 (get<> 특수화 필요)
        LogEntry entry("테스트", "DEBUG");
        auto [message, level_str] = extractMessageAndLevel(entry);
        std::cout << "Message: " << message << ", Level: " << level_str << std::endl;
    }
    
private:
    std::pair<std::string, std::string> extractMessageAndLevel(const LogEntry& entry) {
        return {entry.message, entry.level};
    }
};
```

### if constexpr

```cpp
#include <type_traits>

// C++17 if constexpr
template<typename T>
class ConditionalLogProcessor {
public:
    void processValue(const T& value) {
        if constexpr (std::is_arithmetic_v<T>) {
            // 숫자 타입인 경우
            std::cout << "숫자 값: " << value << std::endl;
            
            if constexpr (std::is_integral_v<T>) {
                std::cout << "정수입니다" << std::endl;
            } else {
                std::cout << "실수입니다" << std::endl;
            }
        } else if constexpr (std::is_same_v<T, std::string>) {
            // 문자열인 경우
            std::cout << "문자열 값: \"" << value << "\"" << std::endl;
        } else if constexpr (std::is_pointer_v<T>) {
            // 포인터인 경우
            std::cout << "포인터 주소: " << value << std::endl;
            if (value != nullptr) {
                std::cout << "포인터 값: " << *value << std::endl;
            }
        } else {
            // 기타 타입
            std::cout << "알 수 없는 타입" << std::endl;
        }
    }
    
    // 컨테이너 처리
    template<typename Container>
    void processContainer(const Container& container) {
        if constexpr (requires { container.size(); }) {
            std::cout << "컨테이너 크기: " << container.size() << std::endl;
        }
        
        if constexpr (requires { container.begin(); container.end(); }) {
            std::cout << "반복 가능한 컨테이너입니다" << std::endl;
            for (const auto& item : container) {
                processValue(item);
            }
        } else {
            std::cout << "반복 불가능한 타입입니다" << std::endl;
        }
    }
};
```

### std::optional

```cpp
#include <optional>
#include <iostream>

// std::optional 활용
class OptionalLogRetriever {
private:
    std::vector<LogEntry> logs_;
    
public:
    // 예외 대신 optional 사용
    std::optional<LogEntry> findLogById(int id) const {
        if (id < 0 || id >= static_cast<int>(logs_.size())) {
            return std::nullopt;  // 값이 없음을 나타냄
        }
        return logs_[id];
    }
    
    // 조건에 맞는 첫 번째 로그 찾기
    std::optional<LogEntry> findFirstByLevel(const std::string& level) const {
        auto it = std::find_if(logs_.begin(), logs_.end(),
            [&level](const LogEntry& entry) {
                return entry.level == level;
            });
        
        if (it != logs_.end()) {
            return *it;
        }
        return std::nullopt;
    }
    
    void demonstrateOptional() {
        // optional 사용법
        auto maybe_log = findLogById(0);
        
        // 값이 있는지 확인
        if (maybe_log.has_value()) {
            std::cout << "로그 발견: " << maybe_log->message << std::endl;
        }
        
        // 값 추출 (있다면)
        if (maybe_log) {  // has_value()와 동일
            LogEntry log = maybe_log.value();  // 또는 *maybe_log
            std::cout << "레벨: " << log.level << std::endl;
        }
        
        // 기본값 제공
        LogEntry default_log("기본 메시지", "INFO");
        LogEntry actual_log = findLogById(-1).value_or(default_log);
        
        // 체이닝
        auto error_message = findFirstByLevel("ERROR")
            .and_then([](const LogEntry& log) -> std::optional<std::string> {
                if (log.message.length() > 10) {
                    return log.message;
                }
                return std::nullopt;
            })
            .value_or("에러 로그 없음 또는 너무 짧음");
        
        std::cout << "에러 메시지: " << error_message << std::endl;
        
        // 조건부 처리
        findFirstByLevel("FATAL").and_then([](const LogEntry& log) -> std::optional<int> {
            std::cout << "치명적 오류 발견: " << log.message << std::endl;
            return log.message.length();
        }).or_else([]() -> std::optional<int> {
            std::cout << "치명적 오류 없음" << std::endl;
            return 0;
        });
    }
    
    // optional을 반환하는 설정 함수
    std::optional<int> parsePort(const std::string& port_str) const {
        try {
            int port = std::stoi(port_str);
            if (port > 0 && port < 65536) {
                return port;
            }
        } catch (const std::exception&) {
            // 파싱 실패
        }
        return std::nullopt;
    }
};
```

### std::variant

```cpp
#include <variant>
#include <string>

// std::variant 활용
using LogValue = std::variant<int, double, std::string, bool>;

class VariantLogEntry {
private:
    std::string key_;
    LogValue value_;
    
public:
    VariantLogEntry(std::string key, LogValue value) 
        : key_(std::move(key)), value_(std::move(value)) {}
    
    void printValue() const {
        std::cout << key_ << ": ";
        
        // visitor 패턴 사용
        std::visit([](const auto& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, int>) {
                std::cout << "정수 " << val;
            } else if constexpr (std::is_same_v<T, double>) {
                std::cout << "실수 " << val;
            } else if constexpr (std::is_same_v<T, std::string>) {
                std::cout << "문자열 \"" << val << "\"";
            } else if constexpr (std::is_same_v<T, bool>) {
                std::cout << "불린 " << (val ? "true" : "false");
            }
        }, value_);
        
        std::cout << std::endl;
    }
    
    // 타입 검사
    bool isString() const {
        return std::holds_alternative<std::string>(value_);
    }
    
    // 안전한 값 접근
    std::optional<std::string> getAsString() const {
        if (auto* str = std::get_if<std::string>(&value_)) {
            return *str;
        }
        return std::nullopt;
    }
    
    // 값 변환
    std::string toString() const {
        return std::visit([](const auto& val) -> std::string {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, std::string>) {
                return val;
            } else if constexpr (std::is_same_v<T, bool>) {
                return val ? "true" : "false";
            } else {
                return std::to_string(val);
            }
        }, value_);
    }
};

class FlexibleLogProcessor {
public:
    void demonstrateVariant() {
        std::vector<VariantLogEntry> entries = {
            {"port", 8080},
            {"name", std::string("LogCaster")},
            {"version", 1.5},
            {"enabled", true}
        };
        
        for (const auto& entry : entries) {
            entry.printValue();
            std::cout << "  문자열 표현: " << entry.toString() << std::endl;
        }
        
        // variant를 사용한 에러 처리
        using ParseResult = std::variant<int, std::string>;  // 성공 시 int, 실패 시 error message
        
        auto parseInteger = [](const std::string& str) -> ParseResult {
            try {
                return std::stoi(str);
            } catch (const std::exception& e) {
                return std::string("파싱 실패: ") + e.what();
            }
        };
        
        auto result = parseInteger("123");
        
        std::visit([](const auto& res) {
            using T = std::decay_t<decltype(res)>;
            if constexpr (std::is_same_v<T, int>) {
                std::cout << "파싱 성공: " << res << std::endl;
            } else {
                std::cout << "파싱 실패: " << res << std::endl;
            }
        }, result);
    }
};
```

### std::any

```cpp
#include <any>
#include <typeinfo>

// std::any 활용
class AnyLogProcessor {
private:
    std::map<std::string, std::any> properties_;
    
public:
    // 임의 타입 저장
    template<typename T>
    void setProperty(const std::string& name, T&& value) {
        properties_[name] = std::forward<T>(value);
    }
    
    // 타입 안전한 값 추출
    template<typename T>
    std::optional<T> getProperty(const std::string& name) const {
        auto it = properties_.find(name);
        if (it != properties_.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                // 타입이 맞지 않음
            }
        }
        return std::nullopt;
    }
    
    void printProperty(const std::string& name) const {
        auto it = properties_.find(name);
        if (it == properties_.end()) {
            std::cout << name << ": 속성 없음" << std::endl;
            return;
        }
        
        const std::any& value = it->second;
        const std::type_info& type = value.type();
        
        std::cout << name << " (" << type.name() << "): ";
        
        // 알려진 타입들에 대해 출력
        if (type == typeid(int)) {
            std::cout << std::any_cast<int>(value);
        } else if (type == typeid(double)) {
            std::cout << std::any_cast<double>(value);
        } else if (type == typeid(std::string)) {
            std::cout << "\"" << std::any_cast<std::string>(value) << "\"";
        } else if (type == typeid(bool)) {
            std::cout << (std::any_cast<bool>(value) ? "true" : "false");
        } else {
            std::cout << "알 수 없는 타입";
        }
        
        std::cout << std::endl;
    }
    
    void demonstrateAny() {
        // 다양한 타입 저장
        setProperty("max_connections", 100);
        setProperty("server_name", std::string("LogCaster"));
        setProperty("timeout", 30.5);
        setProperty("debug_mode", true);
        
        // 속성 출력
        printProperty("max_connections");
        printProperty("server_name");
        printProperty("timeout");
        printProperty("debug_mode");
        printProperty("nonexistent");
        
        // 타입 안전한 접근
        if (auto max_conn = getProperty<int>("max_connections")) {
            std::cout << "최대 연결 수: " << *max_conn << std::endl;
        }
        
        // 잘못된 타입으로 접근
        if (auto wrong_type = getProperty<std::string>("max_connections")) {
            std::cout << "이것은 실행되지 않음" << std::endl;
        } else {
            std::cout << "타입이 맞지 않음" << std::endl;
        }
    }
};
```

---

## 🎯 4. C++20: 차세대 기능들

### Concepts

```cpp
#include <concepts>
#include <type_traits>

// C++20 Concepts
template<typename T>
concept Printable = requires(T t) {
    std::cout << t;
};

template<typename T>
concept LogEntryLike = requires(T t) {
    { t.getMessage() } -> std::convertible_to<std::string>;
    { t.getLevel() } -> std::convertible_to<std::string>;
};

template<typename T>
concept Container = requires(T t) {
    typename T::value_type;
    { t.begin() } -> std::input_iterator;
    { t.end() } -> std::input_iterator;
    { t.size() } -> std::convertible_to<size_t>;
};

// Concepts를 사용한 함수 정의
template<Printable T>
void print(const T& value) {
    std::cout << "Printing: " << value << std::endl;
}

template<LogEntryLike T>
void processLogEntry(const T& entry) {
    std::cout << "[" << entry.getLevel() << "] " << entry.getMessage() << std::endl;
}

template<Container C>
void printContainer(const C& container) {
    std::cout << "Container size: " << container.size() << std::endl;
    for (const auto& item : container) {
        if constexpr (Printable<typename C::value_type>) {
            std::cout << "  " << item << std::endl;
        }
    }
}

// 복합 concepts
template<typename T>
concept StringLike = std::convertible_to<T, std::string> || 
                    std::same_as<T, const char*>;

template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// requires clause 사용
template<typename T>
void processNumeric(T value) requires Numeric<T> {
    std::cout << "숫자 처리: " << value << std::endl;
    
    if constexpr (std::integral<T>) {
        std::cout << "정수입니다" << std::endl;
    } else {
        std::cout << "실수입니다" << std::endl;
    }
}

class ConceptsExample {
public:
    void demonstrateConcepts() {
        // Printable concept 사용
        print(42);
        print("Hello");
        print(3.14);
        
        // Container concept 사용
        std::vector<int> numbers = {1, 2, 3, 4, 5};
        std::vector<std::string> words = {"hello", "world"};
        
        printContainer(numbers);
        printContainer(words);
        
        // Numeric concept 사용
        processNumeric(42);
        processNumeric(3.14);
        // processNumeric("hello");  // 컴파일 에러!
    }
};
```

### Ranges

```cpp
#include <ranges>
#include <algorithm>
#include <numeric>

// C++20 Ranges
class RangesExample {
public:
    void demonstrateRanges() {
        std::vector<LogEntry> logs = {
            LogEntry("서버 시작", "INFO"),
            LogEntry("연결 실패", "ERROR"),
            LogEntry("사용자 로그인", "INFO"),
            LogEntry("메모리 부족", "ERROR"),
            LogEntry("요청 처리", "DEBUG")
        };
        
        // 전통적인 방식
        std::vector<LogEntry> error_logs_old;
        std::copy_if(logs.begin(), logs.end(), std::back_inserter(error_logs_old),
            [](const LogEntry& log) { return log.level == "ERROR"; });
        
        // C++20 Ranges 방식
        auto error_logs = logs 
            | std::views::filter([](const LogEntry& log) { 
                return log.level == "ERROR"; 
            });
        
        std::cout << "에러 로그들:" << std::endl;
        for (const auto& log : error_logs) {
            std::cout << "  " << log.message << std::endl;
        }
        
        // 복잡한 파이프라인
        auto processed_messages = logs
            | std::views::filter([](const LogEntry& log) { 
                return log.level != "DEBUG"; 
            })
            | std::views::transform([](const LogEntry& log) { 
                return "[" + log.level + "] " + log.message; 
            })
            | std::views::take(3);  // 처음 3개만
        
        std::cout << "\n처리된 메시지들:" << std::endl;
        for (const auto& msg : processed_messages) {
            std::cout << "  " << msg << std::endl;
        }
        
        // 숫자 범위 처리
        auto even_squares = std::views::iota(1, 11)  // 1부터 10까지
            | std::views::filter([](int n) { return n % 2 == 0; })  // 짝수만
            | std::views::transform([](int n) { return n * n; });    // 제곱
        
        std::cout << "\n짝수들의 제곱:" << std::endl;
        for (int square : even_squares) {
            std::cout << "  " << square << std::endl;
        }
        
        // 문자열 처리
        std::string text = "Hello,World,C++20,Ranges";
        auto words = text 
            | std::views::split(',')
            | std::views::transform([](auto&& range) {
                return std::string(range.begin(), range.end());
            });
        
        std::cout << "\n분할된 단어들:" << std::endl;
        for (const auto& word : words) {
            std::cout << "  '" << word << "'" << std::endl;
        }
    }
    
    // 커스텀 range adaptor
    template<typename Range, typename Pred>
    auto debug_filter(Range&& range, Pred pred) {
        return std::forward<Range>(range)
            | std::views::filter([pred](const auto& item) {
                bool result = pred(item);
                std::cout << "필터링: " << item << " -> " 
                          << (result ? "통과" : "거부") << std::endl;
                return result;
            });
    }
};
```

### Coroutines

```cpp
#include <coroutine>
#include <memory>
#include <exception>

// C++20 Coroutines (기본 예시)
template<typename T>
class Generator {
public:
    struct promise_type {
        T current_value;
        
        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        
        std::suspend_always yield_value(T value) {
            current_value = value;
            return {};
        }
        
        void return_void() {}
        void unhandled_exception() {}
    };
    
    struct iterator {
        std::coroutine_handle<promise_type> handle_;
        
        iterator(std::coroutine_handle<promise_type> handle) : handle_(handle) {}
        
        iterator& operator++() {
            handle_.resume();
            if (handle_.done()) {
                handle_ = {};
            }
            return *this;
        }
        
        T operator*() const {
            return handle_.promise().current_value;
        }
        
        bool operator==(const iterator& other) const {
            return handle_ == other.handle_;
        }
        
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    };
    
    iterator begin() {
        if (handle_) {
            handle_.resume();
            if (handle_.done()) {
                return iterator{};
            }
        }
        return iterator{handle_};
    }
    
    iterator end() {
        return iterator{};
    }
    
    Generator(std::coroutine_handle<promise_type> handle) : handle_(handle) {}
    
    ~Generator() {
        if (handle_) {
            handle_.destroy();
        }
    }
    
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    
    Generator(Generator&& other) noexcept : handle_(other.handle_) {
        other.handle_ = {};
    }
    
    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                handle_.destroy();
            }
            handle_ = other.handle_;
            other.handle_ = {};
        }
        return *this;
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

class CoroutineLogProcessor {
public:
    // 코루틴으로 로그 생성
    Generator<LogEntry> generateLogs() {
        for (int i = 0; i < 5; ++i) {
            std::string message = "로그 메시지 " + std::to_string(i);
            std::string level = (i % 2 == 0) ? "INFO" : "ERROR";
            co_yield LogEntry(message, level);
        }
    }
    
    // 피보나치 수열 생성 (예시)
    Generator<int> fibonacci() {
        int a = 0, b = 1;
        while (true) {
            co_yield a;
            auto temp = a + b;
            a = b;
            b = temp;
        }
    }
    
    void demonstrateCoroutines() {
        std::cout << "코루틴으로 생성된 로그들:" << std::endl;
        for (const auto& log : generateLogs()) {
            std::cout << "  [" << log.level << "] " << log.message << std::endl;
        }
        
        std::cout << "\n피보나치 수열 (처음 10개):" << std::endl;
        int count = 0;
        for (int fib : fibonacci()) {
            if (count++ >= 10) break;
            std::cout << "  " << fib << std::endl;
        }
    }
};
```

### Three-way Comparison (Spaceship Operator)

```cpp
#include <compare>

// C++20 Three-way comparison
class ComparableLogEntry {
private:
    std::string message_;
    std::string level_;
    std::chrono::system_clock::time_point timestamp_;
    int priority_;
    
public:
    ComparableLogEntry(std::string message, std::string level, int priority = 0)
        : message_(std::move(message)), level_(std::move(level)), 
          timestamp_(std::chrono::system_clock::now()), priority_(priority) {}
    
    // 삼원 비교 연산자 (spaceship operator)
    auto operator<=>(const ComparableLogEntry& other) const {
        // 우선순위 먼저 비교
        if (auto cmp = priority_ <=> other.priority_; cmp != 0) {
            return cmp;
        }
        
        // 시간순 비교
        if (auto cmp = timestamp_ <=> other.timestamp_; cmp != 0) {
            return cmp;
        }
        
        // 레벨별 비교
        if (auto cmp = level_ <=> other.level_; cmp != 0) {
            return cmp;
        }
        
        // 마지막으로 메시지 비교
        return message_ <=> other.message_;
    }
    
    // 등등 연산자는 자동 생성됨
    bool operator==(const ComparableLogEntry& other) const = default;
    
    // 접근자들
    const std::string& getMessage() const { return message_; }
    const std::string& getLevel() const { return level_; }
    int getPriority() const { return priority_; }
};

class SpaceshipExample {
public:
    void demonstrateSpaceship() {
        ComparableLogEntry log1("첫 번째 로그", "INFO", 1);
        ComparableLogEntry log2("두 번째 로그", "ERROR", 2);
        ComparableLogEntry log3("세 번째 로그", "INFO", 1);
        
        // 모든 비교 연산자가 자동으로 사용 가능
        std::cout << "log1 < log2: " << (log1 < log2) << std::endl;
        std::cout << "log1 == log3: " << (log1 == log3) << std::endl;
        std::cout << "log2 > log1: " << (log2 > log1) << std::endl;
        
        // 컨테이너에서 정렬
        std::vector<ComparableLogEntry> logs = {log3, log1, log2};
        std::sort(logs.begin(), logs.end());
        
        std::cout << "\n정렬된 로그들:" << std::endl;
        for (const auto& log : logs) {
            std::cout << "  [" << log.getLevel() << "] " 
                      << log.getMessage() << " (우선순위: " 
                      << log.getPriority() << ")" << std::endl;
        }
    }
};
```

---

## 🔥 5. C++23: 최신 기능들

### std::expected

```cpp
#include <expected>  // C++23
#include <string>

// C++23 std::expected (예상 구현)
template<typename T, typename E>
class expected {
private:
    union {
        T value_;
        E error_;
    };
    bool has_value_;
    
public:
    expected(const T& value) : value_(value), has_value_(true) {}
    expected(const E& error) : error_(error), has_value_(false) {}
    
    ~expected() {
        if (has_value_) {
            value_.~T();
        } else {
            error_.~E();
        }
    }
    
    bool has_value() const { return has_value_; }
    operator bool() const { return has_value_; }
    
    T& value() & { return value_; }
    const T& value() const& { return value_; }
    T&& value() && { return std::move(value_); }
    
    E& error() & { return error_; }
    const E& error() const& { return error_; }
    
    T value_or(const T& default_value) const {
        return has_value_ ? value_ : default_value;
    }
};

// 에러 타입
enum class LogError {
    InvalidFormat,
    FileNotFound,
    PermissionDenied,
    NetworkError
};

class ExpectedLogProcessor {
public:
    expected<LogEntry, LogError> parseLogLine(const std::string& line) {
        if (line.empty()) {
            return LogError::InvalidFormat;
        }
        
        // 간단한 파싱 시뮬레이션
        if (line.find('[') == std::string::npos) {
            return LogError::InvalidFormat;
        }
        
        // 성공적인 파싱
        return LogEntry(line, "INFO");
    }
    
    expected<std::vector<LogEntry>, LogError> loadLogsFromFile(const std::string& filename) {
        // 파일 존재 확인 (시뮬레이션)
        if (filename.empty()) {
            return LogError::FileNotFound;
        }
        
        std::vector<LogEntry> logs;
        std::vector<std::string> lines = {
            "[INFO] 서버 시작",
            "[ERROR] 연결 실패",
            "잘못된 형식"  // 파싱 에러 발생할 줄
        };
        
        for (const auto& line : lines) {
            auto result = parseLogLine(line);
            if (result) {
                logs.push_back(result.value());
            } else {
                return result.error();  // 에러 전파
            }
        }
        
        return logs;
    }
    
    void demonstrateExpected() {
        // 성공 케이스
        auto result1 = parseLogLine("[INFO] 테스트 메시지");
        if (result1) {
            std::cout << "파싱 성공: " << result1->message << std::endl;
        }
        
        // 실패 케이스
        auto result2 = parseLogLine("");
        if (!result2) {
            std::cout << "파싱 실패: " << static_cast<int>(result2.error()) << std::endl;
        }
        
        // 체이닝과 같은 처리
        auto file_result = loadLogsFromFile("test.log");
        if (file_result) {
            std::cout << "로그 " << file_result->size() << "개 로드됨" << std::endl;
        } else {
            std::cout << "파일 로딩 실패: " << static_cast<int>(file_result.error()) << std::endl;
        }
    }
};
```

### Deducing this

```cpp
// C++23 Deducing this (명시적 객체 매개변수)
class DeducingThisExample {
private:
    std::vector<LogEntry> logs_;
    
public:
    // 전통적인 방식: const와 non-const 버전 모두 구현 필요
    LogEntry& getLog(size_t index) & {
        return logs_[index];
    }
    
    const LogEntry& getLog(size_t index) const& {
        return logs_[index];
    }
    
    LogEntry&& getLog(size_t index) && {
        return std::move(logs_[index]);
    }
    
    // C++23 방식: 하나의 템플릿으로 모든 케이스 처리
    template<typename Self>
    auto&& getLogModern(this Self&& self, size_t index) {
        return std::forward<Self>(self).logs_[index];
    }
    
    // 메서드 체이닝도 더 쉬워짐
    template<typename Self>
    auto&& addLog(this Self&& self, LogEntry entry) {
        self.logs_.push_back(std::move(entry));
        return std::forward<Self>(self);
    }
    
    template<typename Self>
    auto&& sortLogs(this Self&& self) {
        std::sort(self.logs_.begin(), self.logs_.end(), 
                 [](const LogEntry& a, const LogEntry& b) {
                     return a.level < b.level;
                 });
        return std::forward<Self>(self);
    }
    
    void demonstrateDeducingThis() {
        DeducingThisExample processor;
        
        // 메서드 체이닝
        processor.addLog(LogEntry("메시지1", "INFO"))
                .addLog(LogEntry("메시지2", "ERROR"))
                .sortLogs();
        
        // 다양한 참조 타입에서 자동으로 올바른 버전 호출
        const auto& const_ref = processor;
        auto log1 = const_ref.getLogModern(0);  // const 버전 호출
        
        auto log2 = std::move(processor).getLogModern(0);  // 이동 버전 호출
    }
};
```

---

## 🎯 6. LogCaster에서 Modern C++ 종합 활용

### 완전한 Modern C++ LogServer

```cpp
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <variant>
#include <ranges>
#include <concepts>
#include <coroutine>
#include <chrono>

// LogCaster Modern C++ 완전 구현
namespace LogCaster::Modern {

// Concepts 정의
template<typename T>
concept LogLevelType = std::convertible_to<T, std::string>;

template<typename T>
concept LogMessageType = std::convertible_to<T, std::string>;

// Modern LogEntry with all C++ features
class ModernLogEntry {
private:
    std::string message_;
    std::string level_;
    std::chrono::system_clock::time_point timestamp_;
    std::optional<std::string> source_;
    std::variant<int, std::string> extra_data_;
    
public:
    // Perfect forwarding constructor
    template<LogMessageType Message, LogLevelType Level>
    ModernLogEntry(Message&& message, Level&& level, 
                   std::optional<std::string> source = std::nullopt)
        : message_(std::forward<Message>(message))
        , level_(std::forward<Level>(level))
        , timestamp_(std::chrono::system_clock::now())
        , source_(std::move(source)) {}
    
    // Three-way comparison
    auto operator<=>(const ModernLogEntry& other) const {
        if (auto cmp = timestamp_ <=> other.timestamp_; cmp != 0) {
            return cmp;
        }
        if (auto cmp = level_ <=> other.level_; cmp != 0) {
            return cmp;
        }
        return message_ <=> other.message_;
    }
    
    bool operator==(const ModernLogEntry& other) const = default;
    
    // Deducing this (C++23 style simulation)
    template<typename Self>
    auto&& getMessage(this Self&& self) {
        return std::forward<Self>(self).message_;
    }
    
    template<typename Self>
    auto&& getLevel(this Self&& self) {
        return std::forward<Self>(self).level_;
    }
    
    const auto& getTimestamp() const { return timestamp_; }
    const auto& getSource() const { return source_; }
    
    // Structured binding support
    template<size_t N>
    decltype(auto) get() const {
        if constexpr (N == 0) return message_;
        else if constexpr (N == 1) return level_;
        else if constexpr (N == 2) return timestamp_;
        else static_assert(N < 3, "Index out of range");
    }
};

// Coroutine-based log generator
class LogGenerator {
public:
    template<typename T>
    struct Generator {
        struct promise_type {
            T current_value;
            
            Generator get_return_object() {
                return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
            }
            
            std::suspend_always initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            
            std::suspend_always yield_value(T value) {
                current_value = std::move(value);
                return {};
            }
            
            void return_void() {}
            void unhandled_exception() {}
        };
        
        struct iterator {
            std::coroutine_handle<promise_type> handle_;
            
            iterator& operator++() {
                handle_.resume();
                if (handle_.done()) handle_ = {};
                return *this;
            }
            
            T operator*() const { return handle_.promise().current_value; }
            bool operator!=(const iterator& other) const { return handle_ != other.handle_; }
        };
        
        iterator begin() {
            if (handle_) {
                handle_.resume();
                if (handle_.done()) return iterator{};
            }
            return iterator{handle_};
        }
        
        iterator end() { return iterator{}; }
        
        Generator(std::coroutine_handle<promise_type> handle) : handle_(handle) {}
        ~Generator() { if (handle_) handle_.destroy(); }
        
        Generator(const Generator&) = delete;
        Generator& operator=(const Generator&) = delete;
        Generator(Generator&& other) noexcept : handle_(std::exchange(other.handle_, {})) {}
        Generator& operator=(Generator&& other) noexcept {
            if (this != &other) {
                if (handle_) handle_.destroy();
                handle_ = std::exchange(other.handle_, {});
            }
            return *this;
        }
        
    private:
        std::coroutine_handle<promise_type> handle_;
    };
    
    static Generator<ModernLogEntry> generateSampleLogs() {
        std::vector<std::string> levels = {"DEBUG", "INFO", "WARN", "ERROR"};
        std::vector<std::string> messages = {
            "시스템 초기화 완료",
            "사용자 연결됨",
            "메모리 사용량 높음",
            "네트워크 연결 실패"
        };
        
        for (int i = 0; i < 10; ++i) {
            std::string level = levels[i % levels.size()];
            std::string message = messages[i % messages.size()] + " #" + std::to_string(i);
            co_yield ModernLogEntry(std::move(message), std::move(level));
        }
    }
};

// Modern LogServer with all features
class UltimateModernLogServer {
private:
    std::vector<ModernLogEntry> logs_;
    std::map<std::string, size_t> level_counts_;
    
public:
    // Concepts-constrained add method
    template<LogMessageType Message, LogLevelType Level>
    void addLog(Message&& message, Level&& level, 
               std::optional<std::string> source = std::nullopt) {
        logs_.emplace_back(std::forward<Message>(message), 
                          std::forward<Level>(level), 
                          std::move(source));
        
        std::string level_str = std::string(level);
        level_counts_[level_str]++;
    }
    
    // Range-based filtering with concepts
    template<std::predicate<const ModernLogEntry&> Predicate>
    auto getFilteredLogs(Predicate&& pred) const {
        return logs_ | std::views::filter(std::forward<Predicate>(pred));
    }
    
    // Modern searching with optional
    std::optional<ModernLogEntry> findFirstByLevel(const std::string& level) const {
        auto it = std::ranges::find_if(logs_, 
            [&level](const ModernLogEntry& entry) {
                return entry.getLevel() == level;
            });
        
        return (it != logs_.end()) ? std::optional{*it} : std::nullopt;
    }
    
    // Statistical analysis with ranges
    auto getLevelStatistics() const {
        return logs_ 
            | std::views::transform([](const ModernLogEntry& entry) {
                return entry.getLevel();
            })
            | std::views::common;  // For compatibility with algorithms
    }
    
    // Modern error handling with variant
    using ProcessResult = std::variant<size_t, std::string>;  // success count or error
    
    ProcessResult processLogsInBatch(const std::vector<std::string>& messages) {
        try {
            size_t processed = 0;
            for (const auto& message : messages) {
                if (message.empty()) {
                    return std::string("빈 메시지 발견");
                }
                addLog(message, "BATCH");
                ++processed;
            }
            return processed;
        } catch (const std::exception& e) {
            return std::string("처리 오류: ") + e.what();
        }
    }
    
    // Coroutine integration
    void loadSampleData() {
        for (auto&& entry : LogGenerator::generateSampleLogs()) {
            logs_.push_back(std::move(entry));
            level_counts_[entry.getLevel()]++;
        }
    }
    
    // Modern printing with structured bindings
    void printRecentLogs(size_t count = 5) const {
        auto recent_logs = logs_ 
            | std::views::reverse 
            | std::views::take(count);
        
        std::cout << "최근 로그들:" << std::endl;
        for (const auto& [message, level, timestamp] : recent_logs) {
            auto time_t = std::chrono::system_clock::to_time_t(timestamp);
            std::cout << "  [" << level << "] " << message 
                      << " (" << std::ctime(&time_t) << ")" << std::endl;
        }
    }
    
    // Lambda-based custom operations
    template<typename Operation>
    void forEachLog(Operation&& op) const 
        requires std::invocable<Operation, const ModernLogEntry&> {
        std::ranges::for_each(logs_, std::forward<Operation>(op));
    }
    
    // Modern statistics
    void printStatistics() const {
        std::cout << "\n=== 로그 통계 ===" << std::endl;
        std::cout << "총 로그 수: " << logs_.size() << std::endl;
        
        for (const auto& [level, count] : level_counts_) {
            std::cout << level << ": " << count << "개" << std::endl;
        }
        
        // 가장 최근 로그
        if (auto latest = std::ranges::max_element(logs_, 
                [](const auto& a, const auto& b) {
                    return a.getTimestamp() < b.getTimestamp();
                }); latest != logs_.end()) {
            std::cout << "최신 로그: " << latest->getMessage() << std::endl;
        }
    }
    
    // Method chaining with perfect forwarding
    template<typename Self>
    auto&& clearLogs(this Self&& self) {
        self.logs_.clear();
        self.level_counts_.clear();
        return std::forward<Self>(self);
    }
    
    template<typename Self>
    auto&& sortByTimestamp(this Self&& self) {
        std::ranges::sort(self.logs_, 
            [](const auto& a, const auto& b) {
                return a.getTimestamp() < b.getTimestamp();
            });
        return std::forward<Self>(self);
    }
};

} // namespace LogCaster::Modern

// Structured binding support for ModernLogEntry
namespace std {
    template<>
    struct tuple_size<LogCaster::Modern::ModernLogEntry> {
        static constexpr size_t value = 3;
    };
    
    template<size_t N>
    struct tuple_element<N, LogCaster::Modern::ModernLogEntry> {
        using type = decltype(std::declval<LogCaster::Modern::ModernLogEntry>().get<N>());
    };
}

// 사용 예시
void demonstrateModernCpp() {
    using namespace LogCaster::Modern;
    
    UltimateModernLogServer server;
    
    // Sample data loading with coroutines
    server.loadSampleData();
    
    // Modern adding with perfect forwarding
    server.addLog("사용자 로그인", "INFO", "192.168.1.1");
    server.addLog("데이터베이스 연결", "DEBUG");
    
    // Range-based filtering
    auto error_logs = server.getFilteredLogs([](const auto& log) {
        return log.getLevel() == "ERROR";
    });
    
    std::cout << "에러 로그들:" << std::endl;
    for (const auto& log : error_logs) {
        std::cout << "  " << log.getMessage() << std::endl;
    }
    
    // Optional-based searching
    if (auto info_log = server.findFirstByLevel("INFO")) {
        std::cout << "\n첫 번째 INFO 로그: " << info_log->getMessage() << std::endl;
    }
    
    // Modern error handling with variant
    std::vector<std::string> batch_messages = {"메시지1", "메시지2", ""};
    auto result = server.processLogsInBatch(batch_messages);
    
    std::visit([](const auto& res) {
        using T = std::decay_t<decltype(res)>;
        if constexpr (std::is_same_v<T, size_t>) {
            std::cout << "배치 처리 완료: " << res << "개" << std::endl;
        } else {
            std::cout << "배치 처리 실패: " << res << std::endl;
        }
    }, result);
    
    // Method chaining
    server.sortByTimestamp().printRecentLogs(3);
    
    // Statistics
    server.printStatistics();
    
    // Lambda operations
    server.forEachLog([](const auto& log) {
        if (log.getLevel() == "ERROR") {
            std::cout << "🚨 에러 발견: " << log.getMessage() << std::endl;
        }
    });
}
```

---

## 📋 7. 체크리스트와 정리

### Modern C++ 기능 활용 체크리스트

#### C++11 기본 기능
- [ ] auto 키워드로 타입 추론
- [ ] Range-based for 루프
- [ ] Lambda 함수와 캡처
- [ ] Smart pointers (unique_ptr, shared_ptr, weak_ptr)
- [ ] Move semantics와 perfect forwarding
- [ ] nullptr 사용

#### C++14 편의 기능
- [ ] Generic lambdas
- [ ] auto 반환 타입 추론
- [ ] Binary literals와 digit separators
- [ ] std::make_unique 사용

#### C++17 표현력 향상
- [ ] Structured bindings
- [ ] if constexpr
- [ ] std::optional, std::variant, std::any
- [ ] std::string_view 활용

#### C++20 현대적 기능
- [ ] Concepts로 타입 제약
- [ ] Ranges와 views 활용
- [ ] Coroutines (필요시)
- [ ] Three-way comparison (spaceship operator)

#### C++23 최신 기능
- [ ] std::expected (가능시)
- [ ] Deducing this 패턴

### 핵심 포인트 정리

1. **Auto**: 타입 추론으로 코드 간소화
2. **Lambdas**: 함수형 프로그래밍 요소 도입
3. **Smart Pointers**: 자동 메모리 관리
4. **Move Semantics**: 성능 최적화
5. **Structured Bindings**: 다중 값 처리 간소화
6. **Optional/Variant**: 안전한 값 처리
7. **Concepts**: 타입 안전성 향상
8. **Ranges**: 함수형 데이터 처리

---

*🚀 Modern C++는 더 안전하고, 더 표현력 있고, 더 효율적인 코드를 작성할 수 있게 해줍니다. LogCaster 프로젝트에서 이러한 기능들을 적극 활용하여 현대적인 C++ 전문가가 되어보세요!*