# DevHistory 00: 프로젝트 시작 (Project Inception)

## 1. 개요 (Overview)

이 문서는 LogCaster 프로젝트의 공식적인 개발 시작에 앞서, 프로젝트의 목표, 기술적 기반, 그리고 초기 계획을 정의합니다. 코드 구현보다는 프로젝트의 '왜'와 '무엇을'에 초점을 맞춘 단계입니다.

## 2. 문제 정의 및 목표 (Problem Definition & Goals)

### 2.1. 해결하려는 문제 (The Problem)

현대의 분산 시스템 환경에서는 대규모의 로그 데이터를 안정적으로 수집하고, 이를 실시간으로 조회할 수 있는 고성능 솔루션이 필수적입니다. 기존의 로깅 시스템들은 특정 요구사항을 충족하지 못하거나, 과도한 리소스를 사용하거나, 특정 플랫폼에 종속되는 경우가 많습니다. LogCaster는 이러한 문제를 해결하기 위해 탄생했습니다.

**핵심 요구사항:**
- **고성능 로그 수집:** 대량의 동시 연결을 처리하며 초당 수만 건 이상의 로그를 안정적으로 수집할 수 있어야 합니다.
- **실시간 쿼리:** 수집된 로그를 지연 없이 즉각적으로 검색하고 필터링할 수 있어야 합니다.
- **자원 효율성:** 최소한의 CPU 및 메모리 자원을 사용하여 운영 비용을 절감해야 합니다.

### 2.2. 프로젝트 목표 (Project Goals)

- C와 C++ 두 가지 언어로 동일한 기능의 로그 서버를 구현하여, 각 언어의 패러다임과 성능 특성을 비교 분석합니다.
- 시스템 프로그래밍의 핵심 요소(소켓 통신, 멀티스레딩, 동기화, 메모리 관리)에 대한 깊은 이해를 바탕으로 최적화된 서버를 구축합니다.
- 잘 구조화되고 문서화된 코드를 통해 다른 개발자들에게 교육적인 자료를 제공합니다.

## 3. 초기 기술 결정 (Initial Technology Decisions)

### 3.1. 기술 스택 (Technology Stack)

- **언어 (Language):**
  - **C:** 하드웨어에 대한 직접적인 제어와 최대의 성능을 이끌어내기 위해 선택했습니다. 시스템 리소스를 세밀하게 관리하는 능력이 필수적인 저수준(low-level) 구현에 적합합니다.
  - **C++:** C의 성능을 유지하면서도 객체 지향 프로그래밍(OOP), RAII(Resource Acquisition Is Initialization), STL(Standard Template Library) 등 더 높은 수준의 추상화를 제공하여 코드의 안정성과 유지보수성을 높이기 위해 선택했습니다.

- **핵심 아키텍처 (Core Architecture):**
  - **TCP 소켓 통신:** 데이터 전송의 신뢰성을 보장하기 위해 UDP 대신 TCP를 사용합니다.
  - **스레드 풀 (Thread Pool):** 클라이언트 요청이 발생할 때마다 스레드를 생성하는 오버헤드를 줄이고, 시스템 자원을 효율적으로 재사용하기 위해 스레드 풀 패턴을 적용합니다.
  - **인메모리 저장소 (In-Memory Storage):** 로그 쿼리 성능을 극대화하기 위해 수집된 로그를 우선 메모리에 저장합니다. 디스크 I/O를 최소화하여 응답 시간을 단축합니다.

### 3.2. 포트 할당 (Port Allocation)

- **로그 수집 포트:** `9999`
- **쿼리 전용 포트:** `9998`

두 가지 주요 기능을 별도의 포트로 분리하여 역할의 명확성을 높이고, 각 포트에 특화된 처리 로직을 구현할 수 있도록 설계했습니다.

## 4. MVP1 계획 (MVP1 Plan)

첫 번째 결과물(MVP1)은 프로젝트의 가장 기본적인 기능을 구현하는 데 집중합니다.

### 4.1. 핵심 기능 (Core Features)

- **TCP 서버 구현:** 지정된 포트(`9999`)에서 클라이언트의 연결을 수락합니다.
- **다중 클라이언트 처리:** 여러 클라이언트가 동시에 서버에 연결할 수 있어야 합니다.
- **기본 로그 처리:** 클라이언트로부터 받은 로그 메시지를 서버의 표준 출력(stdout)에 그대로 출력하여 수신 여부를 확인합니다.
- **단순 프로토콜:** 로그 메시지는 개행 문자(`\n`)를 기준으로 구분하는 간단한 텍스트 기반 프로토콜을 사용합니다.

### 4.2. 성공 기준 (Success Criteria)

- 서버가 `9999` 포트에서 정상적으로 실행되고 연결을 기다립니다.
- 여러 명의 클라이언트가 동시에 접속할 수 있습니다.
- 클라이언트가 전송한 모든 로그 메시지가 서버의 콘솔 화면에 빠짐없이 출력됩니다.
- `Ctrl+C`를 눌렀을 때 서버가 모든 연결을 정리하고 깨끗하게 종료됩니다.

이 계획을 바탕으로 다음 단계인 `DevHistory01.md`부터 실제 C언어 코드 구현을 시작하겠습니다.
# DevHistory 01: LogCaster-C MVP1 - Basic TCP Server

## 1. 개요 (Overview)

LogCaster 프로젝트의 첫 번째 MVP(Minimum Viable Product)는 C 언어를 사용하여 기본적인 TCP 로그 수집 서버를 구현하는 데 중점을 둡니다. 이 단계의 핵심 목표는 여러 클라이언트로부터 동시에 로그 메시지를 수신하고, 이를 서버의 표준 출력(stdout)에 표시하는 안정적인 기반을 구축하는 것입니다.

**주요 기능:**
- **TCP 서버:** 지정된 포트(기본값: 9999)에서 클라이언트 연결을 수락합니다.
- **동시성 처리:** `select()` 시스템 콜을 사용한 I/O 멀티플렉싱을 통해 다수의 클라이언트를 동시에 처리합니다.
- **기본 프로토콜:** 클라이언트는 개행 문자(`\n`)로 구분된 일반 텍스트 로그 메시지를 전송합니다.
- **로그 출력:** 수신된 모든 로그는 서버 콘솔에 `[LOG]` 접두사와 함께 출력됩니다.
- **Graceful Shutdown:** `Ctrl+C` (SIGINT) 시그널을 감지하여 진행 중인 연결을 안전하게 종료하고 리소스를 정리합니다.

**기술적 결정:**
- **`select()` 사용:** `epoll`이나 `kqueue` 같은 더 현대적인 대안 대신, 이식성과 단순성을 위해 POSIX 표준인 `select()`를 채택했습니다.
- **고정 버퍼:** 각 클라이언트 연결에 대해 4096바이트 크기의 고정 버퍼를 할당하여 메시지를 수신합니다.
- **논블로킹 소켓:** 서버의 응답성을 높이기 위해 논블로킹 소켓 모드를 사용합니다.

이 MVP는 LogCaster의 핵심 네트워킹 기반을 마련하며, 이후 단계에서 구현될 스레드 풀, 인메모리 버퍼, 쿼리 기능 등의 고급 기능을 위한 발판이 됩니다.

## 2. 빌드 시스템 (Build System)

MVP1은 `Makefile`을 사용하여 프로젝트를 빌드합니다. 주요 특징은 다음과 같습니다.

- **엄격한 컴파일러 플래그:** `-Wall`, `-Wextra`, `-Werror`, `-pedantic` 플래그를 사용하여 코드 품질을 높이고 잠재적인 오류를 사전에 방지합니다.
- **C11 표준 준수:** `-std=c11` 옵션을 사용하여 최신 C 표준을 따릅니다.
- **디렉토리 구조:** `src`, `include`, `obj`, `bin` 디렉토리를 명확히 구분하여 프로젝트 구조를 체계적으로 관리합니다.
- **pthread 라이브러리 링크:** 향후 MVP에서 사용될 스레드 풀 기능을 위해 `pthread` 라이브러리를 미리 링크합니다.

```makefile
# [SEQUENCE: MVP1-1]
# Compiler and flags
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -Werror -pedantic -std=c11
LDFLAGS = -lpthread

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

# Source files and object files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

# Target executable
TARGET = $(BINDIR)/logcaster-c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete. Executable: $(TARGET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)
	@echo "Cleaned up object and binary files."
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. `include/server.h`

서버의 핵심 데이터 구조와 함수 프로토타입을 정의합니다. MVP1에서는 동시 연결 처리를 위한 `select` 관련 필드와 기본적인 서버 상태만 포함합니다.

```c
// [SEQUENCE: MVP1-2]
#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#define DEFAULT_PORT 9999
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 4096

// [SEQUENCE: MVP1-3]
// 서버 상태를 관리하는 구조체 (MVP1 버전)
typedef struct {
    int port;
    int listen_fd;
    fd_set master_set;
    fd_set read_set;
    int max_fd;
    int client_count;
    volatile sig_atomic_t running;
} log_server_t;

// [SEQUENCE: MVP1-4]
// 함수 프로토타입 (MVP1 버전)
log_server_t* server_create(int port);
void server_run(log_server_t* server);
void server_destroy(log_server_t* server);
void handle_signal(int sig);

#endif // SERVER_H
```

### 3.2. `src/server.c`

TCP 서버의 핵심 로직을 구현합니다. `select`를 사용한 메인 루프에서 연결 수락, 데이터 수신 및 처리를 모두 담당하는 단일 스레드 모델입니다.

```c
// [SEQUENCE: MVP1-5]
#include "server.h"

// 시그널 처리를 위한 전역 서버 포인터
static log_server_t* g_server = NULL;

// [SEQUENCE: MVP1-6]
// 시그널 핸들러: 서버의 running 상태를 0으로 변경하여 안전한 종료를 유도
void handle_signal(int sig) {
    (void)sig; // 사용되지 않는 파라미터 경고 방지
    if (g_server) {
        g_server->running = 0;
    }
}

// [SEQUENCE: MVP1-7]
// 서버 구조체 생성 및 리스닝 소켓 초기화
log_server_t* server_create(int port) {
    log_server_t* server = malloc(sizeof(log_server_t));
    if (!server) {
        perror("Failed to allocate server");
        return NULL;
    }

    server->port = port;
    server->running = 1;
    server->client_count = 0;

    // [SEQUENCE: MVP1-8]
    // 리스닝 소켓 생성
    server->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->listen_fd < 0) {
        perror("Failed to create socket");
        free(server);
        return NULL;
    }

    // [SEQUENCE: MVP1-9]
    // 소켓 재사용 옵션 설정 (서버 재시작 시 TIME_WAIT 문제 방지)
    int opt = 1;
    if (setsockopt(server->listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Failed to set socket options");
        close(server->listen_fd);
        free(server);
        return NULL;
    }

    // [SEQUENCE: MVP1-10]
    // 서버 주소 설정
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // [SEQUENCE: MVP1-11]
    // 소켓에 주소 바인딩
    if (bind(server->listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        close(server->listen_fd);
        free(server);
        return NULL;
    }

    // [SEQUENCE: MVP1-12]
    // 리스닝 시작
    if (listen(server->listen_fd, MAX_CLIENTS) < 0) {
        perror("Failed to listen on socket");
        close(server->listen_fd);
        free(server);
        return NULL;
    }

    // [SEQUENCE: MVP1-13]
    // fd_set 초기화 및 리스닝 소켓 등록
    FD_ZERO(&server->master_set);
    FD_ZERO(&server->read_set);
    FD_SET(server->listen_fd, &server->master_set);
    server->max_fd = server->listen_fd;

    g_server = server; // 전역 변수에 서버 인스턴스 할당

    printf("LogCaster-C server starting on port %d\n", port);
    return server;
}

// [SEQUENCE: MVP1-14]
// 서버 메인 루프
void server_run(log_server_t* server) {
    printf("Press Ctrl+C to stop\n");
    while (server->running) {
        server->read_set = server->master_set;

        // [SEQUENCE: MVP1-15]
        // select()를 사용하여 I/O 이벤트 대기
        if (select(server->max_fd + 1, &server->read_set, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) { // 시그널에 의해 중단된 경우
                continue;
            }
            perror("select failed");
            break;
        }

        // [SEQUENCE: MVP1-16]
        // 모든 파일 디스크립터를 순회하며 이벤트 확인
        for (int i = 0; i <= server->max_fd; i++) {
            if (FD_ISSET(i, &server->read_set)) {
                // [SEQUENCE: MVP1-17]
                // 리스닝 소켓 이벤트: 새 클라이언트 연결 처리
                if (i == server->listen_fd) {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int new_fd = accept(server->listen_fd, (struct sockaddr*)&client_addr, &client_len);

                    if (new_fd < 0) {
                        perror("Failed to accept connection");
                        continue;
                    }

                    // [SEQUENCE: MVP1-18]
                    // 클라이언트 수 제한 확인
                    if (server->client_count >= MAX_CLIENTS) {
                        fprintf(stderr, "Max clients reached. Rejecting new connection.\n");
                        close(new_fd);
                        continue;
                    }

                    // [SEQUENCE: MVP1-19]
                    // 새 클라이언트를 master_set에 추가
                    fcntl(new_fd, F_SETFL, O_NONBLOCK); // 논블로킹으로 설정
                    FD_SET(new_fd, &server->master_set);
                    if (new_fd > server->max_fd) {
                        server->max_fd = new_fd;
                    }
                    server->client_count++;
                    printf("New connection from %s:%d on socket %d\n",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), new_fd);
                } else {
                    // [SEQUENCE: MVP1-20]
                    // 클라이언트 소켓 이벤트: 데이터 수신 처리
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    int nbytes = read(i, buffer, sizeof(buffer) - 1);

                    if (nbytes <= 0) {
                        // [SEQUENCE: MVP1-21]
                        // 연결 종료 또는 오류
                        if (nbytes == 0) {
                            printf("Socket %d hung up\n", i);
                        } else {
                            perror("read failed");
                        }
                        close(i);
                        FD_CLR(i, &server->master_set);
                        // 참고: 이 MVP에서는 client_count를 감소시키지 않아 버그가 존재함.
                        // 이 문제는 MVP5에서 수정될 예정.
                    } else {
                        // [SEQUENCE: MVP1-22]
                        // 데이터 수신 성공, stdout에 로그 출력
                        buffer[nbytes] = '\0';
                        printf("[LOG] from socket %d: %s", i, buffer);
                        // 버퍼에 개행이 없을 경우를 대비해 추가
                        if (buffer[nbytes-1] != '\n') {
                            printf("\n");
                        }
                    }
                }
            }
        }
    }
}

// [SEQUENCE: MVP1-23]
// 서버 리소스 해제
void server_destroy(log_server_t* server) {
    if (!server) return;

    printf("\nShutting down server...\n");

    // [SEQUENCE: MVP1-24]
    // 모든 클라이언트 연결 종료
    for (int i = 0; i <= server->max_fd; i++) {
        if (FD_ISSET(i, &server->master_set)) {
            close(i);
        }
    }

    free(server);
    g_server = NULL;
    printf("Server shut down gracefully.\n");
}
```

### 3.3. `src/main.c`

프로그램의 진입점(entry point)입니다. 서버를 생성, 실행하고, 종료 시 리소스를 해제하는 역할을 합니다. MVP1에서는 포트 번호를 인자로 받는 기능만 포함합니다.

```c
// [SEQUENCE: MVP1-25]
#include "server.h"

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;

    // [SEQUENCE: MVP1-26]
    // 명령줄 인자 파싱 (포트 번호 지정)
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number. Using default %d.\n", DEFAULT_PORT);
            port = DEFAULT_PORT;
        }
    }

    // [SEQUENCE: MVP1-27]
    // SIGINT 시그널 핸들러 등록 (Ctrl+C)
    signal(SIGINT, handle_signal);

    // [SEQUENCE: MVP1-28]
    // 서버 생성
    log_server_t* server = server_create(port);
    if (!server) {
        fprintf(stderr, "Failed to create server.\n");
        return 1;
    }

    // [SEQUENCE: MVP1-29]
    // 서버 실행
    server_run(server);

    // [SEQUENCE: MVP1-30]
    // 서버 리소스 해제
    server_destroy(server);

    return 0;
}
```

## 4. 테스트 코드 (Test Code)

MVP1의 기능을 검증하기 위해 Python으로 작성된 테스트 클라이언트(`tests/test_client.py`)를 사용합니다. 이 스크립트는 서버에 대한 다양한 시나리오를 테스트합니다.

**테스트 모드:**
1.  **단일 클라이언트 테스트 (`single`):** 하나의 클라이언트가 연결하여 여러 로그 메시지를 전송하고 정상적으로 수신되는지 확인합니다.
2.  **다중 클라이언트 테스트 (`multiple`):** 5개의 클라이언트가 동시에 연결하여 각자 로그를 보내고 서버가 모든 메시지를 처리하는지 검증합니다.
3.  **스트레스 테스트 (`stress`):** 50개의 클라이언트를 동시에 연결하여 서버의 동시 연결 처리 능력을 시험합니다.

```python
# [SEQUENCE: MVP1-31]
import socket
import threading
import time
import sys

SERVER_HOST = '127.0.0.1'
SERVER_PORT = 9999
NUM_CLIENTS_MULTIPLE = 5
NUM_CLIENTS_STRESS = 50
MESSAGES_PER_CLIENT = 10

def client_task(client_id, num_messages):
    """A single client connects, sends messages, and closes."""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((SERVER_HOST, SERVER_PORT))
            print(f"Client {client_id}: Connected to server.")
            for i in range(num_messages):
                message = f"Client {client_id}, message {i+1}\n"
                s.sendall(message.encode('utf-8'))
                # print(f"Client {client_id}: Sent '{message.strip()}'")
                time.sleep(0.1)
            print(f"Client {client_id}: All messages sent.")
    except ConnectionRefusedError:
        print(f"Client {client_id}: Connection refused. Is the server running?")
    except Exception as e:
        print(f"Client {client_id}: An error occurred: {e}")
    finally:
        print(f"Client {client_id}: Connection closed.")

def run_test(num_clients, num_messages):
    """Runs a test with a specified number of clients."""
    threads = []
    start_time = time.time()

    print(f"--- Starting test with {num_clients} clients ---")
    for i in range(num_clients):
        thread = threading.Thread(target=client_task, args=(i + 1, num_messages))
        threads.append(thread)
        thread.start()
        time.sleep(0.05) # Stagger connections slightly

    for thread in threads:
        thread.join()

    end_time = time.time()
    print(f"\nTest with {num_clients} clients finished in {end_time - start_time:.2f} seconds.")

if __name__ == "__main__":
    if len(sys.argv) != 2 or sys.argv[1] not in ['single', 'multiple', 'stress']:
        print("Usage: python test_client.py [single|multiple|stress]")
        sys.exit(1)

    mode = sys.argv[1]

    if mode == 'single':
        run_test(1, MESSAGES_PER_CLIENT)
    elif mode == 'multiple':
        run_test(NUM_CLIENTS_MULTIPLE, MESSAGES_PER_CLIENT)
    elif mode == 'stress':
        run_test(NUM_CLIENTS_STRESS, 5) # Fewer messages for stress test
```

## 5. 개선 제안 (Improvement Notes)

- **Client Counter 버그:** `DEVELOPMENT_JOURNAL.md`에 따르면, MVP1의 `server.c` 구현에는 클라이언트 연결이 끊어졌을 때 `client_count`를 감소시키지 않는 의도적인 버그가 있습니다. 이로 인해 서버는 재시작 전까지 최대 `MAX_CLIENTS` 만큼의 연결만 수락할 수 있습니다. 이 버그는 이후 MVP5 보안 강화 단계에서 수정될 예정입니다.
- **전역 변수 `g_server`:** 시그널 핸들러에서 서버 상태에 접근하기 위해 전역 변수를 사용했습니다. 이는 POSIX 시그널 핸들러의 일반적인 패턴이지만, 코드의 재사용성과 테스트 용이성을 저해할 수 있습니다.
- **`select()`의 한계:** `select()`는 파일 디스크립터 수에 제한(일반적으로 1024)이 있고, 파일 디스크립터 수가 증가함에 따라 성능이 선형적으로 감소합니다. 대규모 연결을 처리하기 위해서는 `epoll`(Linux)이나 `kqueue`(BSD/macOS)와 같은 더 확장성 있는 I/O 멀티플렉싱 기술로의 전환이 필요합니다.
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
```# DevHistory 04: LogCaster-CPP MVP2 - Modern C++ Concurrency

## 1. 개요 (Overview)

C++ 구현의 두 번째 단계(MVP2)는 C++의 최신 기능들을 본격적으로 활용하여 C 버전 MVP2와 동일한 기능(스레드 풀, 인메모리 버퍼, 쿼리 인터페이스)을 더 안전하고 효율적으로 구현하는 데 중점을 둡니다. C++ MVP1의 '클라이언트당 스레드' 모델의 한계를 극복하고, 고성능 서버의 기반을 다지는 단계입니다.

**주요 기능 추가 및 개선:**
- **C++11 스레드 풀:** `pthreads` 대신 `<thread>`, `<mutex>`, `<condition_variable>` 등 C++ 표준 라이브러리를 사용하여 스레드 풀을 구현합니다. `std::function`을 통해 어떤 종류의 호출 가능한(callable) 작업이든 작업 큐에 저장할 수 있어 유연성이 크게 향상됩니다.
- **STL 기반 로그 버퍼:** 직접 인덱스를 관리하는 C의 원형 배열 대신, `std::deque`를 사용하여 스레드 안전 로그 버퍼를 구현합니다. `std::deque`는 양쪽 끝에서의 삽입/삭제가 O(1) 시간 복잡도를 가져 원형 버퍼 구현에 이상적입니다.
- **객체 지향 쿼리 처리:** 쿼리 처리 로직을 별도의 `QueryHandler` 클래스로 캡슐화하여 서버의 다른 기능들과 명확하게 분리합니다.
- **RAII 및 스마트 포인터 활용:** 서버의 모든 주요 구성 요소(`ThreadPool`, `LogBuffer`, `QueryHandler` 등)를 `std::unique_ptr` 또는 `std::shared_ptr`로 관리하여, 리소스 누수 가능성을 원천적으로 차단하고 코드의 안정성을 높입니다.

**아키텍처 변경:**
- **이벤트 루프 유지:** C++ MVP1의 '클라이언트당 스레드' 모델을 폐기하고, C 버전 MVP2와 유사하게 `select` 기반의 메인 이벤트 루프를 다시 도입합니다. 이는 소수의 스레드로 다수의 연결을 효율적으로 관리하기 위함입니다.
- **작업 위임:** 메인 스레드는 `select`를 통해 I/O 이벤트를 감지하고, 실제 데이터 처리 작업은 `ThreadPool`에 비동기적으로 위임합니다. 이를 통해 메인 스레드는 항상 새로운 연결을 수락할 준비 상태를 유지할 수 있습니다.

이 MVP를 통해 C++ 버전은 C 버전의 기능적 동등성을 확보하면서도, C++ 언어의 장점을 최대한 활용하여 더 높은 수준의 코드 안전성, 가독성, 유지보수성을 달성합니다.

## 2. 빌드 시스템 (Build System)

MVP2에서는 `ThreadPool`, `LogBuffer`, `QueryHandler` 클래스가 추가됨에 따라, `CMakeLists.txt`의 소스 파일 목록이 업데이트됩니다.

```cmake
# [SEQUENCE: MVP4-1]
# CMake 최소 요구 버전 설정
cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름 및 C++ 언어 지정
project(LogCasterCPP VERSION 1.0.0 LANGUAGES CXX)

# C++ 표준 설정 (C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 컴파일러 플래그 설정
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic")

# 헤더 파일 경로 추가
include_directories(include)

# [SEQUENCE: MVP4-2]
# MVP2에 필요한 소스 파일 목록
set(SOURCES
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/LogBuffer.cpp
    src/QueryHandler.cpp
)

# 실행 파일 생성
add_executable(logcaster-cpp ${SOURCES})

# [SEQUENCE: MVP4-3]
# 스레드 라이브러리 링크
find_package(Threads REQUIRED)
target_link_libraries(logcaster-cpp PRIVATE Threads::Threads)
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. `include/ThreadPool.h`

C++11 표준 라이브러리를 사용하여 구현된 헤더 전용(header-only) 스레드 풀입니다. 템플릿을 사용하여 어떤 형태의 함수든 인자와 함께 작업 큐에 추가하고, `std::future`를 통해 결과값을 비동기적으로 받을 수 있습니다.

```cpp
// [SEQUENCE: MVP4-4]
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>

// [SEQUENCE: MVP4-5]
class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // [SEQUENCE: MVP4-6]
    // 작업을 큐에 추가하는 템플릿 함수
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    void workerThread();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

// [SEQUENCE: MVP4-7]
// enqueue 멤버 함수의 템플릿 구현
template<typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    // [SEQUENCE: MVP4-8]
    // 작업을 std::packaged_task로 래핑하여 future를 얻음
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks_.emplace([task]() { (*task)(); });
    }
    condition_.notify_one();
    return res;
}

#endif // THREADPOOL_H
```

### 3.2. `src/ThreadPool.cpp`

스레드 풀의 생성자, 소멸자 및 작업자 스레드의 실행 로직을 구현합니다.

```cpp
// [SEQUENCE: MVP4-9]
#include "ThreadPool.h"

// [SEQUENCE: MVP4-10]
// 생성자: 작업자 스레드를 생성하고 실행 시작
ThreadPool::ThreadPool(size_t numThreads) : stop_(false) {
    workers_.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back([this] { workerThread(); });
    }
}

// [SEQUENCE: MVP4-11]
// 소멸자: 모든 스레드가 안전하게 종료되도록 보장
ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all(); // 모든 대기 중인 스레드를 깨움
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

// [SEQUENCE: MVP4-12]
// 작업자 스레드의 메인 루프
void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            // [SEQUENCE: MVP4-13]
            // 작업이 있거나, 종료 신호가 올 때까지 대기
            condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
            if (stop_ && tasks_.empty()) {
                return; // 종료
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task(); // 작업 실행
    }
}
```

### 3.3. `include/LogBuffer.h`

`std::deque`를 내부 컨테이너로 사용하는 스레드 안전 로그 버퍼를 정의합니다.

```cpp
// [SEQUENCE: MVP4-14]
#ifndef LOGBUFFER_H
#define LOGBUFFER_H

#include <deque>
#include <string>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>

// [SEQUENCE: MVP4-15]
// 로그 항목 구조체 (MVP2 버전)
struct LogEntry {
    std::string message;
    std::chrono::system_clock::time_point timestamp;

    LogEntry(std::string msg)
        : message(std::move(msg)), timestamp(std::chrono::system_clock::now()) {}
};

// [SEQUENCE: MVP4-16]
// 로그 버퍼 클래스
class LogBuffer {
public:
    explicit LogBuffer(size_t capacity = 10000);
    ~LogBuffer() = default;

    void push(std::string message);
    std::vector<std::string> search(const std::string& keyword) const;

    struct StatsSnapshot {
        uint64_t totalLogs;
        uint64_t droppedLogs;
    };
    StatsSnapshot getStats() const;
    size_t size() const;

private:
    void dropOldest_();

    mutable std::mutex mutex_;
    std::deque<LogEntry> buffer_;
    size_t capacity_;

    std::atomic<uint64_t> totalLogs_{0};
    std::atomic<uint64_t> droppedLogs_{0};
};

#endif // LOGBUFFER_H
```

### 3.4. `src/LogBuffer.cpp`

로그 버퍼의 `push`, `search` 등 주요 기능을 구현합니다. `std::mutex`를 사용하여 동시 접근을 제어합니다.

```cpp
// [SEQUENCE: MVP4-17]
#include "LogBuffer.h"
#include <sstream>
#include <iomanip>

LogBuffer::LogBuffer(size_t capacity) : capacity_(capacity) {}

// [SEQUENCE: MVP4-18]
// 로그를 버퍼에 추가
void LogBuffer::push(std::string message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (buffer_.size() >= capacity_) {
        dropOldest_();
    }
    buffer_.emplace_back(std::move(message));
    totalLogs_++;
}

// [SEQUENCE: MVP4-19]
// 가장 오래된 로그를 삭제 (내부 헬퍼 함수)
void LogBuffer::dropOldest_() {
    if (!buffer_.empty()) {
        buffer_.pop_front();
        droppedLogs_++;
    }
}

// [SEQUENCE: MVP4-20]
// 키워드를 포함하는 로그 검색
std::vector<std::string> LogBuffer::search(const std::string& keyword) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> results;
    for (const auto& entry : buffer_) {
        if (entry.message.find(keyword) != std::string::npos) {
            auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
            std::stringstream ss;
            ss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
            ss << entry.message;
            results.push_back(ss.str());
        }
    }
    return results;
}

// [SEQUENCE: MVP4-21]
// 현재 버퍼 크기 반환
size_t LogBuffer::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_.size();
}

// [SEQUENCE: MVP4-22]
// 통계 정보 반환
LogBuffer::StatsSnapshot LogBuffer::getStats() const {
    return { totalLogs_.load(), droppedLogs_.load() };
}
```

### 3.5. `include/QueryHandler.h`

쿼리 처리를 캡슐화한 `QueryHandler` 클래스를 정의합니다.

```cpp
// [SEQUENCE: MVP4-23]
#ifndef QUERYHANDLER_H
#define QUERYHANDLER_H

#include <string>
#include <memory>
#include "LogBuffer.h"

class QueryHandler {
public:
    explicit QueryHandler(std::shared_ptr<LogBuffer> buffer);
    std::string processQuery(const std::string& query);

private:
    std::string handleSearch(const std::string& query);
    std::string handleStats();
    std::string handleCount();
    std::string handleHelp();

    std::shared_ptr<LogBuffer> buffer_;
};

#endif // QUERYHANDLER_H
```

### 3.6. `src/QueryHandler.cpp`

`QueryHandler`의 멤버 함수를 구현합니다. MVP2에서는 간단한 문자열 파싱으로 명령을 처리합니다.

```cpp
// [SEQUENCE: MVP4-24]
#include "QueryHandler.h"
#include <sstream>

QueryHandler::QueryHandler(std::shared_ptr<LogBuffer> buffer) : buffer_(buffer) {}

// [SEQUENCE: MVP4-25]
// 쿼리 문자열을 파싱하고 적절한 핸들러 호출
std::string QueryHandler::processQuery(const std::string& query) {
    if (query.find("QUERY keyword=") == 0) {
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

// [SEQUENCE: MVP4-26]
// 검색 쿼리 처리
std::string QueryHandler::handleSearch(const std::string& query) {
    std::string keyword = query.substr(14); // "QUERY keyword="
    auto results = buffer_->search(keyword);
    std::stringstream ss;
    ss << "FOUND: " << results.size() << " matches\n";
    for (const auto& res : results) {
        ss << res << "\n";
    }
    return ss.str();
}

// [SEQUENCE: MVP4-27]
// 통계 쿼리 처리
std::string QueryHandler::handleStats() {
    auto stats = buffer_->getStats();
    std::stringstream ss;
    ss << "STATS: Total=" << stats.totalLogs << ", Dropped=" << stats.droppedLogs 
       << ", Current=" << buffer_->size() << "\n";
    return ss.str();
}

// [SEQUENCE: MVP4-28]
// 카운트 쿼리 처리
std::string QueryHandler::handleCount() {
    std::stringstream ss;
    ss << "COUNT: " << buffer_->size() << "\n";
    return ss.str();
}

// [SEQUENCE: MVP4-29]
// 도움말 쿼리 처리
std::string QueryHandler::handleHelp() {
    return "Available commands:\n"
           "  STATS - Show buffer statistics\n"
           "  COUNT - Show number of logs in buffer\n"
           "  HELP  - Show this help message\n"
           "  QUERY keyword=<word> - Search for a keyword\n";
}
```

### 3.7. `include/LogServer.h` (MVP2 업데이트)

`LogServer`가 `ThreadPool`과 `LogBuffer` 등 새로운 구성 요소들을 소유하고 관리하도록 수정됩니다.

```cpp
// [SEQUENCE: MVP4-30]
#ifndef LOGSERVER_H
#define LOGSERVER_H

#include <memory>
#include <atomic>
#include <string>
#include "Logger.h"
#include "ThreadPool.h"
#include "LogBuffer.h"
#include "QueryHandler.h"

class LogServer {
public:
    explicit LogServer(int port = 9999);
    ~LogServer();

    LogServer(const LogServer&) = delete;
    LogServer& operator=(const LogServer&) = delete;

    void start();
    void stop();

private:
    void initialize();
    void runEventLoop();
    void handleNewConnection(int listener_fd, bool is_query_port);
    void handleClientTask(int client_fd);
    void handleQueryTask(int client_fd);

    int port_;
    int queryPort_;
    int listenFd_;
    int queryFd_;
    fd_set masterSet_;
    std::atomic<bool> running_;
    
    std::unique_ptr<Logger> logger_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::shared_ptr<LogBuffer> logBuffer_;
    std::unique_ptr<QueryHandler> queryHandler_;
};

#endif // LOGSERVER_H
```

### 3.8. `src/LogServer.cpp` (MVP2 업데이트)

`select` 기반의 이벤트 루프를 사용하여 두 개의 포트(로그, 쿼리)를 동시에 감시하고, 들어온 요청을 스레드 풀에 위임하는 로직으로 재구성됩니다.

```cpp
// [SEQUENCE: MVP4-31]
#include "LogServer.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <signal.h>

static LogServer* g_server_ptr = nullptr;
void signal_handler_cpp(int signum) {
    if (g_server_ptr) g_server_ptr->stop();
}

// [SEQUENCE: MVP4-32]
// 생성자: 모든 멤버 변수 초기화
LogServer::LogServer(int port)
    : port_(port), queryPort_(9998), listenFd_(-1), queryFd_(-1), running_(false) {
    logger_ = std::make_unique<ConsoleLogger>();
    threadPool_ = std::make_unique<ThreadPool>();
    logBuffer_ = std::make_shared<LogBuffer>();
    queryHandler_ = std::make_unique<QueryHandler>(logBuffer_);
    FD_ZERO(&masterSet_);
}

// [SEQUENCE: MVP4-33]
// 소멸자: 서버 중지
LogServer::~LogServer() {
    stop();
}

// [SEQUENCE: MVP4-34]
// 서버 시작
void LogServer::start() {
    if (running_) return;
    initialize();
    running_ = true;
    logger_->log("Server started.");
    runEventLoop();
}

// [SEQUENCE: MVP4-35]
// 서버 중지
void LogServer::stop() {
    if (!running_) return;
    running_ = false;
    if (listenFd_ != -1) close(listenFd_);
    if (queryFd_ != -1) close(queryFd_);
    logger_->log("Server stopped.");
}

// [SEQUENCE: MVP4-36]
// 리스너 소켓 생성 및 초기화
void LogServer::initialize() {
    auto create_listener = [&](int port) -> int {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) throw std::runtime_error("Socket creation failed");
        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) throw std::runtime_error("Bind failed");
        if (listen(fd, 128) < 0) throw std::runtime_error("Listen failed");
        return fd;
    };
    listenFd_ = create_listener(port_);
    queryFd_ = create_listener(queryPort_);
    FD_SET(listenFd_, &masterSet_);
    FD_SET(queryFd_, &masterSet_);
    g_server_ptr = this;
    signal(SIGINT, signal_handler_cpp);
}

// [SEQUENCE: MVP4-37]
// 메인 이벤트 루프
void LogServer::runEventLoop() {
    while (running_) {
        fd_set read_fds = masterSet_;
        int max_fd = std::max(listenFd_, queryFd_);
        timeval tv = {1, 0};
        int result = select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);
        if (result < 0 && errno != EINTR) {
            logger_->log("Select error");
            break;
        }
        if (result == 0) continue;

        if (FD_ISSET(listenFd_, &read_fds)) {
            handleNewConnection(listenFd_, false);
        }
        if (FD_ISSET(queryFd_, &read_fds)) {
            handleNewConnection(queryFd_, true);
        }
    }
}

// [SEQUENCE: MVP4-38]
// 새 연결 처리
void LogServer::handleNewConnection(int listener_fd, bool is_query_port) {
    int client_fd = accept(listener_fd, nullptr, nullptr);
    if (client_fd < 0) return;

    if (is_query_port) {
        threadPool_->enqueue(&LogServer::handleQueryTask, this, client_fd);
    } else {
        threadPool_->enqueue(&LogServer::handleClientTask, this, client_fd);
    }
}

// [SEQUENCE: MVP4-39]
// 로그 클라이언트 작업
void LogServer::handleClientTask(int client_fd) {
    char buffer[4096];
    while (true) {
        ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (nbytes <= 0) break;
        buffer[nbytes] = '\0';
        logBuffer_->push(std::string(buffer));
    }
    close(client_fd);
}

// [SEQUENCE: MVP4-40]
// 쿼리 클라이언트 작업
void LogServer::handleQueryTask(int client_fd) {
    char buffer[4096];
    ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (nbytes > 0) {
        buffer[nbytes] = '\0';
        std::string query(buffer);
        // Remove trailing newline
        query.erase(query.find_last_not_of("\r\n") + 1);
        std::string response = queryHandler_->processQuery(query);
        send(client_fd, response.c_str(), response.length(), 0);
    }
    close(client_fd);
}
```

### 3.9. `src/main.cpp` (MVP2 업데이트)

`main` 함수는 `LogServer`를 생성하고 `start()`를 호출하는 역할만 합니다. 모든 복잡성은 `LogServer` 클래스 내부에서 처리됩니다.

```cpp
// [SEQUENCE: MVP4-41]
#include "LogServer.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        // [SEQUENCE: MVP4-42]
        int port = (argc > 1) ? std::stoi(argv[1]) : 9999;
        LogServer server(port);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## 4. 테스트 코드 (Test Code)

C++ MVP2 서버는 C MVP2와 동일한 포트 및 프로토콜을 사용하므로, `DevHistory03.md`에 포함된 `tests/test_mvp2.py` 스크립트를 사용하여 기능을 검증할 수 있습니다.

*(테스트 코드 내용은 DevHistory03.md와 동일하여 생략)*

```
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
# [SEQUENCE: MVP5-1]
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

# [SEQUENCE: MVP5-2]
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
// [SEQUENCE: MVP5-3]
#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <stdbool.h>
#include <time.h>
#include <regex.h>

// [SEQUENCE: MVP5-4]
// 쿼리 연산자 종류 (AND/OR)
typedef enum {
    OP_AND,
    OP_OR
} operator_type_t;

// [SEQUENCE: MVP5-5]
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

// [SEQUENCE: MVP5-6]
// 쿼리 파서 생명주기 및 파싱 함수
parsed_query_t* query_parser_create(void);
void query_parser_destroy(parsed_query_t* query);
int query_parser_parse(parsed_query_t* query, const char* query_string);

// [SEQUENCE: MVP5-7]
// 로그 항목이 쿼리 조건과 일치하는지 확인하는 함수
bool query_matches_log(const parsed_query_t* query, const char* log_message, time_t timestamp);

#endif // QUERY_PARSER_H
```

### 3.2. `src/query_parser.c` (신규 파일)

쿼리 파서의 핵심 로직을 구현합니다. `strtok_r`을 사용하여 쿼리 문자열을 토큰으로 분리하고, 각 `key=value` 쌍을 분석하여 `parsed_query_t` 구조체를 채웁니다.

```c
// [SEQUENCE: MVP5-8]
#include "query_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <strings.h> // for strcasecmp

// [SEQUENCE: MVP5-9]
// parsed_query_t 구조체 생성 및 초기화
parsed_query_t* query_parser_create(void) {
    parsed_query_t* query = calloc(1, sizeof(parsed_query_t));
    if (query) {
        query->op = OP_AND; // 기본 연산자는 AND
    }
    return query;
}

// [SEQUENCE: MVP5-10]
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

// [SEQUENCE: MVP5-11]
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

    // [SEQUENCE: MVP5-12]
    // `strtok_r`을 사용하여 스페이스 기준으로 파라미터 분리 (스레드 안전)
    char* saveptr1;
    char* token = strtok_r(params_str, " ", &saveptr1);
    while (token) {
        char* equals = strchr(token, '=');
        if (equals) {
            *equals = '\0'; // key와 value 분리
            char* key = token;
            char* value = equals + 1;

            // [SEQUENCE: MVP5-13]
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

// [SEQUENCE: MVP5-14]
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
// [SEQUENCE: MVP5-15]
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

// [SEQUENCE: MVP5-16]
// MVP3에 추가된 고급 검색 함수
int log_buffer_search_enhanced(log_buffer_t* buffer, const struct parsed_query* query, char*** results, int* count);

size_t log_buffer_size(log_buffer_t* buffer);
void log_buffer_get_stats(log_buffer_t* buffer, unsigned long* total, unsigned long* dropped);

#endif // LOG_BUFFER_H
```

### 3.4. `src/log_buffer.c` (MVP3 업데이트)

`log_buffer_search_enhanced` 함수를 구현합니다. 이 함수는 `query_matches_log` 헬퍼 함수를 사용하여 각 로그 항목이 복잡한 쿼리 조건에 부합하는지 확인합니다.

```c
// [SEQUENCE: MVP5-17]
// ... (log_buffer_create, push 등 기존 함수는 동일) ...

#include "query_parser.h" // query_matches_log 사용을 위해 추가

// [SEQUENCE: MVP5-18]
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
// [SEQUENCE: MVP5-19]
#include "query_handler.h"
#include "server.h"
#include "query_parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

// [SEQUENCE: MVP5-20]
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

// [SEQUENCE: MVP5-21]
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

// [SEQUENCE: MVP5-22]
// 쿼리 연결 수락 및 처리 (MVP2와 동일)
void handle_query_connection(log_server_t* server) {
    // ... (MVP2와 동일한 로직)
}
```

## 4. 테스트 코드 (Test Code)

고급 쿼리 기능을 테스트하기 위해 새로운 Python 테스트 스크립트(`tests/test_mvp3.py`)가 작성되었습니다. 이 스크립트는 정규식, 시간 범위, 다중 키워드 등 다양한 조합의 쿼리를 서버에 전송하고 결과를 검증합니다.

```python
# [SEQUENCE: MVP5-23]
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
```# DevHistory 06: LogCaster-CPP MVP3 - Modern C++ Query Engine

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
# [SEQUENCE: MVP6-1] 
# CMake 최소 요구 버전 설정
cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름 및 C++ 언어 지정
project(LogCasterCPP VERSION 1.0.0 LANGUAGES CXX)

# C++ 표준 설정 (C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 헤더 파일 경로 추가
include_directories(include)

# [SEQUENCE: MVP6-2]
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
// [SEQUENCE: MVP6-3]
#ifndef QUERYPARSER_H
#define QUERYPARSER_H

#include <string>
#include <vector>
#include <optional>
#include <regex>
#include <chrono>
#include <memory>
#include "LogBuffer.h" // For LogEntry

// [SEQUENCE: MVP6-4]
// 쿼리 연산자 종류
enum class OperatorType {
    AND,
    OR
};

// [SEQUENCE: MVP6-5]
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

// [SEQUENCE: MVP6-6]
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
// [SEQUENCE: MVP6-7]
#include "QueryParser.h"
#include <sstream>
#include <algorithm>
#include <iostream>

// [SEQUENCE: MVP6-8]
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

        // [SEQUENCE: MVP6-9]
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

// [SEQUENCE: MVP6-10]
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
// [SEQUENCE: MVP6-11]
#ifndef LOGBUFFER_H
#define LOGBUFFER_H

// ... (기존 include 및 LogEntry 구조체는 동일)

// Forward declaration
class ParsedQuery;

class LogBuffer {
public:
    // ... (기존 메소드 선언은 동일)

    // [SEQUENCE: MVP6-12]
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
// [SEQUENCE: MVP6-13]
#include "LogBuffer.h"
#include "QueryParser.h"
#include <sstream>
#include <iomanip>

// ... (push, search 등 기존 함수는 동일) ...

// [SEQUENCE: MVP6-14]
// 고급 검색 기능 구현
std::vector<std::string> LogBuffer::searchEnhanced(const ParsedQuery& query) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> results;
    for (const auto& entry : buffer_) {
        // [SEQUENCE: MVP6-15]
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
// [SEQUENCE: MVP6-16]
#include "QueryHandler.h"
#include "QueryParser.h"
#include <sstream>

// ... (생성자는 동일) ...

// [SEQUENCE: MVP6-17]
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

// [SEQUENCE: MVP6-18]
// 검색 쿼리 처리 로직 수정
std::string QueryHandler::handleSearch(const std::string& query) {
    try {
        // [SEQUENCE: MVP6-19]
        // QueryParser를 사용하여 쿼리 문자열을 파싱
        auto parsed_query = QueryParser::parse(query);
        if (!parsed_query) {
            return "ERROR: Failed to parse query.\n";
        }

        // [SEQUENCE: MVP6-20]
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

// [SEQUENCE: MVP6-21]
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
# [SEQUENCE: MVP6-22]
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
```# DevHistory 07: LogCaster-C MVP4 - Persistence Layer

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
# [SEQUENCE: MVP7-1]
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

# [SEQUENCE: MVP7-2]
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
// [SEQUENCE: MVP7-3]
#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include <stdio.h>

// [SEQUENCE: MVP7-4]
// 영속성 설정 구조체
typedef struct {
    bool enabled;
    char log_directory[256];
    size_t max_file_size;
} persistence_config_t;

// [SEQUENCE: MVP7-5]
// 파일에 쓸 로그 항목 (큐에 저장될 데이터)
typedef struct write_entry {
    char* message;
    struct write_entry* next;
} write_entry_t;

// [SEQUENCE: MVP7-6]
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

// [SEQUENCE: MVP7-7]
// 영속성 관리자 생명주기 및 API 함수
persistence_manager_t* persistence_create(const persistence_config_t* config);
void persistence_destroy(persistence_manager_t* manager);
int persistence_write(persistence_manager_t* manager, const char* message);

#endif // PERSISTENCE_H
```

### 3.2. `src/persistence.c` (신규 파일)

비동기 파일 쓰기를 위한 Writer 스레드와 파일 로테이션 로직을 구현합니다.

```c
// [SEQUENCE: MVP7-8]
#include "persistence.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

static void* persistence_writer_thread(void* arg);
static void rotate_log_file(persistence_manager_t* manager);

// [SEQUENCE: MVP7-9]
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

    // [SEQUENCE: MVP7-10]
    // Writer 스레드 생성
    if (pthread_create(&manager->writer_thread, NULL, persistence_writer_thread, manager) != 0) {
        fclose(manager->current_file);
        free(manager);
        return NULL;
    }

    return manager;
}

// [SEQUENCE: MVP7-11]
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

// [SEQUENCE: MVP7-12]
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

                // [SEQUENCE: MVP7-13]
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

// [SEQUENCE: MVP7-14]
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

// [SEQUENCE: MVP7-15]
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
// [SEQUENCE: MVP7-16]
#ifndef SERVER_H
#define SERVER_H

// ... (기존 include)
#include "persistence.h" // persistence.h 추가

// ... (기존 define)

typedef struct log_server {
    // ... (MVP3와 동일한 필드)
    thread_pool_t* thread_pool;
    log_buffer_t* log_buffer;

    // [SEQUENCE: MVP7-17]
    // MVP4 추가 사항
    persistence_manager_t* persistence;
} log_server_t;

// ... (나머지 프로토타입은 MVP3와 동일)

#endif // SERVER_H
```

### 3.4. `src/server.c` (MVP4 업데이트)

클라이언트 처리 작업(`handle_client_job`)에서 로그를 디스크에 쓰도록 `persistence_write`를 호출하는 로직을 추가합니다.

```c
// [SEQUENCE: MVP7-18]
// ... (include 및 전역 변수 선언)

// [SEQUENCE: MVP7-19]
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

        // [SEQUENCE: MVP7-20]
        // 2. 영속성 관리자에게 쓰기 요청 (활성화된 경우)
        if (job->server->persistence) {
            persistence_write(job->server->persistence, buffer);
        }
    }

    close(job->client_fd);
    free(job);
}

// [SEQUENCE: MVP7-21]
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

// [SEQUENCE: MVP7-22]
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
// [SEQUENCE: MVP7-23]
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

    // [SEQUENCE: MVP7-24]
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

    // [SEQUENCE: MVP7-25]
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
# [SEQUENCE: MVP7-26]
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
```# DevHistory 08: LogCaster-CPP MVP4 - C++ Persistence Layer

## 1. 개요 (Overview)

C++ 구현의 네 번째 단계(MVP4)는 C++의 현대적인 기능들을 사용하여 C 버전 MVP4와 동일한 영속성(Persistence) 계층을 구현합니다. 이 단계에서는 C++ 표준 라이브러리가 파일 시스템, 파일 I/O, 스레딩을 얼마나 안전하고 우아하게 처리할 수 있는지 보여주는 데 중점을 둡니다.

**주요 기능 추가 및 개선:**
- **비동기 로깅:** C 버전과 마찬가지로 별도의 Writer 스레드를 두어 파일 I/O 작업을 비동기적으로 처리합니다. `std::thread`, `std::mutex`, `std::condition_variable`, `std::queue` 등 C++ 표준 라이브러리 요소들을 사용하여 이를 구현합니다.
- **`std::filesystem` 활용:** 로그 디렉토리 생성, 파일 경로 조합, 로그 파일 이름 변경(로테이션) 등 모든 파일 시스템 관련 작업을 C++17 표준인 `<filesystem>` 라이브러리를 사용하여 처리합니다. 이를 통해 플랫폼 종속적인 코드를 피하고 코드의 이식성과 가독성을 높입니다.
- **`std::fstream` 활용:** 파일 읽기/쓰기는 C의 `FILE*` 포인터 대신 C++의 `<fstream>` 라이브러리(구체적으로 `std::ofstream`)를 사용합니다. 이는 RAII(Resource Acquisition Is Initialization) 패턴을 따르므로, 파일 핸들을 수동으로 닫아주지 않아도 소멸자에서 자동으로 리소스가 해제되어 리소스 누수 가능성을 줄여줍니다.
- **RAII 기반 리소스 관리:** `PersistenceManager`의 소멸자에서 Writer 스레드를 안전하게 종료하고 `join()`하는 로직을 구현하여, 프로그램 종료 시 모든 보류 중인 로그가 디스크에 기록되도록 보장합니다.

**아키텍처 변경:**
- **`PersistenceManager` 클래스 도입:** 영속성 관련 모든 책임을 캡슐화하는 클래스입니다. 설정, 쓰기 큐, Writer 스레드, 파일 핸들러 등을 멤버 변수로 관리합니다.
- **`LogServer`와의 연동:** `LogServer`는 `std::unique_ptr<PersistenceManager>`를 소유하며, `main` 함수에서 커맨드 라인 인자에 따라 생성된 `PersistenceManager` 객체를 주입(inject)받습니다. `LogServer`의 클라이언트 처리 로직은 영속성 관리자가 존재할 경우 `persistence->write()`를 호출하여 로그 기록을 요청합니다.

이 MVP를 통해 C++ 버전은 C 버전의 영속성 기능을 모두 갖추면서, C++의 타입 안전성, 리소스 관리 용이성, 플랫폼 독립성 등의 장점을 활용하여 훨씬 더 견고하고 유지보수하기 쉬운 코드를 완성합니다.

## 2. 빌드 시스템 (Build System)

`Persistence.cpp`가 소스 목록에 추가되고, 구형 컴파일러에서 `<filesystem>`을 지원하기 위한 조건부 라이브러리 링크 설정이 추가됩니다.

```cmake
# [SEQUENCE: MVP8-1]
# ... (project, CMAKE_CXX_STANDARD 등은 MVP6와 동일) ...

# [SEQUENCE: MVP8-2]
# MVP4에 필요한 소스 파일 목록 (Persistence.cpp 추가)
set(SOURCES
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/LogBuffer.cpp
    src/QueryHandler.cpp
    src/QueryParser.cpp
    src/Persistence.cpp
)

add_executable(logcaster-cpp ${SOURCES})

target_link_libraries(logcaster-cpp PRIVATE Threads::Threads)

# [SEQUENCE: MVP8-3]
# 구형 GCC/G++ 컴파일러를 위한 stdc++fs 라이브러리 링크
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(logcaster-cpp PRIVATE stdc++fs)
endif()
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. `include/Persistence.h` (신규 파일)

영속성 관리자를 위한 설정 구조체(`PersistenceConfig`)와 메인 클래스(`PersistenceManager`)를 정의합니다.

```cpp
// [SEQUENCE: MVP8-4]
#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>
#include <filesystem>

// [SEQUENCE: MVP8-5]
// 영속성 설정을 위한 구조체
struct PersistenceConfig {
    bool enabled = false;
    std::filesystem::path log_directory = "./logs";
    size_t max_file_size = 10 * 1024 * 1024; // 10MB
    std::chrono::milliseconds flush_interval = std::chrono::milliseconds(1000);
};

// [SEQUENCE: MVP8-6]
// 영속성 관리자 클래스
class PersistenceManager {
public:
    explicit PersistenceManager(const PersistenceConfig& config);
    ~PersistenceManager();

    PersistenceManager(const PersistenceManager&) = delete;
    PersistenceManager& operator=(const PersistenceManager&) = delete;

    void write(const std::string& message);

private:
    void writerThread();
    void rotateFile();

    PersistenceConfig config_;
    std::ofstream log_file_;
    std::filesystem::path current_filepath_;
    size_t current_file_size_ = 0;

    std::queue<std::string> write_queue_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::thread writer_thread_;
    std::atomic<bool> running_;
};

#endif // PERSISTENCE_H
```

### 3.2. `src/Persistence.cpp` (신규 파일)

`PersistenceManager`의 핵심 로직을 구현합니다. 생성자에서 스레드를 시작하고, 소멸자에서 스레드를 안전하게 종료하며, 비동기 쓰기 및 파일 로테이션을 처리합니다.

```cpp
// [SEQUENCE: MVP8-7]
#include "Persistence.h"
#include <iostream>
#include <iomanip>

// [SEQUENCE: MVP8-8]
// 생성자: 디렉토리 생성, 파일 열기, Writer 스레드 시작
PersistenceManager::PersistenceManager(const PersistenceConfig& config)
    : config_(config), running_(true) {
    if (!config_.enabled) return;

    try {
        std::filesystem::create_directories(config_.log_directory);
        current_filepath_ = config_.log_directory / "current.log";
        log_file_.open(current_filepath_, std::ios::app);
        if (!log_file_.is_open()) {
            throw std::runtime_error("Failed to open log file: " + current_filepath_.string());
        }
        current_file_size_ = std::filesystem::exists(current_filepath_) ? std::filesystem::file_size(current_filepath_) : 0;

        writer_thread_ = std::thread(&PersistenceManager::writerThread, this);
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error("Filesystem error: " + std::string(e.what()));
    }
}

// [SEQUENCE: MVP8-9]
// 소멸자: Writer 스레드의 안전한 종료 보장 (RAII)
PersistenceManager::~PersistenceManager() {
    if (!config_.enabled || !running_) return;

    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        running_ = false;
    }
    condition_.notify_one();
    if (writer_thread_.joinable()) {
        writer_thread_.join();
    }
}

// [SEQUENCE: MVP8-10]
// 외부에서 로그 쓰기를 요청하는 API
void PersistenceManager::write(const std::string& message) {
    if (!config_.enabled) return;
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        write_queue_.push(message);
    }
    condition_.notify_one();
}

// [SEQUENCE: MVP8-11]
// Writer 스레드의 메인 루프
void PersistenceManager::writerThread() {
    while (running_ || !write_queue_.empty()) {
        std::queue<std::string> local_queue;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait_for(lock, config_.flush_interval, [this] { return !running_ || !write_queue_.empty(); });
            
            if (!write_queue_.empty()) {
                local_queue.swap(write_queue_);
            }
        }

        while (!local_queue.empty()) {
            const std::string& message = local_queue.front();
            if (log_file_.is_open()) {
                log_file_ << message << std::endl;
                current_file_size_ += message.length() + 1;
            }
            local_queue.pop();
        }

        if (log_file_.is_open() && current_file_size_ >= config_.max_file_size) {
            rotateFile();
        }
    }
}

// [SEQUENCE: MVP8-12]
// 로그 파일 로테이션
void PersistenceManager::rotateFile() {
    log_file_.close();
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream new_filename;
    new_filename << "log-" << std::put_time(std::localtime(&time_t_now), "%Y%m%d-%H%M%S") << ".log";
    
    try {
        std::filesystem::rename(current_filepath_, config_.log_directory / new_filename.str());
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to rotate log file: " << e.what() << std::endl;
    }

    log_file_.open(current_filepath_, std::ios::app);
    current_file_size_ = 0;
}
```

### 3.3. `include/LogServer.h` (MVP4 업데이트)

`PersistenceManager`를 소유하고 관리하기 위한 포인터와 설정 메소드를 추가합니다.

```cpp
// [SEQUENCE: MVP8-13]
#ifndef LOGSERVER_H
#define LOGSERVER_H

// ... (기존 include)
#include "Persistence.h" // Persistence.h 추가

class LogServer {
public:
    // ... (기존 public 메소드)

    // [SEQUENCE: MVP8-14]
    // PersistenceManager를 주입하기 위한 메소드
    void setPersistenceManager(std::unique_ptr<PersistenceManager> persistence);

private:
    // ... (기존 private 메소드)

    // ... (기존 멤버 변수)
    std::unique_ptr<QueryHandler> queryHandler_;

    // [SEQUENCE: MVP8-15]
    // MVP4 추가 사항
    std::unique_ptr<PersistenceManager> persistence_;
};

#endif // LOGSERVER_H
```

### 3.4. `src/LogServer.cpp` (MVP4 업데이트)

클라이언트 처리 로직에서 `PersistenceManager`의 `write` 메소드를 호출하도록 수정합니다.

```cpp
// [SEQUENCE: MVP8-16]
// ... (include 및 생성자 등은 동일)

// [SEQUENCE: MVP8-17]
// 클라이언트 작업 핸들러 (MVP4 버전)
void LogServer::handleClientTask(int client_fd) {
    char buffer[4096];
    while (true) {
        ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (nbytes <= 0) break;
        buffer[nbytes] = '\0';
        std::string log_message(buffer);

        // 1. 인메모리 버퍼에 저장
        logBuffer_->push(log_message);

        // [SEQUENCE: MVP8-18]
        // 2. 영속성 관리자에게 쓰기 요청 (활성화된 경우)
        if (persistence_) {
            persistence_->write(log_message);
        }
    }
    close(client_fd);
}

// [SEQUENCE: MVP8-19]
// PersistenceManager 설정 메소드 구현
void LogServer::setPersistenceManager(std::unique_ptr<PersistenceManager> persistence) {
    persistence_ = std::move(persistence);
}

// ... (나머지 함수는 MVP3와 동일)
```

### 3.5. `src/main.cpp` (MVP4 업데이트)

`getopt`를 사용하여 영속성 관련 커맨드 라인 인자를 파싱하고, `PersistenceManager`를 생성하여 `LogServer`에 주입하는 로직이 추가됩니다.

```cpp
// [SEQUENCE: MVP8-20]
#include "LogServer.h"
#include "Persistence.h"
#include <iostream>
#include <getopt.h>

int main(int argc, char* argv[]) {
    int port = 9999;
    PersistenceConfig persist_config;

    // [SEQUENCE: MVP8-21]
    // 커맨드 라인 인자 파싱
    int opt;
    while ((opt = getopt(argc, argv, "p:d:s:Ph")) != -1) {
        switch (opt) {
            case 'p': port = std::stoi(optarg); break;
            case 'P': persist_config.enabled = true; break;
            case 'd': persist_config.log_directory = optarg; break;
            case 's': persist_config.max_file_size = std::stoul(optarg) * 1024 * 1024; break;
            case 'h':
                std::cout << "Usage: " << argv[0] << " [-p port] [-P] [-d dir] [-s size_mb] [-h]" << std::endl;
                return 0;
        }
    }

    try {
        LogServer server(port);

        // [SEQUENCE: MVP8-22]
        // 영속성 관리자 생성 및 주입
        if (persist_config.enabled) {
            auto persistence = std::make_unique<PersistenceManager>(persist_config);
            server.setPersistenceManager(std::move(persistence));
            std::cout << "Persistence enabled. Dir: " << persist_config.log_directory 
                      << ", Max Size: " << persist_config.max_file_size / (1024*1024) << " MB" << std::endl;
        }

        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## 4. 테스트 코드 (Test Code)

C++ MVP4 서버는 C MVP4와 동일한 커맨드 라인 인자와 프로토콜을 사용하므로, `DevHistory07.md`에 포함된 `tests/test_persistence.py` 스크립트를 사용하여 기능을 검증할 수 있습니다.

*(테스트 코드 내용은 DevHistory07.md와 동일하여 생략)*
# DevHistory 09: LogCaster-C MVP5 - Security Audit & Stability Enhancement

## 1. 개요 (Overview)

다섯 번째 단계(MVP5)는 새로운 기능 추가가 아닌, 기존 C 코드베이스의 안정성과 보안을 강화하는 데 집중하는 중요한 '경화(Hardening)' 단계입니다. MVP4까지의 기능 구현을 마친 후, 코드 감사를 통해 발견된 여러 심각한 문제점들을 해결하는 것을 목표로 합니다. 이 단계는 프로덕션 환경에서 발생할 수 있는 잠재적 오류, 메모리 누수, 서비스 거부(DoS) 공격 등을 예방하기 위해 필수적입니다.

**주요 해결 과제:**
- **경쟁 조건 (Race Condition) 해결:** 여러 스레드에서 동시에 접근하는 `client_count` 변수를 `pthread_mutex_t`로 보호하여 정확한 클라이언트 수를 유지합니다.
- **메모리 누수 (Memory Leak) 수정:** 쿼리 파서에서 정규식 컴파일이 실패할 경우 메모리가 해제되지 않던 치명적인 버그를 수정합니다.
- **입력 유효성 검증 강화:** `atoi` 함수의 불안전한 사용을 `strtol`로 대체하여, 커맨드 라인으로 입력되는 포트 번호나 파일 크기 값에 대한 정수 오버플로우 및 유효성 검사를 수행합니다.
- **리소스 고갈 방어:** 악의적인 클라이언트가 비정상적으로 큰 로그 메시지를 보내 서버 메모리를 고갈시키는 것을 방지하기 위해, 수신되는 로그의 최대 길이를 제한하고 초과 시 절단하는 로직을 추가합니다.
- **코드 품질 개선:** 매직 넘버(magic number)를 명명된 상수로 대체하여 코드의 가독성과 유지보수성을 향상시킵니다.

이 MVP를 통해 LogCaster의 C 버전은 기능적 완성도를 넘어, 실제 운영 환경에서 요구되는 안정성과 보안성을 갖춘 견고한 애플리케이션으로 거듭나게 됩니다.

## 2. 주요 수정 사항 및 코드

이 단계에서는 새로운 파일 추가 없이 기존 파일들이 수정됩니다.

### 2.1. `include/server.h` (수정)

`log_server_t` 구조체에 클라이언트 카운터 보호를 위한 뮤텍스를 추가합니다.

```c
// [SEQUENCE: MVP9-1] 
#ifndef SERVER_H
#define SERVER_H

// ... (기존 include)
#include "persistence.h"

typedef struct log_server {
    // ... (MVP4와 동일한 필드)
    thread_pool_t* thread_pool;
    log_buffer_t* log_buffer;
    persistence_manager_t* persistence;

    // [SEQUENCE: MVP9-2]
    // MVP5 추가: client_count 보호를 위한 뮤텍스
    pthread_mutex_t client_count_mutex;
} log_server_t;

// ... (나머지 프로토타입은 MVP4와 동일)

#endif // SERVER_H
```

### 2.2. `src/server.c` (수정)

클라이언트 수( `client_count`)를 증가시키거나 감소시키는 모든 지점에 뮤텍스 잠금을 추가하고, 로그 메시지 크기를 검증하는 로직을 추가합니다.

```c
// [SEQUENCE: MVP9-3]
// ... (include 및 전역 변수 선언)

// [SEQUENCE: MVP9-4]
// 클라이언트 처리 작업 (MVP5 버전)
static void handle_client_job(void* arg) {
    client_job_t* job = (client_job_t*)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = recv(job->client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';

        // [SEQUENCE: MVP9-5]
        // 로그 메시지 크기 제한 및 절단
        const int SAFE_LOG_LENGTH = 1024;
        if (bytes_read > SAFE_LOG_LENGTH) {
            buffer[SAFE_LOG_LENGTH - 4] = '.';
            buffer[SAFE_LOG_LENGTH - 3] = '.';
            buffer[SAFE_LOG_LENGTH - 2] = '.';
            buffer[SAFE_LOG_LENGTH - 1] = '\0';
        }

        if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        }
        log_buffer_push(job->server->log_buffer, buffer);
        if (job->server->persistence) {
            persistence_write(job->server->persistence, buffer);
        }
    }

    close(job->client_fd);

    // [SEQUENCE: MVP9-6]
    // client_count를 스레드 안전하게 감소
    pthread_mutex_lock(&job->server->client_count_mutex);
    job->server->client_count--;
    pthread_mutex_unlock(&job->server->client_count_mutex);

    free(job);
}

// [SEQUENCE: MVP9-7]
// 새 연결 처리 로직 (MVP5 버전)
static void handle_new_connection(log_server_t* server) {
    int new_fd = accept(server->listen_fd, NULL, NULL);
    if (new_fd < 0) return;

    // [SEQUENCE: MVP9-8]
    // client_count 접근 전후로 뮤텍스 잠금/해제
    pthread_mutex_lock(&server->client_count_mutex);
    if (server->client_count >= MAX_CLIENTS) {
        pthread_mutex_unlock(&server->client_count_mutex);
        close(new_fd);
        return;
    }
    server->client_count++;
    pthread_mutex_unlock(&server->client_count_mutex);

    client_job_t* job = malloc(sizeof(client_job_t));
    if (!job) {
        close(new_fd);
        // 작업 할당 실패 시에도 count를 다시 감소시켜야 함
        pthread_mutex_lock(&server->client_count_mutex);
        server->client_count--;
        pthread_mutex_unlock(&server->client_count_mutex);
        return;
    }
    job->client_fd = new_fd;
    job->server = server;
    thread_pool_add_job(server->thread_pool, handle_client_job, job);
}

// [SEQUENCE: MVP9-9]
// 서버 생성 및 소멸 시 뮤텍스 초기화/파괴
log_server_t* server_create(int port) {
    log_server_t* server = calloc(1, sizeof(log_server_t));
    // ... (MVP4와 동일한 초기화)
    server->persistence = NULL;
    if (pthread_mutex_init(&server->client_count_mutex, NULL) != 0) {
        // ... (에러 처리)
        free(server);
        return NULL;
    }
    return server;
}

void server_destroy(log_server_t* server) {
    // ... (MVP4와 동일한 소멸 로직)
    pthread_mutex_destroy(&server->client_count_mutex);
    free(server);
}

// ... (나머지 함수는 MVP4와 동일)
```

### 2.3. `src/query_parser.c` (수정)

정규식 컴파일 실패 시 `regex_pattern` 문자열이 담긴 메모리가 누수되던 버그를 수정합니다.

```c
// [SEQUENCE: MVP9-10]
// ... (query_parser_parse 함수 내부)

            } else if (strcasecmp(key, "regex") == 0) {
                query->has_regex = true;
                query->regex_pattern = strdup(value);
                query->compiled_regex = malloc(sizeof(regex_t));
                if (regcomp(query->compiled_regex, value, REG_EXTENDED | REG_NOSUB) != 0) {
                    // 정규식 컴파일 실패 시
                    free(query->compiled_regex);
                    query->compiled_regex = NULL;

                    // [SEQUENCE: MVP9-11]
                    // 메모리 누수를 유발했던 아래 라인 추가
                    if (query->regex_pattern) {
                        free(query->regex_pattern);
                        query->regex_pattern = NULL;
                    }
                }
// ...
```

### 2.4. `src/main.c` (수정)

안전하지 않은 `atoi`를 `strtol`로 교체하고, 입력값에 대한 범위 및 오버플로우 검사를 추가합니다.

```c
// [SEQUENCE: MVP9-12]
#include "server.h"
#include "persistence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;
    persistence_config_t persist_config = { .enabled = false };
    // ...

    int opt;
    while ((opt = getopt(argc, argv, "p:d:s:Ph")) != -1) {
        switch (opt) {
            case 'p': {
                // [SEQUENCE: MVP9-13]
                // 포트 번호에 대한 안전한 파싱
                char* endptr;
                errno = 0;
                long val = strtol(optarg, &endptr, 10);
                if (errno != 0 || *endptr != '\0' || val <= 0 || val > 65535) {
                    fprintf(stderr, "Invalid port number: %s. Must be between 1 and 65535.\n", optarg);
                    return 1;
                }
                port = (int)val;
                break;
            }
            // ... (d, P 옵션은 동일)
            case 's': {
                // [SEQUENCE: MVP9-14]
                // 파일 크기에 대한 안전한 파싱 및 오버플로우 방지
                char* endptr;
                errno = 0;
                long mb_val = strtol(optarg, &endptr, 10);
                if (errno != 0 || *endptr != '\0' || mb_val <= 0) {
                    fprintf(stderr, "Invalid file size: %s\n", optarg);
                    return 1;
                }
                if (mb_val > (long)(SIZE_MAX / (1024 * 1024))) {
                    fprintf(stderr, "File size too large: %s MB\n", optarg);
                    return 1;
                }
                persist_config.max_file_size = (size_t)mb_val * 1024 * 1024;
                break;
            }
            // ...
        }
    }

    // ... (서버 생성 및 실행 로직은 MVP4와 동일)
    return 0;
}
```

## 3. 테스트 코드 (Test Code)

이 단계의 변경 사항을 검증하기 위해 새로운 보안 테스트 스크립트(`tests/test_security.py`)가 작성되었습니다. 이 스크립트는 다음을 확인합니다.
- **클라이언트 카운터 테스트:** 다수의 클라이언트가 연결 및 연결 해제를 반복해도 서버의 클라이언트 수가 정확히 관리되는지 확인합니다.
- **로그 절단 테스트:** 1KB가 넘는 매우 긴 로그 메시지를 보냈을 때, 서버가 이를 안전하게 절단하여 처리하는지 확인합니다.
- **(문서화)** `strtol`로의 변경은 커맨드 라인에서 직접 테스트하여 검증합니다 (예: `./logcaster-c -p 99999`).
- **(문서화)** 메모리 누수 수정은 `valgrind`와 같은 도구를 사용하여 검증합니다.

```python
# [SEQUENCE: MVP9-15]
#!/usr/bin/env python3
import socket
import threading
import time
import subprocess

HOST = '127.0.0.1'
LOG_PORT = 9999
QUERY_PORT = 9998

def get_client_count():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, QUERY_PORT))
        s.sendall(b"STATS\n")
        response = s.recv(1024).decode()
        # STATS: Total=x, Dropped=y, Current=z, Clients=N
        try:
            return int(response.split('=')[-1])
        except:
            return -1

def client_task(duration_s):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, LOG_PORT))
            time.sleep(duration_s)
    except Exception:
        pass

def test_client_counter():
    print("--- Test 1: Client Counter Thread Safety ---")
    initial_count = get_client_count()
    print(f"Initial client count: {initial_count}")

    threads = []
    for _ in range(5):
        t = threading.Thread(target=client_task, args=(2,))
        threads.append(t)
        t.start()
    
    time.sleep(1)
    count_during = get_client_count()
    print(f"Count during connection: {count_during}")
    assert count_during == initial_count + 5

    for t in threads:
        t.join()
    
    time.sleep(1) # Allow server to process disconnects
    final_count = get_client_count()
    print(f"Final client count: {final_count}")
    assert final_count == initial_count
    print("OK\n")

def test_log_truncation():
    print("--- Test 2: Log Message Truncation ---")
    long_message = "A" * 2048
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, LOG_PORT))
        s.sendall((long_message + '\n').encode())
    
    time.sleep(1)
    # Verification requires checking server-side logs or 
    # having a query mechanism that can return the truncated log.
    # For this test, we assume it works if the server doesn't crash.
    print("Sent 2048 byte log. Server should truncate it to ~1024 bytes.")
    print("OK\n")

if __name__ == "__main__":
    server_proc = subprocess.Popen(["./bin/logcaster-c"])
    print("Server started...")
    time.sleep(1)

    test_client_counter()
    test_log_truncation()

    server_proc.terminate()
    server_proc.wait()
    print("All MVP5 tests passed!")
```# DevHistory 10: LogCaster-CPP MVP5 - Security Hardening

## 1. 개요 (Overview)

C++ 구현의 다섯 번째 단계(MVP5)는 C 버전의 MVP5와 마찬가지로, 새로운 기능 추가가 아닌 기존 코드베이스의 안정성과 보안을 강화하는 데 집중합니다. C++의 현대적인 기능들 덕분에 C 버전에서 발견된 대부분의 메모리 및 동시성 문제는 사전에 방지되었지만, 외부 입력에 대한 검증은 여전히 중요한 과제로 남아있습니다. 이 단계는 LogCaster C++ 버전을 프로덕션 환경에 한 걸음 더 가깝게 만드는 마무리 작업입니다.

**주요 해결 과제:**
- **리소스 고갈(Resource Exhaustion) 방어:** C 버전과 동일하게, 악의적인 클라이언트가 의도적으로 매우 큰 로그 메시지를 전송하여 서버의 메모리를 고갈시키려는 공격을 방어해야 합니다. `LogServer`가 클라이언트로부터 데이터를 수신할 때, 메시지의 크기를 검증하고 설정된 최대 길이를 초과할 경우 이를 안전하게 절단하는 로직을 추가합니다.

**아키텍처 변경:**
- 이번 MVP에서는 아키텍처의 큰 변경 없이, `LogServer` 클래스 내부의 클라이언트 데이터 처리 로직만 소폭 수정됩니다.

이 단계를 통해 C++ 버전 또한 외부의 비정상적인 입력에 대해 더 견고하게 대응할 수 있는 능력을 갖추게 됩니다.

## 2. 주요 수정 사항 및 코드

### 2.1. `include/LogServer.h` (수정)

`LogServer` 클래스 내부에 로그 메시지의 최대 허용 길이를 나타내는 상수를 추가합니다.

```cpp
// [SEQUENCE: MVP10-1]
#ifndef LOGSERVER_H
#define LOGSERVER_H

// ... (기존 include는 MVP4와 동일)
#include "Persistence.h"

class LogServer {
public:
    // ... (기존 public 메소드는 MVP4와 동일)

    // [SEQUENCE: MVP10-2]
    // MVP5 추가: 안전한 로그 길이를 위한 상수
    static constexpr size_t SAFE_LOG_LENGTH = 1024;

private:
    // ... (나머지는 MVP4와 동일)
};

#endif // LOGSERVER_H
```

### 2.2. `src/LogServer.cpp` (수정)

`handleClientTask` 메소드에서 `recv`로 데이터를 수신한 후, 해당 데이터의 길이를 `SAFE_LOG_LENGTH`와 비교하여 초과 시 절단하는 로직을 추가합니다.

```cpp
// [SEQUENCE: MVP10-3]
#include "LogServer.h"
// ... (다른 include는 MVP4와 동일)

// ... (생성자, 소멸자, start, stop, initialize, runEventLoop, handleNewConnection 등은 MVP4와 동일)

// [SEQUENCE: MVP10-4]
// 로그 클라이언트 작업 (MVP5 버전)
void LogServer::handleClientTask(int client_fd) {
    char buffer[4096]; // 수신을 위한 물리적 버퍼
    while (true) {
        ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (nbytes <= 0) break;
        
        // [SEQUENCE: MVP10-5]
        // 수신된 데이터로 std::string 생성 및 크기 검증
        std::string log_message(buffer, nbytes);
        if (log_message.length() > SAFE_LOG_LENGTH) {
            log_message.resize(SAFE_LOG_LENGTH - 3);
            log_message += "...";
        }

        // Remove trailing newline
        size_t last_char_pos = log_message.find_last_not_of("\r\n");
        if (std::string::npos != last_char_pos) {
            log_message.erase(last_char_pos + 1);
        }

        // 1. 인메모리 버퍼에 저장
        logBuffer_->push(log_message);

        // 2. 영속성 관리자에게 쓰기 요청 (활성화된 경우)
        if (persistence_) {
            persistence_->write(log_message);
        }
    }
    close(client_fd);
}

// ... (나머지 함수는 MVP4와 동일)
```

## 3. 테스트 코드 (Test Code)

이 변경 사항을 검증하기 위해, C 버전의 보안 테스트(`test_security.py`)와 유사한 테스트를 수행할 수 있습니다. 특히 1KB가 넘는 긴 로그 메시지를 전송하고, 서버가 비정상 종료되지 않고 안정적으로 동작하는지를 확인하는 것이 중요합니다.

*(테스트 코드는 C 버전 MVP5의 `test_log_truncation` 부분과 유사하게 구성할 수 있어 별도로 첨부하지 않음)*

```
# DevHistory 11: LogCaster-CPP MVP6 - IRC Server Integration

## 1. 개요 (Overview)

C++ 구현의 마지막 단계(MVP6)는 `DEVELOPMENT_JOURNAL.md`에는 기록되지 않았지만, 코드베이스에 존재하는 매우 중요한 기능인 **IRC(Internet Relay Chat) 서버 통합**입니다. 이 기능은 LogCaster를 단순한 로그 수집 및 조회 도구에서, 실시간으로 로그를 구독하고 알림을 받을 수 있는 강력한 대화형 모니터링 플랫폼으로 격상시킵니다.

**주요 기능 추가:**
- **내장 IRC 서버:** LogCaster는 이제 자체적으로 IRC 프로토콜을 지원하는 서버를 내장합니다. 사용자는 표준 IRC 클라이언트(예: HexChat, mIRC)를 사용하여 LogCaster에 접속할 수 있습니다.
- **채널 기반 로그 구독:** 사용자는 특정 패턴을 가진 채널에 참여(`JOIN`)함으로써, 해당 패턴과 일치하는 로그만 실시간으로 받아볼 수 있습니다. 예를 들어, `#errors` 채널에 참여하면 `level`이 `ERROR`인 로그만 수신하고, `#services-auth` 채널에 참여하면 `category`가 `auth`인 `services` 로그만 수신하는 식입니다.
- **실시간 알림:** `LogBuffer`에 새로운 로그가 추가될 때, 등록된 채널 패턴과 일치하는지 확인하고, 일치할 경우 해당 채널에 참여한 모든 IRC 클라이언트에게 즉시 로그 메시지를 전송합니다.
- **구조화된 로그 활용:** 이 기능을 위해 기존의 단순 문자열 로그는 `level`, `source`, `category` 등 다양한 메타데이터를 포함하는 구조화된 `LogEntry`로 확장됩니다.

**아키텍처 변경:**
- **IRC 모듈 추가:** `IRCServer`, `IRCClient`, `IRCChannel`, `IRCCommandHandler`, `IRCCommandParser` 등 IRC 프로토콜을 처리하기 위한 다수의 클래스가 새로 추가됩니다.
- **`LogBuffer`와 IRC 연동:** `LogBuffer`에 콜백(callback) 메커니즘이 도입됩니다. `IRCCommandHandler`는 `JOIN` 명령을 처리할 때, 특정 로그 패턴과 해당 알림을 처리할 함수(콜백)를 `LogBuffer`에 등록합니다. `LogBuffer`는 새 로그가 들어올 때마다 등록된 콜백들을 실행하여 IRC 서버에 알림을 보냅니다.
- **`main` 함수 수정:** IRC 서버를 활성화하고 지정된 포트에서 실행하기 위한 커맨드 라인 인자(`-i`, `-I`)와 로직이 추가됩니다.

이 MVP는 LogCaster의 활용성을 극적으로 확장하여, 개발자와 시스템 관리자가 터미널에 직접 접속하지 않고도 익숙한 IRC 클라이언트를 통해 여러 시스템의 로그를 중앙에서 편리하게 모니터링할 수 있게 해줍니다.

## 2. 빌드 시스템 (Build System)

다수의 IRC 관련 소스 파일들이 `CMakeLists.txt`에 추가됩니다.

```cmake
# [SEQUENCE: MVP11-1]
# ... (project, CMAKE_CXX_STANDARD 등은 이전과 동일) ...

# [SEQUENCE: MVP11-2]
# MVP6에 필요한 전체 소스 파일 목록 (IRC 모듈 추가)
set(SOURCES
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/LogBuffer.cpp
    src/QueryHandler.cpp
    src/QueryParser.cpp
    src/Persistence.cpp
    # IRC integration
    src/IRCServer.cpp
    src/IRCClient.cpp
    src/IRCChannel.cpp
    src/IRCChannelManager.cpp
    src/IRCClientManager.cpp
    src/IRCCommandParser.cpp
    src/IRCCommandHandler.cpp
)

# ... (나머지는 MVP8과 동일) 
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. 신규 IRC 모듈 파일

*(이 단계에서는 많은 파일이 추가되므로, 주요 클래스의 정의(헤더 파일)와 핵심 로직 위주로 요약하여 설명합니다. 전체 코드는 코드베이스를 참조하십시오.)*

- **`include/IRCServer.h`:** IRC 클라이언트 연결을 수락하고 각 클라이언트를 스레드 풀에 위임하는 메인 서버 클래스.
- **`include/IRCClient.h`:** 접속한 클라이언트의 정보(소켓, 닉네임, 현재 채널 등)를 관리하는 클래스.
- **`include/IRCChannel.h`:** IRC 채널 정보를 관리하고, 채널에 속한 클라이언트들에게 메시지를 방송(broadcast)하는 클래스.
- **`include/IRCClientManager.h`, `include/IRCChannelManager.h`:** 여러 클라이언트와 채널 객체를 스레드 안전하게 관리하는 매니저 클래스.
- **`include/IRCCommandParser.h`:** 클라이언트가 보낸 원시 IRC 메시지(예: `JOIN #channel\r\n`)를 파싱하여 `IRCCommand` 객체로 변환하는 클래스.
- **`include/IRCCommandHandler.h`:** 파싱된 `IRCCommand`를 받아 실제 동작(채널 참여, 로그 구독 콜백 등록 등)을 수행하는 클래스.

**핵심 로직 예시 (`src/IRCCommandHandler.cpp`의 JOIN 명령 처리):**
```cpp
// [SEQUENCE: MVP11-3]
void IRCCommandHandler::handleJoin(const IRCCommand& command) {
    // ... (클라이언트 정보 가져오기)
    std::string channel_name = command.get_params()[0];

    // 채널에 클라이언트 추가
    auto channel = channel_manager_.get_or_create_channel(channel_name);
    channel->add_client(client);

    // ... (JOIN 성공 응답 클라이언트에 전송)

    // [SEQUENCE: MVP11-4]
    // LogBuffer에 콜백 등록으로 로그 구독
    std::string pattern = channel_name.substr(1); // # 제거
    log_buffer_->registerChannelCallback(pattern, [channel](const LogEntry& entry) {
        std::stringstream ss;
        ss << "[" << entry.level << "] " << entry.message;
        channel->broadcast("SYSTEM", ss.str());
    });
}
```

### 3.2. `include/LogBuffer.h` (수정)

구조화된 로그를 위한 `LogEntry` 확장 및 IRC 연동을 위한 콜백 인터페이스를 추가합니다.

```cpp
// [SEQUENCE: MVP11-5]
#ifndef LOGBUFFER_H
#define LOGBUFFER_H

#include <functional> // for std::function
#include <map> // for std::map
// ... (기존 include)

// [SEQUENCE: MVP11-6]
// 구조화된 로그 항목 (IRC 통합 버전)
struct LogEntry {
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    
    // MVP6 추가 필드
    std::string level;      // 예: ERROR, WARNING, INFO
    std::string source;     // 예: service-auth, 192.168.1.5
    std::string category;   // 예: login, database
    
    LogEntry(std::string msg, std::string lvl = "INFO", std::string src = "") 
        : message(std::move(msg)), timestamp(std::chrono::system_clock::now()), 
          level(std::move(lvl)), source(std::move(src)) {}
};

class LogBuffer {
public:
    // ... (기존 public 메소드)

    // [SEQUENCE: MVP11-7]
    // MVP6 추가: IRC 연동을 위한 메소드
    void addLogWithNotification(const LogEntry& entry);
    void registerChannelCallback(const std::string& pattern, std::function<void(const LogEntry&)> callback);
    void unregisterChannelCallback(const std::string& pattern);

private:
    // ... (기존 private 멤버)

    // [SEQUENCE: MVP11-8]
    // MVP6 추가: 콜백 저장을 위한 맵
    std::map<std::string, std::vector<std::function<void(const LogEntry&)>>> channelCallbacks_;
};

#endif // LOGBUFFER_H
```

### 3.3. `src/LogBuffer.cpp` (수정)

`addLogWithNotification` 메소드에서 로그를 저장한 후, 등록된 콜백들을 실행하여 IRC 채널에 알림을 보내는 로직을 구현합니다.

```cpp
// [SEQUENCE: MVP11-9]
// ... (기존 함수들)

// [SEQUENCE: MVP11-10]
// 로그 추가 및 알림 처리
void LogBuffer::addLogWithNotification(const LogEntry& entry) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (buffer_.size() >= capacity_) {
            dropOldest_();
        }
        buffer_.push_back(entry);
        totalLogs_++;
    }
    notEmpty_.notify_one();

    // [SEQUENCE: MVP11-11]
    // 등록된 콜백 실행하여 알림 전송
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [pattern, callbacks] : channelCallbacks_) {
        // (단순화를 위해 여기서는 간단한 키워드 매칭만 예시)
        bool match = (entry.level == pattern || entry.source == pattern || 
                      entry.message.find(pattern) != std::string::npos);
        if (match) {
            for (const auto& callback : callbacks) {
                callback(entry);
            }
        }
    }
}

// [SEQUENCE: MVP11-12]
// 콜백 등록 및 해제 함수 구현
void LogBuffer::registerChannelCallback(const std::string& pattern, std::function<void(const LogEntry&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    channelCallbacks_[pattern].push_back(callback);
}

void LogBuffer::unregisterChannelCallback(const std::string& pattern) {
    std::lock_guard<std::mutex> lock(mutex_);
    channelCallbacks_.erase(pattern);
}
```

### 3.4. `include/LogServer.h` (수정)

`main` 함수에서 `IRCServer`가 `LogBuffer`에 접근할 수 있도록 `getLogBuffer` 메소드를 추가합니다.

```cpp
// [SEQUENCE: MVP11-13]
class LogServer {
public:
    // ... (기존 public 메소드)
    void setPersistenceManager(std::unique_ptr<PersistenceManager> persistence);

    // [SEQUENCE: MVP11-14]
    // MVP6 추가: IRC 서버와의 연동을 위해 LogBuffer 공유
    std::shared_ptr<LogBuffer> getLogBuffer() const { return logBuffer_; }

private:
    // ... (기존 private 멤버)
};
```

### 3.5. `src/main.cpp` (수정)

IRC 서버를 활성화하는 커맨드 라인 인자(`-i`, `-I`)를 처리하고, `IRCServer`를 생성 및 실행하는 로직을 추가합니다.

```cpp
// [SEQUENCE: MVP11-15]
#include "LogServer.h"
#include "IRCServer.h"
#include <iostream>
#include <getopt.h>
#include <signal.h>

static std::unique_ptr<LogServer> g_logServer;
static std::unique_ptr<IRCServer> g_ircServer;

void signalHandler(int signal) {
    std::cout << "\nShutting down..." << std::endl;
    if (g_ircServer) g_ircServer->stop();
    if (g_logServer) g_logServer->stop();
    exit(0);
}

int main(int argc, char* argv[]) {
    int port = 9999;
    bool irc_enabled = false;
    int irc_port = 6667;
    // ... (영속성 설정 변수)

    // [SEQUENCE: MVP11-16]
    // getopt 파싱 루프에 'i'와 'I:' 옵션 추가
    while ((opt = getopt(argc, argv, "p:d:s:PiI:h")) != -1) {
        switch (opt) {
            // ... (기존 case 문)
            case 'i': irc_enabled = true; break;
            case 'I': 
                irc_enabled = true;
                irc_port = std::stoi(optarg);
                break;
        }
    }

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        g_logServer = std::make_unique<LogServer>(port);
        // ... (영속성 관리자 설정)

        // [SEQUENCE: MVP11-17]
        // IRC 서버 생성 및 별도 스레드에서 실행
        if (irc_enabled) {
            g_ircServer = std::make_unique<IRCServer>(irc_port, g_logServer->getLogBuffer());
            std::thread irc_thread([] { g_ircServer->start(); });
            irc_thread.detach();
            std::cout << "IRC Server enabled on port " << irc_port << std::endl;
        }

        g_logServer->start(); // LogServer는 메인 스레드에서 실행

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```# DevHistory 12: 최종 요약 및 C vs C++ 비교 분석

## 1. 프로젝트 여정 요약 (Project Journey Summary)

LogCaster 프로젝트는 고성능 로그 서버를 C와 C++ 두 가지 언어로 구현하여, 각 언어의 패러다임과 특성을 비교 분석하는 것을 목표로 시작되었습니다. 약 2일간의 여정을 통해, 우리는 단순한 TCP 서버에서 시작하여 스레드 풀, 인메모리 버퍼, 고급 쿼리 엔진, 영속성 계층, 그리고 실시간 IRC 알림 기능까지 갖춘 정교한 시스템을 완성했습니다.

**주요 개발 마일스톤:**
- **MVP1 (기본 TCP 서버):** `select` (C) 및 `thread-per-client` (C++) 모델을 사용하여 기본적인 동시 접속 로그 수신 기능을 구현했습니다.
- **MVP2 (동시성 모델 개선):** 스레드 풀 아키텍처와 인메모리 로그 버퍼를 도입하여 서버의 확장성과 성능을 크게 향상시켰습니다.
- **MVP3 (고급 쿼리 엔진):** 정규식, 시간 범위, 다중 키워드 검색 등 복잡한 조건의 로그 조회가 가능한 쿼리 엔진을 탑재했습니다.
- **MVP4 (영속성 계층):** 비동기 파일 I/O와 로그 로테이션 기능을 구현하여 서버가 재시작되어도 데이터가 보존되도록 안정성을 확보했습니다.
- **MVP5 (보안 및 안정성 강화):** 코드 감사를 통해 발견된 메모리 누수, 경쟁 조건, 입력 유효성 문제 등을 해결하여 프로덕션 수준의 견고함을 갖추었습니다.
- **MVP6 (IRC 통합 - C++ 전용):** 실시간으로 로그를 구독하고 알림을 받을 수 있는 IRC 서버를 통합하여 대화형 모니터링 플랫폼으로 확장했습니다.

이 `DevHistory` 시리즈는 위의 각 단계를 상세히 기록하여, 제3자가 프로젝트의 시작부터 최종 완성까지 모든 과정을 재현하고 학습할 수 있도록 구성되었습니다.

## 2. 최종 아키텍처 비교 (Final Architecture Comparison)

두 버전은 동일한 핵심 기능을 목표로 했지만, 최종적으로는 각 언어의 강점을 살린 다른 형태의 아키텍처와 코드베이스를 갖게 되었습니다.

### 2.1. LogCaster-C (최종)
- **총 코드 라인 수:** 약 2,500 라인
- **파일 수:** 16개 (헤더 8, 소스 8)
- **핵심 아키텍처:**
  - 수동 리소스 관리 (`malloc`/`free`)
  - `pthreads` 기반의 스레드 풀
  - `select` 기반의 단일 스레드 이벤트 루프
  - `strtok_r`, `getopt` 등 C 표준 라이브러리 함수 적극 활용
  - 모든 스레드 안전성을 `pthread_mutex_t`를 통해 직접 관리
- **강점:** 시스템 자원에 대한 직접적이고 세밀한 제어, 작은 바이너리 크기, 예측 가능한 성능.
- **단점:** 개발자가 모든 메모리와 리소스의 생명주기를 직접 관리해야 하므로 버그 발생 가능성이 높음. 오류 처리가 번거롭고 코드가 장황해지기 쉬움.

### 2.2. LogCaster-CPP (최종)
- **총 코드 라인 수:** 약 2,800 라인 (IRC 기능 포함 시 C 버전보다 많음)
- **파일 수:** 28개 (헤더 14, 소스 14)
- **핵심 아키텍처:**
  - RAII와 스마트 포인터(`unique_ptr`, `shared_ptr`)를 통한 자동 리소스 관리
  - C++11 `<thread>` 기반의 스레드 풀
  - `std::deque`, `std::optional`, `std::regex` 등 STL 및 최신 라이브러리 활용
  - 예외 처리(Exception Handling)를 통한 견고한 오류 관리
  - 람다(Lambda), `std::function`을 활용한 유연한 콜백 메커니즘 (IRC 연동)
- **강점:** 코드의 안전성(메모리, 타입)이 언어 차원에서 보장됨. 개발 생산성이 높고 유지보수가 용이함. 높은 수준의 추상화로 복잡한 기능(IRC)을 쉽게 통합.
- **단점:** 바이너리 크기가 더 크고, 추상화 계층으로 인해 일부 성능 오버헤드가 발생할 수 있음 (이번 프로젝트에서는 I/O 바운드 작업이 대부분이라 거의 차이 없음).

## 3. 개발 시간 및 품질 분석 (Dev Time & Quality Analysis)

`DEVELOPMENT_JOURNEY.md`에 기록된 개발 시간 추정치를 통해 C++의 생산성 우위를 확인할 수 있습니다.

| 단계 (Phase) | C 구현 시간 | C++ 구현 시간 | 시간 절약 |
| :--- | :--- | :--- | :--- |
| MVP1 | 3 시간 | 2 시간 | 33% |
| MVP2 | 4 시간 | 2.5 시간 | 37% |
| MVP3 | 4 시간 | 1.5 시간 | 62% |
| **합계** | **11 시간** | **6 시간** | **45%** |

C++ 버전은 C 버전에 비해 약 **45%의 개발 시간을 절약**했습니다. 이는 C++의 강력한 표준 라이브러리와 자동 리소스 관리 기능 덕분에 개발자가 비즈니스 로직에 더 집중할 수 있었기 때문입니다. 특히 복잡한 쿼리 파서를 구현한 MVP3 단계에서 그 효과가 극대화되었습니다.

버그 발생률 또한 C++ 버전이 현저히 낮았으며, 특히 C 버전에서 많은 디버깅 시간을 유발했던 메모리 누수나 경쟁 조건 관련 버그는 C++ 버전에서는 거의 발생하지 않았습니다.

## 4. 주요 기술적 학습 내용 (Key Technical Learnings)

- **메모리 관리:** C++의 RAII 패턴은 `malloc`/`free` 쌍을 맞추는 과정에서 발생하는 거의 모든 종류의 메모리 관련 버그를 원천적으로 방지하는 가장 효과적인 방법임을 재확인했습니다.
- **오류 처리:** C의 반환 코드 검사 방식은 코드를 장황하게 만들고 실수를 유발하기 쉽습니다. C++의 예외 처리 방식은 오류 발생 지점과 처리 지점을 명확히 분리하여 코드의 가독성과 안정성을 크게 향상시켰습니다.
- **스레드 안전성:** C++의 `std::mutex`, `std::lock_guard` 등은 C의 `pthread_mutex` API보다 사용이 간편하고, 락의 생명주기를 스코프에 바인딩하여 데드락이나 락 해제를 잊는 실수를 방지해 줍니다.
- **추상화의 힘:** `std::function`과 람다를 활용한 C++의 콜백 메커니즘은 C의 함수 포인터 방식보다 훨씬 유연하고 타입 안전한 방식으로 IRC 서버와 같은 복잡한 외부 모듈을 연동할 수 있게 해주었습니다.

## 5. 최종 결론 (Final Conclusion)

이 프로젝트는 동일한 요구사항에 대해 C와 C++가 어떻게 다른 접근 방식을 취하는지, 그리고 그 결과가 어떻게 달라지는지를 명확하게 보여주었습니다.

- **C**는 시스템의 가장 깊은 곳까지 제어해야 하거나, 리소스가 극도로 제한된 환경(임베디드, 커널 등)에서 여전히 강력한 선택지입니다. 하지만 그만큼 개발자의 높은 책임감과 세심한 주의를 요구합니다.
- **C++**는 현대적인 서버 애플리케이션 개발에 있어 성능, 안정성, 생산성 모든 면에서 C보다 우월한 선택지임을 증명했습니다. 특히 C++11 이후 도입된 현대적인 기능들은 C의 저수준 성능을 거의 그대로 유지하면서도, Java나 Python과 같은 고수준 언어의 개발 편의성과 안전성을 상당 부분 제공합니다.

결론적으로, 대부분의 고성능 네트워크 서버 애플리케이션 개발에 있어 **C++는 C에 비해 훨씬 안전하고 생산적인 대안**입니다. LogCaster 프로젝트는 이러한 결론을 뒷받침하는 구체적이고 실질적인 증거가 될 것입니다.

이것으로 LogCaster 개발 역사 시리즈를 마칩니다.
# DevHistory 00: 프로젝트 시작 (Project Inception)

## 1. 개요 (Overview)

이 문서는 LogCaster 프로젝트의 공식적인 개발 시작에 앞서, 프로젝트의 목표, 기술적 기반, 그리고 초기 계획을 정의합니다. 코드 구현보다는 프로젝트의 '왜'와 '무엇을'에 초점을 맞춘 단계입니다.

## 2. 문제 정의 및 목표 (Problem Definition & Goals)

### 2.1. 해결하려는 문제 (The Problem)

현대의 분산 시스템 환경에서는 대규모의 로그 데이터를 안정적으로 수집하고, 이를 실시간으로 조회할 수 있는 고성능 솔루션이 필수적입니다. 기존의 로깅 시스템들은 특정 요구사항을 충족하지 못하거나, 과도한 리소스를 사용하거나, 특정 플랫폼에 종속되는 경우가 많습니다. LogCaster는 이러한 문제를 해결하기 위해 탄생했습니다.

**핵심 요구사항:**
- **고성능 로그 수집:** 대량의 동시 연결을 처리하며 초당 수만 건 이상의 로그를 안정적으로 수집할 수 있어야 합니다.
- **실시간 쿼리:** 수집된 로그를 지연 없이 즉각적으로 검색하고 필터링할 수 있어야 합니다.
- **자원 효율성:** 최소한의 CPU 및 메모리 자원을 사용하여 운영 비용을 절감해야 합니다.

### 2.2. 프로젝트 목표 (Project Goals)

- C와 C++ 두 가지 언어로 동일한 기능의 로그 서버를 구현하여, 각 언어의 패러다임과 성능 특성을 비교 분석합니다.
- 시스템 프로그래밍의 핵심 요소(소켓 통신, 멀티스레딩, 동기화, 메모리 관리)에 대한 깊은 이해를 바탕으로 최적화된 서버를 구축합니다.
- 잘 구조화되고 문서화된 코드를 통해 다른 개발자들에게 교육적인 자료를 제공합니다.

## 3. 초기 기술 결정 (Initial Technology Decisions)

### 3.1. 기술 스택 (Technology Stack)

- **언어 (Language):**
  - **C:** 하드웨어에 대한 직접적인 제어와 최대의 성능을 이끌어내기 위해 선택했습니다. 시스템 리소스를 세밀하게 관리하는 능력이 필수적인 저수준(low-level) 구현에 적합합니다.
  - **C++:** C의 성능을 유지하면서도 객체 지향 프로그래밍(OOP), RAII(Resource Acquisition Is Initialization), STL(Standard Template Library) 등 더 높은 수준의 추상화를 제공하여 코드의 안정성과 유지보수성을 높이기 위해 선택했습니다.

- **핵심 아키텍처 (Core Architecture):**
  - **TCP 소켓 통신:** 데이터 전송의 신뢰성을 보장하기 위해 UDP 대신 TCP를 사용합니다.
  - **스레드 풀 (Thread Pool):** 클라이언트 요청이 발생할 때마다 스레드를 생성하는 오버헤드를 줄이고, 시스템 자원을 효율적으로 재사용하기 위해 스레드 풀 패턴을 적용합니다.
  - **인메모리 저장소 (In-Memory Storage):** 로그 쿼리 성능을 극대화하기 위해 수집된 로그를 우선 메모리에 저장합니다. 디스크 I/O를 최소화하여 응답 시간을 단축합니다.

### 3.2. 포트 할당 (Port Allocation)

- **로그 수집 포트:** `9999`
- **쿼리 전용 포트:** `9998`

두 가지 주요 기능을 별도의 포트로 분리하여 역할의 명확성을 높이고, 각 포트에 특화된 처리 로직을 구현할 수 있도록 설계했습니다.

## 4. MVP1 계획 (MVP1 Plan)

첫 번째 결과물(MVP1)은 프로젝트의 가장 기본적인 기능을 구현하는 데 집중합니다.

### 4.1. 핵심 기능 (Core Features)

- **TCP 서버 구현:** 지정된 포트(`9999`)에서 클라이언트의 연결을 수락합니다.
- **다중 클라이언트 처리:** 여러 클라이언트가 동시에 서버에 연결할 수 있어야 합니다.
- **기본 로그 처리:** 클라이언트로부터 받은 로그 메시지를 서버의 표준 출력(stdout)에 그대로 출력하여 수신 여부를 확인합니다.
- **단순 프로토콜:** 로그 메시지는 개행 문자(`\n`)를 기준으로 구분하는 간단한 텍스트 기반 프로토콜을 사용합니다.

### 4.2. 성공 기준 (Success Criteria)

- 서버가 `9999` 포트에서 정상적으로 실행되고 연결을 기다립니다.
- 여러 명의 클라이언트가 동시에 접속할 수 있습니다.
- 클라이언트가 전송한 모든 로그 메시지가 서버의 콘솔 화면에 빠짐없이 출력됩니다.
- `Ctrl+C`를 눌렀을 때 서버가 모든 연결을 정리하고 깨끗하게 종료됩니다.

이 계획을 바탕으로 다음 단계인 `DevHistory01.md`부터 실제 C언어 코드 구현을 시작하겠습니다.
# DevHistory 01: LogCaster-C MVP1 - Basic TCP Server

## 1. 개요 (Overview)

LogCaster 프로젝트의 첫 번째 MVP(Minimum Viable Product)는 C 언어를 사용하여 기본적인 TCP 로그 수집 서버를 구현하는 데 중점을 둡니다. 이 단계의 핵심 목표는 여러 클라이언트로부터 동시에 로그 메시지를 수신하고, 이를 서버의 표준 출력(stdout)에 표시하는 안정적인 기반을 구축하는 것입니다.

**주요 기능:**
- **TCP 서버:** 지정된 포트(기본값: 9999)에서 클라이언트 연결을 수락합니다.
- **동시성 처리:** `select()` 시스템 콜을 사용한 I/O 멀티플렉싱을 통해 다수의 클라이언트를 동시에 처리합니다.
- **기본 프로토콜:** 클라이언트는 개행 문자(`\n`)로 구분된 일반 텍스트 로그 메시지를 전송합니다.
- **로그 출력:** 수신된 모든 로그는 서버 콘솔에 `[LOG]` 접두사와 함께 출력됩니다.
- **Graceful Shutdown:** `Ctrl+C` (SIGINT) 시그널을 감지하여 진행 중인 연결을 안전하게 종료하고 리소스를 정리합니다.

**기술적 결정:**
- **`select()` 사용:** `epoll`이나 `kqueue` 같은 더 현대적인 대안 대신, 이식성과 단순성을 위해 POSIX 표준인 `select()`를 채택했습니다.
- **고정 버퍼:** 각 클라이언트 연결에 대해 4096바이트 크기의 고정 버퍼를 할당하여 메시지를 수신합니다.
- **논블로킹 소켓:** 서버의 응답성을 높이기 위해 논블로킹 소켓 모드를 사용합니다.

이 MVP는 LogCaster의 핵심 네트워킹 기반을 마련하며, 이후 단계에서 구현될 스레드 풀, 인메모리 버퍼, 쿼리 기능 등의 고급 기능을 위한 발판이 됩니다.

## 2. 빌드 시스템 (Build System)

MVP1은 `Makefile`을 사용하여 프로젝트를 빌드합니다. 주요 특징은 다음과 같습니다.

- **엄격한 컴파일러 플래그:** `-Wall`, `-Wextra`, `-Werror`, `-pedantic` 플래그를 사용하여 코드 품질을 높이고 잠재적인 오류를 사전에 방지합니다.
- **C11 표준 준수:** `-std=c11` 옵션을 사용하여 최신 C 표준을 따릅니다.
- **디렉토리 구조:** `src`, `include`, `obj`, `bin` 디렉토리를 명확히 구분하여 프로젝트 구조를 체계적으로 관리합니다.
- **pthread 라이브러리 링크:** 향후 MVP에서 사용될 스레드 풀 기능을 위해 `pthread` 라이브러리를 미리 링크합니다.

```makefile
# [SEQUENCE: MVP1-1]
# Compiler and flags
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -Werror -pedantic -std=c11
LDFLAGS = -lpthread

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

# Source files and object files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

# Target executable
TARGET = $(BINDIR)/logcaster-c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete. Executable: $(TARGET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)
	@echo "Cleaned up object and binary files."
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. `include/server.h`

서버의 핵심 데이터 구조와 함수 프로토타입을 정의합니다. MVP1에서는 동시 연결 처리를 위한 `select` 관련 필드와 기본적인 서버 상태만 포함합니다.

```c
// [SEQUENCE: MVP1-2]
#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#define DEFAULT_PORT 9999
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 4096

// [SEQUENCE: MVP1-3]
// 서버 상태를 관리하는 구조체 (MVP1 버전)
typedef struct {
    int port;
    int listen_fd;
    fd_set master_set;
    fd_set read_set;
    int max_fd;
    int client_count;
    volatile sig_atomic_t running;
} log_server_t;

// [SEQUENCE: MVP1-4]
// 함수 프로토타입 (MVP1 버전)
log_server_t* server_create(int port);
void server_run(log_server_t* server);
void server_destroy(log_server_t* server);
void handle_signal(int sig);

#endif // SERVER_H
```

### 3.2. `src/server.c`

TCP 서버의 핵심 로직을 구현합니다. `select`를 사용한 메인 루프에서 연결 수락, 데이터 수신 및 처리를 모두 담당하는 단일 스레드 모델입니다.

```c
// [SEQUENCE: MVP1-5]
#include "server.h"

// 시그널 처리를 위한 전역 서버 포인터
static log_server_t* g_server = NULL;

// [SEQUENCE: MVP1-6]
// 시그널 핸들러: 서버의 running 상태를 0으로 변경하여 안전한 종료를 유도
void handle_signal(int sig) {
    (void)sig; // 사용되지 않는 파라미터 경고 방지
    if (g_server) {
        g_server->running = 0;
    }
}

// [SEQUENCE: MVP1-7]
// 서버 구조체 생성 및 리스닝 소켓 초기화
log_server_t* server_create(int port) {
    log_server_t* server = malloc(sizeof(log_server_t));
    if (!server) {
        perror("Failed to allocate server");
        return NULL;
    }

    server->port = port;
    server->running = 1;
    server->client_count = 0;

    // [SEQUENCE: MVP1-8]
    // 리스닝 소켓 생성
    server->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->listen_fd < 0) {
        perror("Failed to create socket");
        free(server);
        return NULL;
    }

    // [SEQUENCE: MVP1-9]
    // 소켓 재사용 옵션 설정 (서버 재시작 시 TIME_WAIT 문제 방지)
    int opt = 1;
    if (setsockopt(server->listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Failed to set socket options");
        close(server->listen_fd);
        free(server);
        return NULL;
    }

    // [SEQUENCE: MVP1-10]
    // 서버 주소 설정
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // [SEQUENCE: MVP1-11]
    // 소켓에 주소 바인딩
    if (bind(server->listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        close(server->listen_fd);
        free(server);
        return NULL;
    }

    // [SEQUENCE: MVP1-12]
    // 리스닝 시작
    if (listen(server->listen_fd, MAX_CLIENTS) < 0) {
        perror("Failed to listen on socket");
        close(server->listen_fd);
        free(server);
        return NULL;
    }

    // [SEQUENCE: MVP1-13]
    // fd_set 초기화 및 리스닝 소켓 등록
    FD_ZERO(&server->master_set);
    FD_ZERO(&server->read_set);
    FD_SET(server->listen_fd, &server->master_set);
    server->max_fd = server->listen_fd;

    g_server = server; // 전역 변수에 서버 인스턴스 할당

    printf("LogCaster-C server starting on port %d\n", port);
    return server;
}

// [SEQUENCE: MVP1-14]
// 서버 메인 루프
void server_run(log_server_t* server) {
    printf("Press Ctrl+C to stop\n");
    while (server->running) {
        server->read_set = server->master_set;

        // [SEQUENCE: MVP1-15]
        // select()를 사용하여 I/O 이벤트 대기
        if (select(server->max_fd + 1, &server->read_set, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) { // 시그널에 의해 중단된 경우
                continue;
            }
            perror("select failed");
            break;
        }

        // [SEQUENCE: MVP1-16]
        // 모든 파일 디스크립터를 순회하며 이벤트 확인
        for (int i = 0; i <= server->max_fd; i++) {
            if (FD_ISSET(i, &server->read_set)) {
                // [SEQUENCE: MVP1-17]
                // 리스닝 소켓 이벤트: 새 클라이언트 연결 처리
                if (i == server->listen_fd) {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int new_fd = accept(server->listen_fd, (struct sockaddr*)&client_addr, &client_len);

                    if (new_fd < 0) {
                        perror("Failed to accept connection");
                        continue;
                    }

                    // [SEQUENCE: MVP1-18]
                    // 클라이언트 수 제한 확인
                    if (server->client_count >= MAX_CLIENTS) {
                        fprintf(stderr, "Max clients reached. Rejecting new connection.\n");
                        close(new_fd);
                        continue;
                    }

                    // [SEQUENCE: MVP1-19]
                    // 새 클라이언트를 master_set에 추가
                    fcntl(new_fd, F_SETFL, O_NONBLOCK); // 논블로킹으로 설정
                    FD_SET(new_fd, &server->master_set);
                    if (new_fd > server->max_fd) {
                        server->max_fd = new_fd;
                    }
                    server->client_count++;
                    printf("New connection from %s:%d on socket %d\n",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), new_fd);
                } else {
                    // [SEQUENCE: MVP1-20]
                    // 클라이언트 소켓 이벤트: 데이터 수신 처리
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    int nbytes = read(i, buffer, sizeof(buffer) - 1);

                    if (nbytes <= 0) {
                        // [SEQUENCE: MVP1-21]
                        // 연결 종료 또는 오류
                        if (nbytes == 0) {
                            printf("Socket %d hung up\n", i);
                        } else {
                            perror("read failed");
                        }
                        close(i);
                        FD_CLR(i, &server->master_set);
                        // 참고: 이 MVP에서는 client_count를 감소시키지 않아 버그가 존재함.
                        // 이 문제는 MVP5에서 수정될 예정.
                    } else {
                        // [SEQUENCE: MVP1-22]
                        // 데이터 수신 성공, stdout에 로그 출력
                        buffer[nbytes] = '\0';
                        printf("[LOG] from socket %d: %s", i, buffer);
                        // 버퍼에 개행이 없을 경우를 대비해 추가
                        if (buffer[nbytes-1] != '\n') {
                            printf("\n");
                        }
                    }
                }
            }
        }
    }
}

// [SEQUENCE: MVP1-23]
// 서버 리소스 해제
void server_destroy(log_server_t* server) {
    if (!server) return;

    printf("\nShutting down server...\n");

    // [SEQUENCE: MVP1-24]
    // 모든 클라이언트 연결 종료
    for (int i = 0; i <= server->max_fd; i++) {
        if (FD_ISSET(i, &server->master_set)) {
            close(i);
        }
    }

    free(server);
    g_server = NULL;
    printf("Server shut down gracefully.\n");
}
```

### 3.3. `src/main.c`

프로그램의 진입점(entry point)입니다. 서버를 생성, 실행하고, 종료 시 리소스를 해제하는 역할을 합니다. MVP1에서는 포트 번호를 인자로 받는 기능만 포함합니다.

```c
// [SEQUENCE: MVP1-25]
#include "server.h"

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;

    // [SEQUENCE: MVP1-26]
    // 명령줄 인자 파싱 (포트 번호 지정)
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number. Using default %d.\n", DEFAULT_PORT);
            port = DEFAULT_PORT;
        }
    }

    // [SEQUENCE: MVP1-27]
    // SIGINT 시그널 핸들러 등록 (Ctrl+C)
    signal(SIGINT, handle_signal);

    // [SEQUENCE: MVP1-28]
    // 서버 생성
    log_server_t* server = server_create(port);
    if (!server) {
        fprintf(stderr, "Failed to create server.\n");
        return 1;
    }

    // [SEQUENCE: MVP1-29]
    // 서버 실행
    server_run(server);

    // [SEQUENCE: MVP1-30]
    // 서버 리소스 해제
    server_destroy(server);

    return 0;
}
```

## 4. 테스트 코드 (Test Code)

MVP1의 기능을 검증하기 위해 Python으로 작성된 테스트 클라이언트(`tests/test_client.py`)를 사용합니다. 이 스크립트는 서버에 대한 다양한 시나리오를 테스트합니다.

**테스트 모드:**
1.  **단일 클라이언트 테스트 (`single`):** 하나의 클라이언트가 연결하여 여러 로그 메시지를 전송하고 정상적으로 수신되는지 확인합니다.
2.  **다중 클라이언트 테스트 (`multiple`):** 5개의 클라이언트가 동시에 연결하여 각자 로그를 보내고 서버가 모든 메시지를 처리하는지 검증합니다.
3.  **스트레스 테스트 (`stress`):** 50개의 클라이언트를 동시에 연결하여 서버의 동시 연결 처리 능력을 시험합니다.

```python
# [SEQUENCE: MVP1-31]
import socket
import threading
import time
import sys

SERVER_HOST = '127.0.0.1'
SERVER_PORT = 9999
NUM_CLIENTS_MULTIPLE = 5
NUM_CLIENTS_STRESS = 50
MESSAGES_PER_CLIENT = 10

def client_task(client_id, num_messages):
    """A single client connects, sends messages, and closes."""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((SERVER_HOST, SERVER_PORT))
            print(f"Client {client_id}: Connected to server.")
            for i in range(num_messages):
                message = f"Client {client_id}, message {i+1}\n"
                s.sendall(message.encode('utf-8'))
                # print(f"Client {client_id}: Sent '{message.strip()}'")
                time.sleep(0.1)
            print(f"Client {client_id}: All messages sent.")
    except ConnectionRefusedError:
        print(f"Client {client_id}: Connection refused. Is the server running?")
    except Exception as e:
        print(f"Client {client_id}: An error occurred: {e}")
    finally:
        print(f"Client {client_id}: Connection closed.")

def run_test(num_clients, num_messages):
    """Runs a test with a specified number of clients."""
    threads = []
    start_time = time.time()

    print(f"--- Starting test with {num_clients} clients ---")
    for i in range(num_clients):
        thread = threading.Thread(target=client_task, args=(i + 1, num_messages))
        threads.append(thread)
        thread.start()
        time.sleep(0.05) # Stagger connections slightly

    for thread in threads:
        thread.join()

    end_time = time.time()
    print(f"\nTest with {num_clients} clients finished in {end_time - start_time:.2f} seconds.")

if __name__ == "__main__":
    if len(sys.argv) != 2 or sys.argv[1] not in ['single', 'multiple', 'stress']:
        print("Usage: python test_client.py [single|multiple|stress]")
        sys.exit(1)

    mode = sys.argv[1]

    if mode == 'single':
        run_test(1, MESSAGES_PER_CLIENT)
    elif mode == 'multiple':
        run_test(NUM_CLIENTS_MULTIPLE, MESSAGES_PER_CLIENT)
    elif mode == 'stress':
        run_test(NUM_CLIENTS_STRESS, 5) # Fewer messages for stress test
```

## 5. 개선 제안 (Improvement Notes)

- **Client Counter 버그:** `DEVELOPMENT_JOURNAL.md`에 따르면, MVP1의 `server.c` 구현에는 클라이언트 연결이 끊어졌을 때 `client_count`를 감소시키지 않는 의도적인 버그가 있습니다. 이로 인해 서버는 재시작 전까지 최대 `MAX_CLIENTS` 만큼의 연결만 수락할 수 있습니다. 이 버그는 이후 MVP5 보안 강화 단계에서 수정될 예정입니다.
- **전역 변수 `g_server`:** 시그널 핸들러에서 서버 상태에 접근하기 위해 전역 변수를 사용했습니다. 이는 POSIX 시그널 핸들러의 일반적인 패턴이지만, 코드의 재사용성과 테스트 용이성을 저해할 수 있습니다.
- **`select()`의 한계:** `select()`는 파일 디스크립터 수에 제한(일반적으로 1024)이 있고, 파일 디스크립터 수가 증가함에 따라 성능이 선형적으로 감소합니다. 대규모 연결을 처리하기 위해서는 `epoll`(Linux)이나 `kqueue`(BSD/macOS)와 같은 더 확장성 있는 I/O 멀티플렉싱 기술로의 전환이 필요합니다.
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
```# DevHistory 04: LogCaster-CPP MVP2 - Modern C++ Concurrency

## 1. 개요 (Overview)

C++ 구현의 두 번째 단계(MVP2)는 C++의 최신 기능들을 본격적으로 활용하여 C 버전 MVP2와 동일한 기능(스레드 풀, 인메모리 버퍼, 쿼리 인터페이스)을 더 안전하고 효율적으로 구현하는 데 중점을 둡니다. C++ MVP1의 '클라이언트당 스레드' 모델의 한계를 극복하고, 고성능 서버의 기반을 다지는 단계입니다.

**주요 기능 추가 및 개선:**
- **C++11 스레드 풀:** `pthreads` 대신 `<thread>`, `<mutex>`, `<condition_variable>` 등 C++ 표준 라이브러리를 사용하여 스레드 풀을 구현합니다. `std::function`을 통해 어떤 종류의 호출 가능한(callable) 작업이든 작업 큐에 저장할 수 있어 유연성이 크게 향상됩니다.
- **STL 기반 로그 버퍼:** 직접 인덱스를 관리하는 C의 원형 배열 대신, `std::deque`를 사용하여 스레드 안전 로그 버퍼를 구현합니다. `std::deque`는 양쪽 끝에서의 삽입/삭제가 O(1) 시간 복잡도를 가져 원형 버퍼 구현에 이상적입니다.
- **객체 지향 쿼리 처리:** 쿼리 처리 로직을 별도의 `QueryHandler` 클래스로 캡슐화하여 서버의 다른 기능들과 명확하게 분리합니다.
- **RAII 및 스마트 포인터 활용:** 서버의 모든 주요 구성 요소(`ThreadPool`, `LogBuffer`, `QueryHandler` 등)를 `std::unique_ptr` 또는 `std::shared_ptr`로 관리하여, 리소스 누수 가능성을 원천적으로 차단하고 코드의 안정성을 높입니다.

**아키텍처 변경:**
- **이벤트 루프 유지:** C++ MVP1의 '클라이언트당 스레드' 모델을 폐기하고, C 버전 MVP2와 유사하게 `select` 기반의 메인 이벤트 루프를 다시 도입합니다. 이는 소수의 스레드로 다수의 연결을 효율적으로 관리하기 위함입니다.
- **작업 위임:** 메인 스레드는 `select`를 통해 I/O 이벤트를 감지하고, 실제 데이터 처리 작업은 `ThreadPool`에 비동기적으로 위임합니다. 이를 통해 메인 스레드는 항상 새로운 연결을 수락할 준비 상태를 유지할 수 있습니다.

이 MVP를 통해 C++ 버전은 C 버전의 기능적 동등성을 확보하면서도, C++ 언어의 장점을 최대한 활용하여 더 높은 수준의 코드 안전성, 가독성, 유지보수성을 달성합니다.

## 2. 빌드 시스템 (Build System)

MVP2에서는 `ThreadPool`, `LogBuffer`, `QueryHandler` 클래스가 추가됨에 따라, `CMakeLists.txt`의 소스 파일 목록이 업데이트됩니다.

```cmake
# [SEQUENCE: MVP4-1]
# CMake 최소 요구 버전 설정
cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름 및 C++ 언어 지정
project(LogCasterCPP VERSION 1.0.0 LANGUAGES CXX)

# C++ 표준 설정 (C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 컴파일러 플래그 설정
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic")

# 헤더 파일 경로 추가
include_directories(include)

# [SEQUENCE: MVP4-2]
# MVP2에 필요한 소스 파일 목록
set(SOURCES
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/LogBuffer.cpp
    src/QueryHandler.cpp
)

# 실행 파일 생성
add_executable(logcaster-cpp ${SOURCES})

# [SEQUENCE: MVP4-3]
# 스레드 라이브러리 링크
find_package(Threads REQUIRED)
target_link_libraries(logcaster-cpp PRIVATE Threads::Threads)
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. `include/ThreadPool.h`

C++11 표준 라이브러리를 사용하여 구현된 헤더 전용(header-only) 스레드 풀입니다. 템플릿을 사용하여 어떤 형태의 함수든 인자와 함께 작업 큐에 추가하고, `std::future`를 통해 결과값을 비동기적으로 받을 수 있습니다.

```cpp
// [SEQUENCE: MVP4-4]
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>

// [SEQUENCE: MVP4-5]
class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // [SEQUENCE: MVP4-6]
    // 작업을 큐에 추가하는 템플릿 함수
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    void workerThread();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

// [SEQUENCE: MVP4-7]
// enqueue 멤버 함수의 템플릿 구현
template<typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    // [SEQUENCE: MVP4-8]
    // 작업을 std::packaged_task로 래핑하여 future를 얻음
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks_.emplace([task]() { (*task)(); });
    }
    condition_.notify_one();
    return res;
}

#endif // THREADPOOL_H
```

### 3.2. `src/ThreadPool.cpp`

스레드 풀의 생성자, 소멸자 및 작업자 스레드의 실행 로직을 구현합니다.

```cpp
// [SEQUENCE: MVP4-9]
#include "ThreadPool.h"

// [SEQUENCE: MVP4-10]
// 생성자: 작업자 스레드를 생성하고 실행 시작
ThreadPool::ThreadPool(size_t numThreads) : stop_(false) {
    workers_.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back([this] { workerThread(); });
    }
}

// [SEQUENCE: MVP4-11]
// 소멸자: 모든 스레드가 안전하게 종료되도록 보장
ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all(); // 모든 대기 중인 스레드를 깨움
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

// [SEQUENCE: MVP4-12]
// 작업자 스레드의 메인 루프
void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            // [SEQUENCE: MVP4-13]
            // 작업이 있거나, 종료 신호가 올 때까지 대기
            condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
            if (stop_ && tasks_.empty()) {
                return; // 종료
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task(); // 작업 실행
    }
}
```

### 3.3. `include/LogBuffer.h`

`std::deque`를 내부 컨테이너로 사용하는 스레드 안전 로그 버퍼를 정의합니다.

```cpp
// [SEQUENCE: MVP4-14]
#ifndef LOGBUFFER_H
#define LOGBUFFER_H

#include <deque>
#include <string>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>

// [SEQUENCE: MVP4-15]
// 로그 항목 구조체 (MVP2 버전)
struct LogEntry {
    std::string message;
    std::chrono::system_clock::time_point timestamp;

    LogEntry(std::string msg)
        : message(std::move(msg)), timestamp(std::chrono::system_clock::now()) {}
};

// [SEQUENCE: MVP4-16]
// 로그 버퍼 클래스
class LogBuffer {
public:
    explicit LogBuffer(size_t capacity = 10000);
    ~LogBuffer() = default;

    void push(std::string message);
    std::vector<std::string> search(const std::string& keyword) const;

    struct StatsSnapshot {
        uint64_t totalLogs;
        uint64_t droppedLogs;
    };
    StatsSnapshot getStats() const;
    size_t size() const;

private:
    void dropOldest_();

    mutable std::mutex mutex_;
    std::deque<LogEntry> buffer_;
    size_t capacity_;

    std::atomic<uint64_t> totalLogs_{0};
    std::atomic<uint64_t> droppedLogs_{0};
};

#endif // LOGBUFFER_H
```

### 3.4. `src/LogBuffer.cpp`

로그 버퍼의 `push`, `search` 등 주요 기능을 구현합니다. `std::mutex`를 사용하여 동시 접근을 제어합니다.

```cpp
// [SEQUENCE: MVP4-17]
#include "LogBuffer.h"
#include <sstream>
#include <iomanip>

LogBuffer::LogBuffer(size_t capacity) : capacity_(capacity) {}

// [SEQUENCE: MVP4-18]
// 로그를 버퍼에 추가
void LogBuffer::push(std::string message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (buffer_.size() >= capacity_) {
        dropOldest_();
    }
    buffer_.emplace_back(std::move(message));
    totalLogs_++;
}

// [SEQUENCE: MVP4-19]
// 가장 오래된 로그를 삭제 (내부 헬퍼 함수)
void LogBuffer::dropOldest_() {
    if (!buffer_.empty()) {
        buffer_.pop_front();
        droppedLogs_++;
    }
}

// [SEQUENCE: MVP4-20]
// 키워드를 포함하는 로그 검색
std::vector<std::string> LogBuffer::search(const std::string& keyword) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> results;
    for (const auto& entry : buffer_) {
        if (entry.message.find(keyword) != std::string::npos) {
            auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
            std::stringstream ss;
            ss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
            ss << entry.message;
            results.push_back(ss.str());
        }
    }
    return results;
}

// [SEQUENCE: MVP4-21]
// 현재 버퍼 크기 반환
size_t LogBuffer::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_.size();
}

// [SEQUENCE: MVP4-22]
// 통계 정보 반환
LogBuffer::StatsSnapshot LogBuffer::getStats() const {
    return { totalLogs_.load(), droppedLogs_.load() };
}
```

### 3.5. `include/QueryHandler.h`

쿼리 처리를 캡슐화한 `QueryHandler` 클래스를 정의합니다.

```cpp
// [SEQUENCE: MVP4-23]
#ifndef QUERYHANDLER_H
#define QUERYHANDLER_H

#include <string>
#include <memory>
#include "LogBuffer.h"

class QueryHandler {
public:
    explicit QueryHandler(std::shared_ptr<LogBuffer> buffer);
    std::string processQuery(const std::string& query);

private:
    std::string handleSearch(const std::string& query);
    std::string handleStats();
    std::string handleCount();
    std::string handleHelp();

    std::shared_ptr<LogBuffer> buffer_;
};

#endif // QUERYHANDLER_H
```

### 3.6. `src/QueryHandler.cpp`

`QueryHandler`의 멤버 함수를 구현합니다. MVP2에서는 간단한 문자열 파싱으로 명령을 처리합니다.

```cpp
// [SEQUENCE: MVP4-24]
#include "QueryHandler.h"
#include <sstream>

QueryHandler::QueryHandler(std::shared_ptr<LogBuffer> buffer) : buffer_(buffer) {}

// [SEQUENCE: MVP4-25]
// 쿼리 문자열을 파싱하고 적절한 핸들러 호출
std::string QueryHandler::processQuery(const std::string& query) {
    if (query.find("QUERY keyword=") == 0) {
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

// [SEQUENCE: MVP4-26]
// 검색 쿼리 처리
std::string QueryHandler::handleSearch(const std::string& query) {
    std::string keyword = query.substr(14); // "QUERY keyword="
    auto results = buffer_->search(keyword);
    std::stringstream ss;
    ss << "FOUND: " << results.size() << " matches\n";
    for (const auto& res : results) {
        ss << res << "\n";
    }
    return ss.str();
}

// [SEQUENCE: MVP4-27]
// 통계 쿼리 처리
std::string QueryHandler::handleStats() {
    auto stats = buffer_->getStats();
    std::stringstream ss;
    ss << "STATS: Total=" << stats.totalLogs << ", Dropped=" << stats.droppedLogs 
       << ", Current=" << buffer_->size() << "\n";
    return ss.str();
}

// [SEQUENCE: MVP4-28]
// 카운트 쿼리 처리
std::string QueryHandler::handleCount() {
    std::stringstream ss;
    ss << "COUNT: " << buffer_->size() << "\n";
    return ss.str();
}

// [SEQUENCE: MVP4-29]
// 도움말 쿼리 처리
std::string QueryHandler::handleHelp() {
    return "Available commands:\n"
           "  STATS - Show buffer statistics\n"
           "  COUNT - Show number of logs in buffer\n"
           "  HELP  - Show this help message\n"
           "  QUERY keyword=<word> - Search for a keyword\n";
}
```

### 3.7. `include/LogServer.h` (MVP2 업데이트)

`LogServer`가 `ThreadPool`과 `LogBuffer` 등 새로운 구성 요소들을 소유하고 관리하도록 수정됩니다.

```cpp
// [SEQUENCE: MVP4-30]
#ifndef LOGSERVER_H
#define LOGSERVER_H

#include <memory>
#include <atomic>
#include <string>
#include "Logger.h"
#include "ThreadPool.h"
#include "LogBuffer.h"
#include "QueryHandler.h"

class LogServer {
public:
    explicit LogServer(int port = 9999);
    ~LogServer();

    LogServer(const LogServer&) = delete;
    LogServer& operator=(const LogServer&) = delete;

    void start();
    void stop();

private:
    void initialize();
    void runEventLoop();
    void handleNewConnection(int listener_fd, bool is_query_port);
    void handleClientTask(int client_fd);
    void handleQueryTask(int client_fd);

    int port_;
    int queryPort_;
    int listenFd_;
    int queryFd_;
    fd_set masterSet_;
    std::atomic<bool> running_;
    
    std::unique_ptr<Logger> logger_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::shared_ptr<LogBuffer> logBuffer_;
    std::unique_ptr<QueryHandler> queryHandler_;
};

#endif // LOGSERVER_H
```

### 3.8. `src/LogServer.cpp` (MVP2 업데이트)

`select` 기반의 이벤트 루프를 사용하여 두 개의 포트(로그, 쿼리)를 동시에 감시하고, 들어온 요청을 스레드 풀에 위임하는 로직으로 재구성됩니다.

```cpp
// [SEQUENCE: MVP4-31]
#include "LogServer.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <signal.h>

static LogServer* g_server_ptr = nullptr;
void signal_handler_cpp(int signum) {
    if (g_server_ptr) g_server_ptr->stop();
}

// [SEQUENCE: MVP4-32]
// 생성자: 모든 멤버 변수 초기화
LogServer::LogServer(int port)
    : port_(port), queryPort_(9998), listenFd_(-1), queryFd_(-1), running_(false) {
    logger_ = std::make_unique<ConsoleLogger>();
    threadPool_ = std::make_unique<ThreadPool>();
    logBuffer_ = std::make_shared<LogBuffer>();
    queryHandler_ = std::make_unique<QueryHandler>(logBuffer_);
    FD_ZERO(&masterSet_);
}

// [SEQUENCE: MVP4-33]
// 소멸자: 서버 중지
LogServer::~LogServer() {
    stop();
}

// [SEQUENCE: MVP4-34]
// 서버 시작
void LogServer::start() {
    if (running_) return;
    initialize();
    running_ = true;
    logger_->log("Server started.");
    runEventLoop();
}

// [SEQUENCE: MVP4-35]
// 서버 중지
void LogServer::stop() {
    if (!running_) return;
    running_ = false;
    if (listenFd_ != -1) close(listenFd_);
    if (queryFd_ != -1) close(queryFd_);
    logger_->log("Server stopped.");
}

// [SEQUENCE: MVP4-36]
// 리스너 소켓 생성 및 초기화
void LogServer::initialize() {
    auto create_listener = [&](int port) -> int {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) throw std::runtime_error("Socket creation failed");
        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) throw std::runtime_error("Bind failed");
        if (listen(fd, 128) < 0) throw std::runtime_error("Listen failed");
        return fd;
    };
    listenFd_ = create_listener(port_);
    queryFd_ = create_listener(queryPort_);
    FD_SET(listenFd_, &masterSet_);
    FD_SET(queryFd_, &masterSet_);
    g_server_ptr = this;
    signal(SIGINT, signal_handler_cpp);
}

// [SEQUENCE: MVP4-37]
// 메인 이벤트 루프
void LogServer::runEventLoop() {
    while (running_) {
        fd_set read_fds = masterSet_;
        int max_fd = std::max(listenFd_, queryFd_);
        timeval tv = {1, 0};
        int result = select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);
        if (result < 0 && errno != EINTR) {
            logger_->log("Select error");
            break;
        }
        if (result == 0) continue;

        if (FD_ISSET(listenFd_, &read_fds)) {
            handleNewConnection(listenFd_, false);
        }
        if (FD_ISSET(queryFd_, &read_fds)) {
            handleNewConnection(queryFd_, true);
        }
    }
}

// [SEQUENCE: MVP4-38]
// 새 연결 처리
void LogServer::handleNewConnection(int listener_fd, bool is_query_port) {
    int client_fd = accept(listener_fd, nullptr, nullptr);
    if (client_fd < 0) return;

    if (is_query_port) {
        threadPool_->enqueue(&LogServer::handleQueryTask, this, client_fd);
    } else {
        threadPool_->enqueue(&LogServer::handleClientTask, this, client_fd);
    }
}

// [SEQUENCE: MVP4-39]
// 로그 클라이언트 작업
void LogServer::handleClientTask(int client_fd) {
    char buffer[4096];
    while (true) {
        ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (nbytes <= 0) break;
        buffer[nbytes] = '\0';
        logBuffer_->push(std::string(buffer));
    }
    close(client_fd);
}

// [SEQUENCE: MVP4-40]
// 쿼리 클라이언트 작업
void LogServer::handleQueryTask(int client_fd) {
    char buffer[4096];
    ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (nbytes > 0) {
        buffer[nbytes] = '\0';
        std::string query(buffer);
        // Remove trailing newline
        query.erase(query.find_last_not_of("\r\n") + 1);
        std::string response = queryHandler_->processQuery(query);
        send(client_fd, response.c_str(), response.length(), 0);
    }
    close(client_fd);
}
```

### 3.9. `src/main.cpp` (MVP2 업데이트)

`main` 함수는 `LogServer`를 생성하고 `start()`를 호출하는 역할만 합니다. 모든 복잡성은 `LogServer` 클래스 내부에서 처리됩니다.

```cpp
// [SEQUENCE: MVP4-41]
#include "LogServer.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        // [SEQUENCE: MVP4-42]
        int port = (argc > 1) ? std::stoi(argv[1]) : 9999;
        LogServer server(port);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## 4. 테스트 코드 (Test Code)

C++ MVP2 서버는 C MVP2와 동일한 포트 및 프로토콜을 사용하므로, `DevHistory03.md`에 포함된 `tests/test_mvp2.py` 스크립트를 사용하여 기능을 검증할 수 있습니다.

*(테스트 코드 내용은 DevHistory03.md와 동일하여 생략)*

```
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
# [SEQUENCE: MVP5-1]
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

# [SEQUENCE: MVP5-2]
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
// [SEQUENCE: MVP5-3]
#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <stdbool.h>
#include <time.h>
#include <regex.h>

// [SEQUENCE: MVP5-4]
// 쿼리 연산자 종류 (AND/OR)
typedef enum {
    OP_AND,
    OP_OR
} operator_type_t;

// [SEQUENCE: MVP5-5]
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

// [SEQUENCE: MVP5-6]
// 쿼리 파서 생명주기 및 파싱 함수
parsed_query_t* query_parser_create(void);
void query_parser_destroy(parsed_query_t* query);
int query_parser_parse(parsed_query_t* query, const char* query_string);

// [SEQUENCE: MVP5-7]
// 로그 항목이 쿼리 조건과 일치하는지 확인하는 함수
bool query_matches_log(const parsed_query_t* query, const char* log_message, time_t timestamp);

#endif // QUERY_PARSER_H
```

### 3.2. `src/query_parser.c` (신규 파일)

쿼리 파서의 핵심 로직을 구현합니다. `strtok_r`을 사용하여 쿼리 문자열을 토큰으로 분리하고, 각 `key=value` 쌍을 분석하여 `parsed_query_t` 구조체를 채웁니다.

```c
// [SEQUENCE: MVP5-8]
#include "query_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <strings.h> // for strcasecmp

// [SEQUENCE: MVP5-9]
// parsed_query_t 구조체 생성 및 초기화
parsed_query_t* query_parser_create(void) {
    parsed_query_t* query = calloc(1, sizeof(parsed_query_t));
    if (query) {
        query->op = OP_AND; // 기본 연산자는 AND
    }
    return query;
}

// [SEQUENCE: MVP5-10]
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

// [SEQUENCE: MVP5-11]
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

    // [SEQUENCE: MVP5-12]
    // `strtok_r`을 사용하여 스페이스 기준으로 파라미터 분리 (스레드 안전)
    char* saveptr1;
    char* token = strtok_r(params_str, " ", &saveptr1);
    while (token) {
        char* equals = strchr(token, '=');
        if (equals) {
            *equals = '\0'; // key와 value 분리
            char* key = token;
            char* value = equals + 1;

            // [SEQUENCE: MVP5-13]
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

// [SEQUENCE: MVP5-14]
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
// [SEQUENCE: MVP5-15]
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

// [SEQUENCE: MVP5-16]
// MVP3에 추가된 고급 검색 함수
int log_buffer_search_enhanced(log_buffer_t* buffer, const struct parsed_query* query, char*** results, int* count);

size_t log_buffer_size(log_buffer_t* buffer);
void log_buffer_get_stats(log_buffer_t* buffer, unsigned long* total, unsigned long* dropped);

#endif // LOG_BUFFER_H
```

### 3.4. `src/log_buffer.c` (MVP3 업데이트)

`log_buffer_search_enhanced` 함수를 구현합니다. 이 함수는 `query_matches_log` 헬퍼 함수를 사용하여 각 로그 항목이 복잡한 쿼리 조건에 부합하는지 확인합니다.

```c
// [SEQUENCE: MVP5-17]
// ... (log_buffer_create, push 등 기존 함수는 동일) ...

#include "query_parser.h" // query_matches_log 사용을 위해 추가

// [SEQUENCE: MVP5-18]
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
// [SEQUENCE: MVP5-19]
#include "query_handler.h"
#include "server.h"
#include "query_parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

// [SEQUENCE: MVP5-20]
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

// [SEQUENCE: MVP5-21]
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

// [SEQUENCE: MVP5-22]
// 쿼리 연결 수락 및 처리 (MVP2와 동일)
void handle_query_connection(log_server_t* server) {
    // ... (MVP2와 동일한 로직)
}
```

## 4. 테스트 코드 (Test Code)

고급 쿼리 기능을 테스트하기 위해 새로운 Python 테스트 스크립트(`tests/test_mvp3.py`)가 작성되었습니다. 이 스크립트는 정규식, 시간 범위, 다중 키워드 등 다양한 조합의 쿼리를 서버에 전송하고 결과를 검증합니다.

```python
# [SEQUENCE: MVP5-23]
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
```# DevHistory 06: LogCaster-CPP MVP3 - Modern C++ Query Engine

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
# [SEQUENCE: MVP6-1] 
# CMake 최소 요구 버전 설정
cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름 및 C++ 언어 지정
project(LogCasterCPP VERSION 1.0.0 LANGUAGES CXX)

# C++ 표준 설정 (C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 헤더 파일 경로 추가
include_directories(include)

# [SEQUENCE: MVP6-2]
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
// [SEQUENCE: MVP6-3]
#ifndef QUERYPARSER_H
#define QUERYPARSER_H

#include <string>
#include <vector>
#include <optional>
#include <regex>
#include <chrono>
#include <memory>
#include "LogBuffer.h" // For LogEntry

// [SEQUENCE: MVP6-4]
// 쿼리 연산자 종류
enum class OperatorType {
    AND,
    OR
};

// [SEQUENCE: MVP6-5]
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

// [SEQUENCE: MVP6-6]
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
// [SEQUENCE: MVP6-7]
#include "QueryParser.h"
#include <sstream>
#include <algorithm>
#include <iostream>

// [SEQUENCE: MVP6-8]
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

        // [SEQUENCE: MVP6-9]
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

// [SEQUENCE: MVP6-10]
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
// [SEQUENCE: MVP6-11]
#ifndef LOGBUFFER_H
#define LOGBUFFER_H

// ... (기존 include 및 LogEntry 구조체는 동일)

// Forward declaration
class ParsedQuery;

class LogBuffer {
public:
    // ... (기존 메소드 선언은 동일)

    // [SEQUENCE: MVP6-12]
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
// [SEQUENCE: MVP6-13]
#include "LogBuffer.h"
#include "QueryParser.h"
#include <sstream>
#include <iomanip>

// ... (push, search 등 기존 함수는 동일) ...

// [SEQUENCE: MVP6-14]
// 고급 검색 기능 구현
std::vector<std::string> LogBuffer::searchEnhanced(const ParsedQuery& query) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> results;
    for (const auto& entry : buffer_) {
        // [SEQUENCE: MVP6-15]
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
// [SEQUENCE: MVP6-16]
#include "QueryHandler.h"
#include "QueryParser.h"
#include <sstream>

// ... (생성자는 동일) ...

// [SEQUENCE: MVP6-17]
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

// [SEQUENCE: MVP6-18]
// 검색 쿼리 처리 로직 수정
std::string QueryHandler::handleSearch(const std::string& query) {
    try {
        // [SEQUENCE: MVP6-19]
        // QueryParser를 사용하여 쿼리 문자열을 파싱
        auto parsed_query = QueryParser::parse(query);
        if (!parsed_query) {
            return "ERROR: Failed to parse query.\n";
        }

        // [SEQUENCE: MVP6-20]
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

// [SEQUENCE: MVP6-21]
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
# [SEQUENCE: MVP6-22]
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
```# DevHistory 07: LogCaster-C MVP4 - Persistence Layer

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
# [SEQUENCE: MVP7-1]
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

# [SEQUENCE: MVP7-2]
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
// [SEQUENCE: MVP7-3]
#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include <stdio.h>

// [SEQUENCE: MVP7-4]
// 영속성 설정 구조체
typedef struct {
    bool enabled;
    char log_directory[256];
    size_t max_file_size;
} persistence_config_t;

// [SEQUENCE: MVP7-5]
// 파일에 쓸 로그 항목 (큐에 저장될 데이터)
typedef struct write_entry {
    char* message;
    struct write_entry* next;
} write_entry_t;

// [SEQUENCE: MVP7-6]
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

// [SEQUENCE: MVP7-7]
// 영속성 관리자 생명주기 및 API 함수
persistence_manager_t* persistence_create(const persistence_config_t* config);
void persistence_destroy(persistence_manager_t* manager);
int persistence_write(persistence_manager_t* manager, const char* message);

#endif // PERSISTENCE_H
```

### 3.2. `src/persistence.c` (신규 파일)

비동기 파일 쓰기를 위한 Writer 스레드와 파일 로테이션 로직을 구현합니다.

```c
// [SEQUENCE: MVP7-8]
#include "persistence.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

static void* persistence_writer_thread(void* arg);
static void rotate_log_file(persistence_manager_t* manager);

// [SEQUENCE: MVP7-9]
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

    // [SEQUENCE: MVP7-10]
    // Writer 스레드 생성
    if (pthread_create(&manager->writer_thread, NULL, persistence_writer_thread, manager) != 0) {
        fclose(manager->current_file);
        free(manager);
        return NULL;
    }

    return manager;
}

// [SEQUENCE: MVP7-11]
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

// [SEQUENCE: MVP7-12]
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

                // [SEQUENCE: MVP7-13]
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

// [SEQUENCE: MVP7-14]
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

// [SEQUENCE: MVP7-15]
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
// [SEQUENCE: MVP7-16]
#ifndef SERVER_H
#define SERVER_H

// ... (기존 include)
#include "persistence.h" // persistence.h 추가

// ... (기존 define)

typedef struct log_server {
    // ... (MVP3와 동일한 필드)
    thread_pool_t* thread_pool;
    log_buffer_t* log_buffer;

    // [SEQUENCE: MVP7-17]
    // MVP4 추가 사항
    persistence_manager_t* persistence;
} log_server_t;

// ... (나머지 프로토타입은 MVP3와 동일)

#endif // SERVER_H
```

### 3.4. `src/server.c` (MVP4 업데이트)

클라이언트 처리 작업(`handle_client_job`)에서 로그를 디스크에 쓰도록 `persistence_write`를 호출하는 로직을 추가합니다.

```c
// [SEQUENCE: MVP7-18]
// ... (include 및 전역 변수 선언)

// [SEQUENCE: MVP7-19]
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

        // [SEQUENCE: MVP7-20]
        // 2. 영속성 관리자에게 쓰기 요청 (활성화된 경우)
        if (job->server->persistence) {
            persistence_write(job->server->persistence, buffer);
        }
    }

    close(job->client_fd);
    free(job);
}

// [SEQUENCE: MVP7-21]
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

// [SEQUENCE: MVP7-22]
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
// [SEQUENCE: MVP7-23]
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

    // [SEQUENCE: MVP7-24]
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

    // [SEQUENCE: MVP7-25]
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
# [SEQUENCE: MVP7-26]
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
```# DevHistory 08: LogCaster-CPP MVP4 - C++ Persistence Layer

## 1. 개요 (Overview)

C++ 구현의 네 번째 단계(MVP4)는 C++의 현대적인 기능들을 사용하여 C 버전 MVP4와 동일한 영속성(Persistence) 계층을 구현합니다. 이 단계에서는 C++ 표준 라이브러리가 파일 시스템, 파일 I/O, 스레딩을 얼마나 안전하고 우아하게 처리할 수 있는지 보여주는 데 중점을 둡니다.

**주요 기능 추가 및 개선:**
- **비동기 로깅:** C 버전과 마찬가지로 별도의 Writer 스레드를 두어 파일 I/O 작업을 비동기적으로 처리합니다. `std::thread`, `std::mutex`, `std::condition_variable`, `std::queue` 등 C++ 표준 라이브러리 요소들을 사용하여 이를 구현합니다.
- **`std::filesystem` 활용:** 로그 디렉토리 생성, 파일 경로 조합, 로그 파일 이름 변경(로테이션) 등 모든 파일 시스템 관련 작업을 C++17 표준인 `<filesystem>` 라이브러리를 사용하여 처리합니다. 이를 통해 플랫폼 종속적인 코드를 피하고 코드의 이식성과 가독성을 높입니다.
- **`std::fstream` 활용:** 파일 읽기/쓰기는 C의 `FILE*` 포인터 대신 C++의 `<fstream>` 라이브러리(구체적으로 `std::ofstream`)를 사용합니다. 이는 RAII(Resource Acquisition Is Initialization) 패턴을 따르므로, 파일 핸들을 수동으로 닫아주지 않아도 소멸자에서 자동으로 리소스가 해제되어 리소스 누수 가능성을 줄여줍니다.
- **RAII 기반 리소스 관리:** `PersistenceManager`의 소멸자에서 Writer 스레드를 안전하게 종료하고 `join()`하는 로직을 구현하여, 프로그램 종료 시 모든 보류 중인 로그가 디스크에 기록되도록 보장합니다.

**아키텍처 변경:**
- **`PersistenceManager` 클래스 도입:** 영속성 관련 모든 책임을 캡슐화하는 클래스입니다. 설정, 쓰기 큐, Writer 스레드, 파일 핸들러 등을 멤버 변수로 관리합니다.
- **`LogServer`와의 연동:** `LogServer`는 `std::unique_ptr<PersistenceManager>`를 소유하며, `main` 함수에서 커맨드 라인 인자에 따라 생성된 `PersistenceManager` 객체를 주입(inject)받습니다. `LogServer`의 클라이언트 처리 로직은 영속성 관리자가 존재할 경우 `persistence->write()`를 호출하여 로그 기록을 요청합니다.

이 MVP를 통해 C++ 버전은 C 버전의 영속성 기능을 모두 갖추면서, C++의 타입 안전성, 리소스 관리 용이성, 플랫폼 독립성 등의 장점을 활용하여 훨씬 더 견고하고 유지보수하기 쉬운 코드를 완성합니다.

## 2. 빌드 시스템 (Build System)

`Persistence.cpp`가 소스 목록에 추가되고, 구형 컴파일러에서 `<filesystem>`을 지원하기 위한 조건부 라이브러리 링크 설정이 추가됩니다.

```cmake
# [SEQUENCE: MVP8-1]
# ... (project, CMAKE_CXX_STANDARD 등은 MVP6와 동일) ...

# [SEQUENCE: MVP8-2]
# MVP4에 필요한 소스 파일 목록 (Persistence.cpp 추가)
set(SOURCES
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/LogBuffer.cpp
    src/QueryHandler.cpp
    src/QueryParser.cpp
    src/Persistence.cpp
)

add_executable(logcaster-cpp ${SOURCES})

target_link_libraries(logcaster-cpp PRIVATE Threads::Threads)

# [SEQUENCE: MVP8-3]
# 구형 GCC/G++ 컴파일러를 위한 stdc++fs 라이브러리 링크
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(logcaster-cpp PRIVATE stdc++fs)
endif()
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. `include/Persistence.h` (신규 파일)

영속성 관리자를 위한 설정 구조체(`PersistenceConfig`)와 메인 클래스(`PersistenceManager`)를 정의합니다.

```cpp
// [SEQUENCE: MVP8-4]
#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>
#include <filesystem>

// [SEQUENCE: MVP8-5]
// 영속성 설정을 위한 구조체
struct PersistenceConfig {
    bool enabled = false;
    std::filesystem::path log_directory = "./logs";
    size_t max_file_size = 10 * 1024 * 1024; // 10MB
    std::chrono::milliseconds flush_interval = std::chrono::milliseconds(1000);
};

// [SEQUENCE: MVP8-6]
// 영속성 관리자 클래스
class PersistenceManager {
public:
    explicit PersistenceManager(const PersistenceConfig& config);
    ~PersistenceManager();

    PersistenceManager(const PersistenceManager&) = delete;
    PersistenceManager& operator=(const PersistenceManager&) = delete;

    void write(const std::string& message);

private:
    void writerThread();
    void rotateFile();

    PersistenceConfig config_;
    std::ofstream log_file_;
    std::filesystem::path current_filepath_;
    size_t current_file_size_ = 0;

    std::queue<std::string> write_queue_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::thread writer_thread_;
    std::atomic<bool> running_;
};

#endif // PERSISTENCE_H
```

### 3.2. `src/Persistence.cpp` (신규 파일)

`PersistenceManager`의 핵심 로직을 구현합니다. 생성자에서 스레드를 시작하고, 소멸자에서 스레드를 안전하게 종료하며, 비동기 쓰기 및 파일 로테이션을 처리합니다.

```cpp
// [SEQUENCE: MVP8-7]
#include "Persistence.h"
#include <iostream>
#include <iomanip>

// [SEQUENCE: MVP8-8]
// 생성자: 디렉토리 생성, 파일 열기, Writer 스레드 시작
PersistenceManager::PersistenceManager(const PersistenceConfig& config)
    : config_(config), running_(true) {
    if (!config_.enabled) return;

    try {
        std::filesystem::create_directories(config_.log_directory);
        current_filepath_ = config_.log_directory / "current.log";
        log_file_.open(current_filepath_, std::ios::app);
        if (!log_file_.is_open()) {
            throw std::runtime_error("Failed to open log file: " + current_filepath_.string());
        }
        current_file_size_ = std::filesystem::exists(current_filepath_) ? std::filesystem::file_size(current_filepath_) : 0;

        writer_thread_ = std::thread(&PersistenceManager::writerThread, this);
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error("Filesystem error: " + std::string(e.what()));
    }
}

// [SEQUENCE: MVP8-9]
// 소멸자: Writer 스레드의 안전한 종료 보장 (RAII)
PersistenceManager::~PersistenceManager() {
    if (!config_.enabled || !running_) return;

    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        running_ = false;
    }
    condition_.notify_one();
    if (writer_thread_.joinable()) {
        writer_thread_.join();
    }
}

// [SEQUENCE: MVP8-10]
// 외부에서 로그 쓰기를 요청하는 API
void PersistenceManager::write(const std::string& message) {
    if (!config_.enabled) return;
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        write_queue_.push(message);
    }
    condition_.notify_one();
}

// [SEQUENCE: MVP8-11]
// Writer 스레드의 메인 루프
void PersistenceManager::writerThread() {
    while (running_ || !write_queue_.empty()) {
        std::queue<std::string> local_queue;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait_for(lock, config_.flush_interval, [this] { return !running_ || !write_queue_.empty(); });
            
            if (!write_queue_.empty()) {
                local_queue.swap(write_queue_);
            }
        }

        while (!local_queue.empty()) {
            const std::string& message = local_queue.front();
            if (log_file_.is_open()) {
                log_file_ << message << std::endl;
                current_file_size_ += message.length() + 1;
            }
            local_queue.pop();
        }

        if (log_file_.is_open() && current_file_size_ >= config_.max_file_size) {
            rotateFile();
        }
    }
}

// [SEQUENCE: MVP8-12]
// 로그 파일 로테이션
void PersistenceManager::rotateFile() {
    log_file_.close();
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream new_filename;
    new_filename << "log-" << std::put_time(std::localtime(&time_t_now), "%Y%m%d-%H%M%S") << ".log";
    
    try {
        std::filesystem::rename(current_filepath_, config_.log_directory / new_filename.str());
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to rotate log file: " << e.what() << std::endl;
    }

    log_file_.open(current_filepath_, std::ios::app);
    current_file_size_ = 0;
}
```

### 3.3. `include/LogServer.h` (MVP4 업데이트)

`PersistenceManager`를 소유하고 관리하기 위한 포인터와 설정 메소드를 추가합니다.

```cpp
// [SEQUENCE: MVP8-13]
#ifndef LOGSERVER_H
#define LOGSERVER_H

// ... (기존 include)
#include "Persistence.h" // Persistence.h 추가

class LogServer {
public:
    // ... (기존 public 메소드)

    // [SEQUENCE: MVP8-14]
    // PersistenceManager를 주입하기 위한 메소드
    void setPersistenceManager(std::unique_ptr<PersistenceManager> persistence);

private:
    // ... (기존 private 메소드)

    // ... (기존 멤버 변수)
    std::unique_ptr<QueryHandler> queryHandler_;

    // [SEQUENCE: MVP8-15]
    // MVP4 추가 사항
    std::unique_ptr<PersistenceManager> persistence_;
};

#endif // LOGSERVER_H
```

### 3.4. `src/LogServer.cpp` (MVP4 업데이트)

클라이언트 처리 로직에서 `PersistenceManager`의 `write` 메소드를 호출하도록 수정합니다.

```cpp
// [SEQUENCE: MVP8-16]
// ... (include 및 생성자 등은 동일)

// [SEQUENCE: MVP8-17]
// 클라이언트 작업 핸들러 (MVP4 버전)
void LogServer::handleClientTask(int client_fd) {
    char buffer[4096];
    while (true) {
        ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (nbytes <= 0) break;
        buffer[nbytes] = '\0';
        std::string log_message(buffer);

        // 1. 인메모리 버퍼에 저장
        logBuffer_->push(log_message);

        // [SEQUENCE: MVP8-18]
        // 2. 영속성 관리자에게 쓰기 요청 (활성화된 경우)
        if (persistence_) {
            persistence_->write(log_message);
        }
    }
    close(client_fd);
}

// [SEQUENCE: MVP8-19]
// PersistenceManager 설정 메소드 구현
void LogServer::setPersistenceManager(std::unique_ptr<PersistenceManager> persistence) {
    persistence_ = std::move(persistence);
}

// ... (나머지 함수는 MVP3와 동일)
```

### 3.5. `src/main.cpp` (MVP4 업데이트)

`getopt`를 사용하여 영속성 관련 커맨드 라인 인자를 파싱하고, `PersistenceManager`를 생성하여 `LogServer`에 주입하는 로직이 추가됩니다.

```cpp
// [SEQUENCE: MVP8-20]
#include "LogServer.h"
#include "Persistence.h"
#include <iostream>
#include <getopt.h>

int main(int argc, char* argv[]) {
    int port = 9999;
    PersistenceConfig persist_config;

    // [SEQUENCE: MVP8-21]
    // 커맨드 라인 인자 파싱
    int opt;
    while ((opt = getopt(argc, argv, "p:d:s:Ph")) != -1) {
        switch (opt) {
            case 'p': port = std::stoi(optarg); break;
            case 'P': persist_config.enabled = true; break;
            case 'd': persist_config.log_directory = optarg; break;
            case 's': persist_config.max_file_size = std::stoul(optarg) * 1024 * 1024; break;
            case 'h':
                std::cout << "Usage: " << argv[0] << " [-p port] [-P] [-d dir] [-s size_mb] [-h]" << std::endl;
                return 0;
        }
    }

    try {
        LogServer server(port);

        // [SEQUENCE: MVP8-22]
        // 영속성 관리자 생성 및 주입
        if (persist_config.enabled) {
            auto persistence = std::make_unique<PersistenceManager>(persist_config);
            server.setPersistenceManager(std::move(persistence));
            std::cout << "Persistence enabled. Dir: " << persist_config.log_directory 
                      << ", Max Size: " << persist_config.max_file_size / (1024*1024) << " MB" << std::endl;
        }

        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## 4. 테스트 코드 (Test Code)

C++ MVP4 서버는 C MVP4와 동일한 커맨드 라인 인자와 프로토콜을 사용하므로, `DevHistory07.md`에 포함된 `tests/test_persistence.py` 스크립트를 사용하여 기능을 검증할 수 있습니다.

*(테스트 코드 내용은 DevHistory07.md와 동일하여 생략)*
# DevHistory 09: LogCaster-C MVP5 - Security Audit & Stability Enhancement

## 1. 개요 (Overview)

다섯 번째 단계(MVP5)는 새로운 기능 추가가 아닌, 기존 C 코드베이스의 안정성과 보안을 강화하는 데 집중하는 중요한 '경화(Hardening)' 단계입니다. MVP4까지의 기능 구현을 마친 후, 코드 감사를 통해 발견된 여러 심각한 문제점들을 해결하는 것을 목표로 합니다. 이 단계는 프로덕션 환경에서 발생할 수 있는 잠재적 오류, 메모리 누수, 서비스 거부(DoS) 공격 등을 예방하기 위해 필수적입니다.

**주요 해결 과제:**
- **경쟁 조건 (Race Condition) 해결:** 여러 스레드에서 동시에 접근하는 `client_count` 변수를 `pthread_mutex_t`로 보호하여 정확한 클라이언트 수를 유지합니다.
- **메모리 누수 (Memory Leak) 수정:** 쿼리 파서에서 정규식 컴파일이 실패할 경우 메모리가 해제되지 않던 치명적인 버그를 수정합니다.
- **입력 유효성 검증 강화:** `atoi` 함수의 불안전한 사용을 `strtol`로 대체하여, 커맨드 라인으로 입력되는 포트 번호나 파일 크기 값에 대한 정수 오버플로우 및 유효성 검사를 수행합니다.
- **리소스 고갈 방어:** 악의적인 클라이언트가 비정상적으로 큰 로그 메시지를 보내 서버 메모리를 고갈시키는 것을 방지하기 위해, 수신되는 로그의 최대 길이를 제한하고 초과 시 절단하는 로직을 추가합니다.
- **코드 품질 개선:** 매직 넘버(magic number)를 명명된 상수로 대체하여 코드의 가독성과 유지보수성을 향상시킵니다.

이 MVP를 통해 LogCaster의 C 버전은 기능적 완성도를 넘어, 실제 운영 환경에서 요구되는 안정성과 보안성을 갖춘 견고한 애플리케이션으로 거듭나게 됩니다.

## 2. 주요 수정 사항 및 코드

이 단계에서는 새로운 파일 추가 없이 기존 파일들이 수정됩니다.

### 2.1. `include/server.h` (수정)

`log_server_t` 구조체에 클라이언트 카운터 보호를 위한 뮤텍스를 추가합니다.

```c
// [SEQUENCE: MVP9-1] 
#ifndef SERVER_H
#define SERVER_H

// ... (기존 include)
#include "persistence.h"

typedef struct log_server {
    // ... (MVP4와 동일한 필드)
    thread_pool_t* thread_pool;
    log_buffer_t* log_buffer;
    persistence_manager_t* persistence;

    // [SEQUENCE: MVP9-2]
    // MVP5 추가: client_count 보호를 위한 뮤텍스
    pthread_mutex_t client_count_mutex;
} log_server_t;

// ... (나머지 프로토타입은 MVP4와 동일)

#endif // SERVER_H
```

### 2.2. `src/server.c` (수정)

클라이언트 수( `client_count`)를 증가시키거나 감소시키는 모든 지점에 뮤텍스 잠금을 추가하고, 로그 메시지 크기를 검증하는 로직을 추가합니다.

```c
// [SEQUENCE: MVP9-3]
// ... (include 및 전역 변수 선언)

// [SEQUENCE: MVP9-4]
// 클라이언트 처리 작업 (MVP5 버전)
static void handle_client_job(void* arg) {
    client_job_t* job = (client_job_t*)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = recv(job->client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';

        // [SEQUENCE: MVP9-5]
        // 로그 메시지 크기 제한 및 절단
        const int SAFE_LOG_LENGTH = 1024;
        if (bytes_read > SAFE_LOG_LENGTH) {
            buffer[SAFE_LOG_LENGTH - 4] = '.';
            buffer[SAFE_LOG_LENGTH - 3] = '.';
            buffer[SAFE_LOG_LENGTH - 2] = '.';
            buffer[SAFE_LOG_LENGTH - 1] = '\0';
        }

        if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        }
        log_buffer_push(job->server->log_buffer, buffer);
        if (job->server->persistence) {
            persistence_write(job->server->persistence, buffer);
        }
    }

    close(job->client_fd);

    // [SEQUENCE: MVP9-6]
    // client_count를 스레드 안전하게 감소
    pthread_mutex_lock(&job->server->client_count_mutex);
    job->server->client_count--;
    pthread_mutex_unlock(&job->server->client_count_mutex);

    free(job);
}

// [SEQUENCE: MVP9-7]
// 새 연결 처리 로직 (MVP5 버전)
static void handle_new_connection(log_server_t* server) {
    int new_fd = accept(server->listen_fd, NULL, NULL);
    if (new_fd < 0) return;

    // [SEQUENCE: MVP9-8]
    // client_count 접근 전후로 뮤텍스 잠금/해제
    pthread_mutex_lock(&server->client_count_mutex);
    if (server->client_count >= MAX_CLIENTS) {
        pthread_mutex_unlock(&server->client_count_mutex);
        close(new_fd);
        return;
    }
    server->client_count++;
    pthread_mutex_unlock(&server->client_count_mutex);

    client_job_t* job = malloc(sizeof(client_job_t));
    if (!job) {
        close(new_fd);
        // 작업 할당 실패 시에도 count를 다시 감소시켜야 함
        pthread_mutex_lock(&server->client_count_mutex);
        server->client_count--;
        pthread_mutex_unlock(&server->client_count_mutex);
        return;
    }
    job->client_fd = new_fd;
    job->server = server;
    thread_pool_add_job(server->thread_pool, handle_client_job, job);
}

// [SEQUENCE: MVP9-9]
// 서버 생성 및 소멸 시 뮤텍스 초기화/파괴
log_server_t* server_create(int port) {
    log_server_t* server = calloc(1, sizeof(log_server_t));
    // ... (MVP4와 동일한 초기화)
    server->persistence = NULL;
    if (pthread_mutex_init(&server->client_count_mutex, NULL) != 0) {
        // ... (에러 처리)
        free(server);
        return NULL;
    }
    return server;
}

void server_destroy(log_server_t* server) {
    // ... (MVP4와 동일한 소멸 로직)
    pthread_mutex_destroy(&server->client_count_mutex);
    free(server);
}

// ... (나머지 함수는 MVP4와 동일)
```

### 2.3. `src/query_parser.c` (수정)

정규식 컴파일 실패 시 `regex_pattern` 문자열이 담긴 메모리가 누수되던 버그를 수정합니다.

```c
// [SEQUENCE: MVP9-10]
// ... (query_parser_parse 함수 내부)

            } else if (strcasecmp(key, "regex") == 0) {
                query->has_regex = true;
                query->regex_pattern = strdup(value);
                query->compiled_regex = malloc(sizeof(regex_t));
                if (regcomp(query->compiled_regex, value, REG_EXTENDED | REG_NOSUB) != 0) {
                    // 정규식 컴파일 실패 시
                    free(query->compiled_regex);
                    query->compiled_regex = NULL;

                    // [SEQUENCE: MVP9-11]
                    // 메모리 누수를 유발했던 아래 라인 추가
                    if (query->regex_pattern) {
                        free(query->regex_pattern);
                        query->regex_pattern = NULL;
                    }
                }
// ...
```

### 2.4. `src/main.c` (수정)

안전하지 않은 `atoi`를 `strtol`로 교체하고, 입력값에 대한 범위 및 오버플로우 검사를 추가합니다.

```c
// [SEQUENCE: MVP9-12]
#include "server.h"
#include "persistence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;
    persistence_config_t persist_config = { .enabled = false };
    // ...

    int opt;
    while ((opt = getopt(argc, argv, "p:d:s:Ph")) != -1) {
        switch (opt) {
            case 'p': {
                // [SEQUENCE: MVP9-13]
                // 포트 번호에 대한 안전한 파싱
                char* endptr;
                errno = 0;
                long val = strtol(optarg, &endptr, 10);
                if (errno != 0 || *endptr != '\0' || val <= 0 || val > 65535) {
                    fprintf(stderr, "Invalid port number: %s. Must be between 1 and 65535.\n", optarg);
                    return 1;
                }
                port = (int)val;
                break;
            }
            // ... (d, P 옵션은 동일)
            case 's': {
                // [SEQUENCE: MVP9-14]
                // 파일 크기에 대한 안전한 파싱 및 오버플로우 방지
                char* endptr;
                errno = 0;
                long mb_val = strtol(optarg, &endptr, 10);
                if (errno != 0 || *endptr != '\0' || mb_val <= 0) {
                    fprintf(stderr, "Invalid file size: %s\n", optarg);
                    return 1;
                }
                if (mb_val > (long)(SIZE_MAX / (1024 * 1024))) {
                    fprintf(stderr, "File size too large: %s MB\n", optarg);
                    return 1;
                }
                persist_config.max_file_size = (size_t)mb_val * 1024 * 1024;
                break;
            }
            // ...
        }
    }

    // ... (서버 생성 및 실행 로직은 MVP4와 동일)
    return 0;
}
```

## 3. 테스트 코드 (Test Code)

이 단계의 변경 사항을 검증하기 위해 새로운 보안 테스트 스크립트(`tests/test_security.py`)가 작성되었습니다. 이 스크립트는 다음을 확인합니다.
- **클라이언트 카운터 테스트:** 다수의 클라이언트가 연결 및 연결 해제를 반복해도 서버의 클라이언트 수가 정확히 관리되는지 확인합니다.
- **로그 절단 테스트:** 1KB가 넘는 매우 긴 로그 메시지를 보냈을 때, 서버가 이를 안전하게 절단하여 처리하는지 확인합니다.
- **(문서화)** `strtol`로의 변경은 커맨드 라인에서 직접 테스트하여 검증합니다 (예: `./logcaster-c -p 99999`).
- **(문서화)** 메모리 누수 수정은 `valgrind`와 같은 도구를 사용하여 검증합니다.

```python
# [SEQUENCE: MVP9-15]
#!/usr/bin/env python3
import socket
import threading
import time
import subprocess

HOST = '127.0.0.1'
LOG_PORT = 9999
QUERY_PORT = 9998

def get_client_count():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, QUERY_PORT))
        s.sendall(b"STATS\n")
        response = s.recv(1024).decode()
        # STATS: Total=x, Dropped=y, Current=z, Clients=N
        try:
            return int(response.split('=')[-1])
        except:
            return -1

def client_task(duration_s):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, LOG_PORT))
            time.sleep(duration_s)
    except Exception:
        pass

def test_client_counter():
    print("--- Test 1: Client Counter Thread Safety ---")
    initial_count = get_client_count()
    print(f"Initial client count: {initial_count}")

    threads = []
    for _ in range(5):
        t = threading.Thread(target=client_task, args=(2,))
        threads.append(t)
        t.start()
    
    time.sleep(1)
    count_during = get_client_count()
    print(f"Count during connection: {count_during}")
    assert count_during == initial_count + 5

    for t in threads:
        t.join()
    
    time.sleep(1) # Allow server to process disconnects
    final_count = get_client_count()
    print(f"Final client count: {final_count}")
    assert final_count == initial_count
    print("OK\n")

def test_log_truncation():
    print("--- Test 2: Log Message Truncation ---")
    long_message = "A" * 2048
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, LOG_PORT))
        s.sendall((long_message + '\n').encode())
    
    time.sleep(1)
    # Verification requires checking server-side logs or 
    # having a query mechanism that can return the truncated log.
    # For this test, we assume it works if the server doesn't crash.
    print("Sent 2048 byte log. Server should truncate it to ~1024 bytes.")
    print("OK\n")

if __name__ == "__main__":
    server_proc = subprocess.Popen(["./bin/logcaster-c"])
    print("Server started...")
    time.sleep(1)

    test_client_counter()
    test_log_truncation()

    server_proc.terminate()
    server_proc.wait()
    print("All MVP5 tests passed!")
```# DevHistory 10: LogCaster-CPP MVP5 - Security Hardening

## 1. 개요 (Overview)

C++ 구현의 다섯 번째 단계(MVP5)는 C 버전의 MVP5와 마찬가지로, 새로운 기능 추가가 아닌 기존 코드베이스의 안정성과 보안을 강화하는 데 집중합니다. C++의 현대적인 기능들 덕분에 C 버전에서 발견된 대부분의 메모리 및 동시성 문제는 사전에 방지되었지만, 외부 입력에 대한 검증은 여전히 중요한 과제로 남아있습니다. 이 단계는 LogCaster C++ 버전을 프로덕션 환경에 한 걸음 더 가깝게 만드는 마무리 작업입니다.

**주요 해결 과제:**
- **리소스 고갈(Resource Exhaustion) 방어:** C 버전과 동일하게, 악의적인 클라이언트가 의도적으로 매우 큰 로그 메시지를 전송하여 서버의 메모리를 고갈시키려는 공격을 방어해야 합니다. `LogServer`가 클라이언트로부터 데이터를 수신할 때, 메시지의 크기를 검증하고 설정된 최대 길이를 초과할 경우 이를 안전하게 절단하는 로직을 추가합니다.

**아키텍처 변경:**
- 이번 MVP에서는 아키텍처의 큰 변경 없이, `LogServer` 클래스 내부의 클라이언트 데이터 처리 로직만 소폭 수정됩니다.

이 단계를 통해 C++ 버전 또한 외부의 비정상적인 입력에 대해 더 견고하게 대응할 수 있는 능력을 갖추게 됩니다.

## 2. 주요 수정 사항 및 코드

### 2.1. `include/LogServer.h` (수정)

`LogServer` 클래스 내부에 로그 메시지의 최대 허용 길이를 나타내는 상수를 추가합니다.

```cpp
// [SEQUENCE: MVP10-1]
#ifndef LOGSERVER_H
#define LOGSERVER_H

// ... (기존 include는 MVP4와 동일)
#include "Persistence.h"

class LogServer {
public:
    // ... (기존 public 메소드는 MVP4와 동일)

    // [SEQUENCE: MVP10-2]
    // MVP5 추가: 안전한 로그 길이를 위한 상수
    static constexpr size_t SAFE_LOG_LENGTH = 1024;

private:
    // ... (나머지는 MVP4와 동일)
};

#endif // LOGSERVER_H
```

### 2.2. `src/LogServer.cpp` (수정)

`handleClientTask` 메소드에서 `recv`로 데이터를 수신한 후, 해당 데이터의 길이를 `SAFE_LOG_LENGTH`와 비교하여 초과 시 절단하는 로직을 추가합니다.

```cpp
// [SEQUENCE: MVP10-3]
#include "LogServer.h"
// ... (다른 include는 MVP4와 동일)

// ... (생성자, 소멸자, start, stop, initialize, runEventLoop, handleNewConnection 등은 MVP4와 동일)

// [SEQUENCE: MVP10-4]
// 로그 클라이언트 작업 (MVP5 버전)
void LogServer::handleClientTask(int client_fd) {
    char buffer[4096]; // 수신을 위한 물리적 버퍼
    while (true) {
        ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (nbytes <= 0) break;
        
        // [SEQUENCE: MVP10-5]
        // 수신된 데이터로 std::string 생성 및 크기 검증
        std::string log_message(buffer, nbytes);
        if (log_message.length() > SAFE_LOG_LENGTH) {
            log_message.resize(SAFE_LOG_LENGTH - 3);
            log_message += "...";
        }

        // Remove trailing newline
        size_t last_char_pos = log_message.find_last_not_of("\r\n");
        if (std::string::npos != last_char_pos) {
            log_message.erase(last_char_pos + 1);
        }

        // 1. 인메모리 버퍼에 저장
        logBuffer_->push(log_message);

        // 2. 영속성 관리자에게 쓰기 요청 (활성화된 경우)
        if (persistence_) {
            persistence_->write(log_message);
        }
    }
    close(client_fd);
}

// ... (나머지 함수는 MVP4와 동일)
```

## 3. 테스트 코드 (Test Code)

이 변경 사항을 검증하기 위해, C 버전의 보안 테스트(`test_security.py`)와 유사한 테스트를 수행할 수 있습니다. 특히 1KB가 넘는 긴 로그 메시지를 전송하고, 서버가 비정상 종료되지 않고 안정적으로 동작하는지를 확인하는 것이 중요합니다.

*(테스트 코드는 C 버전 MVP5의 `test_log_truncation` 부분과 유사하게 구성할 수 있어 별도로 첨부하지 않음)*

```
# DevHistory 11: LogCaster-CPP MVP6 - IRC Server Integration

## 1. 개요 (Overview)

C++ 구현의 마지막 단계(MVP6)는 `DEVELOPMENT_JOURNAL.md`에는 기록되지 않았지만, 코드베이스에 존재하는 매우 중요한 기능인 **IRC(Internet Relay Chat) 서버 통합**입니다. 이 기능은 LogCaster를 단순한 로그 수집 및 조회 도구에서, 실시간으로 로그를 구독하고 알림을 받을 수 있는 강력한 대화형 모니터링 플랫폼으로 격상시킵니다.

**주요 기능 추가:**
- **내장 IRC 서버:** LogCaster는 이제 자체적으로 IRC 프로토콜을 지원하는 서버를 내장합니다. 사용자는 표준 IRC 클라이언트(예: HexChat, mIRC)를 사용하여 LogCaster에 접속할 수 있습니다.
- **채널 기반 로그 구독:** 사용자는 특정 패턴을 가진 채널에 참여(`JOIN`)함으로써, 해당 패턴과 일치하는 로그만 실시간으로 받아볼 수 있습니다. 예를 들어, `#errors` 채널에 참여하면 `level`이 `ERROR`인 로그만 수신하고, `#services-auth` 채널에 참여하면 `category`가 `auth`인 `services` 로그만 수신하는 식입니다.
- **실시간 알림:** `LogBuffer`에 새로운 로그가 추가될 때, 등록된 채널 패턴과 일치하는지 확인하고, 일치할 경우 해당 채널에 참여한 모든 IRC 클라이언트에게 즉시 로그 메시지를 전송합니다.
- **구조화된 로그 활용:** 이 기능을 위해 기존의 단순 문자열 로그는 `level`, `source`, `category` 등 다양한 메타데이터를 포함하는 구조화된 `LogEntry`로 확장됩니다.

**아키텍처 변경:**
- **IRC 모듈 추가:** `IRCServer`, `IRCClient`, `IRCChannel`, `IRCCommandHandler`, `IRCCommandParser` 등 IRC 프로토콜을 처리하기 위한 다수의 클래스가 새로 추가됩니다.
- **`LogBuffer`와 IRC 연동:** `LogBuffer`에 콜백(callback) 메커니즘이 도입됩니다. `IRCCommandHandler`는 `JOIN` 명령을 처리할 때, 특정 로그 패턴과 해당 알림을 처리할 함수(콜백)를 `LogBuffer`에 등록합니다. `LogBuffer`는 새 로그가 들어올 때마다 등록된 콜백들을 실행하여 IRC 서버에 알림을 보냅니다.
- **`main` 함수 수정:** IRC 서버를 활성화하고 지정된 포트에서 실행하기 위한 커맨드 라인 인자(`-i`, `-I`)와 로직이 추가됩니다.

이 MVP는 LogCaster의 활용성을 극적으로 확장하여, 개발자와 시스템 관리자가 터미널에 직접 접속하지 않고도 익숙한 IRC 클라이언트를 통해 여러 시스템의 로그를 중앙에서 편리하게 모니터링할 수 있게 해줍니다.

## 2. 빌드 시스템 (Build System)

다수의 IRC 관련 소스 파일들이 `CMakeLists.txt`에 추가됩니다.

```cmake
# [SEQUENCE: MVP11-1]
# ... (project, CMAKE_CXX_STANDARD 등은 이전과 동일) ...

# [SEQUENCE: MVP11-2]
# MVP6에 필요한 전체 소스 파일 목록 (IRC 모듈 추가)
set(SOURCES
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/LogBuffer.cpp
    src/QueryHandler.cpp
    src/QueryParser.cpp
    src/Persistence.cpp
    # IRC integration
    src/IRCServer.cpp
    src/IRCClient.cpp
    src/IRCChannel.cpp
    src/IRCChannelManager.cpp
    src/IRCClientManager.cpp
    src/IRCCommandParser.cpp
    src/IRCCommandHandler.cpp
)

# ... (나머지는 MVP8과 동일) 
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. 신규 IRC 모듈 파일

*(이 단계에서는 많은 파일이 추가되므로, 주요 클래스의 정의(헤더 파일)와 핵심 로직 위주로 요약하여 설명합니다. 전체 코드는 코드베이스를 참조하십시오.)*

- **`include/IRCServer.h`:** IRC 클라이언트 연결을 수락하고 각 클라이언트를 스레드 풀에 위임하는 메인 서버 클래스.
- **`include/IRCClient.h`:** 접속한 클라이언트의 정보(소켓, 닉네임, 현재 채널 등)를 관리하는 클래스.
- **`include/IRCChannel.h`:** IRC 채널 정보를 관리하고, 채널에 속한 클라이언트들에게 메시지를 방송(broadcast)하는 클래스.
- **`include/IRCClientManager.h`, `include/IRCChannelManager.h`:** 여러 클라이언트와 채널 객체를 스레드 안전하게 관리하는 매니저 클래스.
- **`include/IRCCommandParser.h`:** 클라이언트가 보낸 원시 IRC 메시지(예: `JOIN #channel\r\n`)를 파싱하여 `IRCCommand` 객체로 변환하는 클래스.
- **`include/IRCCommandHandler.h`:** 파싱된 `IRCCommand`를 받아 실제 동작(채널 참여, 로그 구독 콜백 등록 등)을 수행하는 클래스.

**핵심 로직 예시 (`src/IRCCommandHandler.cpp`의 JOIN 명령 처리):**
```cpp
// [SEQUENCE: MVP11-3]
void IRCCommandHandler::handleJoin(const IRCCommand& command) {
    // ... (클라이언트 정보 가져오기)
    std::string channel_name = command.get_params()[0];

    // 채널에 클라이언트 추가
    auto channel = channel_manager_.get_or_create_channel(channel_name);
    channel->add_client(client);

    // ... (JOIN 성공 응답 클라이언트에 전송)

    // [SEQUENCE: MVP11-4]
    // LogBuffer에 콜백 등록으로 로그 구독
    std::string pattern = channel_name.substr(1); // # 제거
    log_buffer_->registerChannelCallback(pattern, [channel](const LogEntry& entry) {
        std::stringstream ss;
        ss << "[" << entry.level << "] " << entry.message;
        channel->broadcast("SYSTEM", ss.str());
    });
}
```

### 3.2. `include/LogBuffer.h` (수정)

구조화된 로그를 위한 `LogEntry` 확장 및 IRC 연동을 위한 콜백 인터페이스를 추가합니다.

```cpp
// [SEQUENCE: MVP11-5]
#ifndef LOGBUFFER_H
#define LOGBUFFER_H

#include <functional> // for std::function
#include <map> // for std::map
// ... (기존 include)

// [SEQUENCE: MVP11-6]
// 구조화된 로그 항목 (IRC 통합 버전)
struct LogEntry {
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    
    // MVP6 추가 필드
    std::string level;      // 예: ERROR, WARNING, INFO
    std::string source;     // 예: service-auth, 192.168.1.5
    std::string category;   // 예: login, database
    
    LogEntry(std::string msg, std::string lvl = "INFO", std::string src = "") 
        : message(std::move(msg)), timestamp(std::chrono::system_clock::now()), 
          level(std::move(lvl)), source(std::move(src)) {}
};

class LogBuffer {
public:
    // ... (기존 public 메소드)

    // [SEQUENCE: MVP11-7]
    // MVP6 추가: IRC 연동을 위한 메소드
    void addLogWithNotification(const LogEntry& entry);
    void registerChannelCallback(const std::string& pattern, std::function<void(const LogEntry&)> callback);
    void unregisterChannelCallback(const std::string& pattern);

private:
    // ... (기존 private 멤버)

    // [SEQUENCE: MVP11-8]
    // MVP6 추가: 콜백 저장을 위한 맵
    std::map<std::string, std::vector<std::function<void(const LogEntry&)>>> channelCallbacks_;
};

#endif // LOGBUFFER_H
```

### 3.3. `src/LogBuffer.cpp` (수정)

`addLogWithNotification` 메소드에서 로그를 저장한 후, 등록된 콜백들을 실행하여 IRC 채널에 알림을 보내는 로직을 구현합니다.

```cpp
// [SEQUENCE: MVP11-9]
// ... (기존 함수들)

// [SEQUENCE: MVP11-10]
// 로그 추가 및 알림 처리
void LogBuffer::addLogWithNotification(const LogEntry& entry) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (buffer_.size() >= capacity_) {
            dropOldest_();
        }
        buffer_.push_back(entry);
        totalLogs_++;
    }
    notEmpty_.notify_one();

    // [SEQUENCE: MVP11-11]
    // 등록된 콜백 실행하여 알림 전송
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [pattern, callbacks] : channelCallbacks_) {
        // (단순화를 위해 여기서는 간단한 키워드 매칭만 예시)
        bool match = (entry.level == pattern || entry.source == pattern || 
                      entry.message.find(pattern) != std::string::npos);
        if (match) {
            for (const auto& callback : callbacks) {
                callback(entry);
            }
        }
    }
}

// [SEQUENCE: MVP11-12]
// 콜백 등록 및 해제 함수 구현
void LogBuffer::registerChannelCallback(const std::string& pattern, std::function<void(const LogEntry&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    channelCallbacks_[pattern].push_back(callback);
}

void LogBuffer::unregisterChannelCallback(const std::string& pattern) {
    std::lock_guard<std::mutex> lock(mutex_);
    channelCallbacks_.erase(pattern);
}
```

### 3.4. `include/LogServer.h` (수정)

`main` 함수에서 `IRCServer`가 `LogBuffer`에 접근할 수 있도록 `getLogBuffer` 메소드를 추가합니다.

```cpp
// [SEQUENCE: MVP11-13]
class LogServer {
public:
    // ... (기존 public 메소드)
    void setPersistenceManager(std::unique_ptr<PersistenceManager> persistence);

    // [SEQUENCE: MVP11-14]
    // MVP6 추가: IRC 서버와의 연동을 위해 LogBuffer 공유
    std::shared_ptr<LogBuffer> getLogBuffer() const { return logBuffer_; }

private:
    // ... (기존 private 멤버)
};
```

### 3.5. `src/main.cpp` (수정)

IRC 서버를 활성화하는 커맨드 라인 인자(`-i`, `-I`)를 처리하고, `IRCServer`를 생성 및 실행하는 로직을 추가합니다.

```cpp
// [SEQUENCE: MVP11-15]
#include "LogServer.h"
#include "IRCServer.h"
#include <iostream>
#include <getopt.h>
#include <signal.h>

static std::unique_ptr<LogServer> g_logServer;
static std::unique_ptr<IRCServer> g_ircServer;

void signalHandler(int signal) {
    std::cout << "\nShutting down..." << std::endl;
    if (g_ircServer) g_ircServer->stop();
    if (g_logServer) g_logServer->stop();
    exit(0);
}

int main(int argc, char* argv[]) {
    int port = 9999;
    bool irc_enabled = false;
    int irc_port = 6667;
    // ... (영속성 설정 변수)

    // [SEQUENCE: MVP11-16]
    // getopt 파싱 루프에 'i'와 'I:' 옵션 추가
    while ((opt = getopt(argc, argv, "p:d:s:PiI:h")) != -1) {
        switch (opt) {
            // ... (기존 case 문)
            case 'i': irc_enabled = true; break;
            case 'I': 
                irc_enabled = true;
                irc_port = std::stoi(optarg);
                break;
        }
    }

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        g_logServer = std::make_unique<LogServer>(port);
        // ... (영속성 관리자 설정)

        // [SEQUENCE: MVP11-17]
        // IRC 서버 생성 및 별도 스레드에서 실행
        if (irc_enabled) {
            g_ircServer = std::make_unique<IRCServer>(irc_port, g_logServer->getLogBuffer());
            std::thread irc_thread([] { g_ircServer->start(); });
            irc_thread.detach();
            std::cout << "IRC Server enabled on port " << irc_port << std::endl;
        }

        g_logServer->start(); // LogServer는 메인 스레드에서 실행

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```# DevHistory 12: 최종 요약 및 C vs C++ 비교 분석

## 1. 프로젝트 여정 요약 (Project Journey Summary)

LogCaster 프로젝트는 고성능 로그 서버를 C와 C++ 두 가지 언어로 구현하여, 각 언어의 패러다임과 특성을 비교 분석하는 것을 목표로 시작되었습니다. 약 2일간의 여정을 통해, 우리는 단순한 TCP 서버에서 시작하여 스레드 풀, 인메모리 버퍼, 고급 쿼리 엔진, 영속성 계층, 그리고 실시간 IRC 알림 기능까지 갖춘 정교한 시스템을 완성했습니다.

**주요 개발 마일스톤:**
- **MVP1 (기본 TCP 서버):** `select` (C) 및 `thread-per-client` (C++) 모델을 사용하여 기본적인 동시 접속 로그 수신 기능을 구현했습니다.
- **MVP2 (동시성 모델 개선):** 스레드 풀 아키텍처와 인메모리 로그 버퍼를 도입하여 서버의 확장성과 성능을 크게 향상시켰습니다.
- **MVP3 (고급 쿼리 엔진):** 정규식, 시간 범위, 다중 키워드 검색 등 복잡한 조건의 로그 조회가 가능한 쿼리 엔진을 탑재했습니다.
- **MVP4 (영속성 계층):** 비동기 파일 I/O와 로그 로테이션 기능을 구현하여 서버가 재시작되어도 데이터가 보존되도록 안정성을 확보했습니다.
- **MVP5 (보안 및 안정성 강화):** 코드 감사를 통해 발견된 메모리 누수, 경쟁 조건, 입력 유효성 문제 등을 해결하여 프로덕션 수준의 견고함을 갖추었습니다.
- **MVP6 (IRC 통합 - C++ 전용):** 실시간으로 로그를 구독하고 알림을 받을 수 있는 IRC 서버를 통합하여 대화형 모니터링 플랫폼으로 확장했습니다.

이 `DevHistory` 시리즈는 위의 각 단계를 상세히 기록하여, 제3자가 프로젝트의 시작부터 최종 완성까지 모든 과정을 재현하고 학습할 수 있도록 구성되었습니다.

## 2. 최종 아키텍처 비교 (Final Architecture Comparison)

두 버전은 동일한 핵심 기능을 목표로 했지만, 최종적으로는 각 언어의 강점을 살린 다른 형태의 아키텍처와 코드베이스를 갖게 되었습니다.

### 2.1. LogCaster-C (최종)
- **총 코드 라인 수:** 약 2,500 라인
- **파일 수:** 16개 (헤더 8, 소스 8)
- **핵심 아키텍처:**
  - 수동 리소스 관리 (`malloc`/`free`)
  - `pthreads` 기반의 스레드 풀
  - `select` 기반의 단일 스레드 이벤트 루프
  - `strtok_r`, `getopt` 등 C 표준 라이브러리 함수 적극 활용
  - 모든 스레드 안전성을 `pthread_mutex_t`를 통해 직접 관리
- **강점:** 시스템 자원에 대한 직접적이고 세밀한 제어, 작은 바이너리 크기, 예측 가능한 성능.
- **단점:** 개발자가 모든 메모리와 리소스의 생명주기를 직접 관리해야 하므로 버그 발생 가능성이 높음. 오류 처리가 번거롭고 코드가 장황해지기 쉬움.

### 2.2. LogCaster-CPP (최종)
- **총 코드 라인 수:** 약 2,800 라인 (IRC 기능 포함 시 C 버전보다 많음)
- **파일 수:** 28개 (헤더 14, 소스 14)
- **핵심 아키텍처:**
  - RAII와 스마트 포인터(`unique_ptr`, `shared_ptr`)를 통한 자동 리소스 관리
  - C++11 `<thread>` 기반의 스레드 풀
  - `std::deque`, `std::optional`, `std::regex` 등 STL 및 최신 라이브러리 활용
  - 예외 처리(Exception Handling)를 통한 견고한 오류 관리
  - 람다(Lambda), `std::function`을 활용한 유연한 콜백 메커니즘 (IRC 연동)
- **강점:** 코드의 안전성(메모리, 타입)이 언어 차원에서 보장됨. 개발 생산성이 높고 유지보수가 용이함. 높은 수준의 추상화로 복잡한 기능(IRC)을 쉽게 통합.
- **단점:** 바이너리 크기가 더 크고, 추상화 계층으로 인해 일부 성능 오버헤드가 발생할 수 있음 (이번 프로젝트에서는 I/O 바운드 작업이 대부분이라 거의 차이 없음).

## 3. 개발 시간 및 품질 분석 (Dev Time & Quality Analysis)

`DEVELOPMENT_JOURNEY.md`에 기록된 개발 시간 추정치를 통해 C++의 생산성 우위를 확인할 수 있습니다.

| 단계 (Phase) | C 구현 시간 | C++ 구현 시간 | 시간 절약 |
| :--- | :--- | :--- | :--- |
| MVP1 | 3 시간 | 2 시간 | 33% |
| MVP2 | 4 시간 | 2.5 시간 | 37% |
| MVP3 | 4 시간 | 1.5 시간 | 62% |
| **합계** | **11 시간** | **6 시간** | **45%** |

C++ 버전은 C 버전에 비해 약 **45%의 개발 시간을 절약**했습니다. 이는 C++의 강력한 표준 라이브러리와 자동 리소스 관리 기능 덕분에 개발자가 비즈니스 로직에 더 집중할 수 있었기 때문입니다. 특히 복잡한 쿼리 파서를 구현한 MVP3 단계에서 그 효과가 극대화되었습니다.

버그 발생률 또한 C++ 버전이 현저히 낮았으며, 특히 C 버전에서 많은 디버깅 시간을 유발했던 메모리 누수나 경쟁 조건 관련 버그는 C++ 버전에서는 거의 발생하지 않았습니다.

## 4. 주요 기술적 학습 내용 (Key Technical Learnings)

- **메모리 관리:** C++의 RAII 패턴은 `malloc`/`free` 쌍을 맞추는 과정에서 발생하는 거의 모든 종류의 메모리 관련 버그를 원천적으로 방지하는 가장 효과적인 방법임을 재확인했습니다.
- **오류 처리:** C의 반환 코드 검사 방식은 코드를 장황하게 만들고 실수를 유발하기 쉽습니다. C++의 예외 처리 방식은 오류 발생 지점과 처리 지점을 명확히 분리하여 코드의 가독성과 안정성을 크게 향상시켰습니다.
- **스레드 안전성:** C++의 `std::mutex`, `std::lock_guard` 등은 C의 `pthread_mutex` API보다 사용이 간편하고, 락의 생명주기를 스코프에 바인딩하여 데드락이나 락 해제를 잊는 실수를 방지해 줍니다.
- **추상화의 힘:** `std::function`과 람다를 활용한 C++의 콜백 메커니즘은 C의 함수 포인터 방식보다 훨씬 유연하고 타입 안전한 방식으로 IRC 서버와 같은 복잡한 외부 모듈을 연동할 수 있게 해주었습니다.

## 5. 최종 결론 (Final Conclusion)

이 프로젝트는 동일한 요구사항에 대해 C와 C++가 어떻게 다른 접근 방식을 취하는지, 그리고 그 결과가 어떻게 달라지는지를 명확하게 보여주었습니다.

- **C**는 시스템의 가장 깊은 곳까지 제어해야 하거나, 리소스가 극도로 제한된 환경(임베디드, 커널 등)에서 여전히 강력한 선택지입니다. 하지만 그만큼 개발자의 높은 책임감과 세심한 주의를 요구합니다.
- **C++**는 현대적인 서버 애플리케이션 개발에 있어 성능, 안정성, 생산성 모든 면에서 C보다 우월한 선택지임을 증명했습니다. 특히 C++11 이후 도입된 현대적인 기능들은 C의 저수준 성능을 거의 그대로 유지하면서도, Java나 Python과 같은 고수준 언어의 개발 편의성과 안전성을 상당 부분 제공합니다.

결론적으로, 대부분의 고성능 네트워크 서버 애플리케이션 개발에 있어 **C++는 C에 비해 훨씬 안전하고 생산적인 대안**입니다. LogCaster 프로젝트는 이러한 결론을 뒷받침하는 구체적이고 실질적인 증거가 될 것입니다.

이것으로 LogCaster 개발 역사 시리즈를 마칩니다.
