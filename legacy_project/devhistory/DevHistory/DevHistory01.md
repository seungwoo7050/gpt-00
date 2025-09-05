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
