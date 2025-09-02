# DevHistory 03: LogCaster-C MVP2 - Thread Pool & In-Memory Buffer

## 1. 개요 (Overview)

C 구현의 두 번째 단계(MVP2)는 서버의 동시성 모델을 근본적으로 개선하고, 수신된 로그를 실제로 저장 및 조회할 수 있는 기반을 마련하는 데 중점을 둡니다. 이 단계는 `select` 기반의 단일 스레드 처리 모델에서 벗어나, 더 높은 처리량과 확장성을 제공하는 아키텍처로 전환하는 중요한 마일스톤입니다.

**주요 기능 추가:**
- **스레드 풀 (Thread Pool):** 클라이언트 요청을 처리하기 위한 고정된 수의 작업자 스레드 풀을 구현합니다. 이를 통해 클라이언트 연결마다 스레드를 생성하는 오버헤드를 제거하고 시스템 리소스를 효율적으로 관리합니다.
- **인메모리 로그 버퍼 (In-Memory Log Buffer):** 수신된 로그를 표준 출력으로 내보내는 대신, 메모리 내의 스레드 안전(thread-safe) 원형 버퍼(circular buffer)에 저장합니다. 이는 로그의 임시 저장소 역할을 하며, 실시간 쿼리의 기반이 됩니다.
- **쿼리 인터페이스:** 로그 수집 포트(`9999`)와 별도로, 쿼리 전용 포트(`9998`)를 추가합니다. 이 포트를 통해 서버의 상태를 확인하거나 저장된 로그를 검색하는 간단한 명령을 실행할 수 있습니다.

**아키텍처 변경:**
- **생산자-소비자 패턴:** 메인 스레드(`server_run`)는 클라이언트 연결을 수락하는 '생산자' 역할을 합니다. 수락된 연결(작업)은 작업 큐(job queue)에 추가됩니다. 스레드 풀의 작업자 스레드들은 '소비자'로서 작업 큐에서 작업을 가져와 실제 데이터 읽기 및 로그 버퍼 저장을 처리합니다.
- **동기화:** 스레드 풀의 작업 큐와 로그 버퍼는 `pthread_mutex_t`와 `pthread_cond_t`를 사용하여 여러 스레드로부터의 동시 접근을 안전하게 처리합니다.

이 MVP를 통해 LogCaster는 단순한 로그 출력기에서 벗어나, 여러 요청을 동시에 처리하고 데이터를 저장하며 조회할 수 있는 서버의 기본 골격을 갖추게 됩니다.

## 2. 빌드 시스템 (Build System)

MVP2에서는 `thread_pool.c`, `log_buffer.c`, `query_handler.c` 등 새로운 소스 파일들이 추가되었습니다. `Makefile`은 이 파일들을 포함하도록 업데이트됩니다.

```makefile
# [SEQUENCE: MVP3-1]
# LogCaster-C Makefile - MVP2 with thread pool
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11 -O2 -D_GNU_SOURCE
LDFLAGS = -pthread
INCLUDES = -I./include

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Target executable
TARGET = $(BIN_DIR)/logcaster-c

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Build target
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Clean complete"

.PHONY: all clean directories
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. `include/thread_pool.h`

작업 큐와 작업자 스레드들을 관리하는 스레드 풀의 자료구조와 API를 정의합니다.

```c
// [SEQUENCE: MVP3-2]
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdbool.h>

#define DEFAULT_THREAD_COUNT 4
#define MAX_THREAD_COUNT 32

// [SEQUENCE: MVP3-3]
// 스레드 풀이 처리할 작업을 정의하는 구조체
typedef struct thread_job {
    void (*function)(void* arg); // 작업 함수 포인터
    void* arg;                   // 작업 함수에 전달될 인자
    struct thread_job* next;     // 다음 작업을 가리키는 포인터 (연결 리스트)
} thread_job_t;

// [SEQUENCE: MVP3-4]
// 스레드 풀을 관리하는 메인 구조체
typedef struct {
    pthread_t* threads;          // 작업자 스레드 배열
    int thread_count;            // 스레드 수
    
    thread_job_t* job_queue_head; // 작업 큐의 시작
    thread_job_t* job_queue_tail; // 작업 큐의 끝
    int job_count;               // 큐에 있는 작업의 수
    
    // 동기화 프리미티브
    pthread_mutex_t queue_mutex; // 작업 큐 접근을 위한 뮤텍스
    pthread_cond_t job_available;  // 새로운 작업이 추가되었음을 알리는 조건 변수
    pthread_cond_t all_jobs_done;  // 모든 작업이 완료되었음을 알리는 조건 변수
    
    // 풀 상태 관리
    volatile bool shutdown;        // 풀 종료 여부 플래그
    int working_threads;         // 현재 작업 중인 스레드 수
} thread_pool_t;

// [SEQUENCE: MVP3-5]
// 스레드 풀 생명주기 함수
thread_pool_t* thread_pool_create(int thread_count);
void thread_pool_destroy(thread_pool_t* pool);

// [SEQUENCE: MVP3-6]
// 작업 제출 및 대기 함수
int thread_pool_add_job(thread_pool_t* pool, void (*function)(void*), void* arg);
void thread_pool_wait(thread_pool_t* pool);

#endif // THREAD_POOL_H
```

### 3.2. `src/thread_pool.c`

스레드 풀의 생성, 소멸, 작업 추가 및 작업자 스레드의 실행 로직을 구현합니다.

```c
// [SEQUENCE: MVP3-7]
#include "thread_pool.h"
#include <stdlib.h>
#include <stdio.h>

// [SEQUENCE: MVP3-8]
// 작업자 스레드가 실행하는 함수
static void* thread_pool_worker(void* arg) {
    thread_pool_t* pool = (thread_pool_t*)arg;
    thread_job_t* job;
    
    while (1) {
        // [SEQUENCE: MVP3-9]
        // 작업 큐에 접근하기 위해 뮤텍스 잠금
        pthread_mutex_lock(&pool->queue_mutex);
        
        // 작업 큐가 비어있고, 풀이 종료되지 않았으면 대기
        while (pool->job_count == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->job_available, &pool->queue_mutex);
        }
        
        // 풀이 종료 신호를 받았으면 루프 탈출
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }
        
        // [SEQUENCE: MVP3-10]
        // 작업 큐에서 작업(job)을 하나 가져옴
        job = pool->job_queue_head;
        pool->job_queue_head = job->next;
        if (!pool->job_queue_head) {
            pool->job_queue_tail = NULL;
        }
        pool->job_count--;
        pool->working_threads++;
        
        pthread_mutex_unlock(&pool->queue_mutex);
        
        // [SEQUENCE: MVP3-11]
        // 잠금을 해제한 상태에서 실제 작업 수행
        if (job) {
            job->function(job->arg);
            free(job);
        }

        pthread_mutex_lock(&pool->queue_mutex);
        pool->working_threads--;
        if (!pool->shutdown && pool->working_threads == 0 && pool->job_count == 0) {
            pthread_cond_signal(&pool->all_jobs_done);
        }
        pthread_mutex_unlock(&pool->queue_mutex);
    }
    
    pthread_exit(NULL);
}

// [SEQUENCE: MVP3-12]
// 스레드 풀 생성
thread_pool_t* thread_pool_create(int thread_count) {
    if (thread_count <= 0 || thread_count > MAX_THREAD_COUNT) {
        thread_count = DEFAULT_THREAD_COUNT;
    }
    
    thread_pool_t* pool = calloc(1, sizeof(thread_pool_t));
    if (!pool) return NULL;

    pool->thread_count = thread_count;
    pool->threads = malloc(sizeof(pthread_t) * thread_count);
    if (!pool->threads) {
        free(pool);
        return NULL;
    }
    
    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->job_available, NULL);
    pthread_cond_init(&pool->all_jobs_done, NULL);
    
    // [SEQUENCE: MVP3-13]
    // 작업자 스레드 생성
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&pool->threads[i], NULL, thread_pool_worker, pool) != 0) {
            thread_pool_destroy(pool); // 실패 시 정리
            return NULL;
        }
    }
    
    return pool;
}

// [SEQUENCE: MVP3-14]
// 작업 큐에 작업 추가
int thread_pool_add_job(thread_pool_t* pool, void (*function)(void*), void* arg) {
    if (!pool || !function || pool->shutdown) return -1;

    thread_job_t* job = malloc(sizeof(thread_job_t));
    if (!job) return -1;

    job->function = function;
    job->arg = arg;
    job->next = NULL;

    pthread_mutex_lock(&pool->queue_mutex);
    
    if (pool->job_queue_tail) {
        pool->job_queue_tail->next = job;
        pool->job_queue_tail = job;
    } else {
        pool->job_queue_head = job;
        pool->job_queue_tail = job;
    }
    pool->job_count++;

    // [SEQUENCE: MVP3-15]
    // 대기 중인 스레드에게 새 작업이 있음을 알림
    pthread_cond_signal(&pool->job_available);
    pthread_mutex_unlock(&pool->queue_mutex);

    return 0;
}

// [SEQUENCE: MVP3-16]
// 모든 작업이 완료될 때까지 대기
void thread_pool_wait(thread_pool_t* pool) {
    if (!pool) return;
    pthread_mutex_lock(&pool->queue_mutex);
    while (pool->job_count > 0 || pool->working_threads > 0) {
        pthread_cond_wait(&pool->all_jobs_done, &pool->queue_mutex);
    }
    pthread_mutex_unlock(&pool->queue_mutex);
}

// [SEQUENCE: MVP3-17]
// 스레드 풀 종료 및 리소스 해제
void thread_pool_destroy(thread_pool_t* pool) {
    if (!pool) return;

    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = true;
    pthread_cond_broadcast(&pool->job_available);
    pthread_mutex_unlock(&pool->queue_mutex);

    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    // 남은 작업 큐 정리
    thread_job_t* job;
    while (pool->job_queue_head) {
        job = pool->job_queue_head;
        pool->job_queue_head = job->next;
        free(job);
    }

    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->job_available);
    pthread_cond_destroy(&pool->all_jobs_done);

    free(pool->threads);
    free(pool);
}
```

### 3.3. `include/log_buffer.h`

로그 메시지를 저장하는 스레드 안전 원형 버퍼의 자료구조와 API를 정의합니다.

```c
// [SEQUENCE: MVP3-18]
#ifndef LOG_BUFFER_H
#define LOG_BUFFER_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#define DEFAULT_BUFFER_SIZE 10000

// [SEQUENCE: MVP3-19]
// 로그 항목을 표현하는 구조체
typedef struct {
    char* message;
    time_t timestamp;
} log_entry_t;

// [SEQUENCE: MVP3-20]
// 스레드 안전 원형 로그 버퍼 구조체
typedef struct {
    log_entry_t** entries;    // 로그 항목 포인터의 원형 배열
    size_t capacity;          // 최대 용량
    size_t size;              // 현재 크기
    size_t head;              // 다음 쓰기 위치
    size_t tail;              // 다음 읽기 위치
    
    pthread_mutex_t mutex;    // 버퍼 접근을 위한 뮤텍스
    
    // 통계 정보
    unsigned long total_logs;
    unsigned long dropped_logs;
} log_buffer_t;

// [SEQUENCE: MVP3-21]
// 버퍼 생명주기 함수
log_buffer_t* log_buffer_create(size_t capacity);
void log_buffer_destroy(log_buffer_t* buffer);

// [SEQUENCE: MVP3-22]
// 버퍼 조작 함수
int log_buffer_push(log_buffer_t* buffer, const char* message);
int log_buffer_search(log_buffer_t* buffer, const char* keyword, char*** results, int* count);

// [SEQUENCE: MVP3-23]
// 버퍼 상태 조회 함수
size_t log_buffer_size(log_buffer_t* buffer);
void log_buffer_get_stats(log_buffer_t* buffer, unsigned long* total, unsigned long* dropped);

#endif // LOG_BUFFER_H
```

### 3.4. `src/log_buffer.c`

로그 버퍼의 생성, 소멸, 로그 추가(`push`), 기본 검색(`search`) 로직을 구현합니다.

```c
// [SEQUENCE: MVP3-24]
#include "log_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// [SEQUENCE: MVP3-25]
// 로그 버퍼 생성
log_buffer_t* log_buffer_create(size_t capacity) {
    if (capacity == 0) capacity = DEFAULT_BUFFER_SIZE;
    
    log_buffer_t* buffer = calloc(1, sizeof(log_buffer_t));
    if (!buffer) return NULL;
    
    buffer->entries = calloc(capacity, sizeof(log_entry_t*));
    if (!buffer->entries) {
        free(buffer);
        return NULL;
    }
    
    buffer->capacity = capacity;
    pthread_mutex_init(&buffer->mutex, NULL);
    
    return buffer;
}

// [SEQUENCE: MVP3-26]
// 로그 버퍼에 로그 추가
int log_buffer_push(log_buffer_t* buffer, const char* message) {
    if (!buffer || !message) return -1;
    
    log_entry_t* entry = malloc(sizeof(log_entry_t));
    if (!entry) return -1;
    
    entry->message = strdup(message);
    if (!entry->message) {
        free(entry);
        return -1;
    }
    entry->timestamp = time(NULL);
    
    pthread_mutex_lock(&buffer->mutex);
    
    // [SEQUENCE: MVP3-27]
    // 버퍼가 가득 찼으면 가장 오래된 로그를 삭제 (덮어쓰기)
    if (buffer->size == buffer->capacity) {
        log_entry_t* old_entry = buffer->entries[buffer->tail];
        free(old_entry->message);
        free(old_entry);
        buffer->tail = (buffer->tail + 1) % buffer->capacity;
        buffer->size--;
        buffer->dropped_logs++;
    }
    
    // [SEQUENCE: MVP3-28]
    // 새로운 로그를 head 위치에 추가
    buffer->entries[buffer->head] = entry;
    buffer->head = (buffer->head + 1) % buffer->capacity;
    buffer->size++;
    buffer->total_logs++;
    
    pthread_mutex_unlock(&buffer->mutex);
    return 0;
}

// [SEQUENCE: MVP3-29]
// 키워드를 포함하는 로그 검색 (MVP2 기본 버전)
int log_buffer_search(log_buffer_t* buffer, const char* keyword, char*** results, int* count) {
    if (!buffer || !keyword || !results || !count) return -1;
    
    pthread_mutex_lock(&buffer->mutex);
    
    // 1. 일치하는 로그 개수 세기
    *count = 0;
    for (size_t i = 0; i < buffer->size; i++) {
        size_t idx = (buffer->tail + i) % buffer->capacity;
        if (strstr(buffer->entries[idx]->message, keyword)) {
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
    
    // 3. 결과 복사
    int result_idx = 0;
    for (size_t i = 0; i < buffer->size && result_idx < *count; i++) {
        size_t idx = (buffer->tail + i) % buffer->capacity;
        if (strstr(buffer->entries[idx]->message, keyword)) {
            (*results)[result_idx++] = strdup(buffer->entries[idx]->message);
        }
    }
    
    pthread_mutex_unlock(&buffer->mutex);
    return 0;
}

// [SEQUENCE: MVP3-30]
// 버퍼 상태 조회 함수들
size_t log_buffer_size(log_buffer_t* buffer) {
    if (!buffer) return 0;
    pthread_mutex_lock(&buffer->mutex);
    size_t size = buffer->size;
    pthread_mutex_unlock(&buffer->mutex);
    return size;
}

void log_buffer_get_stats(log_buffer_t* buffer, unsigned long* total, unsigned long* dropped) {
    if (!buffer) return;
    pthread_mutex_lock(&buffer->mutex);
    *total = buffer->total_logs;
    *dropped = buffer->dropped_logs;
    pthread_mutex_unlock(&buffer->mutex);
}

// [SEQUENCE: MVP3-31]
// 로그 버퍼 소멸
void log_buffer_destroy(log_buffer_t* buffer) {
    if (!buffer) return;
    for (size_t i = 0; i < buffer->size; i++) {
        size_t idx = (buffer->tail + i) % buffer->capacity;
        free(buffer->entries[idx]->message);
        free(buffer->entries[idx]);
    }
    pthread_mutex_destroy(&buffer->mutex);
    free(buffer->entries);
    free(buffer);
}
```

### 3.5. `include/query_handler.h`

쿼리 포트로 들어온 요청을 처리하는 함수의 프로토타입을 정의합니다.

```c
// [SEQUENCE: MVP3-32]
#ifndef QUERY_HANDLER_H
#define QUERY_HANDLER_H

// Forward declaration to avoid circular dependency
struct log_server;

void handle_query_connection(struct log_server* server);

#endif // QUERY_HANDLER_H
```

### 3.6. `src/query_handler.c`

쿼리 연결을 수락하고, 들어온 명령을 파싱하여 해당하는 작업을 수행합니다. MVP2에서는 `STATS`, `COUNT`, `QUERY keyword=` 세 가지 명령을 처리합니다.

```c
// [SEQUENCE: MVP3-33]
#include "query_handler.h"
#include "server.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

// [SEQUENCE: MVP3-34]
// 쿼리 명령 처리
static void process_query_command(log_server_t* server, int client_fd, const char* command) {
    char response[BUFFER_SIZE];

    if (strncmp(command, "QUERY keyword=", 14) == 0) {
        const char* keyword = command + 14;
        char** results = NULL;
        int count = 0;
        if (log_buffer_search(server->log_buffer, keyword, &results, &count) == 0) {
            snprintf(response, sizeof(response), "FOUND: %d matches\n", count);
            send(client_fd, response, strlen(response), 0);
            for (int i = 0; i < count; i++) {
                snprintf(response, sizeof(response), "%s\n", results[i]);
                send(client_fd, response, strlen(response), 0);
                free(results[i]);
            }
            if (results) free(results);
        } else {
            snprintf(response, sizeof(response), "ERROR: Search failed\n");
            send(client_fd, response, strlen(response), 0);
        }
    } else if (strcmp(command, "STATS") == 0) {
        unsigned long total, dropped;
        log_buffer_get_stats(server->log_buffer, &total, &dropped);
        snprintf(response, sizeof(response), "STATS: Total=%lu, Dropped=%lu, Current=%zu, Clients=%d\n",
                 total, dropped, log_buffer_size(server->log_buffer), server->client_count);
        send(client_fd, response, strlen(response), 0);
    } else if (strcmp(command, "COUNT") == 0) {
        snprintf(response, sizeof(response), "COUNT: %zu\n", log_buffer_size(server->log_buffer));
        send(client_fd, response, strlen(response), 0);
    } else {
        snprintf(response, sizeof(response), "ERROR: Unknown command\n");
        send(client_fd, response, strlen(response), 0);
    }
}

// [SEQUENCE: MVP3-35]
// 쿼리 연결 수락 및 처리
void handle_query_connection(log_server_t* server) {
    int client_fd = accept(server->query_fd, NULL, NULL);
    if (client_fd < 0) return;

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        }
        process_query_command(server, client_fd, buffer);
    }
    close(client_fd);
}
```

### 3.7. `include/server.h` (MVP2 업데이트)

`log_server_t` 구조체에 스레드 풀과 로그 버퍼 포인터를 추가하고, 쿼리 포트 관련 필드를 추가합니다. 클라이언트 작업을 스레드 풀에 전달하기 위한 `client_job_t` 구조체도 새로 정의합니다.

```c
// [SEQUENCE: MVP3-36]
#ifndef SERVER_H
#define SERVER_H

#include <sys/select.h>
#include <signal.h>
#include "thread_pool.h"
#include "log_buffer.h"

#define DEFAULT_PORT 9999
#define QUERY_PORT 9998
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 4096

// [SEQUENCE: MVP3-37]
// 서버 구조체 (MVP2 버전)
typedef struct log_server {
    int port;
    int query_port;
    int listen_fd;
    int query_fd;
    fd_set master_set;
    fd_set read_set;
    int max_fd;
    int client_count;
    volatile sig_atomic_t running;
    
    // MVP2 추가 사항
    thread_pool_t* thread_pool;
    log_buffer_t* log_buffer;
} log_server_t;

// [SEQUENCE: MVP3-38]
// 클라이언트 작업을 위한 컨텍스트 구조체
typedef struct {
    int client_fd;
    log_server_t* server;
} client_job_t;

// [SEQUENCE: MVP3-39]
// 서버 생명주기 함수
log_server_t* server_create(int port);
void server_destroy(log_server_t* server);
int server_init(log_server_t* server);
void server_run(log_server_t* server);

#endif // SERVER_H
```

### 3.8. `src/server.c` (MVP2 업데이트)

서버의 메인 로직이 크게 변경됩니다. `select` 루프는 이제 연결 수락만 담당하고, 실제 데이터 처리는 `handle_client_job` 함수로 분리되어 스레드 풀에서 실행됩니다.

```c
// [SEQUENCE: MVP3-40]
#include "server.h"
#include "query_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

static log_server_t* g_server = NULL;

static void sigint_handler(int sig) {
    (void)sig;
    if (g_server) g_server->running = 0;
}

// [SEQUENCE: MVP3-41]
// 스레드 풀에서 실행될 클라이언트 처리 작업
static void handle_client_job(void* arg) {
    client_job_t* job = (client_job_t*)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // 데이터 수신 및 로그 버퍼에 저장
    while ((bytes_read = recv(job->client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        }
        log_buffer_push(job->server->log_buffer, buffer);
    }

    // 연결 종료 처리
    close(job->client_fd);
    // 참고: 이 버전에서는 client_count를 감소시키지 않아 버그가 존재함 (MVP5에서 수정)
    free(job);
}

// [SEQUENCE: MVP3-42]
// 서버 생성 (MVP2)
log_server_t* server_create(int port) {
    log_server_t* server = calloc(1, sizeof(log_server_t));
    if (!server) return NULL;

    server->port = port;
    server->query_port = QUERY_PORT;
    server->running = 1;

    server->thread_pool = thread_pool_create(DEFAULT_THREAD_COUNT);
    if (!server->thread_pool) { free(server); return NULL; }

    server->log_buffer = log_buffer_create(DEFAULT_BUFFER_SIZE);
    if (!server->log_buffer) { thread_pool_destroy(server->thread_pool); free(server); return NULL; }

    return server;
}

// [SEQUENCE: MVP3-43]
// 리스너 소켓 초기화 헬퍼 함수
static int init_listener(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return -1; }
    if (listen(fd, MAX_CLIENTS) < 0) { close(fd); return -1; }
    return fd;
}

// [SEQUENCE: MVP3-44]
// 서버 초기화 (두 개의 리스너)
int server_init(log_server_t* server) {
    server->listen_fd = init_listener(server->port);
    server->query_fd = init_listener(server->query_port);

    if (server->listen_fd < 0 || server->query_fd < 0) return -1;

    FD_ZERO(&server->master_set);
    FD_SET(server->listen_fd, &server->master_set);
    FD_SET(server->query_fd, &server->master_set);
    server->max_fd = (server->listen_fd > server->query_fd) ? server->listen_fd : server->query_fd;

    g_server = server;
    signal(SIGINT, sigint_handler);
    return 0;
}

// [SEQUENCE: MVP3-45]
// 서버 메인 루프 (MVP2)
void server_run(log_server_t* server) {
    printf("LogCaster-C MVP2 server running...\nLog port: %d, Query port: %d\n", server->port, server->query_port);

    while (server->running) {
        server->read_set = server->master_set;
        if (select(server->max_fd + 1, &server->read_set, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) continue;
            perror("select"); break;
        }

        for (int i = 0; i <= server->max_fd; i++) {
            if (FD_ISSET(i, &server->read_set)) {
                if (i == server->listen_fd) {
                    // [SEQUENCE: MVP3-46]
                    // 새 로그 클라이언트 연결 -> 스레드 풀에 작업 제출
                    int new_fd = accept(server->listen_fd, NULL, NULL);
                    if (new_fd < 0) continue;

                    server->client_count++; // 버그: 감소 로직 없음
                    client_job_t* job = malloc(sizeof(client_job_t));
                    job->client_fd = new_fd;
                    job->server = server;
                    thread_pool_add_job(server->thread_pool, handle_client_job, job);
                } else if (i == server->query_fd) {
                    // [SEQUENCE: MVP3-47]
                    // 새 쿼리 클라이언트 연결 -> 직접 처리
                    handle_query_connection(server);
                }
            }
        }
    }
}

// [SEQUENCE: MVP3-48]
// 서버 소멸 (MVP2)
void server_destroy(log_server_t* server) {
    if (!server) return;
    printf("\nShutting down server...\n");
    thread_pool_wait(server->thread_pool);
    thread_pool_destroy(server->thread_pool);
    log_buffer_destroy(server->log_buffer);
    if (server->listen_fd >= 0) close(server->listen_fd);
    if (server->query_fd >= 0) close(server->query_fd);
    free(server);
    printf("Server shut down gracefully.\n");
}
```

### 3.9. `src/main.c` (MVP2 업데이트)

MVP2에서는 별도의 커맨드 라인 인자가 없으므로, `main` 함수는 서버를 생성, 초기화, 실행, 소멸시키는 역할만 단순하게 수행합니다.

```c
// [SEQUENCE: MVP3-49]
#include "server.h"
#include <stdio.h>

int main(void) {
    // [SEQUENCE: MVP3-50]
    // 서버 생성
    log_server_t* server = server_create(DEFAULT_PORT);
    if (!server) {
        fprintf(stderr, "Failed to create server.\n");
        return 1;
    }

    // [SEQUENCE: MVP3-51]
    // 서버 초기화
    if (server_init(server) < 0) {
        fprintf(stderr, "Failed to initialize server.\n");
        server_destroy(server);
        return 1;
    }

    // [SEQUENCE: MVP3-52]
    // 서버 실행 및 소멸
    server_run(server);
    server_destroy(server);

    return 0;
}
```

## 4. 테스트 코드 (Test Code)

MVP2의 새로운 기능(스레드 풀, 쿼리 인터페이스)을 검증하기 위해 새로운 Python 테스트 스크립트(`tests/test_mvp2.py`)가 작성되었습니다. 이 스크립트는 다수의 클라이언트를 동시에 연결하여 로그를 보내고, 별도로 쿼리 포트에 접속하여 서버 상태와 로그 내용을 확인합니다.

```python
# [SEQUENCE: MVP3-53]
#!/usr/bin/env python3
"""
Test client for LogCaster-C MVP2
Tests thread pool functionality and query interface
"""

import socket
import sys
import time
import threading

def send_logs(client_id, host='localhost', port=9999, count=10):
    """Connect to server and send test logs"""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect((host, port))
            # print(f"Client {client_id}: Connected to log server")
            for i in range(count):
                message = f"[Client {client_id}] Log message {i+1} - Thread pool test\n"
                sock.sendall(message.encode())
                time.sleep(0.05)
            # print(f"Client {client_id}: Sent {count} messages")
    except Exception as e:
        print(f"Client {client_id}: Error - {e}")

def query_server(command, host='localhost', port=9998):
    """Send query to server and display response"""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect((host, port))
            sock.sendall((command + '\n').encode())
            response = sock.recv(4096).decode()
            return response
    except Exception as e:
        return f"Query error: {e}"

def test_thread_pool():
    print("=== Test 1: Thread Pool Concurrency ===")
    threads = []
    client_count = 10
    for i in range(client_count):
        thread = threading.Thread(target=send_logs, args=(i+1, 'localhost', 9999, 5))
        threads.append(thread)
        thread.start()
    for thread in threads:
        thread.join()
    print(f"All {client_count} clients completed sending logs.")
    time.sleep(1)

def test_query_interface():
    print("\n=== Test 2: Query Interface ===")
    print("Querying stats...")
    response = query_server("STATS")
    print(f"Response: {response.strip()}")
    print("\nQuerying count...")
    response = query_server("COUNT")
    print(f"Response: {response.strip()}")
    print("\nSearching for 'message 3'...")
    response = query_server("QUERY keyword=message 3")
    print(f"Response: {response.strip()}")

if __name__ == "__main__":
    print("LogCaster-C MVP2 Test Suite")
    test_thread_pool()
    test_query_interface()
    print("\nAll tests complete.")
```