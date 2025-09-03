# 8. 시스템 프로그래밍 심화 🖥️
## 시그널, 파일 I/O, 정규표현식
*LogCaster 프로젝트를 위한 완벽 가이드*

---

## 📋 문서 정보
- **난이도**: 🟡 중급 → 🔴 고급
- **예상 학습 시간**: 10-12시간
- **전제 조건**: 
  - C/C++ 중급 이상
  - Unix/Linux 시스템 기초
  - 파일 디스크립터 이해
  - 프로세스 개념 이해
- **실습 환경**: Linux/macOS, GCC/G++, POSIX 호환 시스템

## 🎯 이 문서에서 배울 내용

시스템 프로그래밍은 운영체제와 직접 상호작용하는 프로그래밍입니다. LogCaster 서버를 구현할 때 필요한 핵심 시스템 프로그래밍 기법들을 배워보겠습니다:

🔹 **시그널 처리**: 서버의 우아한 종료와 시그널 안전성
🔹 **정규표현식**: 효율적인 로그 패턴 매칭과 검색
🔹 **파일 I/O**: 로그 지속성과 파일 로테이션
🔹 **시스템 자원 관리**: 메모리 매핑과 최적화

### 🎮 LogCaster에서의 활용
```
🖥️ LogCaster 시스템 레벨 기능들
├── 🛑 SIGINT/SIGTERM 처리 → 우아한 서버 종료
├── 🔍 정규표현식 검색 → "ERROR.*timeout" 패턴 매칭
├── 💾 로그 파일 저장 → 자동 백업과 로테이션
└── ⚡ 메모리 매핑 → 대용량 로그 빠른 접근
```

---

## 🛑 1. 시그널 처리 (Signal Handling)

### 🎯 시그널이란?

시그널은 운영체제가 프로세스에게 보내는 **비동기 알림**입니다. 마치 긴급 상황에서 울리는 사이렌 같은 역할을 하죠.

```
💡 실생활 비유: 시그널 = 긴급 전화
- SIGINT (Ctrl+C): "지금 당장 멈춰!"
- SIGTERM: "정리하고 천천히 종료해줘"  
- SIGKILL: "강제 종료!" (처리 불가)
```

### 🔧 기본 시그널 처리 - C 버전

```c
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// [SEQUENCE: 1] 전역 플래그로 서버 상태 관리
volatile sig_atomic_t server_running = 1;

// [SEQUENCE: 2] 시그널 핸들러 함수
void signal_handler(int signum) {
    switch(signum) {
        case SIGINT:
            printf("\n[INFO] SIGINT received. Graceful shutdown initiated...\n");
            server_running = 0;  // 서버 루프 종료 플래그
            break;
        case SIGTERM:
            printf("\n[INFO] SIGTERM received. Graceful shutdown initiated...\n");
            server_running = 0;
            break;
        default:
            printf("\n[WARNING] Unknown signal %d received\n", signum);
    }
}

// [SEQUENCE: 3] 시그널 핸들러 등록
void setup_signal_handlers() {
    // signal() 함수 사용 (간단하지만 제한적)
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // SIGPIPE 무시 (클라이언트 연결 끊김 시)
    signal(SIGPIPE, SIG_IGN);
}

// [SEQUENCE: 4] LogCaster 서버 메인 루프 예시
int main() {
    setup_signal_handlers();
    
    printf("LogCaster Server starting...\n");
    
    // 메인 서버 루프
    while (server_running) {
        // 1. 클라이언트 연결 수락
        // 2. 로그 수신 및 처리
        // 3. 쿼리 응답
        
        usleep(100000);  // 100ms 대기 (실제로는 select/poll 사용)
        printf(".");  // 서버 동작 표시
        fflush(stdout);
    }
    
    printf("\n[INFO] Server shutdown complete.\n");
    return 0;
}
```

### 🛡️ 고급 시그널 처리 - sigaction() 사용

```c
#include <signal.h>
#include <string.h>

// [SEQUENCE: 5] sigaction을 사용한 안전한 시그널 처리
void setup_advanced_signal_handlers() {
    struct sigaction sa;
    
    // 시그널 핸들러 설정 초기화
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    
    // SA_RESTART: 시그널로 인해 중단된 시스템 콜 자동 재시작
    sa.sa_flags = SA_RESTART;
    
    // 시그널 핸들러 실행 중 다른 시그널 차단
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGTERM);
    
    // 시그널 핸들러 등록
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(1);
    }
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM");
        exit(1);
    }
}
```

### ⚡ C++ 버전 시그널 처리

```cpp
#include <csignal>
#include <atomic>
#include <iostream>

class LogCasterServer {
private:
    // [SEQUENCE: 6] C++11 atomic을 사용한 스레드 안전한 플래그
    static std::atomic<bool> running_;
    
public:
    // 시그널 핸들러는 여전히 C 스타일 함수여야 함
    static void signalHandler(int signum) {
        switch(signum) {
            case SIGINT:
                std::cout << "\n[INFO] SIGINT received. Shutting down gracefully...\n";
                running_.store(false);
                break;
            case SIGTERM:
                std::cout << "\n[INFO] SIGTERM received. Shutting down gracefully...\n";
                running_.store(false);
                break;
        }
    }
    
    void setupSignalHandlers() {
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
        std::signal(SIGPIPE, SIG_IGN);
    }
    
    void run() {
        setupSignalHandlers();
        running_.store(true);
        
        std::cout << "LogCaster C++ Server starting...\n";
        
        while (running_.load()) {
            // 서버 로직
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "." << std::flush;
        }
        
        std::cout << "\n[INFO] Server shutdown complete.\n";
    }
};

// static 멤버 초기화
std::atomic<bool> LogCasterServer::running_{false};
```

### 🚨 시그널 안전성 (Signal Safety)

**중요한 원칙**: 시그널 핸들러에서는 **async-signal-safe** 함수만 사용해야 합니다!

```c
// ❌ 시그널 핸들러에서 사용하면 안 되는 함수들
void unsafe_signal_handler(int signum) {
    printf("Signal received!\n");     // ❌ printf는 signal-safe하지 않음
    malloc(100);                      // ❌ malloc은 signal-safe하지 않음
    fclose(file_ptr);                 // ❌ fclose는 signal-safe하지 않음
}

// ✅ 안전한 시그널 핸들러
void safe_signal_handler(int signum) {
    // write()는 async-signal-safe
    const char msg[] = "Signal received\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    
    // 단순한 변수 할당만 수행
    server_running = 0;
}
```

**Async-Signal-Safe 함수 목록 (일부):**
- `write()`, `read()`, `open()`, `close()`
- `signal()`, `sigaction()`
- 단순한 변수 할당 (`volatile sig_atomic_t` 권장)

---

## 🔍 2. 정규표현식 (Regular Expressions)

### 🎨 정규표현식이란?

정규표현식은 **텍스트 패턴을 표현하는 언어**입니다. LogCaster에서 로그를 검색할 때 매우 유용합니다.

```
💡 실생활 비유: 정규표현식 = 검색의 마법 주문
- "ERROR.*timeout" → "ERROR"로 시작하고 "timeout"으로 끝나는 모든 로그
- "[0-9]{4}-[0-9]{2}-[0-9]{2}" → 날짜 형식 (2024-07-28)
- "192\.168\.[0-9]+\.[0-9]+" → IP 주소 패턴
```

### 🔧 C에서 POSIX 정규표현식 사용

```c
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// [SEQUENCE: 7] LogCaster 로그 패턴 매칭 함수
typedef struct {
    regex_t compiled_regex;
    char pattern[256];
    int is_compiled;
} log_pattern_t;

// [SEQUENCE: 8] 정규표현식 컴파일
int compile_log_pattern(log_pattern_t* pattern, const char* regex_str) {
    strncpy(pattern->pattern, regex_str, sizeof(pattern->pattern) - 1);
    
    // REG_EXTENDED: 확장 정규표현식 사용
    // REG_ICASE: 대소문자 구분 안 함
    int flags = REG_EXTENDED | REG_ICASE;
    
    int result = regcomp(&pattern->compiled_regex, regex_str, flags);
    
    if (result != 0) {
        char error_buffer[256];
        regerror(result, &pattern->compiled_regex, error_buffer, sizeof(error_buffer));
        printf("Regex compilation failed: %s\n", error_buffer);
        pattern->is_compiled = 0;
        return -1;
    }
    
    pattern->is_compiled = 1;
    return 0;
}

// [SEQUENCE: 9] 로그 메시지 패턴 매칭
int match_log_pattern(const log_pattern_t* pattern, const char* log_message) {
    if (!pattern->is_compiled) {
        return -1;
    }
    
    regmatch_t matches[10];  // 최대 10개 그룹 매칭
    
    int result = regexec(&pattern->compiled_regex, log_message, 10, matches, 0);
    
    if (result == 0) {
        printf("[MATCH] Pattern '%s' found in: %s\n", pattern->pattern, log_message);
        
        // 매칭된 그룹들 출력 (선택사항)
        for (int i = 0; i < 10 && matches[i].rm_so != -1; i++) {
            int start = matches[i].rm_so;
            int end = matches[i].rm_eo;
            printf("  Group %d: %.*s\n", i, end - start, log_message + start);
        }
        
        return 1;  // 매칭됨
    } else if (result == REG_NOMATCH) {
        return 0;  // 매칭되지 않음
    } else {
        char error_buffer[256];
        regerror(result, &pattern->compiled_regex, error_buffer, sizeof(error_buffer));
        printf("Regex execution error: %s\n", error_buffer);
        return -1;  // 오류
    }
}

// [SEQUENCE: 10] 리소스 정리
void free_log_pattern(log_pattern_t* pattern) {
    if (pattern->is_compiled) {
        regfree(&pattern->compiled_regex);
        pattern->is_compiled = 0;
    }
}

// [SEQUENCE: 11] LogCaster 검색 예제
void logcaster_search_example() {
    log_pattern_t error_pattern;
    log_pattern_t ip_pattern;
    
    // ERROR 관련 로그 검색 패턴
    compile_log_pattern(&error_pattern, "ERROR.*connection.*timeout");
    
    // IP 주소 추출 패턴 (그룹 캡처 사용)
    compile_log_pattern(&ip_pattern, "([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})");
    
    // 테스트 로그 메시지들
    const char* test_logs[] = {
        "2024-07-28 10:30:15 ERROR: Database connection timeout after 30 seconds",
        "2024-07-28 10:31:22 INFO: Client connected from 192.168.1.100",
        "2024-07-28 10:32:45 WARNING: High memory usage detected",
        "2024-07-28 10:33:10 ERROR: Network connection timeout, retrying...",
        NULL
    };
    
    printf("=== LogCaster Regex Search Demo ===\n");
    
    for (int i = 0; test_logs[i] != NULL; i++) {
        printf("\nTesting log: %s\n", test_logs[i]);
        
        if (match_log_pattern(&error_pattern, test_logs[i])) {
            printf("  → Matches ERROR pattern!\n");
        }
        
        if (match_log_pattern(&ip_pattern, test_logs[i])) {
            printf("  → Contains IP address!\n");
        }
    }
    
    // 메모리 정리
    free_log_pattern(&error_pattern);
    free_log_pattern(&ip_pattern);
}
```

### ⚡ C++에서 std::regex 사용

```cpp
#include <regex>
#include <string>
#include <vector>
#include <iostream>

class LogPatternMatcher {
private:
    std::regex pattern_;
    std::string pattern_str_;
    
public:
    // [SEQUENCE: 12] C++ 정규표현식 생성자
    LogPatternMatcher(const std::string& pattern) 
        : pattern_str_(pattern) {
        try {
            // std::regex_constants::icase: 대소문자 구분 안 함
            pattern_ = std::regex(pattern, std::regex_constants::icase);
        } catch (const std::regex_error& e) {
            std::cerr << "Regex compilation error: " << e.what() << std::endl;
            throw;
        }
    }
    
    // [SEQUENCE: 13] 단순 매칭 확인
    bool matches(const std::string& text) const {
        return std::regex_search(text, pattern_);
    }
    
    // [SEQUENCE: 14] 매칭 결과와 그룹 추출
    std::vector<std::string> extractMatches(const std::string& text) const {
        std::vector<std::string> results;
        std::smatch match_results;
        
        if (std::regex_search(text, match_results, pattern_)) {
            for (const auto& match : match_results) {
                results.push_back(match.str());
            }
        }
        
        return results;
    }
    
    // [SEQUENCE: 15] 모든 매칭 항목 찾기
    std::vector<std::string> findAll(const std::string& text) const {
        std::vector<std::string> results;
        std::sregex_iterator iter(text.begin(), text.end(), pattern_);
        std::sregex_iterator end;
        
        for (; iter != end; ++iter) {
            results.push_back(iter->str());
        }
        
        return results;
    }
    
    const std::string& getPattern() const { return pattern_str_; }
};

// [SEQUENCE: 16] LogCaster C++ 검색 시스템
class LogSearchEngine {
private:
    std::vector<std::string> logs_;
    
public:
    void addLog(const std::string& log) {
        logs_.push_back(log);
    }
    
    std::vector<std::string> search(const std::string& pattern) const {
        std::vector<std::string> matches;
        
        try {
            LogPatternMatcher matcher(pattern);
            
            for (const auto& log : logs_) {
                if (matcher.matches(log)) {
                    matches.push_back(log);
                }
            }
        } catch (const std::regex_error& e) {
            std::cerr << "Search error: " << e.what() << std::endl;
        }
        
        return matches;
    }
    
    // IP 주소만 추출하는 특별한 함수
    std::vector<std::string> extractIPs() const {
        std::vector<std::string> ips;
        LogPatternMatcher ip_matcher(R"(\b(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\b)");
        
        for (const auto& log : logs_) {
            auto found_ips = ip_matcher.findAll(log);
            ips.insert(ips.end(), found_ips.begin(), found_ips.end());
        }
        
        return ips;
    }
};

// [SEQUENCE: 17] 사용 예제
void cpp_regex_example() {
    LogSearchEngine search_engine;
    
    // 테스트 로그 추가
    search_engine.addLog("2024-07-28 10:30:15 ERROR: Database connection timeout from 192.168.1.100");
    search_engine.addLog("2024-07-28 10:31:22 INFO: Client connected from 10.0.0.45");
    search_engine.addLog("2024-07-28 10:32:45 WARNING: High memory usage detected");
    search_engine.addLog("2024-07-28 10:33:10 ERROR: Network timeout, server 172.16.0.1 unreachable");
    
    std::cout << "=== LogCaster C++ Regex Search Demo ===\n\n";
    
    // ERROR 로그 검색
    auto error_logs = search_engine.search("ERROR.*timeout");
    std::cout << "ERROR timeout logs:\n";
    for (const auto& log : error_logs) {
        std::cout << "  " << log << "\n";
    }
    
    // 모든 IP 주소 추출
    auto ips = search_engine.extractIPs();
    std::cout << "\nExtracted IP addresses:\n";
    for (const auto& ip : ips) {
        std::cout << "  " << ip << "\n";
    }
}
```

### 📝 자주 사용하는 LogCaster 패턴들

```cpp
// [SEQUENCE: 18] LogCaster에서 유용한 정규표현식 패턴들
const std::map<std::string, std::string> LOGCRAFTER_PATTERNS = {
    // 로그 레벨 매칭
    {"error_logs", R"(ERROR|FATAL|CRITICAL)"},
    {"warning_logs", R"(WARN|WARNING)"},
    {"info_logs", R"(INFO|DEBUG|TRACE)"},
    
    // 시간 패턴
    {"timestamp", R"(\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2})"},
    {"time_range", R"(1[0-2]:[0-5][0-9]:[0-5][0-9])"},  // 10:00:00~12:59:59
    
    // 네트워크 관련
    {"ip_address", R"(\b(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\b)"},
    {"port_number", R"(:(\d{1,5})\b)"},
    {"url", R"(https?://[^\s]+)"},
    
    // 오류 패턴
    {"connection_error", R"(connection.*(?:timeout|refused|failed))"},
    {"memory_error", R"(out of memory|memory.*(?:leak|exhausted))"},
    {"disk_error", R"(disk.*(?:full|space|write.*failed))"},
    
    // 성능 관련
    {"slow_query", R"(query.*took.*(?:[5-9]\d\d+|\d{4,}).*ms)"},  // 500ms 이상
    {"high_cpu", R"(cpu.*(?:[8-9]\d|100)%)"},  // 80% 이상
    
    // 보안 관련
    {"failed_login", R"((?:login|authentication).*(?:failed|denied))"},
    {"suspicious_ip", R"((?:attack|intrusion|suspicious).*\d+\.\d+\.\d+\.\d+)"}
};
```

---

## 💾 3. 파일 I/O 및 지속성

### 📁 LogCaster 파일 저장 시스템

로그를 메모리에만 저장하면 서버가 재시작될 때 모든 데이터가 사라집니다. 파일 시스템을 활용해서 로그를 영구적으로 보관해보겠습니다.

### 🔧 C 버전 파일 I/O

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

// [SEQUENCE: 19] LogCaster 파일 관리 구조체
typedef struct {
    char base_filename[256];     // 기본 파일명
    char current_filename[256];  // 현재 사용 중인 파일명
    FILE* current_file;          // 현재 파일 포인터
    size_t max_file_size;        // 최대 파일 크기 (바이트)
    size_t current_file_size;    // 현재 파일 크기
    int rotation_count;          // 로테이션 카운터
} log_file_manager_t;

// [SEQUENCE: 20] 파일 매니저 초기화
int init_log_file_manager(log_file_manager_t* manager, 
                         const char* base_filename, 
                         size_t max_size) {
    strncpy(manager->base_filename, base_filename, sizeof(manager->base_filename) - 1);
    manager->max_file_size = max_size;
    manager->current_file_size = 0;
    manager->rotation_count = 0;
    manager->current_file = NULL;
    
    // 첫 번째 파일 생성
    snprintf(manager->current_filename, sizeof(manager->current_filename),
             "%s.%d.log", manager->base_filename, manager->rotation_count);
    
    manager->current_file = fopen(manager->current_filename, "a");
    if (!manager->current_file) {
        perror("Failed to open log file");
        return -1;
    }
    
    // 기존 파일 크기 확인
    fseek(manager->current_file, 0, SEEK_END);
    manager->current_file_size = ftell(manager->current_file);
    
    printf("[INFO] Log file manager initialized: %s (current size: %zu bytes)\n",
           manager->current_filename, manager->current_file_size);
    
    return 0;
}

// [SEQUENCE: 21] 파일 로테이션 수행
int rotate_log_file(log_file_manager_t* manager) {
    // 현재 파일 닫기
    if (manager->current_file) {
        fclose(manager->current_file);
        manager->current_file = NULL;
    }
    
    printf("[INFO] Rotating log file %s (size: %zu bytes)\n",
           manager->current_filename, manager->current_file_size);
    
    // 새 파일 생성
    manager->rotation_count++;
    snprintf(manager->current_filename, sizeof(manager->current_filename),
             "%s.%d.log", manager->base_filename, manager->rotation_count);
    
    manager->current_file = fopen(manager->current_filename, "a");
    if (!manager->current_file) {
        perror("Failed to create new log file");
        return -1;
    }
    
    manager->current_file_size = 0;
    
    printf("[INFO] New log file created: %s\n", manager->current_filename);
    return 0;
}

// [SEQUENCE: 22] 로그 메시지 파일에 쓰기
int write_log_to_file(log_file_manager_t* manager, const char* log_message) {
    if (!manager->current_file) {
        return -1;
    }
    
    // 현재 시간 가져오기
    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);
    
    // 타임스탬프 포함한 로그 포맷
    char formatted_log[1024];
    int written = snprintf(formatted_log, sizeof(formatted_log),
                          "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
                          local_time->tm_year + 1900,
                          local_time->tm_mon + 1,
                          local_time->tm_mday,
                          local_time->tm_hour,
                          local_time->tm_min,
                          local_time->tm_sec,
                          log_message);
    
    // 파일에 쓰기
    size_t bytes_written = fwrite(formatted_log, 1, written, manager->current_file);
    fflush(manager->current_file);  // 즉시 디스크에 쓰기
    
    manager->current_file_size += bytes_written;
    
    // 파일 크기 확인 후 로테이션 필요 시 수행
    if (manager->current_file_size >= manager->max_file_size) {
        return rotate_log_file(manager);
    }
    
    return 0;
}

// [SEQUENCE: 23] 로그 파일에서 검색
int search_log_files(const log_file_manager_t* manager, const char* search_term) {
    char search_pattern[512];
    snprintf(search_pattern, sizeof(search_pattern), "%s.*.log", manager->base_filename);
    
    printf("=== Searching for '%s' in log files ===\n", search_term);
    
    // 간단한 grep 명령어 실행 (실제로는 직접 파일을 읽어서 처리)
    char command[1024];
    snprintf(command, sizeof(command), "grep -n '%s' %s", search_term, search_pattern);
    
    int result = system(command);
    
    if (result == 0) {
        printf("Search completed successfully.\n");
    } else {
        printf("No matches found or search failed.\n");
    }
    
    return result;
}

// [SEQUENCE: 24] 리소스 정리
void cleanup_log_file_manager(log_file_manager_t* manager) {
    if (manager->current_file) {
        fclose(manager->current_file);
        manager->current_file = NULL;
    }
    
    printf("[INFO] Log file manager cleaned up.\n");
}

// [SEQUENCE: 25] 사용 예제
void c_file_io_example() {
    log_file_manager_t manager;
    
    // 1MB 크기 제한으로 로그 파일 매니저 초기화
    if (init_log_file_manager(&manager, "logcaster", 1024 * 1024) != 0) {
        return;
    }
    
    // 테스트 로그 메시지들
    const char* test_messages[] = {
        "Server started successfully",
        "Client connected from 192.168.1.100",
        "ERROR: Database connection failed",
        "WARNING: High memory usage detected",
        "Client disconnected",
        NULL
    };
    
    // 로그 메시지들을 파일에 저장
    for (int i = 0; test_messages[i] != NULL; i++) {
        write_log_to_file(&manager, test_messages[i]);
        usleep(100000);  // 100ms 대기
    }
    
    // 파일에서 ERROR 검색
    search_log_files(&manager, "ERROR");
    
    // 정리
    cleanup_log_file_manager(&manager);
}
```

### ⚡ C++ 버전 파일 I/O

```cpp
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

class LogFileManager {
private:
    std::string base_filename_;
    std::string current_filename_;
    std::ofstream current_file_;
    std::size_t max_file_size_;
    std::size_t current_file_size_;
    int rotation_count_;
    
    // [SEQUENCE: 26] 타임스탬프 생성
    std::string getCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
    
    // [SEQUENCE: 27] 새 파일명 생성
    std::string generateFilename() const {
        return base_filename_ + "." + std::to_string(rotation_count_) + ".log";
    }
    
public:
    // [SEQUENCE: 28] 생성자
    LogFileManager(const std::string& base_filename, std::size_t max_size = 1024 * 1024)
        : base_filename_(base_filename)
        , max_file_size_(max_size)
        , current_file_size_(0)
        , rotation_count_(0) {
        
        openNewFile();
    }
    
    // [SEQUENCE: 29] 소멸자 (RAII)
    ~LogFileManager() {
        if (current_file_.is_open()) {
            current_file_.close();
        }
    }
    
    // [SEQUENCE: 30] 새 파일 열기
    bool openNewFile() {
        if (current_file_.is_open()) {
            current_file_.close();
        }
        
        current_filename_ = generateFilename();
        current_file_.open(current_filename_, std::ios::app);
        
        if (!current_file_.is_open()) {
            std::cerr << "Failed to open log file: " << current_filename_ << std::endl;
            return false;
        }
        
        // 기존 파일 크기 확인
        if (std::filesystem::exists(current_filename_)) {
            current_file_size_ = std::filesystem::file_size(current_filename_);
        } else {
            current_file_size_ = 0;
        }
        
        std::cout << "[INFO] Opened log file: " << current_filename_ 
                  << " (current size: " << current_file_size_ << " bytes)\n";
        
        return true;
    }
    
    // [SEQUENCE: 31] 파일 로테이션
    bool rotateFile() {
        std::cout << "[INFO] Rotating log file " << current_filename_ 
                  << " (size: " << current_file_size_ << " bytes)\n";
        
        current_file_.close();
        rotation_count_++;
        
        return openNewFile();
    }
    
    // [SEQUENCE: 32] 로그 메시지 쓰기
    bool writeLog(const std::string& message) {
        if (!current_file_.is_open()) {
            return false;
        }
        
        std::string formatted_log = "[" + getCurrentTimestamp() + "] " + message + "\n";
        
        current_file_ << formatted_log;
        current_file_.flush();  // 즉시 디스크에 쓰기
        
        current_file_size_ += formatted_log.length();
        
        // 파일 크기 확인 후 로테이션
        if (current_file_size_ >= max_file_size_) {
            return rotateFile();
        }
        
        return true;
    }
    
    // [SEQUENCE: 33] 로그 파일에서 검색 (C++ regex 사용)
    std::vector<std::string> searchLogs(const std::string& pattern) const {
        std::vector<std::string> results;
        
        try {
            std::regex search_regex(pattern, std::regex_constants::icase);
            
            // 모든 로그 파일 검색
            for (int i = 0; i <= rotation_count_; i++) {
                std::string filename = base_filename_ + "." + std::to_string(i) + ".log";
                
                if (!std::filesystem::exists(filename)) {
                    continue;
                }
                
                std::ifstream file(filename);
                std::string line;
                int line_number = 1;
                
                while (std::getline(file, line)) {
                    if (std::regex_search(line, search_regex)) {
                        results.push_back(filename + ":" + std::to_string(line_number) + ": " + line);
                    }
                    line_number++;
                }
            }
        } catch (const std::regex_error& e) {
            std::cerr << "Regex error: " << e.what() << std::endl;
        }
        
        return results;
    }
    
    // [SEQUENCE: 34] 파일 정보 조회
    void printFileInfo() const {
        std::cout << "=== LogCaster File Manager Status ===\n";
        std::cout << "Base filename: " << base_filename_ << "\n";
        std::cout << "Current file: " << current_filename_ << "\n";
        std::cout << "Current size: " << current_file_size_ << " bytes\n";
        std::cout << "Max size: " << max_file_size_ << " bytes\n";
        std::cout << "Rotation count: " << rotation_count_ << "\n";
        
        // 모든 로그 파일 나열
        std::cout << "Existing log files:\n";
        for (int i = 0; i <= rotation_count_; i++) {
            std::string filename = base_filename_ + "." + std::to_string(i) + ".log";
            if (std::filesystem::exists(filename)) {
                auto size = std::filesystem::file_size(filename);
                std::cout << "  " << filename << " (" << size << " bytes)\n";
            }
        }
    }
};

// [SEQUENCE: 35] 사용 예제
void cpp_file_io_example() {
    LogFileManager log_manager("logcaster_cpp", 1024);  // 1KB 제한 (테스트용)
    
    // 테스트 로그 메시지들
    std::vector<std::string> test_messages = {
        "Server started successfully",
        "Client connected from 192.168.1.100",
        "ERROR: Database connection failed",
        "WARNING: High memory usage detected (85%)",
        "Processing user request: login attempt",
        "ERROR: Authentication failed for user 'hacker'",
        "Client disconnected gracefully",
        "Server performance: 1000 req/sec",
        "DEBUG: Memory usage: 512MB",
        "INFO: Daily backup completed successfully"
    };
    
    // 로그 메시지들을 파일에 저장
    for (const auto& message : test_messages) {
        log_manager.writeLog(message);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // 파일 정보 출력
    log_manager.printFileInfo();
    
    // ERROR 로그 검색
    std::cout << "\n=== Searching for ERROR logs ===\n";
    auto error_results = log_manager.searchLogs("ERROR");
    for (const auto& result : error_results) {
        std::cout << result << "\n";
    }
    
    // 메모리 관련 로그 검색
    std::cout << "\n=== Searching for memory-related logs ===\n";
    auto memory_results = log_manager.searchLogs("memory|Memory");
    for (const auto& result : memory_results) {
        std::cout << result << "\n";
    }
}
```

### 🗂️ 메모리 매핑을 사용한 고성능 파일 I/O

대용량 로그 파일을 빠르게 처리하기 위해 메모리 매핑(mmap)을 사용할 수 있습니다:

```c
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// [SEQUENCE: 36] 메모리 매핑을 사용한 로그 파일 읽기
typedef struct {
    char* mapped_data;
    size_t file_size;
    int fd;
} mmap_log_reader_t;

// [SEQUENCE: 37] 로그 파일을 메모리에 매핑
int mmap_open_log_file(mmap_log_reader_t* reader, const char* filename) {
    reader->fd = open(filename, O_RDONLY);
    if (reader->fd == -1) {
        perror("open");
        return -1;
    }
    
    struct stat file_stat;
    if (fstat(reader->fd, &file_stat) == -1) {
        perror("fstat");
        close(reader->fd);
        return -1;
    }
    
    reader->file_size = file_stat.st_size;
    
    reader->mapped_data = mmap(NULL, reader->file_size, PROT_READ, MAP_PRIVATE, reader->fd, 0);
    if (reader->mapped_data == MAP_FAILED) {
        perror("mmap");
        close(reader->fd);
        return -1;
    }
    
    printf("[INFO] Memory mapped log file: %s (%zu bytes)\n", filename, reader->file_size);
    return 0;
}

// [SEQUENCE: 38] 메모리 매핑된 파일에서 빠른 검색
int mmap_search_logs(const mmap_log_reader_t* reader, const char* search_term) {
    if (!reader->mapped_data) {
        return -1;
    }
    
    printf("Searching for '%s' in memory-mapped file...\n", search_term);
    
    const char* current = reader->mapped_data;
    const char* end = reader->mapped_data + reader->file_size;
    const char* line_start = current;
    int line_number = 1;
    int matches = 0;
    
    while (current < end) {
        if (*current == '\n') {
            // 한 줄 완성, 검색 수행
            size_t line_length = current - line_start;
            
            if (line_length > 0 && memmem(line_start, line_length, search_term, strlen(search_term))) {
                printf("Line %d: %.*s\n", line_number, (int)line_length, line_start);
                matches++;
            }
            
            line_start = current + 1;
            line_number++;
        }
        current++;
    }
    
    printf("Found %d matches.\n", matches);
    return matches;
}

// [SEQUENCE: 39] 메모리 매핑 해제
void mmap_close_log_file(mmap_log_reader_t* reader) {
    if (reader->mapped_data && reader->mapped_data != MAP_FAILED) {
        munmap(reader->mapped_data, reader->file_size);
        reader->mapped_data = NULL;
    }
    
    if (reader->fd != -1) {
        close(reader->fd);
        reader->fd = -1;
    }
}
```

---

## ⚠️ 자주 하는 실수와 해결법

### 1. 시그널 핸들러에서 안전하지 않은 함수 사용
```c
// ❌ 잘못된 예시
void signal_handler(int signum) {
    printf("Signal %d received\n", signum);  // printf는 signal-safe하지 않음!
    exit(0);  // exit()도 signal-safe하지 않음!
}

// ✅ 올바른 예시
volatile sig_atomic_t g_shutdown = 0;
void signal_handler(int signum) {
    g_shutdown = 1;  // 플래그만 설정
    // 또는 write() 사용
    const char msg[] = "Signal received\n";
    write(STDERR_FILENO, msg, sizeof(msg)-1);
}
```

### 2. 정규표현식 컴파일 실패 처리 안 함
```c
// ❌ 잘못된 예시
regex_t regex;
regcomp(&regex, pattern, REG_EXTENDED);  // 에러 체크 없음!
regexec(&regex, text, 0, NULL, 0);

// ✅ 올바른 예시
regex_t regex;
int ret = regcomp(&regex, pattern, REG_EXTENDED);
if (ret != 0) {
    char error_msg[256];
    regerror(ret, &regex, error_msg, sizeof(error_msg));
    fprintf(stderr, "Regex error: %s\n", error_msg);
    return -1;
}
```

### 3. 파일 디스크립터 누수
```c
// ❌ 잘못된 예시
FILE* fp = fopen("log.txt", "r");
if (some_error_condition) {
    return -1;  // fclose() 없이 리턴!
}

// ✅ 올바른 예시
FILE* fp = fopen("log.txt", "r");
if (!fp) return -1;

if (some_error_condition) {
    fclose(fp);
    return -1;
}
// 또는 cleanup 레이블 사용
```

### 4. mmap 후 munmap 누락
```c
// ❌ 잘못된 예시
void* data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
// 사용 후 munmap 없이 종료!

// ✅ 올바른 예시
void* data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
if (data == MAP_FAILED) {
    perror("mmap");
    return -1;
}
// 사용
munmap(data, size);  // 반드시 해제
```

### 5. 로그 로테이션 시 경쟁 조건
```c
// ❌ 잘못된 예시 - 멀티스레드 환경에서 위험
if (file_size > MAX_SIZE) {
    fclose(log_file);  // 다른 스레드가 쓰기 시도할 수 있음!
    rename("log.txt", "log.old");
    log_file = fopen("log.txt", "w");
}

// ✅ 올바른 예시 - 동기화 사용
pthread_mutex_lock(&log_mutex);
if (file_size > MAX_SIZE) {
    rotate_log_file_safe();
}
pthread_mutex_unlock(&log_mutex);
```

## 🎯 실습 프로젝트

### 프로젝트 1: 고급 로그 분석기 (난이도: ⭐⭐⭐)
```c
// 구현할 기능:
// 1. 실시간 로그 모니터링 (tail -f 같은)
// 2. 정규표현식 필터링
// 3. 통계 수집 (에러율, 응답시간 등)
// 4. 알림 기능 (특정 패턴 감지 시)

// 도전 과제:
// - inotify로 파일 변경 감지
// - 멀티스레드 처리
// - 대용량 파일 처리
```

### 프로젝트 2: 프로세스 감시 도구 (난이도: ⭐⭐⭐⭐)
```c
// 구현할 기능:
// 1. 자식 프로세스 모니터링
// 2. 시그널 전달 및 관리
// 3. 리소스 사용량 추적
// 4. 크래시 덤프 수집

// 고급 기능:
// - ptrace 사용
// - 코어 덤프 분석
// - 프로세스 재시작
```

### 프로젝트 3: 분산 로그 수집 시스템 (난이도: ⭐⭐⭐⭐⭐)
```c
// 구현할 기능:
// 1. 여러 서버의 로그 수집
// 2. 중앙 집중식 저장
// 3. 실시간 검색 및 분석
// 4. 장애 복구

// 확장 기능:
// - 로그 압축/암호화
// - 분산 검색
// - 웹 인터페이스
```

## ✅ 학습 체크리스트

### 🟢 기초 (1-2주)
- [ ] 기본 시그널 이해와 처리
- [ ] signal() vs sigaction()
- [ ] 기본 파일 I/O (fopen, fread, fwrite)
- [ ] 간단한 정규표현식 패턴
- [ ] 로그 파일 생성과 읽기

### 🟡 중급 (3-4주)
- [ ] 시그널 마스킹과 블로킹
- [ ] 비동기 시그널 안전성
- [ ] 고급 파일 I/O (mmap, sendfile)
- [ ] 복잡한 정규표현식 패턴
- [ ] 로그 로테이션 구현
- [ ] inotify 파일 모니터링

### 🔴 고급 (5-8주)
- [ ] 실시간 시그널 (sigqueue)
- [ ] signalfd와 이벤트 루프
- [ ] 락 없는 로그 버퍼
- [ ] 정규표현식 최적화
- [ ] 분산 로그 시스템
- [ ] 성능 프로파일링

### ⚫ 전문가 (3개월+)
- [ ] 커널 모듈 시그널 처리
- [ ] Zero-copy 파일 전송
- [ ] 커스텀 정규표현식 엔진
- [ ] 로그 압축 알고리즘
- [ ] 실시간 로그 분석 엔진

## 📚 추가 학습 자료

### 권장 도서
1. **"Advanced Programming in the UNIX Environment"** - W. Richard Stevens
2. **"The Linux Programming Interface"** - Michael Kerrisk
3. **"Mastering Regular Expressions"** - Jeffrey Friedl

### 온라인 리소스
- [Linux man pages](https://man7.org/linux/man-pages/)
- [POSIX.1-2017 specification](https://pubs.opengroup.org/onlinepubs/9699919799/)
- [Regular-Expressions.info](https://www.regular-expressions.info/)

### 실습 도구
- **strace**: 시스템 콜 추적
- **ltrace**: 라이브러리 호출 추적
- **gdb**: 디버거
- **valgrind**: 메모리 디버깅

## ✅ 4. 다음 단계 준비

이 문서를 완전히 이해했다면:

1. **시그널 처리**로 LogCaster 서버의 우아한 종료를 구현할 수 있어야 합니다
2. **정규표현식**을 사용하여 복잡한 로그 패턴 검색이 가능해야 합니다
3. **파일 I/O**와 **로그 로테이션**으로 영구 저장소를 관리할 수 있어야 합니다
4. **메모리 매핑**을 통한 고성능 파일 처리를 이해해야 합니다

### 🎯 확인 문제

1. SIGINT와 SIGTERM의 차이점은 무엇이며, LogCaster에서 각각 어떻게 처리해야 할까요?

2. 정규표현식 `"ERROR.*connection.*(?:timeout|refused)"` 는 어떤 로그 메시지와 매칭될까요?

3. 로그 파일이 1GB를 넘었을 때 어떤 방식으로 로테이션을 수행하는 것이 좋을까요?

다음 문서에서는 **멀티스레딩과 동시성**에 대해 자세히 알아보겠습니다!

---

*💡 팁: 시스템 프로그래밍은 운영체제와의 대화입니다. 각 기능이 왜 필요한지, 언제 사용해야 하는지를 이해하는 것이 중요합니다!*