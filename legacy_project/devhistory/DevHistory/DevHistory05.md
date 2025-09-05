# DevHistory 05: LogCaster-C MVP3 - Enhanced Query Capabilities

## 1. 개요 (Overview)

C 구현의 세 번째 단계(MVP3)는 서버의 핵심 가치 중 하나인 '실시간 로그 쿼리' 기능을 대폭 강화하는 데 중점을 둡니다. MVP2에서 구현된 단순 키워드 검색을 넘어, 사용자가 더 정교하고 복잡한 조건으로 로그를 필터링할 수 있는 고급 조회 기능을 도입합니다.

**주요 기능 추가:**
- **고급 쿼리 파서 (`QueryParser`):** `QUERY` 명령의 파라미터를 체계적으로 분석하는 새로운 모듈입니다. `key=value` 형식의 여러 파라미터를 인식하고 구조화된 쿼리 객체로 변환합니다.
- **정규식(Regex) 검색:** `regex=` 파라미터를 통해 POSIX 정규 표현식을 사용한 패턴 매칭을 지원합니다. 이를 통해 단순 키워드 매칭보다 훨씬 강력하고 유연한 텍스트 검색이 가능해집니다.
- **시간 범위 필터링:** `time_from=` 및 `time_to=` 파라미터를 사용하여 특정 시간대(Unix timestamp 기준)에 기록된 로그만 조회할 수 있습니다.
- **다중 키워드 검색:** `keywords=` 파라미터에 쉼표로 구분된 여러 키워드를 지정하고, `operator=AND|OR` 파라미터를 통해 이들 간의 논리적 관계(모두 일치 또는 하나라도 일치)를 지정할 수 있습니다.
- **`HELP` 명령 추가:** 새로 추가된 복잡한 쿼리 구문을 설명하는 도움말 메시지를 제공합니다.

**아키텍처 변경:**
- **`QueryParser` 모듈 도입:** 쿼리 문자열을 파싱하는 책임이 `QueryHandler`에서 완전히 새로운 `QueryParser` 모듈로 분리됩니다. 이는 관심사 분리(Separation of Concerns) 원칙을 따르며 코드의 모듈성과 유지보수성을 높입니다.
- **`LogBuffer` 검색 기능 강화:** 기존의 `log_buffer_search` 외에, 파싱된 쿼리 구조체를 인자로 받는 `log_buffer_search_enhanced` 함수가 추가됩니다. 이 함수는 다양한 필터링 조건을 조합하여 검색을 수행합니다.

이 MVP를 통해 LogCaster는 단순한 로그 저장소를 넘어, 복잡한 분석 요구사항에 대응할 수 있는 강력한 실시간 데이터 조회 도구로서의 면모를 갖추게 됩니다.

## 2. 빌드 시스템 (Build System)

새로운 `query_parser.c` 파일이 추가됨에 따라, `Makefile`의 소스 목록(`SRCS`)이 이를 포함하도록 업데이트됩니다. 다른 설정은 MVP2와 동일합니다.

```makefile
# [SEQUENCE: MVP3-1]
# LogCaster-C Makefile - MVP3 with enhanced query
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11 -O2 -D_GNU_SOURCE
LDFLAGS = -pthread
INCLUDES = -I./include

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# [SEQUENCE: MVP3-2]
# Source and object files (query_parser.c added)
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Target executable
TARGET = $(BIN_DIR)/logcaster-c

.PHONY: all clean

all: directories $(TARGET)

directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

$(OBJ_DIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Clean complete"
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. `include/query_parser.h` (신규 파일)

고급 쿼리를 파싱하고, 그 결과를 담는 구조체(`parsed_query_t`)와 관련 함수들을 정의합니다.

```c
// [SEQUENCE: MVP3-3]
#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <stdbool.h>
#include <time.h>
#include <regex.h>

// [SEQUENCE: MVP3-4]
// 쿼리 연산자 종류 (AND/OR)
typedef enum {
    OP_AND,
    OP_OR
} operator_type_t;

// [SEQUENCE: MVP3-5]
// 파싱된 쿼리 정보를 담는 구조체
typedef struct {
    char* keywords[10];          // 다중 키워드 배열
    int keyword_count;           // 키워드 개수
    char* regex_pattern;         // 정규식 패턴 문자열
    regex_t* compiled_regex;     // 컴파일된 정규식 객체
    time_t time_from;            // 시작 시간 필터
    time_t time_to;              // 종료 시간 필터
    operator_type_t op;          // 키워드 간 논리 연산자
    bool has_regex;
    bool has_time_filter;
} parsed_query_t;

// [SEQUENCE: MVP3-6]
// 쿼리 파서 생명주기 및 파싱 함수
parsed_query_t* query_parser_create(void);
void query_parser_destroy(parsed_query_t* query);
int query_parser_parse(parsed_query_t* query, const char* query_string);

// [SEQUENCE: MVP3-7]
// 로그 항목이 쿼리 조건과 일치하는지 확인하는 함수
bool query_matches_log(const parsed_query_t* query, const char* log_message, time_t timestamp);

#endif // QUERY_PARSER_H
```

### 3.2. `src/query_parser.c` (신규 파일)

쿼리 파서의 핵심 로직을 구현합니다. `strtok_r`을 사용하여 쿼리 문자열을 토큰으로 분리하고, 각 `key=value` 쌍을 분석하여 `parsed_query_t` 구조체를 채웁니다.

```c
// [SEQUENCE: MVP3-8]
#include "query_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <strings.h> // for strcasecmp

// [SEQUENCE: MVP3-9]
// parsed_query_t 구조체 생성 및 초기화
parsed_query_t* query_parser_create(void) {
    parsed_query_t* query = calloc(1, sizeof(parsed_query_t));
    if (query) {
        query->op = OP_AND; // 기본 연산자는 AND
    }
    return query;
}

// [SEQUENCE: MVP3-10]
// 파서가 할당한 모든 동적 메모리 해제
void query_parser_destroy(parsed_query_t* query) {
    if (!query) return;
    for (int i = 0; i < query->keyword_count; i++) {
        free(query->keywords[i]);
    }
    if (query->regex_pattern) {
        free(query->regex_pattern);
    }
    if (query->compiled_regex) {
        regfree(query->compiled_regex);
        free(query->compiled_regex);
    }
    free(query);
}

// [SEQUENCE: MVP3-11]
// 쿼리 문자열 파싱
int query_parser_parse(parsed_query_t* query, const char* query_string) {
    if (!query || !query_string) return -1;

    char* query_copy = strdup(query_string);
    if (!query_copy) return -1;

    // "QUERY " 접두사 제거
    char* params_str = query_copy;
    if (strncmp(params_str, "QUERY ", 6) == 0) {
        params_str += 6;
    }

    // [SEQUENCE: MVP3-12]
    // `strtok_r`을 사용하여 스페이스 기준으로 파라미터 분리 (스레드 안전)
    char* saveptr1;
    char* token = strtok_r(params_str, " ", &saveptr1);
    while (token) {
        char* equals = strchr(token, '=');
        if (equals) {
            *equals = '\0'; // key와 value 분리
            char* key = token;
            char* value = equals + 1;

            // [SEQUENCE: MVP3-13]
            // 파라미터 종류에 따라 처리
            if (strcasecmp(key, "keywords") == 0 || strcasecmp(key, "keyword") == 0) {
                char* saveptr2;
                char* keyword_token = strtok_r(value, ",", &saveptr2);
                while (keyword_token && query->keyword_count < 10) {
                    query->keywords[query->keyword_count++] = strdup(keyword_token);
                    keyword_token = strtok_r(NULL, ",", &saveptr2);
                }
            } else if (strcasecmp(key, "regex") == 0) {
                query->has_regex = true;
                query->regex_pattern = strdup(value);
                query->compiled_regex = malloc(sizeof(regex_t));
                if (regcomp(query->compiled_regex, value, REG_EXTENDED | REG_NOSUB) != 0) {
                    // 정규식 컴파일 실패 처리
                    free(query->compiled_regex);
                    query->compiled_regex = NULL;
                }
            } else if (strcasecmp(key, "time_from") == 0) {
                query->has_time_filter = true;
                query->time_from = atol(value);
            } else if (strcasecmp(key, "time_to") == 0) {
                query->has_time_filter = true;
                query->time_to = atol(value);
            } else if (strcasecmp(key, "operator") == 0) {
                if (strcasecmp(value, "OR") == 0) {
                    query->op = OP_OR;
                }
            }
        }
        token = strtok_r(NULL, " ", &saveptr1);
    }

    free(query_copy);
    return 0;
}

// [SEQUENCE: MVP3-14]
// 로그가 쿼리 조건에 부합하는지 검사
bool query_matches_log(const parsed_query_t* query, const char* log_message, time_t timestamp) {
    // 시간 필터 검사
    if (query->has_time_filter) {
        if (query->time_from > 0 && timestamp < query->time_from) return false;
        if (query->time_to > 0 && timestamp > query->time_to) return false;
    }

    // 정규식 필터 검사
    if (query->has_regex && query->compiled_regex) {
        if (regexec(query->compiled_regex, log_message, 0, NULL, 0) != 0) {
            return false; // 정규식 불일치
        }
    }

    // 키워드 필터 검사
    if (query->keyword_count > 0) {
        bool match = (query->op == OP_AND);
        for (int i = 0; i < query->keyword_count; i++) {
            bool found = (strstr(log_message, query->keywords[i]) != NULL);
            if (query->op == OP_AND && !found) return false;
            if (query->op == OP_OR && found) {
                match = true;
                break;
            }
        }
        if (!match) return false;
    }

    return true;
}
```

### 3.3. `include/log_buffer.h` (MVP3 업데이트)

고급 검색을 위한 `log_buffer_search_enhanced` 함수의 프로토타입을 추가합니다.

```c
// [SEQUENCE: MVP3-15]
#ifndef LOG_BUFFER_H
#define LOG_BUFFER_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

// Forward declaration to avoid circular include
struct parsed_query;

#define DEFAULT_BUFFER_SIZE 10000

typedef struct {
    char* message;
    time_t timestamp;
} log_entry_t;

typedef struct {
    log_entry_t** entries;
    size_t capacity;
    size_t size;
    size_t head;
    size_t tail;
    pthread_mutex_t mutex;
    unsigned long total_logs;
    unsigned long dropped_logs;
} log_buffer_t;

log_buffer_t* log_buffer_create(size_t capacity);
void log_buffer_destroy(log_buffer_t* buffer);
int log_buffer_push(log_buffer_t* buffer, const char* message);

// [SEQUENCE: MVP3-16]
// MVP3에 추가된 고급 검색 함수
int log_buffer_search_enhanced(log_buffer_t* buffer, const struct parsed_query* query, char*** results, int* count);

size_t log_buffer_size(log_buffer_t* buffer);
void log_buffer_get_stats(log_buffer_t* buffer, unsigned long* total, unsigned long* dropped);

#endif // LOG_BUFFER_H
```

### 3.4. `src/log_buffer.c` (MVP3 업데이트)

`log_buffer_search_enhanced` 함수를 구현합니다. 이 함수는 `query_matches_log` 헬퍼 함수를 사용하여 각 로그 항목이 복잡한 쿼리 조건에 부합하는지 확인합니다.

```c
// [SEQUENCE: MVP3-17]
// ... (log_buffer_create, push 등 기존 함수는 동일) ...

#include "query_parser.h" // query_matches_log 사용을 위해 추가

// [SEQUENCE: MVP3-18]
// 고급 검색 기능 구현
int log_buffer_search_enhanced(log_buffer_t* buffer, const struct parsed_query* query, char*** results, int* count) {
    if (!buffer || !query || !results || !count) return -1;

    pthread_mutex_lock(&buffer->mutex);

    // 1. 일치하는 로그 개수 세기
    *count = 0;
    for (size_t i = 0; i < buffer->size; i++) {
        size_t idx = (buffer->tail + i) % buffer->capacity;
        if (query_matches_log(query, buffer->entries[idx]->message, buffer->entries[idx]->timestamp)) {
            (*count)++;
        }
    }

    if (*count == 0) {
        pthread_mutex_unlock(&buffer->mutex);
        *results = NULL;
        return 0;
    }

    // 2. 결과 저장을 위한 메모리 할당
    *results = malloc(sizeof(char*) * (*count));
    if (!*results) {
        pthread_mutex_unlock(&buffer->mutex);
        return -1;
    }

    // 3. 결과 복사 (타임스탬프 포함)
    int result_idx = 0;
    for (size_t i = 0; i < buffer->size && result_idx < *count; i++) {
        size_t idx = (buffer->tail + i) % buffer->capacity;
        log_entry_t* entry = buffer->entries[idx];
        if (query_matches_log(query, entry->message, entry->timestamp)) {
            char timestamp_str[32];
            strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%d %H:%M:%S", localtime(&entry->timestamp));
            
            size_t len = strlen(timestamp_str) + strlen(entry->message) + 5; // for "[] " and null
            char* res_str = malloc(len);
            snprintf(res_str, len, "[%s] %s", timestamp_str, entry->message);
            (*results)[result_idx++] = res_str;
        }
    }

    pthread_mutex_unlock(&buffer->mutex);
    return 0;
}

// ... (destroy, size, stats 함수는 동일) ...
```

### 3.5. `src/query_handler.c` (MVP3 업데이트)

`QUERY` 명령을 받았을 때, 새로 추가된 `QueryParser`를 사용하여 명령을 파싱하고, `log_buffer_search_enhanced`를 호출하도록 로직을 수정합니다. `HELP` 명령도 추가됩니다.

```c
// [SEQUENCE: MVP3-19]
#include "query_handler.h"
#include "server.h"
#include "query_parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

// [SEQUENCE: MVP3-20]
// HELP 명령에 대한 응답 생성
static void send_help(int client_fd) {
    const char* help_msg = 
        "LogCaster Query Interface - MVP3\n"
        "================================\n"
        "Commands:\n"
        "  STATS                    - Show statistics\n"
        "  COUNT                    - Show log count\n"
        "  HELP                     - Show this help\n"
        "  QUERY param=value ...    - Enhanced search\n"
        "\n"
        "Query Parameters:\n"
        "  keywords=word1,word2     - Multiple keywords\n"
        "  regex=pattern            - POSIX regex pattern\n"
        "  time_from=timestamp      - Start time (Unix timestamp)\n"
        "  time_to=timestamp        - End time (Unix timestamp)\n"
        "  operator=AND|OR          - Keyword logic (default: AND)\n"
        "\n"
        "Example: QUERY keywords=error,timeout operator=AND regex=.*failed.*\n";
    send(client_fd, help_msg, strlen(help_msg), 0);
}

// [SEQUENCE: MVP3-21]
// 쿼리 명령 처리 로직 (MVP3 버전)
static void process_query_command(log_server_t* server, int client_fd, const char* command) {
    char response[BUFFER_SIZE];

    if (strncmp(command, "QUERY", 5) == 0) {
        parsed_query_t* query = query_parser_create();
        if (query_parser_parse(query, command) != 0) {
            snprintf(response, sizeof(response), "ERROR: Invalid query syntax\n");
            send(client_fd, response, strlen(response), 0);
        } else {
            char** results = NULL;
            int count = 0;
            log_buffer_search_enhanced(server->log_buffer, query, &results, &count);
            snprintf(response, sizeof(response), "FOUND: %d matches\n", count);
            send(client_fd, response, strlen(response), 0);
            for (int i = 0; i < count; i++) {
                snprintf(response, sizeof(response), "%s\n", results[i]);
                send(client_fd, response, strlen(response), 0);
                free(results[i]);
            }
            if (results) free(results);
        }
        query_parser_destroy(query);
    } else if (strcmp(command, "STATS") == 0) {
        // ... (MVP2와 동일)
    } else if (strcmp(command, "COUNT") == 0) {
        // ... (MVP2와 동일)
    } else if (strcmp(command, "HELP") == 0) {
        send_help(client_fd);
    } else {
        snprintf(response, sizeof(response), "ERROR: Unknown command. Use HELP for usage.\n");
        send(client_fd, response, strlen(response), 0);
    }
}

// [SEQUENCE: MVP3-22]
// 쿼리 연결 수락 및 처리 (MVP2와 동일)
void handle_query_connection(log_server_t* server) {
    // ... (MVP2와 동일한 로직)
}
```

## 4. 테스트 코드 (Test Code)

고급 쿼리 기능을 테스트하기 위해 새로운 Python 테스트 스크립트(`tests/test_mvp3.py`)가 작성되었습니다. 이 스크립트는 정규식, 시간 범위, 다중 키워드 등 다양한 조합의 쿼리를 서버에 전송하고 결과를 검증합니다.

```python
# [SEQUENCE: MVP3-23]
#!/usr/bin/env python3
import socket
import time
import threading

HOST = '127.0.0.1'
LOG_PORT = 9999
QUERY_PORT = 9998

def send_logs():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, LOG_PORT))
        logs = [
            "2025-07-28 INFO: Application startup successful",
            "2025-07-28 DEBUG: Database connection pool created with 10 connections",
            "2025-07-28 WARNING: Configuration value 'timeout' is deprecated, use 'connection_timeout' instead",
            "2025-07-28 ERROR: Failed to process user request for user_id=123, reason: timeout",
            "2025-07-28 INFO: User logged in: admin",
            "2025-07-28 ERROR: Critical failure in payment module, transaction_id=xyz-789"
        ]
        for log in logs:
            s.sendall((log + '\n').encode())
            time.sleep(0.1)

def query_server(query):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, QUERY_PORT))
        s.sendall((query + '\n').encode())
        response = s.recv(4096).decode()
        return response

def run_test(description, query, expected_lines):
    print(f"---" {description} "---")
    print(f"> " query)
    response = query_server(query)
    print(response.strip())
    assert response.startswith(f"FOUND: {expected_lines}")
    print("OK\n")

if __name__ == "__main__":
    # Give server a moment to start
    time.sleep(1)
    # Send some initial logs
    log_thread = threading.Thread(target=send_logs)
    log_thread.start()
    # Wait for logs to be processed
    time.sleep(1)

    # Run tests
    run_test("Test 1: Simple keyword search", "QUERY keywords=timeout", 2)
    run_test("Test 2: Regex search", "QUERY regex=user_id=[0-9]+", 1)
    run_test("Test 3: Multi-keyword AND search", "QUERY keywords=Failed,timeout operator=AND", 1)
    run_test("Test 4: Multi-keyword OR search", "QUERY keywords=startup,failure operator=OR", 2)
    
    # Assuming current time is around 2025-07-28
    # These timestamps may need adjustment depending on when the test is run
    # For simplicity, we search for a wide range
    run_test("Test 5: Time range search", f"QUERY time_from=1753689600 time_to=1753862400", 6)
    run_test("Test 6: Combined search (Regex and Keyword)", "QUERY regex=deprecate keywords=WARNING", 1)
    run_test("Test 7: HELP command", "HELP", 13)

    log_thread.join()
    print("All MVP3 tests passed!")
```