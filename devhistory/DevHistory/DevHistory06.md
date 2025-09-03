# DevHistory 06: LogCaster-CPP MVP3 - Modern C++ Query Engine

## 1. 개요 (Overview)

C++ 구현의 세 번째 단계(MVP3)는 C++의 강력한 표준 라이브러리와 최신 언어 기능을 활용하여 C 버전 MVP3와 동일한 고급 쿼리 기능을 구현하고, 더 나아가 코드의 안정성과 표현력을 극대화하는 데 중점을 둡니다. 이 단계는 C++의 추상화 능력이 어떻게 더 안전하고 간결한 코드로 이어지는지를 명확하게 보여줍니다.

**주요 기능 추가 및 개선:**
- **`QueryParser` 클래스:** `key=value` 형태의 쿼리 문자열을 파싱하여, 타입-안전(type-safe)한 `ParsedQuery` 객체로 변환하는 정적(static) 클래스를 도입합니다. 
- **`std::regex`:** C의 POSIX 라이브러리 대신, C++ 표준 라이브러리인 `<regex>`를 사용하여 정규식 검색 기능을 구현합니다. 이를 통해 예외 처리(exception handling)를 통한 오류 관리가 가능해집니다.
- **`std::chrono`:** C의 `time_t` 대신, `<chrono>` 라이브러리의 `system_clock::time_point`를 사용하여 시간 기반 필터링을 구현합니다. 이는 타입 안전성을 높이고 시간과 관련된 버그를 줄여줍니다.
- **`std::optional`:** `ParsedQuery` 구조체 내에서 정규식이나 시간 필터처럼 존재할 수도, 안 할 수도 있는 파라미터를 명확하게 표현하기 위해 `std::optional`을 사용합니다. 이는 포인터를 사용하고 `NULL` 체크를 하던 C 방식보다 훨씬 안전하고 표현력이 뛰어납니다.
- **`HELP` 명령:** `QueryHandler`에 상세한 도움말 기능을 추가하여 사용자가 새로운 쿼리 구문을 쉽게 학습할 수 있도록 합니다.

**아키텍처 변경:**
- **`ParsedQuery` 클래스 도입:** C 버전의 `parsed_query_t` 구조체를 대체하는 클래스입니다. 자체적으로 쿼리 조건 일치 여부를 판단하는 `matches()` 멤버 함수를 포함하여, 데이터와 행위를 캡슐화하는 객체 지향 원칙을 따릅니다.
- **`LogBuffer` 검색 기능 확장:** `ParsedQuery` 객체를 인자로 받는 `searchEnhanced()` 메소드를 추가하여, 복잡한 조건의 검색을 수행합니다.
- **`QueryHandler` 로직 개선:** `QueryHandler`는 `QueryParser`를 호출하여 쿼리를 해석하고, 그 결과를 `LogBuffer`의 `searchEnhanced()`에 전달하는 역할에 집중합니다. 이로써 각 클래스의 책임이 더 명확해집니다.

이 MVP를 통해 C++ 버전은 C 버전의 모든 쿼리 기능을 갖추게 되며, 동시에 C++의 현대적인 기능들이 어떻게 코드의 복잡도를 낮추고 안정성을 향상시키는지를 보여주는 좋은 예시가 됩니다.

## 2. 빌드 시스템 (Build System) 

`QueryParser` 클래스가 새로 추가됨에 따라, `CMakeLists.txt`의 소스 파일 목록에 `src/QueryParser.cpp`가 추가됩니다.

```cmake
# [SEQUENCE: MVP3-1] 
# CMake 최소 요구 버전 설정
cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름 및 C++ 언어 지정
project(LogCasterCPP VERSION 1.0.0 LANGUAGES CXX)

# C++ 표준 설정 (C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 헤더 파일 경로 추가
include_directories(include)

# [SEQUENCE: MVP3-2]
# MVP3에 필요한 소스 파일 목록 (QueryParser.cpp 추가)
set(SOURCES
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/LogBuffer.cpp
    src/QueryHandler.cpp
    src/QueryParser.cpp
)

# 실행 파일 생성
add_executable(logcaster-cpp ${SOURCES})

# 스레드 라이브러리 링크
find_package(Threads REQUIRED)
target_link_libraries(logcaster-cpp PRIVATE Threads::Threads)
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. `include/QueryParser.h` (신규 파일)

`std::optional`, `std::regex` 등 최신 C++ 기능을 사용하여 파싱된 쿼리 정보를 담는 `ParsedQuery` 클래스와, 이를 생성하는 `QueryParser` 클래스를 정의합니다.

```cpp
// [SEQUENCE: MVP3-3]
#ifndef QUERYPARSER_H
#define QUERYPARSER_H

#include <string>
#include <vector>
#include <optional>
#include <regex>
#include <chrono>
#include <memory>
#include "LogBuffer.h" // For LogEntry

// [SEQUENCE: MVP3-4]
// 쿼리 연산자 종류
enum class OperatorType {
    AND,
    OR
};

// [SEQUENCE: MVP3-5]
// 파싱된 쿼리 정보를 담는 클래스
class ParsedQuery {
public:
    bool matches(const std::string& message, const std::chrono::system_clock::time_point& timestamp) const;

private:
    friend class QueryParser; // QueryParser가 private 멤버에 접근할 수 있도록 허용

    std::vector<std::string> keywords_;
    std::optional<std::regex> compiled_regex_;
    std::optional<std::chrono::system_clock::time_point> time_from_;
    std::optional<std::chrono::system_clock::time_point> time_to_;
    OperatorType op_ = OperatorType::AND;
};

// [SEQUENCE: MVP3-6]
// 쿼리 문자열을 파싱하는 정적 클래스
class QueryParser {
public:
    static std::unique_ptr<ParsedQuery> parse(const std::string& query_string);
};

#endif // QUERYPARSER_H
```

### 3.2. `src/QueryParser.cpp` (신규 파일)

`QueryParser`의 파싱 로직과 `ParsedQuery`의 매칭 로직을 구현합니다. `std::stringstream`을 사용하여 문자열을 안전하게 처리하고, `std::regex` 생성 시 발생할 수 있는 예외를 처리합니다.

```cpp
// [SEQUENCE: MVP3-7]
#include "QueryParser.h"
#include <sstream>
#include <algorithm>
#include <iostream>

// [SEQUENCE: MVP3-8]
// 쿼리 문자열을 파싱하여 ParsedQuery 객체를 생성
std::unique_ptr<ParsedQuery> QueryParser::parse(const std::string& query_string) {
    auto parsed_query = std::make_unique<ParsedQuery>();
    std::stringstream ss(query_string);
    std::string segment;
    std::vector<std::string> segments;

    // "QUERY" 단어 스킵
    ss >> segment;

    while (ss >> segment) {
        segments.push_back(segment);
    }

    for (const auto& seg : segments) {
        size_t pos = seg.find('=');
        if (pos == std::string::npos) continue;

        std::string key = seg.substr(0, pos);
        std::string value = seg.substr(pos + 1);

        // [SEQUENCE: MVP3-9]
        // 각 파라미터에 맞게 값 설정
        if (key == "keywords" || key == "keyword") {
            std::stringstream v_ss(value);
            std::string keyword;
            while (std::getline(v_ss, keyword, ',')) {
                parsed_query->keywords_.push_back(keyword);
            }
        } else if (key == "regex") {
            try {
                parsed_query->compiled_regex_.emplace(value, std::regex_constants::icase);
            } catch (const std::regex_error& e) {
                throw std::runtime_error("Invalid regex pattern: " + std::string(e.what()));
            }
        } else if (key == "time_from") {
            parsed_query->time_from_ = std::chrono::system_clock::from_time_t(std::stol(value));
        } else if (key == "time_to") {
            parsed_query->time_to_ = std::chrono::system_clock::from_time_t(std::stol(value));
        } else if (key == "operator") {
            std::transform(value.begin(), value.end(), value.begin(), ::toupper);
            if (value == "OR") {
                parsed_query->op_ = OperatorType::OR;
            }
        }
    }
    return parsed_query;
}

// [SEQUENCE: MVP3-10]
// 로그가 쿼리 조건에 부합하는지 검사
bool ParsedQuery::matches(const std::string& message, const std::chrono::system_clock::time_point& timestamp) const {
    // 시간 필터
    if (time_from_ && timestamp < *time_from_) return false;
    if (time_to_ && timestamp > *time_to_) return false;

    // 정규식 필터
    if (compiled_regex_ && !std::regex_search(message, *compiled_regex_)) {
        return false;
    }

    // 키워드 필터
    if (!keywords_.empty()) {
        if (op_ == OperatorType::AND) {
            for (const auto& kw : keywords_) {
                if (message.find(kw) == std::string::npos) return false;
            }
        } else { // OR
            bool found = false;
            for (const auto& kw : keywords_) {
                if (message.find(kw) != std::string::npos) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
    }
    return true;
}
```

### 3.3. `include/LogBuffer.h` (MVP3 업데이트)

`ParsedQuery`를 사용하는 `searchEnhanced` 메소드의 선언을 추가합니다.

```cpp
// [SEQUENCE: MVP3-11]
#ifndef LOGBUFFER_H
#define LOGBUFFER_H

// ... (기존 include 및 LogEntry 구조체는 동일)

// Forward declaration
class ParsedQuery;

class LogBuffer {
public:
    // ... (기존 메소드 선언은 동일)

    // [SEQUENCE: MVP3-12]
    // MVP3에 추가된 고급 검색 메소드
    std::vector<std::string> searchEnhanced(const ParsedQuery& query) const;

private:
    // ... (기존 멤버 변수는 동일)
};

#endif // LOGBUFFER_H
```

### 3.4. `src/LogBuffer.cpp` (MVP3 업데이트)

`ParsedQuery`의 `matches` 메소드를 호출하여 고급 검색을 수행하는 `searchEnhanced`를 구현합니다.

```cpp
// [SEQUENCE: MVP3-13]
#include "LogBuffer.h"
#include "QueryParser.h"
#include <sstream>
#include <iomanip>

// ... (push, search 등 기존 함수는 동일) ...

// [SEQUENCE: MVP3-14]
// 고급 검색 기능 구현
std::vector<std::string> LogBuffer::searchEnhanced(const ParsedQuery& query) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> results;
    for (const auto& entry : buffer_) {
        // [SEQUENCE: MVP3-15]
        // ParsedQuery 객체에 매칭 로직 위임
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
```

### 3.5. `src/QueryHandler.cpp` (MVP3 업데이트)

`QUERY` 명령을 처리할 때 `QueryParser`를 사용하도록 로직을 수정하고, `HELP` 명령의 내용을 보강합니다.

```cpp
// [SEQUENCE: MVP3-16]
#include "QueryHandler.h"
#include "QueryParser.h"
#include <sstream>

// ... (생성자는 동일) ...

// [SEQUENCE: MVP3-17]
// 쿼리 처리 로직 (MVP3 버전)
std::string QueryHandler::processQuery(const std::string& query) {
    if (query.find("QUERY") == 0) {
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

// [SEQUENCE: MVP3-18]
// 검색 쿼리 처리 로직 수정
std::string QueryHandler::handleSearch(const std::string& query) {
    try {
        // [SEQUENCE: MVP3-19]
        // QueryParser를 사용하여 쿼리 문자열을 파싱
        auto parsed_query = QueryParser::parse(query);
        if (!parsed_query) {
            return "ERROR: Failed to parse query.\n";
        }

        // [SEQUENCE: MVP3-20]
        // LogBuffer의 고급 검색 메소드 호출
        auto results = buffer_->searchEnhanced(*parsed_query);
        std::stringstream ss;
        ss << "FOUND: " << results.size() << " matches\n";
        for (const auto& res : results) {
            ss << res << "\n";
        }
        return ss.str();
    } catch (const std::exception& e) {
        return std::string("ERROR: ") + e.what() + "\n";
    }
}

// ... (handleStats, handleCount는 동일) ...

// [SEQUENCE: MVP3-21]
// HELP 명령 내용 보강
std::string QueryHandler::handleHelp() {
    return "Available commands:\n"
           "  STATS - Show buffer statistics\n"
           "  COUNT - Show number of logs in buffer\n"
           "  HELP  - Show this help message\n"
           "  QUERY <parameters> - Search logs with parameters:\n"
           "\n"
           "Query parameters:\n"
           "  keywords=<w1,w2,..> - Multiple keywords (comma-separated)\n"
           "  operator=<AND|OR>   - Keyword matching logic (default: AND)\n"
           "  regex=<pattern>     - Regular expression pattern (case-insensitive)\n"
           "  time_from=<unix_ts> - Start time (Unix timestamp)\n"
           "  time_to=<unix_ts>   - End time (Unix timestamp)\n"
           "\n"
           "Example: QUERY keywords=error,timeout operator=AND regex=failed\n";
}
```

## 4. 테스트 코드 (Test Code)

C 버전 MVP3의 테스트 코드(`tests/test_mvp3.py`)를 C++ 서버에 맞게 약간 수정하여 사용합니다. 프로토콜은 동일하므로 대부분의 로직을 재사용할 수 있습니다.

```python
# [SEQUENCE: MVP3-22]
#!/usr/bin/env python3
import socket
import time
import threading

# (C-MVP3 테스트 코드와 거의 동일. 포트 및 호스트 설정만 확인)
HOST = '127.0.0.1'
LOG_PORT = 9999
QUERY_PORT = 9998

# ... (send_logs, query_server, run_test 함수는 DevHistory05.md와 동일) ...

if __name__ == "__main__":
    print("--- Testing LogCaster-CPP MVP3 ---")
    # ... (테스트 실행 로직은 DevHistory05.md와 동일) ...
    print("All C++ MVP3 tests passed!")
```