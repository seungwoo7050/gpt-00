# 23. Lambda 함수와 함수형 프로그래밍 완벽 가이드 🔗

*LogCaster 프로젝트를 위한 사전지식 문서*

---

## 📋 문서 정보
- **난이도**: 🟡 중급 → 🔴 고급
- **예상 학습 시간**: 10-12시간
- **전제 조건**: 
  - C++ 기본 문법 숙지
  - 함수 포인터 이해
  - STL 알고리즘 기본 지식
- **실습 환경**: C++11 이상 (C++14/17 권장), GCC 9+/Clang 10+

## 🎯 이 문서에서 배울 내용

**"함수형 프로그래밍의 힘으로 더 우아하고 표현력 있는 코드 작성하기! ✨"**

Lambda 함수는 **코드를 더 간결하고 표현력 있게** 만들어주는 현대 C++의 핵심 기능입니다. 마치 **스위스 아미 나이프**처럼, 다양한 상황에서 유연하게 사용할 수 있는 강력한 도구입니다.

---

## 🚀 1. Lambda 함수 기초

### Lambda의 기본 구조

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

// Lambda 기본 구조: [capture](parameters) -> return_type { body }

class BasicLambdaExamples {
public:
    void demonstrateBasicSyntax() {
        // 1. 가장 간단한 람다
        auto hello = []() {
            std::cout << "Hello Lambda!" << std::endl;
        };
        hello();  // 호출
        
        // 2. 매개변수가 있는 람다
        auto add = [](int a, int b) {
            return a + b;
        };
        std::cout << "5 + 3 = " << add(5, 3) << std::endl;
        
        // 3. 명시적 반환 타입
        auto divide = [](double a, double b) -> double {
            if (b != 0.0) {
                return a / b;
            }
            return 0.0;
        };
        std::cout << "10.0 / 3.0 = " << divide(10.0, 3.0) << std::endl;
        
        // 4. LogCaster 관련 예시
        auto log_formatter = [](const std::string& level, const std::string& message) -> std::string {
            return "[" + level + "] " + message;
        };
        
        std::cout << log_formatter("INFO", "서버 시작됨") << std::endl;
        std::cout << log_formatter("ERROR", "연결 실패") << std::endl;
    }
    
    void demonstrateWithSTL() {
        std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        
        // 전통적인 방식: 함수 객체 클래스 정의 필요
        class IsEven {
        public:
            bool operator()(int n) const { return n % 2 == 0; }
        };
        
        // 람다 방식: 훨씬 간결
        auto is_even_lambda = [](int n) { return n % 2 == 0; };
        
        // STL 알고리즘과 함께 사용
        auto even_count = std::count_if(numbers.begin(), numbers.end(), is_even_lambda);
        std::cout << "짝수 개수: " << even_count << std::endl;
        
        // 인라인으로 직접 사용
        auto odd_count = std::count_if(numbers.begin(), numbers.end(), 
            [](int n) { return n % 2 == 1; });
        std::cout << "홀수 개수: " << odd_count << std::endl;
        
        // 복잡한 조건
        auto special_numbers = std::count_if(numbers.begin(), numbers.end(),
            [](int n) { 
                return n > 3 && n < 8 && n % 2 == 0; 
            });
        std::cout << "특별한 숫자 개수: " << special_numbers << std::endl;
    }
};
```

### Capture 클로즈 완벽 이해

```cpp
// LogCaster에서 Capture 활용 예시
class CaptureExamples {
private:
    std::string server_name_ = "LogCaster";
    int port_ = 9999;
    std::vector<std::string> logs_;
    
public:
    void demonstrateCaptures() {
        std::cout << "=== Capture 예시들 ===" << std::endl;
        
        // 1. 값 캡처 (Copy Capture)
        std::string prefix = "[SERVER] ";
        auto value_capture_logger = [prefix](const std::string& msg) {
            // prefix가 복사되어 저장됨
            std::cout << prefix << msg << std::endl;
        };
        
        value_capture_logger("값 캡처 테스트");
        prefix = "[MODIFIED] ";  // 원본 변경해도 람다 내부는 영향 없음
        value_capture_logger("여전히 [SERVER] 사용");
        
        // 2. 참조 캡처 (Reference Capture)
        std::string dynamic_prefix = "[DYNAMIC] ";
        auto ref_capture_logger = [&dynamic_prefix](const std::string& msg) {
            // dynamic_prefix의 참조를 저장
            std::cout << dynamic_prefix << msg << std::endl;
        };
        
        ref_capture_logger("참조 캡처 테스트");
        dynamic_prefix = "[CHANGED] ";  // 원본 변경하면 람다도 영향받음
        ref_capture_logger("이제 [CHANGED] 사용");
        
        // 3. 멤버 변수 캡처
        auto member_capture_logger = [this](const std::string& msg) {
            // this 포인터를 캡처하여 멤버 변수 접근
            std::cout << "[" << server_name_ << ":" << port_ << "] " << msg << std::endl;
        };
        
        member_capture_logger("멤버 변수 캡처 테스트");
        
        // 4. 혼합 캡처
        int log_count = 0;
        auto mixed_capture = [this, log_count, &dynamic_prefix](const std::string& msg) mutable {
            // this: 멤버 접근, log_count: 값 캡처, dynamic_prefix: 참조 캡처
            // mutable: 값으로 캡처한 변수 수정 가능
            ++log_count;  // mutable 덕분에 수정 가능
            logs_.push_back(msg);  // this를 통해 멤버 변수 수정
            std::cout << dynamic_prefix << "로그 #" << log_count << ": " << msg << std::endl;
        };
        
        mixed_capture("첫 번째 로그");
        mixed_capture("두 번째 로그");
        std::cout << "실제 저장된 로그 수: " << logs_.size() << std::endl;
        
        // 5. 모든 것을 값으로 캡처 [=]
        int error_count = 5;
        std::string warning_level = "HIGH";
        
        auto all_by_value = [=](const std::string& msg) mutable {
            // 주변의 모든 변수를 값으로 캡처
            ++error_count;  // 복사본을 수정
            std::cout << "[" << warning_level << "] 에러 #" << error_count << ": " << msg << std::endl;
        };
        
        all_by_value("심각한 오류");
        std::cout << "원본 error_count: " << error_count << std::endl;  // 여전히 5
        
        // 6. 모든 것을 참조로 캡처 [&]
        auto all_by_reference = [&](const std::string& msg) {
            // 주변의 모든 변수를 참조로 캡처
            ++error_count;  // 원본을 수정
            warning_level = "CRITICAL";
            std::cout << "[" << warning_level << "] 에러 #" << error_count << ": " << msg << std::endl;
        };
        
        all_by_reference("치명적 오류");
        std::cout << "수정된 error_count: " << error_count << std::endl;  // 6으로 증가
        std::cout << "수정된 warning_level: " << warning_level << std::endl;  // CRITICAL
    }
    
    void demonstrateAdvancedCaptures() {
        // 7. 선택적 캡처
        std::string app_name = "LogCaster";
        std::string version = "1.0";
        int max_connections = 1000;
        
        auto selective_capture = [app_name, &version, this](const std::string& msg) {
            // app_name: 값으로, version: 참조로, this: 포인터로 캡처
            std::cout << "[" << app_name << " v" << version << ":" << port_ << "] " << msg << std::endl;
        };
        
        selective_capture("선택적 캡처 테스트");
        version = "2.0";  // 참조 캡처된 version만 영향받음
        selective_capture("버전이 변경됨");
        
        // 8. 초기화 캡처 (C++14)
        auto init_capture = [counter = 0, name = std::string("Logger")](const std::string& msg) mutable {
            ++counter;
            std::cout << name << " #" << counter << ": " << msg << std::endl;
        };
        
        init_capture("첫 번째 메시지");
        init_capture("두 번째 메시지");
        
        // 9. 이동 캡처 (C++14)
        std::unique_ptr<std::string> large_data = std::make_unique<std::string>("대용량 데이터");
        
        auto move_capture = [data = std::move(large_data)](const std::string& msg) {
            // large_data가 람다 내부로 이동됨
            if (data) {
                std::cout << "데이터와 함께: " << msg << " (" << *data << ")" << std::endl;
            }
        };
        
        // large_data는 이제 nullptr
        move_capture("이동 캡처 테스트");
    }
};
```

---

## 🎭 2. 고급 Lambda 기법

### Generic Lambdas (C++14)

```cpp
#include <type_traits>
#include <string>
#include <iostream>

// LogCaster에서 Generic Lambda 활용
class GenericLambdaExamples {
public:
    void demonstrateGenericLambdas() {
        // 기본 generic lambda
        auto generic_printer = [](const auto& value) {
            std::cout << "값: " << value << std::endl;
        };
        
        generic_printer(42);
        generic_printer("Hello");
        generic_printer(3.14);
        
        // LogCaster 로그 포매터
        auto log_formatter = [](const auto& level, const auto& message, const auto& source = "") {
            std::cout << "[" << level << "] ";
            if constexpr (!std::is_same_v<std::decay_t<decltype(source)>, const char*> || 
                         std::string(source) != "") {
                std::cout << "(" << source << ") ";
            }
            std::cout << message << std::endl;
        };
        
        log_formatter("INFO", "서버 시작");
        log_formatter("ERROR", "연결 실패", "192.168.1.100");
        log_formatter(1, 404, "client.cpp");  // 다른 타입들도 가능
        
        // 타입에 따른 다른 동작
        auto type_aware_processor = [](const auto& data) {
            using T = std::decay_t<decltype(data)>;
            
            if constexpr (std::is_arithmetic_v<T>) {
                std::cout << "숫자 처리: " << data;
                if constexpr (std::is_integral_v<T>) {
                    std::cout << " (정수)";
                } else {
                    std::cout << " (실수)";
                }
                std::cout << std::endl;
            } else if constexpr (std::is_same_v<T, std::string>) {
                std::cout << "문자열 처리: \"" << data << "\"" << std::endl;
            } else {
                std::cout << "기타 타입 처리" << std::endl;
            }
        };
        
        type_aware_processor(42);
        type_aware_processor(3.14);
        type_aware_processor(std::string("Hello"));
    }
    
    void demonstrateGenericWithContainers() {
        // 컨테이너 타입에 무관한 처리
        auto print_container = [](const auto& container) {
            std::cout << "컨테이너 내용: ";
            for (const auto& item : container) {
                std::cout << item << " ";
            }
            std::cout << std::endl;
        };
        
        std::vector<int> numbers = {1, 2, 3, 4, 5};
        std::vector<std::string> words = {"hello", "world", "cpp"};
        std::array<double, 3> doubles = {1.1, 2.2, 3.3};
        
        print_container(numbers);
        print_container(words);
        print_container(doubles);
        
        // 조건부 처리가 가능한 generic lambda
        auto conditional_processor = [](auto container, auto condition, auto action) {
            for (auto& item : container) {
                if (condition(item)) {
                    action(item);
                }
            }
        };
        
        std::vector<int> nums = {1, 2, 3, 4, 5, 6};
        conditional_processor(nums,
            [](int n) { return n % 2 == 0; },  // 짝수 조건
            [](int& n) { n *= 10; }            // 10배로 만들기
        );
        
        print_container(nums);  // 20 40 60이 출력됨
    }
};
```

### Lambda와 std::function

```cpp
#include <functional>
#include <map>
#include <memory>

// LogCaster에서 함수 객체 활용
class FunctionObjectExamples {
private:
    // 로그 처리 함수들을 저장하는 맵
    std::map<std::string, std::function<void(const std::string&)>> log_processors_;
    std::map<std::string, std::function<bool(const std::string&)>> log_filters_;
    
public:
    void setupProcessors() {
        // 다양한 종류의 callable을 std::function에 저장
        
        // 1. 람다 함수
        log_processors_["console"] = [](const std::string& msg) {
            std::cout << "[CONSOLE] " << msg << std::endl;
        };
        
        // 2. 함수 포인터
        log_processors_["file"] = writeToFile;
        
        // 3. 함수 객체
        log_processors_["network"] = NetworkLogger{};
        
        // 4. 멤버 함수 (bind 사용)
        log_processors_["database"] = std::bind(&FunctionObjectExamples::logToDatabase, 
                                               this, std::placeholders::_1);
        
        // 필터 설정
        log_filters_["error_only"] = [](const std::string& msg) {
            return msg.find("ERROR") != std::string::npos;
        };
        
        log_filters_["warning_and_error"] = [](const std::string& msg) {
            return msg.find("ERROR") != std::string::npos || 
                   msg.find("WARN") != std::string::npos;
        };
    }
    
    void processLog(const std::string& message, const std::string& processor_name = "console") {
        auto processor_it = log_processors_.find(processor_name);
        if (processor_it != log_processors_.end()) {
            processor_it->second(message);
        } else {
            std::cout << "알 수 없는 프로세서: " << processor_name << std::endl;
        }
    }
    
    void processLogWithFilter(const std::string& message, 
                             const std::string& filter_name,
                             const std::string& processor_name = "console") {
        auto filter_it = log_filters_.find(filter_name);
        auto processor_it = log_processors_.find(processor_name);
        
        if (filter_it != log_filters_.end() && processor_it != log_processors_.end()) {
            if (filter_it->second(message)) {  // 필터 통과
                processor_it->second(message);
            } else {
                std::cout << "필터링됨: " << message << std::endl;
            }
        }
    }
    
    // 고차 함수: 함수를 반환하는 함수
    std::function<void(const std::string&)> createLevelLogger(const std::string& level) {
        return [level](const std::string& msg) {
            std::cout << "[" << level << "] " << msg << std::endl;
        };
    }
    
    // 함수 조합
    std::function<void(const std::string&)> combineProcessors(
        std::function<void(const std::string&)> first,
        std::function<void(const std::string&)> second) {
        
        return [first, second](const std::string& msg) {
            first(msg);
            second(msg);
        };
    }
    
    void demonstrateFunctionObjects() {
        setupProcessors();
        
        // 기본 처리
        processLog("[INFO] 서버 시작됨");
        processLog("[ERROR] 연결 실패", "file");
        
        // 필터링 처리
        processLogWithFilter("[ERROR] 심각한 오류", "error_only");
        processLogWithFilter("[INFO] 정보 메시지", "error_only");
        
        // 고차 함수 사용
        auto info_logger = createLevelLogger("INFO");
        auto error_logger = createLevelLogger("ERROR");
        
        info_logger("정보 메시지");
        error_logger("오류 메시지");
        
        // 함수 조합
        auto console_and_file = combineProcessors(log_processors_["console"], 
                                                log_processors_["file"]);
        console_and_file("[WARN] 경고 메시지");
    }

private:
    // 정적 함수 (함수 포인터로 사용 가능)
    static void writeToFile(const std::string& msg) {
        std::cout << "[FILE] " << msg << std::endl;
        // 실제로는 파일에 쓰기
    }
    
    // 멤버 함수
    void logToDatabase(const std::string& msg) {
        std::cout << "[DATABASE] " << msg << std::endl;
        // 실제로는 데이터베이스에 저장
    }
    
    // 함수 객체 클래스
    struct NetworkLogger {
        void operator()(const std::string& msg) const {
            std::cout << "[NETWORK] " << msg << std::endl;
            // 실제로는 네트워크로 전송
        }
    };
};
```

---

## 🔄 3. 함수형 프로그래밍 패턴

### Map, Filter, Reduce 패턴

```cpp
#include <numeric>
#include <optional>
#include <ranges>  // C++20

// LogCaster에서 함수형 프로그래밍 패턴 활용
struct LogEntry {
    std::string message;
    std::string level;
    int priority;
    std::chrono::system_clock::time_point timestamp;
    
    LogEntry(std::string msg, std::string lvl, int prio = 0)
        : message(std::move(msg)), level(std::move(lvl)), priority(prio)
        , timestamp(std::chrono::system_clock::now()) {}
};

class FunctionalPatterns {
private:
    std::vector<LogEntry> logs_;
    
public:
    void setupTestData() {
        logs_ = {
            LogEntry("서버 시작", "INFO", 1),
            LogEntry("사용자 로그인", "INFO", 1),
            LogEntry("메모리 부족", "WARN", 2),
            LogEntry("연결 실패", "ERROR", 3),
            LogEntry("치명적 오류", "ERROR", 3),
            LogEntry("디버그 정보", "DEBUG", 0)
        };
    }
    
    // MAP 패턴: 각 요소를 변환
    template<typename Container, typename Transform>
    auto map(const Container& container, Transform transform) {
        std::vector<decltype(transform(*container.begin()))> result;
        result.reserve(container.size());
        
        std::transform(container.begin(), container.end(), 
                      std::back_inserter(result), transform);
        return result;
    }
    
    // FILTER 패턴: 조건에 맞는 요소만 선택
    template<typename Container, typename Predicate>
    auto filter(const Container& container, Predicate predicate) {
        Container result;
        std::copy_if(container.begin(), container.end(), 
                    std::back_inserter(result), predicate);
        return result;
    }
    
    // REDUCE 패턴: 요소들을 하나의 값으로 축약
    template<typename Container, typename T, typename BinaryOp>
    T reduce(const Container& container, T init, BinaryOp binary_op) {
        return std::accumulate(container.begin(), container.end(), init, binary_op);
    }
    
    void demonstrateMapFilterReduce() {
        setupTestData();
        
        std::cout << "=== MAP, FILTER, REDUCE 패턴 ===" << std::endl;
        
        // MAP: 로그 메시지를 대문자로 변환
        auto uppercase_messages = map(logs_, [](const LogEntry& log) {
            std::string upper_msg = log.message;
            std::transform(upper_msg.begin(), upper_msg.end(), upper_msg.begin(), ::toupper);
            return upper_msg;
        });
        
        std::cout << "대문자 메시지들:" << std::endl;
        for (const auto& msg : uppercase_messages) {
            std::cout << "  " << msg << std::endl;
        }
        
        // FILTER: ERROR 레벨만 필터링
        auto error_logs = filter(logs_, [](const LogEntry& log) {
            return log.level == "ERROR";
        });
        
        std::cout << "\nERROR 로그들:" << std::endl;
        for (const auto& log : error_logs) {
            std::cout << "  " << log.message << std::endl;
        }
        
        // REDUCE: 총 우선순위 합계 계산
        auto total_priority = reduce(logs_, 0, [](int sum, const LogEntry& log) {
            return sum + log.priority;
        });
        
        std::cout << "\n총 우선순위 합계: " << total_priority << std::endl;
        
        // 복합적 사용: 체이닝
        auto high_priority_error_count = reduce(
            filter(logs_, [](const LogEntry& log) { return log.level == "ERROR" && log.priority > 2; }),
            0,
            [](int count, const LogEntry&) { return count + 1; }
        );
        
        std::cout << "고우선순위 에러 로그 수: " << high_priority_error_count << std::endl;
    }
    
    // C++20 Ranges를 사용한 현대적 접근법
    void demonstrateModernFunctional() {
#ifdef __cpp_lib_ranges  // C++20 ranges가 사용 가능한 경우
        std::cout << "\n=== Modern C++20 Ranges ===" << std::endl;
        
        // 파이프라인 스타일로 데이터 처리
        auto processed_logs = logs_
            | std::views::filter([](const LogEntry& log) { 
                return log.priority > 0; 
            })
            | std::views::transform([](const LogEntry& log) { 
                return "[" + log.level + "] " + log.message; 
            })
            | std::views::take(3);  // 처음 3개만
        
        std::cout << "처리된 로그들 (상위 3개):" << std::endl;
        for (const auto& processed : processed_logs) {
            std::cout << "  " << processed << std::endl;
        }
        
        // 그룹화와 집계
        std::map<std::string, std::vector<LogEntry>> logs_by_level;
        for (const auto& log : logs_) {
            logs_by_level[log.level].push_back(log);
        }
        
        for (const auto& [level, entries] : logs_by_level) {
            auto avg_priority = std::ranges::fold_left(
                entries | std::views::transform([](const LogEntry& log) { 
                    return log.priority; 
                }),
                0.0,
                std::plus<>{}
            ) / entries.size();
            
            std::cout << level << " 평균 우선순위: " << avg_priority << std::endl;
        }
#endif
    }
};
```

### Currying과 Partial Application

```cpp
// 커링과 부분 적용
class CurryingExamples {
public:
    // 기본적인 커링: 여러 인자를 받는 함수를 단일 인자 함수들의 체인으로 변환
    template<typename F>
    auto curry(F&& f) {
        return [f](auto&& first_arg) {
            return [f, first_arg](auto&&... rest_args) {
                return f(first_arg, rest_args...);
            };
        };
    }
    
    // LogCaster 전용 로거 팩토리
    auto createLogFormatter() {
        // 기본 로그 포맷 함수
        auto basic_format = [](const std::string& level, const std::string& source, 
                              const std::string& message) {
            return "[" + level + "] (" + source + ") " + message;
        };
        
        // 커링 적용
        auto curried_format = curry(basic_format);
        
        // 부분 적용으로 특정 레벨 로거 생성
        auto error_logger = curried_format("ERROR");
        auto info_logger = curried_format("INFO");
        
        return std::make_pair(error_logger, info_logger);
    }
    
    void demonstrateCurrying() {
        auto [error_logger, info_logger] = createLogFormatter();
        
        // 소스별 로거 생성
        auto server_error_logger = error_logger("SERVER");
        auto client_error_logger = error_logger("CLIENT");
        auto server_info_logger = info_logger("SERVER");
        
        // 사용
        std::cout << server_error_logger("연결 실패") << std::endl;
        std::cout << client_error_logger("인증 실패") << std::endl;
        std::cout << server_info_logger("서버 시작됨") << std::endl;
    }
    
    // 고급 부분 적용: std::bind 사용
    void demonstratePartialApplication() {
        auto log_with_timestamp = [](const std::string& level, const std::string& source,
                                    const std::string& message, bool include_time) {
            std::string result = "[" + level + "] (" + source + ") " + message;
            if (include_time) {
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);
                result += " @ " + std::string(std::ctime(&time_t));
                result.pop_back();  // 개행 제거
            }
            return result;
        };
        
        // 부분 적용으로 특화된 함수들 생성
        using namespace std::placeholders;
        
        // 항상 타임스탬프를 포함하는 로거
        auto timestamped_logger = std::bind(log_with_timestamp, _1, _2, _3, true);
        
        // 특정 소스에 대한 로거
        auto server_logger = std::bind(log_with_timestamp, _1, "SERVER", _2, _3);
        
        // 사용 예시
        std::cout << timestamped_logger("INFO", "CLIENT", "사용자 로그인") << std::endl;
        std::cout << server_logger("ERROR", "메모리 부족", false) << std::endl;
    }
};
```

### Composition과 Pipe Operations

```cpp
// 함수 합성과 파이프 연산
class CompositionExamples {
public:
    // 함수 합성 도우미
    template<typename F, typename G>
    auto compose(F&& f, G&& g) {
        return [f, g](auto&& x) {
            return f(g(std::forward<decltype(x)>(x)));
        };
    }
    
    // 파이프 연산자 (왼쪽에서 오른쪽으로)
    template<typename T>
    struct Pipe {
        T value;
        
        template<typename F>
        auto operator|(F&& f) {
            return Pipe<decltype(f(value))>{f(value)};
        }
        
        operator T() const { return value; }
    };
    
    template<typename T>
    auto make_pipe(T&& value) {
        return Pipe<std::decay_t<T>>{std::forward<T>(value)};
    }
    
    void demonstrateComposition() {
        // 기본적인 변환 함수들
        auto to_upper = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), ::toupper);
            return s;
        };
        
        auto add_brackets = [](const std::string& s) {
            return "[" + s + "]";
        };
        
        auto add_timestamp = [](const std::string& s) {
            auto now = std::chrono::system_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();
            return s + " @" + std::to_string(ms);
        };
        
        // 함수 합성
        auto format_log = compose(add_timestamp, compose(add_brackets, to_upper));
        
        std::cout << "합성된 함수 결과: " << format_log("error message") << std::endl;
        
        // 파이프 연산 (더 읽기 쉬운 방식)
        auto piped_result = make_pipe(std::string("info message"))
            | to_upper
            | add_brackets
            | add_timestamp;
        
        std::cout << "파이프 결과: " << static_cast<std::string>(piped_result) << std::endl;
    }
    
    // LogCaster 로그 처리 파이프라인
    void demonstrateLogProcessingPipeline() {
        struct LogMessage {
            std::string content;
            std::string level;
            std::string source;
        };
        
        // 파이프라인 단계들
        auto validate_message = [](LogMessage msg) -> std::optional<LogMessage> {
            if (msg.content.empty()) {
                return std::nullopt;
            }
            return msg;
        };
        
        auto normalize_level = [](LogMessage msg) {
            std::transform(msg.level.begin(), msg.level.end(), msg.level.begin(), ::toupper);
            return msg;
        };
        
        auto add_prefix = [](LogMessage msg) {
            msg.content = "[" + msg.level + "] " + msg.content;
            return msg;
        };
        
        auto filter_important = [](LogMessage msg) -> std::optional<LogMessage> {
            if (msg.level == "ERROR" || msg.level == "WARN") {
                return msg;
            }
            return std::nullopt;
        };
        
        // 로그 처리 파이프라인
        std::vector<LogMessage> input_logs = {
            {"서버 시작", "info", "server"},
            {"", "error", "client"},  // 빈 메시지
            {"연결 실패", "error", "network"},
            {"일반 정보", "debug", "app"}
        };
        
        std::vector<LogMessage> processed_logs;
        
        for (const auto& log : input_logs) {
            auto result = validate_message(log)
                .transform(normalize_level)
                .transform(add_prefix)
                .and_then(filter_important);
            
            if (result) {
                processed_logs.push_back(*result);
            }
        }
        
        std::cout << "\n처리된 중요 로그들:" << std::endl;
        for (const auto& log : processed_logs) {
            std::cout << "  " << log.content << " (source: " << log.source << ")" << std::endl;
        }
    }
};
```

---

## 🎯 4. LogCaster에서 Lambda 실전 활용

### 로그 처리 시스템 완전 구현

```cpp
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

// 완전한 Lambda 기반 LogCaster 시스템
namespace LogCaster::Functional {

class AdvancedLogEntry {
public:
    std::string message;
    std::string level;
    std::string source;
    std::chrono::system_clock::time_point timestamp;
    std::map<std::string, std::string> metadata;
    
    AdvancedLogEntry(std::string msg, std::string lvl, std::string src = "")
        : message(std::move(msg)), level(std::move(lvl)), source(std::move(src))
        , timestamp(std::chrono::system_clock::now()) {}
    
    // 메타데이터 추가를 위한 fluent interface
    AdvancedLogEntry& with(const std::string& key, const std::string& value) {
        metadata[key] = value;
        return *this;
    }
};

class FunctionalLogProcessor {
private:
    // 함수 타입 정의
    using LogFilter = std::function<bool(const AdvancedLogEntry&)>;
    using LogTransformer = std::function<AdvancedLogEntry(const AdvancedLogEntry&)>;
    using LogAction = std::function<void(const AdvancedLogEntry&)>;
    using LogFormatter = std::function<std::string(const AdvancedLogEntry&)>;
    
    // 처리 파이프라인
    std::vector<LogFilter> filters_;
    std::vector<LogTransformer> transformers_;
    std::vector<LogAction> actions_;
    
    // 비동기 처리를 위한 큐
    std::queue<AdvancedLogEntry> log_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> running_{false};
    std::thread processor_thread_;
    
public:
    FunctionalLogProcessor() = default;
    
    ~FunctionalLogProcessor() {
        stop();
    }
    
    // Fluent Interface로 파이프라인 구성
    FunctionalLogProcessor& addFilter(LogFilter filter) {
        filters_.push_back(std::move(filter));
        return *this;
    }
    
    FunctionalLogProcessor& addTransformer(LogTransformer transformer) {
        transformers_.push_back(std::move(transformer));
        return *this;
    }
    
    FunctionalLogProcessor& addAction(LogAction action) {
        actions_.push_back(std::move(action));
        return *this;
    }
    
    // 미리 정의된 필터들
    FunctionalLogProcessor& filterByLevel(std::initializer_list<std::string> levels) {
        std::set<std::string> level_set(levels);
        return addFilter([level_set](const AdvancedLogEntry& entry) {
            return level_set.find(entry.level) != level_set.end();
        });
    }
    
    FunctionalLogProcessor& filterBySource(const std::string& source_pattern) {
        return addFilter([source_pattern](const AdvancedLogEntry& entry) {
            return entry.source.find(source_pattern) != std::string::npos;
        });
    }
    
    FunctionalLogProcessor& filterByTimeRange(
        std::chrono::system_clock::time_point start,
        std::chrono::system_clock::time_point end) {
        return addFilter([start, end](const AdvancedLogEntry& entry) {
            return entry.timestamp >= start && entry.timestamp <= end;
        });
    }
    
    // 미리 정의된 변환들
    FunctionalLogProcessor& transformToUpperCase() {
        return addTransformer([](AdvancedLogEntry entry) {
            std::transform(entry.message.begin(), entry.message.end(), 
                          entry.message.begin(), ::toupper);
            return entry;
        });
    }
    
    FunctionalLogProcessor& addMetadata(const std::string& key, const std::string& value) {
        return addTransformer([key, value](AdvancedLogEntry entry) {
            entry.metadata[key] = value;
            return entry;
        });
    }
    
    FunctionalLogProcessor& addDynamicMetadata(const std::string& key, 
                                              std::function<std::string(const AdvancedLogEntry&)> generator) {
        return addTransformer([key, generator](AdvancedLogEntry entry) {
            entry.metadata[key] = generator(entry);
            return entry;
        });
    }
    
    // 미리 정의된 액션들
    FunctionalLogProcessor& printToConsole(LogFormatter formatter = nullptr) {
        if (!formatter) {
            formatter = [](const AdvancedLogEntry& entry) {
                return "[" + entry.level + "] " + entry.message;
            };
        }
        
        return addAction([formatter](const AdvancedLogEntry& entry) {
            std::cout << formatter(entry) << std::endl;
        });
    }
    
    FunctionalLogProcessor& writeToFile(const std::string& filename, 
                                       LogFormatter formatter = nullptr) {
        if (!formatter) {
            formatter = [](const AdvancedLogEntry& entry) {
                auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
                return std::string(std::ctime(&time_t)).substr(0, 24) + " [" + 
                       entry.level + "] " + entry.message;
            };
        }
        
        return addAction([filename, formatter](const AdvancedLogEntry& entry) {
            static std::mutex file_mutex;
            std::lock_guard<std::mutex> lock(file_mutex);
            
            std::ofstream file(filename, std::ios::app);
            if (file.is_open()) {
                file << formatter(entry) << std::endl;
            }
        });
    }
    
    FunctionalLogProcessor& sendAlert(std::function<bool(const AdvancedLogEntry&)> condition,
                                     std::function<void(const AdvancedLogEntry&)> alert_action) {
        return addAction([condition, alert_action](const AdvancedLogEntry& entry) {
            if (condition(entry)) {
                alert_action(entry);
            }
        });
    }
    
    // 로그 처리 시작
    void start() {
        running_ = true;
        processor_thread_ = std::thread([this]() {
            processLogs();
        });
    }
    
    void stop() {
        running_ = false;
        queue_cv_.notify_all();
        if (processor_thread_.joinable()) {
            processor_thread_.join();
        }
    }
    
    // 로그 추가
    void submitLog(AdvancedLogEntry entry) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            log_queue_.push(std::move(entry));
        }
        queue_cv_.notify_one();
    }
    
    // 템플릿 메서드 패턴으로 로그 생성 간소화
    template<typename... Args>
    void log(const std::string& level, const std::string& message, 
             const std::string& source = "", Args&&... metadata_pairs) {
        AdvancedLogEntry entry(message, level, source);
        addMetadataPairs(entry, std::forward<Args>(metadata_pairs)...);
        submitLog(std::move(entry));
    }
    
private:
    // 메타데이터 쌍 추가 (재귀적)
    template<typename... Args>
    void addMetadataPairs(AdvancedLogEntry& entry, const std::string& key, 
                         const std::string& value, Args&&... rest) {
        entry.metadata[key] = value;
        if constexpr (sizeof...(rest) > 0) {
            addMetadataPairs(entry, std::forward<Args>(rest)...);
        }
    }
    
    void addMetadataPairs(AdvancedLogEntry& entry) {
        // 재귀 종료
    }
    
    void processLogs() {
        while (running_) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return !log_queue_.empty() || !running_; });
            
            while (!log_queue_.empty()) {
                auto entry = std::move(log_queue_.front());
                log_queue_.pop();
                lock.unlock();
                
                processLogEntry(entry);
                
                lock.lock();
            }
        }
    }
    
    void processLogEntry(AdvancedLogEntry entry) {
        // 필터 적용
        for (const auto& filter : filters_) {
            if (!filter(entry)) {
                return;  // 필터링됨
            }
        }
        
        // 변환 적용
        for (const auto& transformer : transformers_) {
            entry = transformer(entry);
        }
        
        // 액션 실행
        for (const auto& action : actions_) {
            try {
                action(entry);
            } catch (const std::exception& e) {
                std::cerr << "로그 액션 실행 중 오류: " << e.what() << std::endl;
            }
        }
    }
};

// 사용 예시 및 데모
class FunctionalLoggerDemo {
public:
    void demonstrateAdvancedLogging() {
        std::cout << "=== Advanced Functional Logging Demo ===" << std::endl;
        
        // 복잡한 처리 파이프라인 구성
        FunctionalLogProcessor processor;
        
        processor
            .filterByLevel({"ERROR", "WARN", "CRITICAL"})  // 중요한 로그만
            .addDynamicMetadata("processing_time", [](const auto&) {
                return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count());
            })
            .addMetadata("version", "1.0")
            .printToConsole([](const AdvancedLogEntry& entry) {
                std::string result = "[" + entry.level + "] " + entry.message;
                if (!entry.source.empty()) {
                    result += " (from: " + entry.source + ")";
                }
                if (!entry.metadata.empty()) {
                    result += " {";
                    for (const auto& [key, value] : entry.metadata) {
                        result += key + "=" + value + " ";
                    }
                    result += "}";
                }
                return result;
            })
            .writeToFile("important_logs.txt")
            .sendAlert(
                [](const AdvancedLogEntry& entry) { return entry.level == "CRITICAL"; },
                [](const AdvancedLogEntry& entry) {
                    std::cout << "🚨 CRITICAL ALERT: " << entry.message << std::endl;
                }
            );
        
        processor.start();
        
        // 테스트 로그들 생성
        processor.log("INFO", "서버 시작됨", "server");  // 필터링될 것
        processor.log("ERROR", "데이터베이스 연결 실패", "database", 
                     "retry_count", "3", "timeout", "5000");
        processor.log("WARN", "메모리 사용량 높음", "system", 
                     "usage", "85%");
        processor.log("CRITICAL", "시스템 다운", "server", 
                     "reason", "hardware_failure");
        
        // 처리 완료 대기
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        processor.stop();
    }
    
    // 실시간 로그 분석 예시
    void demonstrateRealTimeAnalysis() {
        std::cout << "\n=== Real-time Log Analysis ===" << std::endl;
        
        // 통계 수집을 위한 람다들
        std::map<std::string, int> level_counts;
        std::mutex stats_mutex;
        
        FunctionalLogProcessor analyzer;
        
        analyzer
            .addAction([&level_counts, &stats_mutex](const AdvancedLogEntry& entry) {
                std::lock_guard<std::mutex> lock(stats_mutex);
                level_counts[entry.level]++;
            })
            .addAction([&level_counts, &stats_mutex](const AdvancedLogEntry& entry) {
                // 주기적으로 통계 출력
                static int log_count = 0;
                if (++log_count % 5 == 0) {
                    std::lock_guard<std::mutex> lock(stats_mutex);
                    std::cout << "현재 통계: ";
                    for (const auto& [level, count] : level_counts) {
                        std::cout << level << "=" << count << " ";
                    }
                    std::cout << std::endl;
                }
            });
        
        analyzer.start();
        
        // 시뮬레이션된 로그 스트림
        std::thread log_generator([&analyzer]() {
            std::vector<std::string> levels = {"INFO", "WARN", "ERROR", "DEBUG"};
            std::vector<std::string> sources = {"web", "db", "cache", "auth"};
            
            for (int i = 0; i < 20; ++i) {
                std::string level = levels[i % levels.size()];
                std::string source = sources[i % sources.size()];
                std::string message = "메시지 #" + std::to_string(i);
                
                analyzer.submitLog(AdvancedLogEntry(message, level, source));
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
        
        log_generator.join();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        analyzer.stop();
        
        // 최종 통계
        std::cout << "최종 통계: ";
        for (const auto& [level, count] : level_counts) {
            std::cout << level << "=" << count << " ";
        }
        std::cout << std::endl;
    }
};

} // namespace LogCaster::Functional
```

---

## 🔧 5. 고급 함수형 기법

### Monads와 Optional Chaining

```cpp
#include <optional>
#include <variant>
#include <expected>  // C++23

// Maybe/Optional Monad 구현
template<typename T>
class Maybe {
private:
    std::optional<T> value_;
    
public:
    Maybe(T value) : value_(std::move(value)) {}
    Maybe(std::nullopt_t) : value_(std::nullopt) {}
    Maybe() : value_(std::nullopt) {}
    
    template<typename F>
    auto map(F&& f) -> Maybe<decltype(f(*value_))> {
        if (value_) {
            return Maybe<decltype(f(*value_))>(f(*value_));
        }
        return Maybe<decltype(f(*value_))>();
    }
    
    template<typename F>
    auto flatMap(F&& f) -> decltype(f(*value_)) {
        if (value_) {
            return f(*value_);
        }
        return decltype(f(*value_))();
    }
    
    bool hasValue() const { return value_.has_value(); }
    const T& getValue() const { return *value_; }
    T getValueOr(const T& defaultValue) const { return value_.value_or(defaultValue); }
};

class MonadicLogProcessor {
private:
    std::map<int, AdvancedLogEntry> log_storage_;
    int next_id_ = 1;
    
public:
    // Maybe를 반환하는 메서드들
    Maybe<int> addLog(const AdvancedLogEntry& entry) {
        if (entry.message.empty()) {
            return Maybe<int>();  // 실패
        }
        
        int id = next_id_++;
        log_storage_[id] = entry;
        return Maybe<int>(id);
    }
    
    Maybe<AdvancedLogEntry> getLog(int id) const {
        auto it = log_storage_.find(id);
        if (it != log_storage_.end()) {
            return Maybe<AdvancedLogEntry>(it->second);
        }
        return Maybe<AdvancedLogEntry>();
    }
    
    Maybe<std::string> getLogMessage(int id) const {
        return getLog(id).map([](const AdvancedLogEntry& entry) {
            return entry.message;
        });
    }
    
    Maybe<std::string> getFormattedLog(int id) const {
        return getLog(id)
            .map([](const AdvancedLogEntry& entry) {
                return "[" + entry.level + "] " + entry.message;
            });
    }
    
    // 체이닝 예시
    void demonstrateMonadicChaining() {
        // 로그 추가 -> 포맷팅 -> 출력 체인
        auto processLogChain = [this](const AdvancedLogEntry& entry) {
            return addLog(entry)  // Maybe<int> 반환
                .flatMap([this](int id) {  // int를 받아서 Maybe<std::string> 반환
                    return getFormattedLog(id);
                })
                .map([](const std::string& formatted) {  // std::string을 받아서 출력 후 bool 반환
                    std::cout << "처리됨: " << formatted << std::endl;
                    return true;
                });
        };
        
        // 성공 케이스
        auto result1 = processLogChain(AdvancedLogEntry("테스트 메시지", "INFO"));
        std::cout << "체이닝 성공: " << result1.hasValue() << std::endl;
        
        // 실패 케이스 (빈 메시지)
        auto result2 = processLogChain(AdvancedLogEntry("", "ERROR"));
        std::cout << "체이닝 실패: " << result2.hasValue() << std::endl;
    }
};
```

### Either/Result Monad

```cpp
// Either/Result 타입 구현 (성공/실패를 나타냄)
template<typename Error, typename Success>
class Result {
private:
    std::variant<Error, Success> value_;
    
public:
    Result(Error error) : value_(std::move(error)) {}
    Result(Success success) : value_(std::move(success)) {}
    
    bool isSuccess() const {
        return std::holds_alternative<Success>(value_);
    }
    
    bool isError() const {
        return std::holds_alternative<Error>(value_);
    }
    
    const Success& getSuccess() const {
        return std::get<Success>(value_);
    }
    
    const Error& getError() const {
        return std::get<Error>(value_);
    }
    
    template<typename F>
    auto map(F&& f) -> Result<Error, decltype(f(getSuccess()))> {
        if (isSuccess()) {
            return Result<Error, decltype(f(getSuccess()))>(f(getSuccess()));
        }
        return Result<Error, decltype(f(getSuccess()))>(getError());
    }
    
    template<typename F>
    auto flatMap(F&& f) -> decltype(f(getSuccess())) {
        if (isSuccess()) {
            return f(getSuccess());
        }
        return decltype(f(getSuccess()))(getError());
    }
    
    template<typename F>
    Result<Error, Success> mapError(F&& f) {
        if (isError()) {
            return Result<Error, Success>(f(getError()));
        }
        return *this;
    }
};

// LogCaster 에러 타입들
enum class LogError {
    InvalidMessage,
    StorageFull,
    NetworkError,
    PermissionDenied
};

class ResultBasedLogProcessor {
private:
    std::vector<AdvancedLogEntry> logs_;
    size_t max_capacity_ = 1000;
    
public:
    Result<LogError, size_t> addLog(const AdvancedLogEntry& entry) {
        if (entry.message.empty()) {
            return Result<LogError, size_t>(LogError::InvalidMessage);
        }
        
        if (logs_.size() >= max_capacity_) {
            return Result<LogError, size_t>(LogError::StorageFull);
        }
        
        logs_.push_back(entry);
        return Result<LogError, size_t>(logs_.size() - 1);
    }
    
    Result<LogError, AdvancedLogEntry> getLog(size_t index) {
        if (index >= logs_.size()) {
            return Result<LogError, AdvancedLogEntry>(LogError::InvalidMessage);
        }
        
        return Result<LogError, AdvancedLogEntry>(logs_[index]);
    }
    
    Result<LogError, std::string> processAndFormat(const AdvancedLogEntry& entry) {
        return addLog(entry)
            .flatMap([this](size_t index) {
                return getLog(index);
            })
            .map([](const AdvancedLogEntry& log) {
                return "[" + log.level + "] " + log.message + 
                       " @" + std::to_string(
                           std::chrono::duration_cast<std::chrono::milliseconds>(
                               log.timestamp.time_since_epoch()).count());
            });
    }
    
    void demonstrateResultChaining() {
        auto handleResult = [](const Result<LogError, std::string>& result) {
            if (result.isSuccess()) {
                std::cout << "성공: " << result.getSuccess() << std::endl;
            } else {
                std::cout << "실패: ";
                switch (result.getError()) {
                    case LogError::InvalidMessage:
                        std::cout << "잘못된 메시지";
                        break;
                    case LogError::StorageFull:
                        std::cout << "스토리지 가득참";
                        break;
                    case LogError::NetworkError:
                        std::cout << "네트워크 오류";
                        break;
                    case LogError::PermissionDenied:
                        std::cout << "권한 거부";
                        break;
                }
                std::cout << std::endl;
            }
        };
        
        // 성공 케이스
        auto result1 = processAndFormat(AdvancedLogEntry("정상 메시지", "INFO"));
        handleResult(result1);
        
        // 실패 케이스
        auto result2 = processAndFormat(AdvancedLogEntry("", "ERROR"));
        handleResult(result2);
        
        // 체이닝된 처리
        auto chainedProcess = [this](const std::vector<AdvancedLogEntry>& entries) {
            return std::transform_reduce(
                entries.begin(), entries.end(),
                Result<LogError, std::vector<std::string>>(std::vector<std::string>{}),
                [](Result<LogError, std::vector<std::string>> acc, 
                   Result<LogError, std::string> current) {
                    if (acc.isError()) return acc;
                    if (current.isError()) return Result<LogError, std::vector<std::string>>(current.getError());
                    
                    auto vec = acc.getSuccess();
                    vec.push_back(current.getSuccess());
                    return Result<LogError, std::vector<std::string>>(std::move(vec));
                },
                [this](const AdvancedLogEntry& entry) {
                    return processAndFormat(entry);
                }
            );
        };
        
        std::vector<AdvancedLogEntry> batch = {
            AdvancedLogEntry("메시지 1", "INFO"),
            AdvancedLogEntry("메시지 2", "WARN"),
            AdvancedLogEntry("", "ERROR")  // 이것 때문에 전체 배치 실패
        };
        
        auto batchResult = chainedProcess(batch);
        if (batchResult.isSuccess()) {
            std::cout << "배치 처리 성공, 결과 수: " << batchResult.getSuccess().size() << std::endl;
        } else {
            std::cout << "배치 처리 실패" << std::endl;
        }
    }
};
```

---

## 📊 6. 성능 고려사항과 베스트 프랙티스

### Lambda 성능 최적화

```cpp
#include <chrono>
#include <numeric>

class LambdaPerformanceGuide {
public:
    void demonstratePerformanceConsiderations() {
        const size_t SIZE = 1000000;
        std::vector<int> numbers(SIZE);
        std::iota(numbers.begin(), numbers.end(), 1);
        
        auto measure_time = [](const std::string& name, auto&& func) {
            auto start = std::chrono::high_resolution_clock::now();
            auto result = func();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << name << ": " << duration.count() << " μs, result: " << result << std::endl;
        };
        
        std::cout << "=== Lambda 성능 비교 ===" << std::endl;
        
        // 1. 함수 포인터 vs 람다
        auto lambda_sum = [](const std::vector<int>& vec) {
            return std::accumulate(vec.begin(), vec.end(), 0);
        };
        
        auto function_pointer_sum = +[](const std::vector<int>& vec) {
            return std::accumulate(vec.begin(), vec.end(), 0);
        };  // + 연산자로 함수 포인터로 변환
        
        measure_time("Lambda", [&]() { return lambda_sum(numbers); });
        measure_time("Function Pointer", [&]() { return function_pointer_sum(numbers); });
        
        // 2. 캡처 방식에 따른 성능 차이
        int multiplier = 2;
        
        // 값 캡처 (복사)
        auto value_capture = [multiplier](const std::vector<int>& vec) {
            return std::transform_reduce(vec.begin(), vec.end(), 0, std::plus<>(),
                [multiplier](int x) { return x * multiplier; });
        };
        
        // 참조 캡처
        auto ref_capture = [&multiplier](const std::vector<int>& vec) {
            return std::transform_reduce(vec.begin(), vec.end(), 0, std::plus<>(),
                [&multiplier](int x) { return x * multiplier; });
        };
        
        measure_time("Value Capture", [&]() { return value_capture(numbers); });
        measure_time("Reference Capture", [&]() { return ref_capture(numbers); });
        
        // 3. Generic vs Specific 람다
        auto generic_processor = [](const auto& container, auto predicate) {
            return std::count_if(container.begin(), container.end(), predicate);
        };
        
        auto specific_processor = [](const std::vector<int>& container, 
                                   std::function<bool(int)> predicate) {
            return std::count_if(container.begin(), container.end(), predicate);
        };
        
        auto is_even = [](int x) { return x % 2 == 0; };
        
        measure_time("Generic Lambda", [&]() { 
            return generic_processor(numbers, is_even); 
        });
        measure_time("Specific Lambda", [&]() { 
            return specific_processor(numbers, is_even); 
        });
    }
    
    void demonstrateBestPractices() {
        std::cout << "\n=== Lambda 베스트 프랙티스 ===" << std::endl;
        
        // ✅ GOOD: 작은 데이터는 값으로 캡처
        int small_value = 42;
        auto good_small_capture = [small_value](int x) {
            return x + small_value;
        };
        
        // ✅ GOOD: 큰 객체는 참조로 캡처
        std::vector<int> large_data(1000, 1);
        auto good_large_capture = [&large_data](int index) {
            return large_data[index % large_data.size()];
        };
        
        // ❌ BAD: 큰 객체를 값으로 캡처 (비효율적)
        // auto bad_large_capture = [large_data](int index) { ... };
        
        // ✅ GOOD: 필요한 변수만 선택적으로 캡처
        std::string prefix = "[LOG] ";
        int level = 2;
        bool debug_mode = true;
        
        auto selective_capture = [prefix, level](const std::string& msg) {
            // debug_mode은 사용하지 않으므로 캡처하지 않음
            return prefix + "Level " + std::to_string(level) + ": " + msg;
        };
        
        // ✅ GOOD: 이동 캡처로 소유권 전달 (C++14)
        std::unique_ptr<std::vector<int>> data = std::make_unique<std::vector<int>>(100);
        auto move_capture = [data = std::move(data)](size_t index) mutable {
            return (*data)[index % data->size()];
        };
        
        // ✅ GOOD: 람다를 변수에 저장할 때 auto 사용
        auto stored_lambda = [](int a, int b) { return a + b; };
        // std::function은 오버헤드가 있으므로 필요한 경우에만 사용
        
        // ✅ GOOD: 즉시 실행 람다 (IIFE - Immediately Invoked Function Expression)
        auto initialized_value = [&]() {
            if (debug_mode) {
                return "DEBUG: " + prefix;
            } else {
                return prefix;
            }
        }();
        
        std::cout << "IIFE 결과: " << initialized_value << std::endl;
        
        // ✅ GOOD: constexpr 람다 (C++17)
        constexpr auto compile_time_lambda = [](int x) constexpr {
            return x * x;
        };
        
        constexpr int result = compile_time_lambda(5);  // 컴파일 타임에 계산됨
        std::cout << "Constexpr 결과: " << result << std::endl;
    }
    
    void demonstrateAntiPatterns() {
        std::cout << "\n=== Lambda 안티패턴들 ===" << std::endl;
        
        // ❌ BAD: 불필요한 std::function 사용
        std::function<int(int, int)> bad_function = [](int a, int b) { return a + b; };
        
        // ✅ GOOD: auto 사용 (타입 추론)
        auto good_lambda = [](int a, int b) { return a + b; };
        
        // ❌ BAD: 과도한 캡처
        int a = 1, b = 2, c = 3, d = 4, e = 5;
        auto bad_capture = [=](int x) {  // 모든 변수를 불필요하게 캡처
            return x + a;  // a만 사용함
        };
        
        // ✅ GOOD: 필요한 변수만 캡처
        auto good_capture = [a](int x) {
            return x + a;
        };
        
        // ❌ BAD: 댕글링 레퍼런스
        std::function<int()> dangerous_lambda;
        {
            int local_var = 100;
            dangerous_lambda = [&local_var]() {  // 위험: local_var이 스코프를 벗어남
                return local_var;
            };
        }
        // dangerous_lambda() 호출 시 undefined behavior!
        
        // ✅ GOOD: 값 캡처로 안전하게
        std::function<int()> safe_lambda;
        {
            int local_var = 100;
            safe_lambda = [local_var]() {  // 안전: 값 복사
                return local_var;
            };
        }
        std::cout << "안전한 람다 결과: " << safe_lambda() << std::endl;
        
        // ❌ BAD: 재귀 람다 (복잡함)
        std::function<int(int)> bad_recursive = [&bad_recursive](int n) -> int {
            return (n <= 1) ? 1 : n * bad_recursive(n - 1);
        };
        
        // ✅ GOOD: 일반 함수 사용
        std::function<int(int)> factorial = [](int n) -> int {
            int result = 1;
            for (int i = 2; i <= n; ++i) {
                result *= i;
            }
            return result;
        };
        
        std::cout << "팩토리얼 5: " << factorial(5) << std::endl;
    }
};
```

---

## 📋 7. 체크리스트와 정리

### Lambda 활용 체크리스트

#### 기본 사용법
- [ ] Lambda 기본 문법 이해 `[capture](params) -> return { body }`
- [ ] 다양한 캡처 방식 활용 (`[=]`, `[&]`, `[var]`, `[&var]`)
- [ ] STL 알고리즘과 lambda 조합
- [ ] mutable keyword 필요시 사용

#### 고급 기법
- [ ] Generic lambdas (C++14) 활용
- [ ] 초기화 캡처와 이동 캡처 사용
- [ ] std::function과 lambda 조합
- [ ] Lambda를 반환하는 고차 함수 작성

#### 함수형 프로그래밍
- [ ] Map, Filter, Reduce 패턴 구현
- [ ] Function composition과 currying
- [ ] Monadic 패턴 (Optional, Either/Result)
- [ ] Pipeline 스타일 데이터 처리

#### 성능 최적화
- [ ] 적절한 캡처 방식 선택 (값 vs 참조)
- [ ] Generic lambda vs std::function 성능 고려
- [ ] 불필요한 캡처 제거
- [ ] IIFE와 constexpr lambda 활용

#### 안전성
- [ ] 댕글링 레퍼런스 방지
- [ ] 순환 참조 회피
- [ ] Exception safety 고려
- [ ] Thread safety 보장 (필요시)

### 핵심 포인트 정리

1. **Lambda는 익명 함수**: 일회성 또는 지역적 로직에 최적
2. **Capture 방식이 중요**: 성능과 안전성에 직결
3. **STL과의 조합**: 알고리즘 + lambda = 강력한 표현력
4. **함수형 패러다임**: 부수 효과 최소화, 조합 가능한 함수
5. **성능 고려**: auto > std::function, 선택적 캡처

---

*🔗 Lambda 함수는 현대 C++의 핵심 기능입니다. 함수형 프로그래밍 패러다임을 통해 더 표현력 있고 안전한 코드를 작성해보세요. LogCaster 프로젝트에서 이러한 기법들을 적극 활용하여 함수형 프로그래밍 전문가가 되어보세요!*