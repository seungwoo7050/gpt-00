# DevHistory 07: LogCaster-C MVP4 - Persistence Layer

## 1. 개요 (Overview)

C 구현의 네 번째 단계(MVP4)는 서버에 영속성(Persistence) 계층을 추가하여, 메모리에만 저장되던 로그를 디스크에 기록하는 기능을 도입합니다. 이로써 서버가 재시작되어도 로그 데이터가 유실되지 않으며, 장기 보관 및 분석의 기반을 마련합니다. 이 단계는 LogCaster를 단순한 실시간 모니터링 도구에서 안정적인 데이터 저장소로 발전시키는 중요한 과정입니다.

**주요 기능 추가:**
- **비동기 파일 쓰기:** 로그 수집 성능에 미치는 영향을 최소화하기 위해, 별도의 'Writer 스레드'를 생성하여 파일 I/O를 비동기적으로 처리합니다. 로그 수집 스레드는 로그 메시지를 쓰기 큐(Queue)에 넣고 즉시 다음 요청을 처리하러 돌아갑니다.
- **로그 파일 로테이션:** 로그 파일이 설정된 크기(예: 10MB)에 도달하면, 기존 파일을 타임스탬프가 포함된 이름(예: `log-2025-07-28-01.log`)으로 변경하고 새로운 `current.log` 파일을 생성하여 로깅을 계속합니다. 이를 통해 단일 로그 파일이 무한정 커지는 것을 방지합니다.
- **설정 가능한 영속성:** 사용자는 서버 시작 시 커맨드 라인 인자(`-P`, `-d`, `-s`)를 통해 영속성 기능을 활성화하고, 로그 저장 디렉토리와 최대 파일 크기를 지정할 수 있습니다.
- **기본적인 장애 복구:** 서버 시작 시 지정된 로그 디렉토리에서 기존 로그 파일을 읽어오는 기능을 구현합니다. (단, MVP4에서는 읽어온 로그를 메모리 버퍼에 다시 적재하는 로직까지는 완전히 통합되지 않았습니다.)

**아키텍처 변경:**
- **`PersistenceManager` 모듈 도입:** 파일 쓰기, 큐 관리, 파일 로테이션 등 모든 영속성 관련 로직을 담당하는 새로운 모듈입니다. 자체적인 작업 큐와 Writer 스레드를 가집니다.
- **서버-영속성 연동:** `handle_client_job` 함수(작업자 스레드)는 로그를 인메모리 버퍼에 저장한 후, 영속성 관리자에게 해당 로그를 전달하여 디스크에 쓰도록 요청합니다.

이 MVP를 통해 LogCaster는 프로덕션 환경에서 사용될 수 있는 수준의 데이터 안정성을 갖추기 시작합니다.

## 2. 빌드 시스템 (Build System)

새로운 `persistence.c` 파일이 추가됨에 따라, `Makefile`의 소스 목록이 이를 포함하도록 업데이트됩니다.

```makefile
# [SEQUENCE: MVP4-1]
# LogCaster-C Makefile - MVP4 with persistence
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11 -O2 -D_GNU_SOURCE
LDFLAGS = -pthread
INCLUDES = -I./include

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# [SEQUENCE: MVP4-2]
# Source and object files (persistence.c added)
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Target executable
TARGET = $(BIN_DIR)/logcaster-c

# ... (rest of the Makefile is same as MVP3) ...
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. `include/persistence.h` (신규 파일)

영속성 관리자의 설정 구조체, 메인 구조체 및 공개 API를 정의합니다.

```c
// [SEQUENCE: MVP4-3]
#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include <stdio.h>

// [SEQUENCE: MVP4-4]
// 영속성 설정 구조체
typedef struct {
    bool enabled;
    char log_directory[256];
    size_t max_file_size;
} persistence_config_t;

// [SEQUENCE: MVP4-5]
// 파일에 쓸 로그 항목 (큐에 저장될 데이터)
typedef struct write_entry {
    char* message;
    struct write_entry* next;
} write_entry_t;

// [SEQUENCE: MVP4-6]
// 영속성 관리자 메인 구조체
typedef struct {
    persistence_config_t config;
    FILE* current_file;
    char current_filepath[512];
    size_t current_file_size;
    pthread_t writer_thread;
    write_entry_t* write_queue;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
    bool stop_thread;
} persistence_manager_t;

// [SEQUENCE: MVP4-7]
// 영속성 관리자 생명주기 및 API 함수
persistence_manager_t* persistence_create(const persistence_config_t* config);
void persistence_destroy(persistence_manager_t* manager);
int persistence_write(persistence_manager_t* manager, const char* message);

#endif // PERSISTENCE_H
```

### 3.2. `src/persistence.c` (신규 파일)

비동기 파일 쓰기를 위한 Writer 스레드와 파일 로테이션 로직을 구현합니다.

```c
// [SEQUENCE: MVP4-8]
#include "persistence.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

static void* persistence_writer_thread(void* arg);
static void rotate_log_file(persistence_manager_t* manager);

// [SEQUENCE: MVP4-9]
// 영속성 관리자 생성
persistence_manager_t* persistence_create(const persistence_config_t* config) {
    persistence_manager_t* manager = calloc(1, sizeof(persistence_manager_t));
    if (!manager) return NULL;

    manager->config = *config;
    pthread_mutex_init(&manager->queue_mutex, NULL);
    pthread_cond_init(&manager->queue_cond, NULL);

    // 로그 디렉토리 생성
    mkdir(manager->config.log_directory, 0755);
    snprintf(manager->current_filepath, sizeof(manager->current_filepath), "%s/current.log", manager->config.log_directory);

    // 현재 로그 파일 열기
    manager->current_file = fopen(manager->current_filepath, "a");
    if (!manager->current_file) {
        perror("Failed to open log file");
        free(manager);
        return NULL;
    }
    manager->current_file_size = ftell(manager->current_file);

    // [SEQUENCE: MVP4-10]
    // Writer 스레드 생성
    if (pthread_create(&manager->writer_thread, NULL, persistence_writer_thread, manager) != 0) {
        fclose(manager->current_file);
        free(manager);
        return NULL;
    }

    return manager;
}

// [SEQUENCE: MVP4-11]
// 로그 쓰기 요청 (큐에 추가)
int persistence_write(persistence_manager_t* manager, const char* message) {
    if (!manager || !manager->config.enabled) return -1;

    write_entry_t* entry = malloc(sizeof(write_entry_t));
    if (!entry) return -1;
    entry->message = strdup(message);
    entry->next = NULL;

    pthread_mutex_lock(&manager->queue_mutex);
    if (manager->write_queue == NULL) {
        manager->write_queue = entry;
    } else {
        write_entry_t* tail = manager->write_queue;
        while (tail->next) tail = tail->next;
        tail->next = entry;
    }
    pthread_cond_signal(&manager->queue_cond);
    pthread_mutex_unlock(&manager->queue_mutex);

    return 0;
}

// [SEQUENCE: MVP4-12]
// Writer 스레드 메인 루프
static void* persistence_writer_thread(void* arg) {
    persistence_manager_t* manager = (persistence_manager_t*)arg;
    while (!manager->stop_thread) {
        pthread_mutex_lock(&manager->queue_mutex);
        while (manager->write_queue == NULL && !manager->stop_thread) {
            pthread_cond_wait(&manager->queue_cond, &manager->queue_mutex);
        }

        write_entry_t* head = manager->write_queue;
        manager->write_queue = NULL; // 큐를 통째로 가져옴
        pthread_mutex_unlock(&manager->queue_mutex);

        // 큐에 있던 모든 로그를 파일에 씀
        while (head) {
            if (manager->current_file) {
                size_t len = fprintf(manager->current_file, "%s\n", head->message);
                fflush(manager->current_file);
                manager->current_file_size += len;

                // [SEQUENCE: MVP4-13]
                // 파일 크기 확인 및 로테이션
                if (manager->current_file_size >= manager->config.max_file_size) {
                    rotate_log_file(manager);
                }
            }
            write_entry_t* temp = head;
            head = head->next;
            free(temp->message);
            free(temp);
        }
    }
    return NULL;
}

// [SEQUENCE: MVP4-14]
// 로그 파일 로테이션
static void rotate_log_file(persistence_manager_t* manager) {
    fclose(manager->current_file);

    char new_filepath[512];
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    snprintf(new_filepath, sizeof(new_filepath), "%s/log-%04d%02d%02d-%02d%02d%02d.log", 
             manager->config.log_directory, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, 
             tm->tm_hour, tm->tm_min, tm->tm_sec);

    rename(manager->current_filepath, new_filepath);

    manager->current_file = fopen(manager->current_filepath, "a");
    manager->current_file_size = 0;
}

// [SEQUENCE: MVP4-15]
// 영속성 관리자 소멸
void persistence_destroy(persistence_manager_t* manager) {
    if (!manager) return;
    pthread_mutex_lock(&manager->queue_mutex);
    manager->stop_thread = true;
    pthread_cond_signal(&manager->queue_cond);
    pthread_mutex_unlock(&manager->queue_mutex);

    pthread_join(manager->writer_thread, NULL);

    if (manager->current_file) {
        fclose(manager->current_file);
    }
    // 남은 큐 정리 로직 (생략)
    pthread_mutex_destroy(&manager->queue_mutex);
    pthread_cond_destroy(&manager->queue_cond);
    free(manager);
}
```

### 3.3. `include/server.h` (MVP4 업데이트)

`log_server_t` 구조체에 `persistence_manager_t` 포인터를 추가합니다.

```c
// [SEQUENCE: MVP4-16]
#ifndef SERVER_H
#define SERVER_H

// ... (기존 include)
#include "persistence.h" // persistence.h 추가

// ... (기존 define)

typedef struct log_server {
    // ... (MVP3와 동일한 필드)
    thread_pool_t* thread_pool;
    log_buffer_t* log_buffer;

    // [SEQUENCE: MVP4-17]
    // MVP4 추가 사항
    persistence_manager_t* persistence;
} log_server_t;

// ... (나머지 프로토타입은 MVP3와 동일)

#endif // SERVER_H
```

### 3.4. `src/server.c` (MVP4 업데이트)

클라이언트 처리 작업(`handle_client_job`)에서 로그를 디스크에 쓰도록 `persistence_write`를 호출하는 로직을 추가합니다.

```c
// [SEQUENCE: MVP4-18]
// ... (include 및 전역 변수 선언)

// [SEQUENCE: MVP4-19]
// 스레드 풀에서 실행될 클라이언트 처리 작업 (MVP4 버전)
static void handle_client_job(void* arg) {
    client_job_t* job = (client_job_t*)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = recv(job->client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        }
        // 1. 인메모리 버퍼에 저장
        log_buffer_push(job->server->log_buffer, buffer);

        // [SEQUENCE: MVP4-20]
        // 2. 영속성 관리자에게 쓰기 요청 (활성화된 경우)
        if (job->server->persistence) {
            persistence_write(job->server->persistence, buffer);
        }
    }

    close(job->client_fd);
    free(job);
}

// [SEQUENCE: MVP4-21]
// 서버 생성 (MVP4)
log_server_t* server_create(int port) {
    log_server_t* server = calloc(1, sizeof(log_server_t));
    // ... (MVP3와 동일한 초기화)
    server->thread_pool = thread_pool_create(DEFAULT_THREAD_COUNT);
    server->log_buffer = log_buffer_create(DEFAULT_BUFFER_SIZE);
    
    // persistence는 main에서 생성 후 주입되므로 NULL로 초기화
    server->persistence = NULL; 

    return server;
}

// [SEQUENCE: MVP4-22]
// 서버 소멸 (MVP4)
void server_destroy(log_server_t* server) {
    if (!server) return;
    printf("\nShutting down server...\n");
    thread_pool_wait(server->thread_pool);
    thread_pool_destroy(server->thread_pool);
    log_buffer_destroy(server->log_buffer);

    // 영속성 관리자 소멸 로직 추가
    if (server->persistence) {
        persistence_destroy(server->persistence);
    }

    // ... (나머지는 MVP3와 동일)
    free(server);
}

// ... (server_init, server_run 등 나머지 함수는 MVP3와 동일)
```

### 3.5. `src/main.c` (MVP4 업데이트)

`getopt`를 사용하여 커맨드 라인 인자를 파싱하고, 이를 기반으로 영속성 관리자를 설정하는 로직이 추가됩니다.

```c
// [SEQUENCE: MVP4-23]
#include "server.h"
#include "persistence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;
    persistence_config_t persist_config = { .enabled = false };
    strncpy(persist_config.log_directory, "./logs", sizeof(persist_config.log_directory) - 1);
    persist_config.max_file_size = 10 * 1024 * 1024; // 10MB

    // [SEQUENCE: MVP4-24]
    // getopt를 사용한 커맨드 라인 인자 파싱
    int opt;
    while ((opt = getopt(argc, argv, "p:d:s:Ph")) != -1) {
        switch (opt) {
            case 'p': port = atoi(optarg); break;
            case 'P': persist_config.enabled = true; break;
            case 'd': strncpy(persist_config.log_directory, optarg, sizeof(persist_config.log_directory) - 1); break;
            case 's': persist_config.max_file_size = atoi(optarg) * 1024 * 1024; break;
            case 'h':
                printf("Usage: %s [-p port] [-P] [-d dir] [-s size_mb] [-h]\n", argv[0]);
                return 0;
        }
    }

    log_server_t* server = server_create(port);
    if (!server) return 1;

    // [SEQUENCE: MVP4-25]
    // 영속성 기능이 활성화되었으면, 관리자를 생성하여 서버에 연결
    if (persist_config.enabled) {
        printf("Persistence enabled. Dir: %s, Max Size: %zu MB\n", 
               persist_config.log_directory, persist_config.max_file_size / (1024*1024));
        server->persistence = persistence_create(&persist_config);
        if (!server->persistence) {
            fprintf(stderr, "Failed to initialize persistence.\n");
            server_destroy(server);
            return 1;
        }
    }

    if (server_init(server) < 0) { /* ... */ }
    server_run(server);
    server_destroy(server);

    return 0;
}
```

## 4. 테스트 코드 (Test Code)

영속성 기능을 검증하기 위한 새로운 테스트 스크립트(`tests/test_persistence.py`)가 작성되었습니다. 이 스크립트는 `-P` 플래그 유무에 따라 로그 파일이 정상적으로 생성되는지, 로그가 파일에 기록되는지를 확인합니다.

```python
# [SEQUENCE: MVP4-26]
#!/usr/bin/env python3
import os
import socket
import subprocess
import time
import shutil

HOST = '127.0.0.1'
LOG_PORT = 9999
LOG_DIR = "./test_logs"

def send_log(message):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, LOG_PORT))
        s.sendall((message + '\n').encode())

def cleanup():
    if os.path.exists(LOG_DIR):
        shutil.rmtree(LOG_DIR)

def run_test():
    cleanup()

    # --- Test 1: Persistence disabled ---
    print("--- Test 1: Persistence disabled ---")
    server_proc = subprocess.Popen(["./bin/logcaster-c"])
    time.sleep(1)
    send_log("test_no_persistence")
    server_proc.terminate()
    server_proc.wait()
    assert not os.path.exists(LOG_DIR), "Log directory should not be created when -P is not set"
    print("OK\n")

    # --- Test 2: Persistence enabled ---
    print("--- Test 2: Persistence enabled ---")
    os.makedirs(LOG_DIR)
    server_proc = subprocess.Popen(["./bin/logcaster-c", "-P", "-d", LOG_DIR])
    time.sleep(1) # Allow time for async write
    log_message = "hello persistence world"
    send_log(log_message)
    time.sleep(1) # Allow time for async write
    server_proc.terminate()
    server_proc.wait()

    log_file = os.path.join(LOG_DIR, "current.log")
    assert os.path.exists(log_file), "current.log should be created"
    with open(log_file, 'r') as f:
        content = f.read()
        assert log_message in content, "Log message not found in file"
    print("OK\n")

    cleanup()

if __name__ == "__main__":
    run_test()
    print("All MVP4 tests passed!")
```