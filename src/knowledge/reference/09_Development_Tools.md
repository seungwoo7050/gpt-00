# 14. 개발 도구 🛠️
## 프로젝트 관리부터 배포까지 실무 가이드

---

## 📋 문서 정보

- **난이도**: 중급-고급 (Intermediate-Advanced)
- **예상 학습 시간**: 10-12시간
- **필요 선수 지식**: 
  - C/C++ 기본 문법
  - Linux 기본 명령어
  - 기본적인 네트워크 개념
  - Git 기초
- **실습 환경**: Linux/Unix 환경, CMake, GCC/Clang

---

## 🎯 이 문서에서 배울 내용

이 문서는 LogCaster 프로젝트 개발 시 필수적인 실무 도구들과 모범 사례를 다룹니다. 디버깅, 프로파일링, 테스트, 크로스 플랫폼 개발 등 실제 프로덕션 환경에서 필요한 핵심 기술들을 설명합니다.

---

## 1. 메모리 디버깅 도구

C/C++에서 메모리 관련 버그는 가장 흔하고 위험한 문제입니다. LogCaster와 같은 서버 애플리케이션에서는 메모리 누수나 잘못된 메모리 접근이 치명적일 수 있습니다.

### Valgrind 사용법

Valgrind는 리눅스/macOS에서 가장 널리 사용되는 메모리 디버깅 도구입니다.

#### 설치

```bash
# Ubuntu/Debian
sudo apt-get install valgrind

# macOS (Homebrew)
brew install valgrind

# CentOS/RHEL
sudo yum install valgrind
```

#### 기본 사용법

```c
// memory_test.c - 의도적인 메모리 문제가 있는 예제
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LogEntry {
    char* message;
    int timestamp;
    struct LogEntry* next;
} LogEntry;

LogEntry* create_log_entry(const char* message, int timestamp) {
    LogEntry* entry = malloc(sizeof(LogEntry));
    entry->message = malloc(strlen(message) + 1);
    strcpy(entry->message, message);
    entry->timestamp = timestamp;
    entry->next = NULL;
    return entry;
}

void free_log_entry(LogEntry* entry) {
    if (entry) {
        free(entry->message);
        free(entry);
    }
}

int main() {
    // 1. 정상적인 사용
    LogEntry* entry1 = create_log_entry("Server started", 1234567890);
    free_log_entry(entry1);
    
    // 2. 메모리 누수 - free하지 않음
    LogEntry* entry2 = create_log_entry("Memory leak example", 1234567891);
    // free_log_entry(entry2); // 의도적으로 주석 처리
    
    // 3. 해제된 메모리 접근 - Use after free
    LogEntry* entry3 = create_log_entry("Use after free", 1234567892);
    free_log_entry(entry3);
    printf("Timestamp: %d\n", entry3->timestamp); // 위험한 접근
    
    // 4. 초기화되지 않은 메모리 읽기
    LogEntry* entry4 = malloc(sizeof(LogEntry));
    printf("Uninitialized timestamp: %d\n", entry4->timestamp);
    free(entry4);
    
    return 0;
}
```

#### Valgrind 실행 및 결과 분석

```bash
# 컴파일 (디버그 정보 포함)
gcc -g -O0 memory_test.c -o memory_test

# Valgrind로 메모리 검사 실행
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./memory_test
```

예상 출력:
```
==12345== Memcheck, a memory error detector
==12345== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==12345== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
==12345== Command: ./memory_test
==12345== 
==12345== Invalid read of size 4
==12345==    at 0x108B2B: main (memory_test.c:35)
==12345==  Address 0x4a47084 is 4 bytes inside a block of size 16 free'd
==12345==    at 0x483CA3F: free (in /usr/lib/x86_64-linux-gnu/valgrind/vgpreload_memcheck-amd64-linux.so)
==12345==    at 0x108B17: free_log_entry (memory_test.c:19)
==12345==    at 0x108B25: main (memory_test.c:33)
==12345== 
==12345== HEAP SUMMARY:
==12345==     in use at exit: 41 bytes in 2 blocks
==12345==   total heap usage: 5 allocs, 3 frees, 1,089 bytes allocated
==12345== 
==12345== 41 bytes in 2 blocks are definitely lost in loss record 2 of 2
==12345==    at 0x483B7F3: malloc (in /usr/lib/x86_64-linux-gnu/valgrind/vgpreload_memcheck-amd64-linux.so)
==12345==    at 0x108A98: create_log_entry (memory_test.c:12)
==12345==    at 0x108AFE: main (memory_test.c:26)
```

### AddressSanitizer (ASan) 사용법

AddressSanitizer는 GCC와 Clang에 내장된 빠른 메모리 오류 검출기입니다.

```bash
# 컴파일 시 AddressSanitizer 활성화
gcc -fsanitize=address -g -O1 memory_test.c -o memory_test_asan

# 실행
./memory_test_asan
```

#### ASan 환경 변수 설정

```bash
# 더 자세한 정보 출력
export ASAN_OPTIONS="verbosity=1:halt_on_error=1"

# 메모리 누수 검사 활성화
export ASAN_OPTIONS="detect_leaks=1"

# 실행
./memory_test_asan
```

---

## 2. 프로파일링과 성능 측정

### gprof를 사용한 함수 호출 프로파일링

```c
// performance_test.c - 프로파일링 예제
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 느린 문자열 처리 함수 (의도적으로 비효율적)
char* slow_string_process(const char* input) {
    int len = strlen(input);
    char* result = malloc(len * 2 + 1);
    
    for (int i = 0; i < len; i++) {
        result[i * 2] = input[i];
        result[i * 2 + 1] = '_';
    }
    result[len * 2] = '\0';
    
    return result;
}

// 빠른 문자열 처리 함수
char* fast_string_process(const char* input) {
    int len = strlen(input);
    char* result = malloc(len * 2 + 1);
    char* ptr = result;
    
    for (const char* src = input; *src; src++) {
        *ptr++ = *src;
        *ptr++ = '_';
    }
    *ptr = '\0';
    
    return result;
}

// 로그 처리 시뮬레이션
void process_logs(int use_fast_version) {
    const char* sample_logs[] = {
        "Server started successfully",
        "Database connection established",
        "Processing user request #12345",
        "Cache miss for key: user_session_abc123",
        "Authentication successful for user: admin"
    };
    
    int num_logs = sizeof(sample_logs) / sizeof(sample_logs[0]);
    
    for (int iteration = 0; iteration < 10000; iteration++) {
        for (int i = 0; i < num_logs; i++) {
            char* processed;
            if (use_fast_version) {
                processed = fast_string_process(sample_logs[i]);
            } else {
                processed = slow_string_process(sample_logs[i]);
            }
            
            // 처리된 로그 사용 (실제로는 파일에 기록하거나 네트워크로 전송)
            if (iteration % 5000 == 0 && i == 0) {
                printf("Processed: %s\n", processed);
            }
            
            free(processed);
        }
    }
}

int main(int argc, char* argv[]) {
    int use_fast = (argc > 1 && strcmp(argv[1], "--fast") == 0);
    
    printf("Running %s version...\n", use_fast ? "fast" : "slow");
    
    clock_t start = clock();
    process_logs(use_fast);
    clock_t end = clock();
    
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Total time: %f seconds\n", cpu_time);
    
    return 0;
}
```

#### gprof 사용법

```bash
# 프로파일링 정보를 포함해서 컴파일
gcc -pg -O0 -g performance_test.c -o performance_test

# 실행 (gmon.out 파일 생성)
./performance_test

# 프로파일링 결과 분석
gprof performance_test gmon.out > profile_report.txt

# 결과 확인
cat profile_report.txt
```

### perf를 사용한 시스템 레벨 프로파일링

```bash
# perf 설치 (Ubuntu)
sudo apt-get install linux-tools-common linux-tools-generic

# CPU 사용률 프로파일링
perf record -g ./performance_test

# 결과 분석
perf report

# 특정 이벤트 모니터링
perf stat -e cache-misses,cache-references,instructions,cycles ./performance_test
```

---

## 3. 단위 테스트 프레임워크

### C를 위한 Unity 테스트 프레임워크

Unity는 C 언어를 위한 간단하고 효과적인 단위 테스트 프레임워크입니다.

#### Unity 설정

```bash
# Unity 다운로드
git clone https://github.com/ThrowTheSwitch/Unity.git
cd Unity
```

#### 테스트 코드 작성

```c
// log_utils.c - 테스트할 로그 유틸리티 함수들
#include "log_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

LogLevel string_to_log_level(const char* level_str) {
    if (strcmp(level_str, "DEBUG") == 0) return LOG_DEBUG;
    if (strcmp(level_str, "INFO") == 0) return LOG_INFO;
    if (strcmp(level_str, "WARNING") == 0) return LOG_WARNING;
    if (strcmp(level_str, "ERROR") == 0) return LOG_ERROR;
    return LOG_UNKNOWN;
}

const char* log_level_to_string(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

int validate_log_message(const char* message) {
    if (message == NULL) return 0;
    if (strlen(message) == 0) return 0;
    if (strlen(message) > MAX_LOG_MESSAGE_LENGTH) return 0;
    return 1;
}

char* format_log_entry(LogLevel level, const char* message, time_t timestamp) {
    if (!validate_log_message(message)) {
        return NULL;
    }
    
    char* formatted = malloc(MAX_LOG_MESSAGE_LENGTH + 100);
    if (formatted == NULL) {
        return NULL;
    }
    
    snprintf(formatted, MAX_LOG_MESSAGE_LENGTH + 100,
             "[%ld][%s] %s", timestamp, log_level_to_string(level), message);
    
    return formatted;
}
```

```c
// log_utils.h
#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <time.h>

#define MAX_LOG_MESSAGE_LENGTH 1024

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3,
    LOG_UNKNOWN = -1
} LogLevel;

LogLevel string_to_log_level(const char* level_str);
const char* log_level_to_string(LogLevel level);
int validate_log_message(const char* message);
char* format_log_entry(LogLevel level, const char* message, time_t timestamp);

#endif
```

```c
// test_log_utils.c - Unity 단위 테스트
#include "Unity/src/unity.h"
#include "log_utils.h"
#include <stdlib.h>
#include <string.h>

void setUp(void) {
    // 각 테스트 전에 실행
}

void tearDown(void) {
    // 각 테스트 후에 실행
}

// 로그 레벨 문자열 변환 테스트
void test_string_to_log_level_valid_inputs(void) {
    TEST_ASSERT_EQUAL(LOG_DEBUG, string_to_log_level("DEBUG"));
    TEST_ASSERT_EQUAL(LOG_INFO, string_to_log_level("INFO"));
    TEST_ASSERT_EQUAL(LOG_WARNING, string_to_log_level("WARNING"));
    TEST_ASSERT_EQUAL(LOG_ERROR, string_to_log_level("ERROR"));
}

void test_string_to_log_level_invalid_inputs(void) {
    TEST_ASSERT_EQUAL(LOG_UNKNOWN, string_to_log_level("INVALID"));
    TEST_ASSERT_EQUAL(LOG_UNKNOWN, string_to_log_level(""));
    TEST_ASSERT_EQUAL(LOG_UNKNOWN, string_to_log_level(NULL));
}

// 로그 레벨 문자열 변환 테스트 (역방향)
void test_log_level_to_string(void) {
    TEST_ASSERT_EQUAL_STRING("DEBUG", log_level_to_string(LOG_DEBUG));
    TEST_ASSERT_EQUAL_STRING("INFO", log_level_to_string(LOG_INFO));
    TEST_ASSERT_EQUAL_STRING("WARNING", log_level_to_string(LOG_WARNING));
    TEST_ASSERT_EQUAL_STRING("ERROR", log_level_to_string(LOG_ERROR));
    TEST_ASSERT_EQUAL_STRING("UNKNOWN", log_level_to_string(LOG_UNKNOWN));
}

// 로그 메시지 검증 테스트
void test_validate_log_message(void) {
    TEST_ASSERT_TRUE(validate_log_message("Valid message"));
    TEST_ASSERT_FALSE(validate_log_message(NULL));
    TEST_ASSERT_FALSE(validate_log_message(""));
    
    // 너무 긴 메시지 테스트
    char long_message[MAX_LOG_MESSAGE_LENGTH + 10];
    memset(long_message, 'A', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\0';
    TEST_ASSERT_FALSE(validate_log_message(long_message));
}

// 로그 엔트리 포맷팅 테스트
void test_format_log_entry(void) {
    time_t test_time = 1234567890;
    char* formatted = format_log_entry(LOG_INFO, "Test message", test_time);
    
    TEST_ASSERT_NOT_NULL(formatted);
    TEST_ASSERT_TRUE(strstr(formatted, "[1234567890]") != NULL);
    TEST_ASSERT_TRUE(strstr(formatted, "[INFO]") != NULL);
    TEST_ASSERT_TRUE(strstr(formatted, "Test message") != NULL);
    
    free(formatted);
}

void test_format_log_entry_invalid_message(void) {
    time_t test_time = 1234567890;
    char* formatted = format_log_entry(LOG_INFO, NULL, test_time);
    
    TEST_ASSERT_NULL(formatted);
}

// 메인 테스트 러너
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_string_to_log_level_valid_inputs);
    RUN_TEST(test_string_to_log_level_invalid_inputs);
    RUN_TEST(test_log_level_to_string);
    RUN_TEST(test_validate_log_message);
    RUN_TEST(test_format_log_entry);
    RUN_TEST(test_format_log_entry_invalid_message);
    
    return UNITY_END();
}
```

#### 테스트 실행

```bash
# 컴파일 및 실행
gcc -I. Unity/src/unity.c log_utils.c test_log_utils.c -o test_runner
./test_runner
```

### C++을 위한 Google Test

```cpp
// log_processor.hpp - 테스트할 C++ 클래스
#ifndef LOG_PROCESSOR_HPP
#define LOG_PROCESSOR_HPP

#include <string>
#include <vector>
#include <unordered_map>

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

struct LogEntry {
    LogLevel level;
    std::string message;
    std::string category;
    long timestamp;
    
    LogEntry(LogLevel l, const std::string& msg, const std::string& cat, long ts)
        : level(l), message(msg), category(cat), timestamp(ts) {}
};

class LogProcessor {
private:
    std::vector<LogEntry> logs_;
    std::unordered_map<LogLevel, int> level_counts_;

public:
    void addLog(const LogEntry& entry);
    std::vector<LogEntry> getLogsByLevel(LogLevel level) const;
    std::vector<LogEntry> getLogsByCategory(const std::string& category) const;
    int getLogCount() const;
    int getLogCountByLevel(LogLevel level) const;
    void clearLogs();
    
    // 통계 함수들
    double getErrorRate() const;
    std::string getMostFrequentCategory() const;
};

#endif
```

```cpp
// log_processor.cpp
#include "log_processor.hpp"
#include <algorithm>
#include <unordered_map>

void LogProcessor::addLog(const LogEntry& entry) {
    logs_.push_back(entry);
    level_counts_[entry.level]++;
}

std::vector<LogEntry> LogProcessor::getLogsByLevel(LogLevel level) const {
    std::vector<LogEntry> filtered;
    std::copy_if(logs_.begin(), logs_.end(), std::back_inserter(filtered),
                 [level](const LogEntry& entry) { return entry.level == level; });
    return filtered;
}

std::vector<LogEntry> LogProcessor::getLogsByCategory(const std::string& category) const {
    std::vector<LogEntry> filtered;
    std::copy_if(logs_.begin(), logs_.end(), std::back_inserter(filtered),
                 [&category](const LogEntry& entry) { return entry.category == category; });
    return filtered;
}

int LogProcessor::getLogCount() const {
    return logs_.size();
}

int LogProcessor::getLogCountByLevel(LogLevel level) const {
    auto it = level_counts_.find(level);
    return (it != level_counts_.end()) ? it->second : 0;
}

void LogProcessor::clearLogs() {
    logs_.clear();
    level_counts_.clear();
}

double LogProcessor::getErrorRate() const {
    if (logs_.empty()) return 0.0;
    
    int error_count = getLogCountByLevel(LogLevel::ERROR);
    return static_cast<double>(error_count) / logs_.size();
}

std::string LogProcessor::getMostFrequentCategory() const {
    if (logs_.empty()) return "";
    
    std::unordered_map<std::string, int> category_counts;
    for (const auto& log : logs_) {
        category_counts[log.category]++;
    }
    
    auto max_it = std::max_element(category_counts.begin(), category_counts.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
    
    return max_it->first;
}
```

```cpp
// test_log_processor.cpp - Google Test 단위 테스트
#include <gtest/gtest.h>
#include "log_processor.hpp"

class LogProcessorTest : public ::testing::Test {
protected:
    void SetUp() override {
        processor_.clearLogs();
    }
    
    void TearDown() override {
        // 정리 작업
    }
    
    LogProcessor processor_;
};

TEST_F(LogProcessorTest, AddLogAndGetCount) {
    EXPECT_EQ(0, processor_.getLogCount());
    
    LogEntry entry(LogLevel::INFO, "Test message", "TEST", 1234567890);
    processor_.addLog(entry);
    
    EXPECT_EQ(1, processor_.getLogCount());
}

TEST_F(LogProcessorTest, GetLogsByLevel) {
    LogEntry info_log(LogLevel::INFO, "Info message", "TEST", 1234567890);
    LogEntry error_log(LogLevel::ERROR, "Error message", "TEST", 1234567891);
    LogEntry debug_log(LogLevel::DEBUG, "Debug message", "TEST", 1234567892);
    
    processor_.addLog(info_log);
    processor_.addLog(error_log);
    processor_.addLog(debug_log);
    
    auto error_logs = processor_.getLogsByLevel(LogLevel::ERROR);
    EXPECT_EQ(1, error_logs.size());
    EXPECT_EQ("Error message", error_logs[0].message);
    
    auto info_logs = processor_.getLogsByLevel(LogLevel::INFO);
    EXPECT_EQ(1, info_logs.size());
    
    auto warning_logs = processor_.getLogsByLevel(LogLevel::WARNING);
    EXPECT_EQ(0, warning_logs.size());
}

TEST_F(LogProcessorTest, GetLogsByCategory) {
    LogEntry db_log(LogLevel::INFO, "DB connected", "DATABASE", 1234567890);
    LogEntry net_log(LogLevel::ERROR, "Connection failed", "NETWORK", 1234567891);
    LogEntry db_log2(LogLevel::WARNING, "Slow query", "DATABASE", 1234567892);
    
    processor_.addLog(db_log);
    processor_.addLog(net_log);
    processor_.addLog(db_log2);
    
    auto db_logs = processor_.getLogsByCategory("DATABASE");
    EXPECT_EQ(2, db_logs.size());
    
    auto net_logs = processor_.getLogsByCategory("NETWORK");
    EXPECT_EQ(1, net_logs.size());
    EXPECT_EQ("Connection failed", net_logs[0].message);
}

TEST_F(LogProcessorTest, GetLogCountByLevel) {
    processor_.addLog(LogEntry(LogLevel::INFO, "Info 1", "TEST", 1234567890));
    processor_.addLog(LogEntry(LogLevel::INFO, "Info 2", "TEST", 1234567891));
    processor_.addLog(LogEntry(LogLevel::ERROR, "Error 1", "TEST", 1234567892));
    
    EXPECT_EQ(2, processor_.getLogCountByLevel(LogLevel::INFO));
    EXPECT_EQ(1, processor_.getLogCountByLevel(LogLevel::ERROR));
    EXPECT_EQ(0, processor_.getLogCountByLevel(LogLevel::DEBUG));
}

TEST_F(LogProcessorTest, GetErrorRate) {
    // 빈 프로세서의 에러율은 0
    EXPECT_DOUBLE_EQ(0.0, processor_.getErrorRate());
    
    // 50% 에러율 테스트
    processor_.addLog(LogEntry(LogLevel::INFO, "Info", "TEST", 1234567890));
    processor_.addLog(LogEntry(LogLevel::ERROR, "Error", "TEST", 1234567891));
    
    EXPECT_DOUBLE_EQ(0.5, processor_.getErrorRate());
    
    // 25% 에러율 테스트
    processor_.addLog(LogEntry(LogLevel::INFO, "Info 2", "TEST", 1234567892));
    processor_.addLog(LogEntry(LogLevel::DEBUG, "Debug", "TEST", 1234567893));
    
    EXPECT_DOUBLE_EQ(0.25, processor_.getErrorRate());
}

TEST_F(LogProcessorTest, GetMostFrequentCategory) {
    // 빈 프로세서
    EXPECT_EQ("", processor_.getMostFrequentCategory());
    
    processor_.addLog(LogEntry(LogLevel::INFO, "DB 1", "DATABASE", 1234567890));
    processor_.addLog(LogEntry(LogLevel::INFO, "Net 1", "NETWORK", 1234567891));
    processor_.addLog(LogEntry(LogLevel::INFO, "DB 2", "DATABASE", 1234567892));
    processor_.addLog(LogEntry(LogLevel::INFO, "DB 3", "DATABASE", 1234567893));
    
    EXPECT_EQ("DATABASE", processor_.getMostFrequentCategory());
}

TEST_F(LogProcessorTest, ClearLogs) {
    processor_.addLog(LogEntry(LogLevel::INFO, "Test", "TEST", 1234567890));
    EXPECT_EQ(1, processor_.getLogCount());
    
    processor_.clearLogs();
    EXPECT_EQ(0, processor_.getLogCount());
    EXPECT_DOUBLE_EQ(0.0, processor_.getErrorRate());
}

// 매개변수화된 테스트 예제
class LogLevelTest : public ::testing::TestWithParam<LogLevel> {};

TEST_P(LogLevelTest, LogLevelHandling) {
    LogProcessor processor;
    LogLevel level = GetParam();
    
    LogEntry entry(level, "Test message", "TEST", 1234567890);
    processor.addLog(entry);
    
    auto logs = processor.getLogsByLevel(level);
    EXPECT_EQ(1, logs.size());
    EXPECT_EQ(level, logs[0].level);
}

INSTANTIATE_TEST_SUITE_P(
    AllLogLevels,
    LogLevelTest,
    ::testing::Values(LogLevel::DEBUG, LogLevel::INFO, LogLevel::WARNING, LogLevel::ERROR)
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

#### Google Test 빌드 및 실행

```bash
# Google Test 설치 (Ubuntu)
sudo apt-get install libgtest-dev cmake

# 빌드
mkdir build && cd build
cmake ..
make

# 테스트 실행
./test_log_processor
```

---

## 4. 크로스 플랫폼 개발

### CMake를 사용한 플랫폼 독립적 빌드

```cmake
# CMakeLists.txt - 크로스 플랫폼 LogCaster 프로젝트
cmake_minimum_required(VERSION 3.10)
project(LogCaster VERSION 1.0.0)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 플랫폼별 컴파일러 플래그
if(MSVC)
    # Windows Visual Studio
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    add_definitions(-D_WIN32_WINNT=0x0601)  # Windows 7 이상
else()
    # GCC/Clang (Linux/macOS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -DDEBUG")
    endif()
endif()

# 플랫폼별 라이브러리 찾기
if(WIN32)
    # Windows 소켓 라이브러리
    set(PLATFORM_LIBS ws2_32)
elseif(UNIX AND NOT APPLE)
    # Linux
    find_package(Threads REQUIRED)
    set(PLATFORM_LIBS ${CMAKE_THREAD_LIBS_INIT})
elseif(APPLE)
    # macOS
    find_package(Threads REQUIRED)
    set(PLATFORM_LIBS ${CMAKE_THREAD_LIBS_INIT})
endif()

# 소스 파일들
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

# 헤더 파일 디렉토리
include_directories(include)

# 실행 파일 생성
add_executable(logcaster ${SOURCES})

# 라이브러리 링크
target_link_libraries(logcaster ${PLATFORM_LIBS})

# 설치 규칙
install(TARGETS logcaster DESTINATION bin)

# 테스트 (선택적)
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 패키징
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
```

### 플랫폼별 조건부 컴파일

```cpp
// platform_utils.hpp - 플랫폼별 유틸리티
#ifndef PLATFORM_UTILS_HPP
#define PLATFORM_UTILS_HPP

#include <string>

// 플랫폼 감지
#ifdef _WIN32
    #define PLATFORM_WINDOWS
    #include <windows.h>
    #include <winsock2.h>
#elif defined(__linux__)
    #define PLATFORM_LINUX
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#elif defined(__APPLE__)
    #define PLATFORM_MACOS
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

class PlatformUtils {
public:
    // 플랫폼별 초기화
    static bool initialize();
    static void cleanup();
    
    // 파일 경로 처리
    static std::string getPathSeparator();
    static std::string joinPath(const std::string& dir, const std::string& file);
    static std::string getExecutablePath();
    
    // 네트워크 초기화
    static bool initializeNetwork();
    static void cleanupNetwork();
    
    // 스레드 관련
    static void sleepMilliseconds(int ms);
    static int getCurrentThreadId();
    
    // 시간 관련
    static long long getCurrentTimestamp();
    static std::string formatTimestamp(long long timestamp);
};

#endif
```

```cpp
// platform_utils.cpp
#include "platform_utils.hpp"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

#ifdef PLATFORM_WINDOWS
    #include <direct.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <limits.h>
#endif

bool PlatformUtils::initialize() {
    #ifdef PLATFORM_WINDOWS
        // Windows 전용 초기화
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            return false;
        }
    #endif
    
    return true;
}

void PlatformUtils::cleanup() {
    #ifdef PLATFORM_WINDOWS
        WSACleanup();
    #endif
}

std::string PlatformUtils::getPathSeparator() {
    #ifdef PLATFORM_WINDOWS
        return "\\";
    #else
        return "/";
    #endif
}

std::string PlatformUtils::joinPath(const std::string& dir, const std::string& file) {
    if (dir.empty()) return file;
    if (file.empty()) return dir;
    
    std::string separator = getPathSeparator();
    if (dir.back() == separator[0]) {
        return dir + file;
    } else {
        return dir + separator + file;
    }
}

std::string PlatformUtils::getExecutablePath() {
    #ifdef PLATFORM_WINDOWS
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        return std::string(buffer);
    #else
        char buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1) {
            buffer[len] = '\0';
            return std::string(buffer);
        }
        return "";
    #endif
}

bool PlatformUtils::initializeNetwork() {
    return initialize();  // 네트워크 초기화는 전체 초기화에 포함
}

void PlatformUtils::cleanupNetwork() {
    cleanup();
}

void PlatformUtils::sleepMilliseconds(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int PlatformUtils::getCurrentThreadId() {
    #ifdef PLATFORM_WINDOWS
        return GetCurrentThreadId();
    #else
        return static_cast<int>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    #endif
}

long long PlatformUtils::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

std::string PlatformUtils::formatTimestamp(long long timestamp) {
    auto time_point = std::chrono::system_clock::from_time_t(timestamp / 1000);
    auto time_t = std::chrono::system_clock::to_time_t(time_point);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << (timestamp % 1000);
    
    return ss.str();
}
```

---

## 5. 지속적 통합 (CI/CD)

### GitHub Actions 설정

```yaml
# .github/workflows/ci.yml
name: LogCaster CI/CD

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  test:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
        build_type: [Debug, Release]
        
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies (Ubuntu)
      if: matrix.os == 'ubuntu-latest'
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake build-essential valgrind
        
    - name: Install dependencies (Windows)
      if: matrix.os == 'windows-latest'
      run: |
        choco install cmake
        
    - name: Install dependencies (macOS)
      if: matrix.os == 'macos-latest'
      run: |
        brew install cmake
    
    - name: Configure CMake
      run: cmake -B build -DCMAKE_BUILD_TYPE=${{ matrix.build_type }} -DBUILD_TESTS=ON
      
    - name: Build
      run: cmake --build build --config ${{ matrix.build_type }}
      
    - name: Run tests
      working-directory: build
      run: ctest -C ${{ matrix.build_type }} --output-on-failure
      
    - name: Run memory check (Ubuntu Debug only)
      if: matrix.os == 'ubuntu-latest' && matrix.build_type == 'Debug'
      working-directory: build
      run: |
        valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./logcaster --test-mode
        
    - name: Upload test results
      uses: actions/upload-artifact@v3
      if: failure()
      with:
        name: test-results-${{ matrix.os }}-${{ matrix.build_type }}
        path: build/Testing/
```

---

## 🔥 6. 흔한 실수와 해결방법

### 6.1 메모리 디버깅 실수

#### [SEQUENCE: 39] 옵션 없이 Valgrind 사용
```bash
# ❌ 잘못된 예: 기본 옵션만 사용
valgrind ./program

# ✅ 올바른 예: 상세한 옵션 사용
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
         --track-origins=yes --verbose ./program
```

### 6.2 테스트 관련 실수

#### [SEQUENCE: 40] 테스트 커버리지 오해
```cpp
// ❌ 잘못된 예: 100% 커버리지에 집착
void test_everything() {
    // 모든 경로를 테스트하려고 시도
    // 실제로는 중요하지 않은 코드까지 테스트
}

// ✅ 올바른 예: 핵심 기능 위주 테스트
void test_critical_functionality() {
    // 핵심 비즈니스 로직 테스트
    // 에지 케이스 테스트
    // 성능 크리티컬 부분 테스트
}
```

### 6.3 크로스 플랫폼 실수

#### [SEQUENCE: 41] 플랫폼 가정
```cpp
// ❌ 잘못된 예: 특정 플랫폼 가정
void read_config() {
    // Windows에서만 동작
    std::ifstream file("C:\\config\\app.conf");
}

// ✅ 올바른 예: 플랫폼 독립적 코드
void read_config() {
    std::string config_path = PlatformUtils::joinPath(
        PlatformUtils::getConfigDir(),
        "app.conf"
    );
    std::ifstream file(config_path);
}
```

---

## 7. 실습 프로젝트

### 프로젝트 1: 메모리 안전 로거 (기초)
**목표**: Valgrind와 ASan으로 검증된 안전한 로거 구현

**구현 사항**:
- 메모리 누수 없는 로그 버퍼
- 스레드 안전 로깅
- 자동 메모리 검사 스크립트
- CI/CD 통합

### 프로젝트 2: 크로스 플랫폼 빌드 시스템 (중급)
**목표**: Windows, Linux, macOS에서 동작하는 LogCaster

**구현 사항**:
- CMake 기반 빌드 시스템
- 플랫폼별 네트워크 코드
- OS별 파일 시스템 처리
- 자동화된 테스트

### 프로젝트 3: 성능 최적화 도구 체인 (고급)
**목표**: 프로파일링 기반 자동 최적화 시스템

**구현 사항**:
- perf/gprof 통합
- 성능 회귀 탐지
- 자동 병목점 분석
- 최적화 제안 리포트

---

## 8. 학습 체크리스트

### 기초 레벨 ✅
- [ ] Valgrind 기본 사용법
- [ ] 간단한 단위 테스트 작성
- [ ] CMake 기초
- [ ] Git 기본 사용

### 중급 레벨 📚
- [ ] AddressSanitizer 활용
- [ ] Google Test/Unity 프레임워크
- [ ] 프로파일링 도구 사용
- [ ] GitHub Actions CI/CD

### 고급 레벨 🚀
- [ ] 크로스 플랫폼 빌드
- [ ] 커스텀 도구 개발
- [ ] 성능 벤치마크 작성
- [ ] 도커 컨테이너화

### 전문가 레벨 🏆
- [ ] 커스텀 프로파일러 개발
- [ ] 커널 레벨 디버깅
- [ ] 분산 테스트 시스템
- [ ] 퍼지 테스팅

---

## 9. 추가 학습 자료

### 추천 도서
- "Working Effectively with Legacy Code" - Michael Feathers
- "The Pragmatic Programmer" - David Thomas & Andrew Hunt
- "Continuous Delivery" - Jez Humble & David Farley

### 온라인 리소스
- [CMake Documentation](https://cmake.org/documentation/)
- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Valgrind Documentation](http://valgrind.org/docs/manual/index.html)

### 실습 도구
- CLion (JetBrains IDE)
- Visual Studio Code + C/C++ Extension
- clang-format (코드 포맷팅)
- cppcheck (정적 분석)

---

## 6. 다음 단계 준비

이 개발 도구 가이드를 이해했다면 LogCaster 프로젝트를 전문적으로 개발할 준비가 되었습니다:

1. **메모리 안전성**: Valgrind, ASan으로 메모리 문제 조기 발견
2. **성능 최적화**: 프로파일링 도구로 병목점 식별 및 개선
3. **코드 품질**: 단위 테스트로 안정성 보장
4. **크로스 플랫폼**: 다양한 환경에서 동작하는 코드 작성
5. **자동화**: CI/CD 파이프라인으로 품질 관리

### 확인 문제

1. Valgrind와 AddressSanitizer의 차이점과 각각의 장단점은?
2. 단위 테스트를 작성할 때 고려해야 할 사항들은?
3. 크로스 플랫폼 개발 시 주의해야 할 점들은?

---

*💡 팁: 이 도구들은 프로덕션 수준의 소프트웨어 개발에 필수적입니다. 각 도구를 실제 프로젝트에 적용해보며 익숙해지세요!*