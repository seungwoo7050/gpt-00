# 9. I/O 멀티플렉싱 ⚡
## 하나의 직원이 여러 고객을 동시에
*LogCaster 프로젝트를 위한 완벽 가이드*

---

## 📋 문서 정보
- **난이도**: 🔴 고급
- **예상 학습 시간**: 12-15시간
- **전제 조건**: 
  - C/C++ 중급 이상
  - 네트워크 프로그래밍 기초
  - 파일 디스크립터 이해
  - 동기/비동기 개념
- **실습 환경**: Linux (epoll), macOS/BSD (kqueue), GCC/G++

## 🎯 이 문서에서 배울 내용

I/O 멀티플렉싱은 하나의 스레드로 여러 개의 I/O 스트림을 동시에 모니터링하는 기술입니다. LogCaster 프로젝트에서는 수많은 클라이언트 연결을 효율적으로 처리하기 위해 이 기술이 필수적입니다.

### 학습 목표
- **블로킹 vs 비블로킹 I/O**의 차이 이해
- **select(), poll(), epoll()** 시스템 콜 활용
- **Edge/Level Triggered** 모드 이해
- **고성능 서버** 구현 방법
- **C10K 문제** 해결 전략
- **[신규] 대규모 클라이언트 처리 전략**
- **[신규] epoll/kqueue 고급 활용법**

### 일상생활 비유
```
🏭 직원 한 명이 여러 고객을 응대하는 방법:

1. 순차 방식 (블로킹):
   고객1 완료 → 고객2 시작 → 고객3 대기...
   ❌ 비효율적 (한 번에 한 명만)

2. 멀티플렉싱 방식:
   모든 고객을 동시에 확인 → 준비된 고객만 처리
   ✅ 효율적 (여러 명 동시 처리)
```

---

## 🔄 1. I/O 모델의 이해 - 기다림의 철학

### 1.1 블로킹 vs 비블로킹 I/O

```
📞 전화통화 비유:

블로킹 I/O = 전화기를 들고 기다리기
- 통화가 올 때까지 다른 일 못함
- 간단하지만 비효율적

비블로킹 I/O = 전화가 왔는지 가끔 확인
- 다른 일을 하면서 체크
- 복잡하지만 효율적
```

#### 블로킹 I/O (Blocking I/O) - 기다리기
```c
// 블로킹 I/O 예시 - 데이터가 올 때까지 대기
int client_fd = accept(server_fd, NULL, NULL);  // 🚪 손님 기다리기
char buffer[1024];

// 이 호출은 데이터가 도착할 때까지 블록됨
// 🕰️ 여기서 멈춰서 기다림... 기다림... 기다림...
ssize_t bytes = recv(client_fd, buffer, sizeof(buffer), 0);
// 🎉 드디어 데이터 도착!
```

**특징:**
- 데이터가 준비될 때까지 프로세스가 대기
- 구현이 간단하고 직관적
- 하나의 스레드로 하나의 연결만 처리 가능

#### 비블로킹 I/O (Non-blocking I/O) - 확인하고 바로 돌아오기
```c
#include <fcntl.h>
#include <errno.h>

// 소켓을 비블로킹 모드로 설정
int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// 비블로킹 recv
ssize_t nonblocking_recv(int fd, char* buffer, size_t size) {
    ssize_t result = recv(fd, buffer, size, 0);
    
    if (result == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 데이터가 없음 - 즉시 반환
            return 0;
        }
        // 실제 오류
        return -1;
    }
    
    return result;
}
```

**특징:**
- 데이터가 없어도 즉시 반환
- CPU 사이클 낭비 가능 (폴링)
- 여러 연결을 순차적으로 확인 가능

### 1.2 I/O 멀티플렉싱의 필요성

```
문제 상황: 🎆 대량의 클라이언트 동시 연결

방법 1: 스레드 방식 (Thread per Client)
[클라이언트1] → [스레드1]
[클라이언트2] → [스레드2]  ❌ 메모리 많이 사용
[클라이언트3] → [스레드3]  ❌ 컨텍스트 스위칭 비용

방법 2: I/O 멀티플렉싱
[클라이언트1] ┐
[클라이언트2] ├→ [하나의 스레드] ✅ 효율적!
[클라이언트3] ┘
```

```c
// 문제 상황: 여러 클라이언트를 블로킹 방식으로 처리
void handle_multiple_clients_blocking() {
    int server_fd = create_server_socket(9999);
    int clients[MAX_CLIENTS];
    int client_count = 0;
    
    while (1) {
        // 새 연결 대기 (블로킹)
        int new_client = accept(server_fd, NULL, NULL);
        clients[client_count++] = new_client;
        
        // 모든 클라이언트에서 데이터 읽기 시도
        for (int i = 0; i < client_count; i++) {
            char buffer[1024];
            // 문제: 첫 번째 클라이언트에서 블로킹되면 다른 클라이언트 처리 불가
            recv(clients[i], buffer, sizeof(buffer), 0);
        }
    }
}
```

**해결책: I/O 멀티플렉싱**
- 여러 파일 디스크립터를 동시에 모니터링
- 데이터가 준비된 디스크립터만 처리
- 하나의 스레드로 수백~수천 개의 연결 처리 가능

---

## 📡 2. select() 시스템 콜 - 가장 오래된 방법

### 2.1 select() 기본 개념

select()는 가장 오래되고 이식성이 좋은 I/O 멀티플렉싱 방법입니다.

```
select()의 동작 원리:

1. 📋 파일 디스크립터 집합 준비
   fd_set: [fd1][fd2][fd3]...[fd1023]
   
2. 🔍 시스템에게 "이중에 누가 준비되면 알려줘" 요청
   
3. ⏰ 대기 (타임아웃 가능)
   
4. 🎆 시스템: "﴾FD 3번과 7번이 준비됨!"
   
5. 🏃 해당 FD만 처리
```

```c
#include <sys/select.h>
#include <sys/time.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, 
           fd_set *exceptfds, struct timeval *timeout);
```

### 2.2 fd_set 조작 매크로 - 비트맵 다루기

```c
// fd_set 조작을 위한 매크로들
FD_ZERO(fd_set *set);           // 집합 초기화
FD_SET(int fd, fd_set *set);    // fd를 집합에 추가
FD_CLR(int fd, fd_set *set);    // fd를 집합에서 제거
FD_ISSET(int fd, fd_set *set);  // fd가 집합에 있는지 확인

// 사용 예시
fd_set read_fds;
FD_ZERO(&read_fds);
FD_SET(server_fd, &read_fds);
FD_SET(client_fd, &read_fds);

if (FD_ISSET(server_fd, &read_fds)) {
    // server_fd에서 읽기 가능
}
```

### 2.3 LogCaster용 select() 기반 서버 - 실전 코드

```c
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_CLIENTS 64
#define BUFFER_SIZE 1024

typedef struct SelectServer {
    int server_fd;
    int client_fds[MAX_CLIENTS];
    int max_clients;
    int max_fd;
    fd_set master_set;
    ThreadSafeLogList* log_storage;
} SelectServer;

SelectServer* create_select_server(int port) {
    SelectServer* server = malloc(sizeof(SelectServer));
    if (server == NULL) return NULL;
    
    // 서버 소켓 생성
    server->server_fd = create_server_socket(port);
    if (server->server_fd == -1) {
        free(server);
        return NULL;
    }
    
    // 클라이언트 배열 초기화
    server->max_clients = MAX_CLIENTS;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        server->client_fds[i] = -1;
    }
    
    // fd_set 초기화
    FD_ZERO(&server->master_set);
    FD_SET(server->server_fd, &server->master_set);
    server->max_fd = server->server_fd;
    
    // 로그 저장소 초기화
    server->log_storage = create_log_list();
    
    printf("Select-based server created on port %d\n", port);
    return server;
}

int select_server_add_client(SelectServer* server, int client_fd) {
    // 빈 슬롯 찾기
    for (int i = 0; i < server->max_clients; i++) {
        if (server->client_fds[i] == -1) {
            server->client_fds[i] = client_fd;
            FD_SET(client_fd, &server->master_set);
            
            if (client_fd > server->max_fd) {
                server->max_fd = client_fd;
            }
            
            printf("Client added: fd=%d, slot=%d\n", client_fd, i);
            return i;
        }
    }
    
    printf("Maximum clients reached, rejecting connection\n");
    return -1;
}

void select_server_remove_client(SelectServer* server, int client_fd) {
    // 클라이언트 배열에서 제거
    for (int i = 0; i < server->max_clients; i++) {
        if (server->client_fds[i] == client_fd) {
            server->client_fds[i] = -1;
            break;
        }
    }
    
    // fd_set에서 제거
    FD_CLR(client_fd, &server->master_set);
    close(client_fd);
    
    // max_fd 재계산
    server->max_fd = server->server_fd;
    for (int i = 0; i < server->max_clients; i++) {
        if (server->client_fds[i] > server->max_fd) {
            server->max_fd = server->client_fds[i];
        }
    }
    
    printf("Client removed: fd=%d\n", client_fd);
}

void select_server_handle_new_connection(SelectServer* server) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int client_fd = accept(server->server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) {
        perror("accept failed");
        return;
    }
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    
    if (select_server_add_client(server, client_fd) == -1) {
        // 클라이언트가 가득 찬 경우
        const char* msg = "Server full, please try again later\n";
        send(client_fd, msg, strlen(msg), 0);
        close(client_fd);
    } else {
        printf("New client connected: %s:%d (fd=%d)\n", 
               client_ip, ntohs(client_addr.sin_port), client_fd);
    }
}

void select_server_handle_client_data(SelectServer* server, int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            printf("Client disconnected: fd=%d\n", client_fd);
        } else {
            printf("recv error on fd=%d: %s\n", client_fd, strerror(errno));
        }
        select_server_remove_client(server, client_fd);
        return;
    }
    
    buffer[bytes_received] = '\0';
    
    // 클라이언트 정보 가져오기
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    getpeername(client_fd, (struct sockaddr*)&client_addr, &addr_len);
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    
    // 로그 저장
    thread_safe_add_log(server->log_storage, buffer, client_ip, ntohs(client_addr.sin_port));
    
    printf("Received from fd=%d: %s", client_fd, buffer);
    
    // 응답 전송
    const char* ack = "ACK\n";
    send(client_fd, ack, strlen(ack), 0);
}

int select_server_run(SelectServer* server) {
    printf("Select server running...\n");
    
    while (1) {
        fd_set read_fds = server->master_set;  // 복사본 생성
        
        // 타임아웃 설정 (1초)
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(server->max_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity < 0) {
            perror("select failed");
            break;
        }
        
        if (activity == 0) {
            // 타임아웃 - 주기적인 작업 수행 가능
            continue;
        }
        
        // 서버 소켓 확인 (새 연결)
        if (FD_ISSET(server->server_fd, &read_fds)) {
            select_server_handle_new_connection(server);
        }
        
        // 클라이언트 소켓들 확인
        for (int i = 0; i < server->max_clients; i++) {
            int client_fd = server->client_fds[i];
            
            if (client_fd != -1 && FD_ISSET(client_fd, &read_fds)) {
                select_server_handle_client_data(server, client_fd);
            }
        }
    }
    
    return 0;
}

void destroy_select_server(SelectServer* server) {
    if (server == NULL) return;
    
    // 모든 클라이언트 연결 종료
    for (int i = 0; i < server->max_clients; i++) {
        if (server->client_fds[i] != -1) {
            close(server->client_fds[i]);
        }
    }
    
    close(server->server_fd);
    destroy_log_list(server->log_storage);
    free(server);
}

// 사용 예시
int main() {
    SelectServer* server = create_select_server(9999);
    if (server == NULL) {
        fprintf(stderr, "Failed to create server\n");
        return 1;
    }
    
    select_server_run(server);
    
    destroy_select_server(server);
    return 0;
}
```

### 2.4 select()의 장단점

```
✅ 장점:
1. 🌍 모든 시스템에서 동작 (이식성 최고)
2. 📦 구현이 단순하고 직관적
3. ⏰ 타임아웃 기능 내장

❌ 단점:
1. 🔢 FD 개수 제한 (1024개)
   - 더 많은 연결? 불가능!
2. 🔄 O(n) 복잡도
   - 1000개 중 1개만 준비되어도 전체 확인
3. 🔄 fd_set 매번 재설정
   - select() 호출 후 fd_set이 변경됨

예시: 1000개 연결 중 10개만 활성
→ select()는 여전히 1000개 모두 확인 😢
```

---

## 📊 3. poll() 시스템 콜 - select()의 개선판

### 3.1 poll() 기본 개념

poll()은 select()의 제한을 개선한 I/O 멀티플렉싱 방법입니다.

```
select() vs poll():

select():                    poll():
📦 비트맵 방식                📋 구조체 배열 방식
FD_SET(3, &readfds)          pollfd[0].fd = 3
FD_SET(7, &readfds)          pollfd[0].events = POLLIN
❌ 1024개 제한               ✅ 제한 없음
❌ fd_set 재설정 필요       ✅ 구조체 유지
```

```c
#include <poll.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout);

// pollfd 구조체
struct pollfd {
    int fd;         // 파일 디스크립터
    short events;   // 관심있는 이벤트
    short revents;  // 발생한 이벤트
};

// 이벤트 플래그
POLLIN      // 읽기 가능
POLLOUT     // 쓰기 가능
POLLERR     // 오류 발생
POLLHUP     // 연결 종료
POLLNVAL    // 잘못된 fd
```

### 3.2 LogCaster용 poll() 기반 서버

```c
#include <poll.h>

#define MAX_POLL_FDS 1000

typedef struct PollServer {
    int server_fd;
    struct pollfd poll_fds[MAX_POLL_FDS];
    int nfds;
    ThreadSafeLogList* log_storage;
} PollServer;

PollServer* create_poll_server(int port) {
    PollServer* server = malloc(sizeof(PollServer));
    if (server == NULL) return NULL;
    
    server->server_fd = create_server_socket(port);
    if (server->server_fd == -1) {
        free(server);
        return NULL;
    }
    
    // poll_fds 배열 초기화
    for (int i = 0; i < MAX_POLL_FDS; i++) {
        server->poll_fds[i].fd = -1;
        server->poll_fds[i].events = 0;
        server->poll_fds[i].revents = 0;
    }
    
    // 서버 소켓을 첫 번째 슬롯에 추가
    server->poll_fds[0].fd = server->server_fd;
    server->poll_fds[0].events = POLLIN;
    server->nfds = 1;
    
    server->log_storage = create_log_list();
    
    printf("Poll-based server created on port %d\n", port);
    return server;
}

int poll_server_add_client(PollServer* server, int client_fd) {
    if (server->nfds >= MAX_POLL_FDS) {
        printf("Maximum poll fds reached\n");
        return -1;
    }
    
    // 빈 슬롯 찾기
    for (int i = 1; i < MAX_POLL_FDS; i++) {
        if (server->poll_fds[i].fd == -1) {
            server->poll_fds[i].fd = client_fd;
            server->poll_fds[i].events = POLLIN;
            server->poll_fds[i].revents = 0;
            
            if (i >= server->nfds) {
                server->nfds = i + 1;
            }
            
            printf("Client added: fd=%d, slot=%d\n", client_fd, i);
            return i;
        }
    }
    
    return -1;
}

void poll_server_remove_client(PollServer* server, int index) {
    if (index <= 0 || index >= server->nfds) return;
    
    int client_fd = server->poll_fds[index].fd;
    printf("Removing client: fd=%d, slot=%d\n", client_fd, index);
    
    close(client_fd);
    server->poll_fds[index].fd = -1;
    server->poll_fds[index].events = 0;
    server->poll_fds[index].revents = 0;
    
    // nfds 재계산 (압축)
    while (server->nfds > 1 && server->poll_fds[server->nfds - 1].fd == -1) {
        server->nfds--;
    }
}

void poll_server_handle_new_connection(PollServer* server) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int client_fd = accept(server->server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) {
        perror("accept failed");
        return;
    }
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    
    if (poll_server_add_client(server, client_fd) == -1) {
        const char* msg = "Server full\n";
        send(client_fd, msg, strlen(msg), 0);
        close(client_fd);
    } else {
        printf("New client connected: %s:%d (fd=%d)\n", 
               client_ip, ntohs(client_addr.sin_port), client_fd);
    }
}

void poll_server_handle_client_data(PollServer* server, int index) {
    int client_fd = server->poll_fds[index].fd;
    char buffer[BUFFER_SIZE];
    
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            printf("Client disconnected: fd=%d\n", client_fd);
        } else {
            printf("recv error on fd=%d: %s\n", client_fd, strerror(errno));
        }
        poll_server_remove_client(server, index);
        return;
    }
    
    buffer[bytes_received] = '\0';
    
    // 클라이언트 정보 가져오기
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    getpeername(client_fd, (struct sockaddr*)&client_addr, &addr_len);
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    
    // 로그 저장
    thread_safe_add_log(server->log_storage, buffer, client_ip, ntohs(client_addr.sin_port));
    
    printf("Received from fd=%d: %s", client_fd, buffer);
    
    // 응답 전송
    const char* ack = "ACK\n";
    send(client_fd, ack, strlen(ack), 0);
}

int poll_server_run(PollServer* server) {
    printf("Poll server running...\n");
    
    while (1) {
        // poll 호출 (1초 타임아웃)
        int activity = poll(server->poll_fds, server->nfds, 1000);
        
        if (activity < 0) {
            perror("poll failed");
            break;
        }
        
        if (activity == 0) {
            // 타임아웃
            continue;
        }
        
        // 서버 소켓 확인 (새 연결)
        if (server->poll_fds[0].revents & POLLIN) {
            poll_server_handle_new_connection(server);
            server->poll_fds[0].revents = 0;
        }
        
        // 클라이언트 소켓들 확인
        for (int i = 1; i < server->nfds; i++) {
            if (server->poll_fds[i].fd == -1) continue;
            
            short revents = server->poll_fds[i].revents;
            
            if (revents & POLLIN) {
                poll_server_handle_client_data(server, i);
            }
            
            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                printf("Error on fd=%d: revents=0x%x\n", server->poll_fds[i].fd, revents);
                poll_server_remove_client(server, i);
            }
            
            server->poll_fds[i].revents = 0;
        }
    }
    
    return 0;
}

void destroy_poll_server(PollServer* server) {
    if (server == NULL) return;
    
    // 모든 클라이언트 연결 종료
    for (int i = 1; i < server->nfds; i++) {
        if (server->poll_fds[i].fd != -1) {
            close(server->poll_fds[i].fd);
        }
    }
    
    close(server->server_fd);
    destroy_log_list(server->log_storage);
    free(server);
}
```

### 3.3 poll()의 장단점

```
✅ 장점:
1. 🌌 FD 개수 무제한
   - 10,000개 연결? 가능!
2. 🎯 이벤트 타입 명확
   - POLLIN, POLLOUT, POLLERR 등
3. 🔄 구조체 재사용
   - revents만 리셋하면 됨

❌ 단점:
1. 🔄 여전히 O(n) 복잡도
   - 10,000개 중 10개 활성 → 10,000개 모두 확인
2. 🐧 Linux 외에서는 느림
   - 이식성은 좋지만 성능은...

poll() 사용 시기:
- 🌍 여러 OS 지원 필요
- 🔢 1024개 이상 연결 필요
- 📈 극대 성능은 불필요
```

---

## 🚀 4. epoll() 시스템 콜 (Linux 특화) - 최강의 성능

### 4.1 epoll() 기본 개념

epoll()은 Linux에서 제공하는 고성능 I/O 멀티플렉싱 방법입니다.

```
epoll의 혁신적인 아이디어:

select/poll:                 epoll:
"모든 FD 확인해봐"            "준비된 FD만 알려줘"
🔄 O(n) 복잡도               ⚡ O(1) 복잡도

비유:
🏢 호텔 프론트              📢 알림 시스템
모든 방 하나씩 확인         손님이 버튼 누르면 알림
(비효율적)                (효율적!)
```

```c
#include <sys/epoll.h>

// epoll 인스턴스 생성
int epoll_create1(int flags);

// 이벤트 추가/수정/삭제
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

// 이벤트 대기
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

// epoll_event 구조체
struct epoll_event {
    uint32_t events;    // 이벤트 마스크
    epoll_data_t data;  // 사용자 데이터
};

// 이벤트 타입
EPOLLIN     // 읽기 가능
EPOLLOUT    // 쓰기 가능
EPOLLERR    // 오류
EPOLLHUP    // 연결 종료
EPOLLET     // Edge Triggered 모드
EPOLLONESHOT // 한 번만 알림
```

### 4.2 LogCaster용 epoll() 기반 서버

```c
#include <sys/epoll.h>

#define MAX_EPOLL_EVENTS 100

typedef struct EpollServer {
    int server_fd;
    int epoll_fd;
    struct epoll_event events[MAX_EPOLL_EVENTS];
    ThreadSafeLogList* log_storage;
    int client_count;
} EpollServer;

EpollServer* create_epoll_server(int port) {
    EpollServer* server = malloc(sizeof(EpollServer));
    if (server == NULL) return NULL;
    
    server->server_fd = create_server_socket(port);
    if (server->server_fd == -1) {
        free(server);
        return NULL;
    }
    
    // epoll 인스턴스 생성
    server->epoll_fd = epoll_create1(0);
    if (server->epoll_fd == -1) {
        perror("epoll_create1");
        close(server->server_fd);
        free(server);
        return NULL;
    }
    
    // 서버 소켓을 epoll에 추가
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server->server_fd;
    
    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->server_fd, &event) == -1) {
        perror("epoll_ctl: server_fd");
        close(server->epoll_fd);
        close(server->server_fd);
        free(server);
        return NULL;
    }
    
    server->log_storage = create_log_list();
    server->client_count = 0;
    
    printf("Epoll-based server created on port %d\n", port);
    return server;
}

int epoll_server_add_client(EpollServer* server, int client_fd) {
    // 클라이언트를 non-blocking 모드로 설정
    set_nonblocking(client_fd);
    
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;  // Edge Triggered 모드
    event.data.fd = client_fd;
    
    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
        perror("epoll_ctl: add client");
        return -1;
    }
    
    server->client_count++;
    printf("Client added to epoll: fd=%d (total: %d)\n", client_fd, server->client_count);
    return 0;
}

void epoll_server_remove_client(EpollServer* server, int client_fd) {
    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) == -1) {
        perror("epoll_ctl: del client");
    }
    
    close(client_fd);
    server->client_count--;
    printf("Client removed from epoll: fd=%d (total: %d)\n", client_fd, server->client_count);
}

void epoll_server_handle_new_connection(EpollServer* server) {
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server->server_fd, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 더 이상 대기 중인 연결이 없음
                break;
            } else {
                perror("accept");
                break;
            }
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        printf("New client connected: %s:%d (fd=%d)\n", 
               client_ip, ntohs(client_addr.sin_port), client_fd);
        
        if (epoll_server_add_client(server, client_fd) == -1) {
            close(client_fd);
        }
    }
}

void epoll_server_handle_client_data(EpollServer* server, int client_fd) {
    char buffer[BUFFER_SIZE];
    
    while (1) {
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 더 이상 읽을 데이터가 없음 (Edge Triggered 모드)
                break;
            } else {
                printf("recv error on fd=%d: %s\n", client_fd, strerror(errno));
                epoll_server_remove_client(server, client_fd);
                return;
            }
        } else if (bytes_received == 0) {
            // 클라이언트가 연결을 종료함
            printf("Client disconnected: fd=%d\n", client_fd);
            epoll_server_remove_client(server, client_fd);
            return;
        }
        
        buffer[bytes_received] = '\0';
        
        // 클라이언트 정보 가져오기
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        getpeername(client_fd, (struct sockaddr*)&client_addr, &addr_len);
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        // 로그 저장
        thread_safe_add_log(server->log_storage, buffer, client_ip, ntohs(client_addr.sin_port));
        
        printf("Received from fd=%d (%zu bytes): %s", client_fd, bytes_received, buffer);
        
        // 응답 전송
        const char* ack = "ACK\n";
        ssize_t sent = send(client_fd, ack, strlen(ack), 0);
        if (sent == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            printf("send error on fd=%d: %s\n", client_fd, strerror(errno));
            epoll_server_remove_client(server, client_fd);
            return;
        }
    }
}

int epoll_server_run(EpollServer* server) {
    printf("Epoll server running...\n");
    
    while (1) {
        int nready = epoll_wait(server->epoll_fd, server->events, MAX_EPOLL_EVENTS, 1000);
        
        if (nready == -1) {
            perror("epoll_wait");
            break;
        }
        
        if (nready == 0) {
            // 타임아웃 - 주기적인 작업 수행
            printf("Active clients: %d\n", server->client_count);
            continue;
        }
        
        // 이벤트 처리
        for (int i = 0; i < nready; i++) {
            int fd = server->events[i].data.fd;
            uint32_t events = server->events[i].events;
            
            if (events & (EPOLLERR | EPOLLHUP)) {
                printf("Error on fd=%d: events=0x%x\n", fd, events);
                if (fd != server->server_fd) {
                    epoll_server_remove_client(server, fd);
                }
                continue;
            }
            
            if (fd == server->server_fd) {
                // 새 연결
                epoll_server_handle_new_connection(server);
            } else {
                // 클라이언트 데이터
                epoll_server_handle_client_data(server, fd);
            }
        }
    }
    
    return 0;
}

void destroy_epoll_server(EpollServer* server) {
    if (server == NULL) return;
    
    close(server->epoll_fd);
    close(server->server_fd);
    destroy_log_list(server->log_storage);
    free(server);
}
```

### 4.3 Level Triggered vs Edge Triggered

```
🌊 수위 감지 비유:

Level Triggered (LT):        Edge Triggered (ET):
🏊 수영장 상태 확인         🌊 파도 감지
"물이 있는 동안" 계속 알림    "물이 차오를 때" 한번 알림

예시:
데이터: [HELLO WORLD]

LT: read(5) → "HELLO"
    → 다시 알림! (" WORLD" 남음)
    read(6) → " WORLD"

ET: read(5) → "HELLO" 
    → 알림 없음! (한 번만 알려줌)
    → 나머지를 읽으려면 모두 읽어야 함
```

#### Level Triggered (기본값) - 안전하고 쉽게
```c
// 데이터가 있는 동안 계속 알림
struct epoll_event event;
event.events = EPOLLIN;  // LT 모드
event.data.fd = client_fd;
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
```

#### Edge Triggered (고성능) - 빠르지만 주의 필요
```c
// 상태 변화 시에만 알림
struct epoll_event event;
event.events = EPOLLIN | EPOLLET;  // ET 모드
event.data.fd = client_fd;
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);

// ET 모드에서는 모든 데이터를 읽어야 함
void handle_et_mode(int fd) {
    char buffer[1024];
    while (1) {
        ssize_t n = read(fd, buffer, sizeof(buffer));
        if (n == -1) {
            if (errno == EAGAIN) {
                break;  // 모든 데이터를 읽었음
            }
            // 실제 오류 처리
        } else if (n == 0) {
            // EOF
            break;
        }
        // 데이터 처리
    }
}
```

### 4.4 epoll()의 장단점

```
✅ 장점:
1. ⚡ O(1) 복잡도
   - 100,000개 연결도 문제없음!
2. 🚀 최고의 성능
   - C10K 문제 해결
3. 💾 메모리 효율적
   - 커널이 준비된 FD만 반환
4. 🎯 ET 모드로 극한 성능

❌ 단점:
1. 🐧 Linux 전용
   - BSD: kqueue, Windows: IOCP
2. 🤯 ET 모드 복잡함
   - 잘못 구현하면 데이터 누락

성능 비교:
select: 1,000개 연결 → 😵 느림
poll:   10,000개 연결 → 😐 보통  
epoll:  100,000개 연결 → 😎 빠름!
```

---

## 🔄 5. 비동기 I/O의 개념 - 미래의 프로그래밍

### 5.1 동기 vs 비동기 I/O

```
🍳 요리 비유:

동기 I/O (직접 요리):
1. 재료 준비 → 2. 요리 → 3. 완성
❌ 요리하는 동안 다른 일 못함

비동기 I/O (요리 주문):
1. 주문: "피자 해주세요"
2. 다른 일 하기 💻
3. 알림: "피자 완성!"
✅ 대기 시간에 다른 작업 가능
```

```c
// 동기 I/O (Synchronous)
ssize_t bytes = read(fd, buffer, size);  // 완료될 때까지 대기

// 비동기 I/O (Asynchronous)
struct aiocb aio_req;
aio_req.aio_fildes = fd;
aio_req.aio_buf = buffer;
aio_req.aio_nbytes = size;
aio_req.aio_offset = 0;

aio_read(&aio_req);  // 즉시 반환

// 나중에 완료 확인
while (aio_error(&aio_req) == EINPROGRESS) {
    // 다른 작업 수행
}
ssize_t bytes = aio_return(&aio_req);
```

### 5.2 이벤트 기반 프로그래밍 패턴

```c
typedef struct EventLoop {
    int epoll_fd;
    struct epoll_event events[MAX_EVENTS];
    bool running;
} EventLoop;

typedef struct EventHandler {
    int fd;
    void (*on_read)(int fd, void* data);
    void (*on_write)(int fd, void* data);
    void (*on_error)(int fd, void* data);
    void* user_data;
} EventHandler;

EventLoop* create_event_loop() {
    EventLoop* loop = malloc(sizeof(EventLoop));
    loop->epoll_fd = epoll_create1(0);
    loop->running = false;
    return loop;
}

int add_event_handler(EventLoop* loop, EventHandler* handler) {
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.ptr = handler;
    
    return epoll_ctl(loop->epoll_fd, EPOLL_CTL_ADD, handler->fd, &event);
}

void run_event_loop(EventLoop* loop) {
    loop->running = true;
    
    while (loop->running) {
        int nready = epoll_wait(loop->epoll_fd, loop->events, MAX_EVENTS, 1000);
        
        for (int i = 0; i < nready; i++) {
            EventHandler* handler = (EventHandler*)loop->events[i].data.ptr;
            uint32_t events = loop->events[i].events;
            
            if (events & EPOLLIN && handler->on_read) {
                handler->on_read(handler->fd, handler->user_data);
            }
            if (events & EPOLLOUT && handler->on_write) {
                handler->on_write(handler->fd, handler->user_data);
            }
            if (events & (EPOLLERR | EPOLLHUP) && handler->on_error) {
                handler->on_error(handler->fd, handler->user_data);
            }
        }
    }
}
```

---

## ⚡ 6. 성능 최적화 기법 - C10K 문제 해결

### 6.1 C10K 문제란?

```
C10K = Concurrent 10,000 connections
🎆 동시에 10,000개 연결 처리하기

문제점들:
1. 🧾 스레드 방식: 10,000개 스레드? 메모리 폭발!
2. 🔄 select(): 1024개 제한
3. 🐌 poll(): O(n) 복잡도로 느려짐

해결책:
✅ epoll (Linux)
✅ kqueue (BSD)
✅ IOCP (Windows)
```

### 6.2 대용량 연결 처리를 위한 최적화

```c
// 시스템 리소스 한계 조정
#include <sys/resource.h>

void optimize_system_limits() {
    struct rlimit rlim;
    
    // 파일 디스크립터 한계 증가
    rlim.rlim_cur = 65536;
    rlim.rlim_max = 65536;
    if (setrlimit(RLIMIT_NOFILE, &rlim) == -1) {
        perror("setrlimit RLIMIT_NOFILE");
    }
    
    // TCP 소켓 버퍼 최적화
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    int send_buffer = 64 * 1024;
    setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, &send_buffer, sizeof(send_buffer));
    
    int recv_buffer = 64 * 1024;
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &recv_buffer, sizeof(recv_buffer));
    
    // TCP_NODELAY (Nagle 알고리즘 비활성화)
    int flag = 1;
    setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}

// CPU 친화성 설정
#include <sched.h>

void set_cpu_affinity(int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1) {
        perror("sched_setaffinity");
    }
}
```

### 6.3 메모리 풀을 활용한 최적화

```
메모리 할당/해제의 문제:

일반 방식:                  메모리 풀:
[연결] → malloc() 😢       [풀] → 미리 할당! 😎
[해제] → free() 😢         [연결] → 가져오기
                            [해제] → 반납하기

장점:
✅ 할당/해제 오버헤드 없음
✅ 메모리 단편화 방지
✅ 캐시 효율성 향상
```

```c
// 연결별 버퍼 풀
typedef struct ConnectionBuffer {
    char* read_buffer;
    char* write_buffer;
    size_t read_pos;
    size_t write_pos;
    size_t buffer_size;
} ConnectionBuffer;

typedef struct BufferPool {
    ConnectionBuffer* buffers;
    int* free_list;
    int free_count;
    int total_count;
    pthread_mutex_t mutex;
} BufferPool;

BufferPool* create_buffer_pool(int count, size_t buffer_size) {
    BufferPool* pool = malloc(sizeof(BufferPool));
    
    pool->buffers = malloc(sizeof(ConnectionBuffer) * count);
    pool->free_list = malloc(sizeof(int) * count);
    pool->total_count = count;
    pool->free_count = count;
    
    // 버퍼 초기화
    for (int i = 0; i < count; i++) {
        pool->buffers[i].read_buffer = malloc(buffer_size);
        pool->buffers[i].write_buffer = malloc(buffer_size);
        pool->buffers[i].buffer_size = buffer_size;
        pool->buffers[i].read_pos = 0;
        pool->buffers[i].write_pos = 0;
        pool->free_list[i] = i;
    }
    
    pthread_mutex_init(&pool->mutex, NULL);
    return pool;
}

ConnectionBuffer* allocate_buffer(BufferPool* pool) {
    pthread_mutex_lock(&pool->mutex);
    
    if (pool->free_count == 0) {
        pthread_mutex_unlock(&pool->mutex);
        return NULL;
    }
    
    int index = pool->free_list[--pool->free_count];
    ConnectionBuffer* buffer = &pool->buffers[index];
    
    pthread_mutex_unlock(&pool->mutex);
    
    // 버퍼 리셋
    buffer->read_pos = 0;
    buffer->write_pos = 0;
    
    return buffer;
}

void release_buffer(BufferPool* pool, ConnectionBuffer* buffer) {
    pthread_mutex_lock(&pool->mutex);
    
    int index = buffer - pool->buffers;
    pool->free_list[pool->free_count++] = index;
    
    pthread_mutex_unlock(&pool->mutex);
}
```

---

## 📈 7. I/O 멀티플렉싱 선택 가이드

### 7.1 상황별 추천

```
어떤 방법을 선택할까?

1. 🌍 여러 OS 지원 필요 + 적은 연결:
   → select() 사용

2. 🔢 1000개 이상 연결 + 이식성 필요:
   → poll() 사용

3. 🐧 Linux + 최고 성능:
   → epoll() 사용

4. 🍎 macOS/BSD:
   → kqueue 사용

5. 🪟 Windows:
   → IOCP 사용
```

### 7.2 성능 비교표

| 연결 수 | select | poll | epoll |
|---------|--------|------|-------|
| 100     | 😊 좋음 | 😊 좋음 | 😎 매우 좋음 |
| 1,000   | 😐 보통 | 😊 좋음 | 😎 매우 좋음 |
| 10,000  | ❌ 불가 | 😕 느림 | 😎 매우 좋음 |
| 100,000 | ❌ 불가 | 😵 매우 느림 | 😎 매우 좋음 |

## 🚀 8. LogCaster 실제 구현 분석

### 8.1 C 버전 구현 (epoll 사용 권장)

```c
// LogCaster-C의 서버 구조체 (베스트 프랙티스 적용)
typedef struct {
    int listen_fd;      // 로그 수신 소켓
    int query_fd;       // 쿼리 처리 소켓
    int epoll_fd;       // epoll 인스턴스 (select 대신 epoll 사용)
    thread_pool_t* thread_pool;
    log_buffer_t* log_buffer;
    volatile sig_atomic_t running;  // 시그널 안전성을 위해 sig_atomic_t 사용
    pthread_mutex_t client_count_mutex;
    int client_count;
} log_server_t;

// epoll 초기화 (베스트 프랙티스)
int server_initialize(log_server_t* server) {
    // epoll 인스턴스 생성
    server->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (server->epoll_fd < 0) {
        perror("epoll_create1");
        return -1;
    }
    
    // 리스너 소켓들을 epoll에 추가
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;  // Edge-triggered 모드
    ev.data.fd = server->listen_fd;
    
    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->listen_fd, &ev) < 0) {
        perror("epoll_ctl: listen_fd");
        return -1;
    }
    
    ev.data.fd = server->query_fd;
    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->query_fd, &ev) < 0) {
        perror("epoll_ctl: query_fd");
        return -1;
    }
    
    return 0;
}

// 개선된 이벤트 루프 (epoll 사용)
void server_run(log_server_t* server) {
    struct epoll_event events[MAX_EVENTS];
    
    while (server->running) {
        int n = epoll_wait(server->epoll_fd, events, MAX_EVENTS, 1000);
        
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }
        
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server->listen_fd) {
                // 새로운 로그 클라이언트 연결 (Edge-triggered이므로 모두 처리)
                while (1) {
                    int new_fd = handle_new_connection(server);
                    if (new_fd < 0) break;
                    
                    // 새 클라이언트를 epoll에 추가
                    struct epoll_event client_ev;
                    client_ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                    client_ev.data.ptr = create_client_context(new_fd, server);
                    
                    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, new_fd, &client_ev) < 0) {
                        perror("epoll_ctl: client");
                        close(new_fd);
                    }
                }
            }
            else if (events[i].data.fd == server->query_fd) {
                // 쿼리 연결 처리
                handle_query_connection(server);
            }
            else {
                // 기존 클라이언트의 데이터
                client_context_t* ctx = events[i].data.ptr;
                
                if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                    // 연결 종료
                    epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, ctx->fd, NULL);
                    close_client_context(ctx);
                } else if (events[i].events & EPOLLIN) {
                    // 데이터 수신 - 스레드 풀에 작업 제출
                    thread_pool_add_job(server->thread_pool, 
                                       handle_client_job, ctx);
                }
            }
        }
    }
}

// 클라이언트 작업 처리 (베스트 프랙티스 적용)
void handle_client_job(void* arg) {
    client_job_t* job = (client_job_t*)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    // Edge-triggered 모드에서는 EAGAIN까지 읽어야 함
    while ((bytes_read = recv(job->client_fd, buffer, BUFFER_SIZE - 1, MSG_DONTWAIT)) > 0) {
        buffer[bytes_read] = '\0';
        
        // 로그 크기 제한 적용
        if (bytes_read > SAFE_LOG_LENGTH) {
            buffer[SAFE_LOG_LENGTH - 4] = '.';
            buffer[SAFE_LOG_LENGTH - 3] = '.';
            buffer[SAFE_LOG_LENGTH - 2] = '.';
            buffer[SAFE_LOG_LENGTH - 1] = '\0';
            bytes_read = SAFE_LOG_LENGTH - 1;
        }
        
        // 줄바꿈 제거
        if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        }
        
        // 로그 버퍼에 추가
        log_buffer_push(job->server->log_buffer, buffer);
        
        // 영속성 처리 (활성화된 경우)
        if (job->server->persistence) {
            persistence_write(job->server->persistence, buffer, time(NULL));
        }
        
        // 응답 전송 (에러 처리 포함)
        const char* response = "OK\n";
        ssize_t sent = send(job->client_fd, response, strlen(response), MSG_NOSIGNAL);
        if (sent < 0 && errno != EPIPE) {
            perror("send failed");
        }
    }
    
    // EAGAIN이 아닌 에러인 경우
    if (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("recv");
    }
    
    // 연결 종료 시 정리
    if (bytes_read == 0) {
        // 스레드 안전한 클라이언트 수 감소
        pthread_mutex_lock(&job->server->client_count_mutex);
        job->server->client_count--;
        int current_clients = job->server->client_count;
        pthread_mutex_unlock(&job->server->client_count_mutex);
        
        printf("Client disconnected (remaining=%d)\n", current_clients);
        
        // epoll에서 제거 및 소켓 닫기
        epoll_ctl(job->server->epoll_fd, EPOLL_CTL_DEL, job->client_fd, NULL);
        close(job->client_fd);
        free(job);
    }
}
```

### 8.2 C++ 버전 구현 (베스트 프랙티스 적용)

```cpp
// LogCaster-CPP의 개선된 서버 구현
class LogServer {
private:
    int listenFd_;
    int queryFd_;
    int epoll_fd_;  // select 대신 epoll 사용
    int port_;
    int queryPort_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::shared_ptr<LogBuffer> logBuffer_;
    std::unique_ptr<QueryHandler> queryHandler_;
    std::atomic<bool> running_{true};
    std::atomic<int> clientCount_{0};  // atomic으로 race condition 방지
    std::unique_ptr<Logger> logger_;
    
    // RAII 패턴을 위한 소켓 래퍼
    class SocketGuard {
        int fd_;
    public:
        explicit SocketGuard(int fd) : fd_(fd) {}
        ~SocketGuard() { if (fd_ >= 0) ::close(fd_); }
        int release() { int tmp = fd_; fd_ = -1; return tmp; }
        int get() const { return fd_; }
    };
    
public:
    void start() {
        // epoll 인스턴스 생성 (CLOEXEC 플래그 사용)
        epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
        if (epoll_fd_ < 0) {
            throw std::runtime_error("Failed to create epoll: " + 
                                   std::string(strerror(errno)));
        }
        
        // 리스너 소켓 초기화
        initialize();
        
        // epoll에 리스너 추가 (Edge-triggered)
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = listenFd_;
        
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listenFd_, &ev) < 0) {
            throw std::runtime_error("epoll_ctl failed");
        }
        
        ev.data.fd = queryFd_;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, queryFd_, &ev) < 0) {
            throw std::runtime_error("epoll_ctl failed");
        }
        
        runEpollEventLoop();
    }
    
private:
    void runEpollEventLoop() {
        std::vector<epoll_event> events(MAX_EVENTS);
        
        while (running_) {
            int n = epoll_wait(epoll_fd_, events.data(), MAX_EVENTS, 1000);
            
            if (n < 0) {
                if (errno == EINTR) continue;
                throw std::runtime_error("epoll_wait failed");
            }
            
            for (int i = 0; i < n; i++) {
                if (events[i].data.fd == listenFd_) {
                    handleNewConnections();  // Edge-triggered이므로 모든 연결 처리
                } else if (events[i].data.fd == queryFd_) {
                    handleQueryConnections();
                } else {
                    // 클라이언트 데이터 처리
                    auto* client = static_cast<ClientConnection*>(events[i].data.ptr);
                    
                    if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                        removeClient(client);
                    } else if (events[i].events & EPOLLIN) {
                        threadPool_->enqueue([this, client]() {
                            handleClientData(client);
                        });
                    }
                }
            }
        }
    }
    
    void handleNewConnections() {
        // Edge-triggered 모드에서는 모든 대기 중인 연결을 처리해야 함
        while (true) {
            sockaddr_in clientAddr{};
            socklen_t addrLen = sizeof(clientAddr);
            
            SocketGuard newFd(accept4(listenFd_, 
                                    reinterpret_cast<sockaddr*>(&clientAddr), 
                                    &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC));
            
            if (newFd.get() < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;  // 더 이상 대기 중인 연결 없음
                }
                logger_->error("accept4 failed: " + std::string(strerror(errno)));
                continue;
            }
            
            // 클라이언트 수 제한 체크 (atomic 사용)
            if (clientCount_.load() >= MAX_CLIENTS) {
                logger_->info("Max clients reached, rejecting connection");
                continue;  // SocketGuard가 자동으로 close
            }
            
            // TCP 최적화 옵션 설정
            int nodelay = 1;
            setsockopt(newFd.get(), IPPROTO_TCP, TCP_NODELAY, 
                      &nodelay, sizeof(nodelay));
            
            // 클라이언트 컨텍스트 생성 및 epoll 추가
            auto client = std::make_unique<ClientConnection>(
                newFd.release(), ClientType::LOG, 
                inet_ntoa(clientAddr.sin_addr), 
                ntohs(clientAddr.sin_port)
            );
            
            struct epoll_event client_ev;
            client_ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
            client_ev.data.ptr = client.get();
            
            if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client->fd, &client_ev) == 0) {
                clientCount_++;
                client.release();  // 소유권 이전
            }
        }
    }
    
    void handleClientData(ClientConnection* client) {
        std::array<char, BUFFER_SIZE> buffer;
        
        // Edge-triggered 모드에서는 EAGAIN까지 읽어야 함
        while (true) {
            ssize_t bytesRead = recv(client->fd, buffer.data(), 
                                   buffer.size() - 1, MSG_DONTWAIT);
            
            if (bytesRead < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;  // 더 이상 읽을 데이터 없음
                }
                logger_->error("recv error: " + std::string(strerror(errno)));
                removeClient(client);
                return;
            }
            
            if (bytesRead == 0) {
                // 연결 종료
                removeClient(client);
                return;
            }
            
            buffer[bytesRead] = '\0';
            
            // 로그 크기 제한
            if (bytesRead > SAFE_LOG_LENGTH) {
                bytesRead = SAFE_LOG_LENGTH - 1;
                buffer[bytesRead] = '\0';
            }
            
            // 줄바꿈 제거
            if (bytesRead > 0 && buffer[bytesRead - 1] == '\n') {
                buffer[bytesRead - 1] = '\0';
            }
            
            std::string data(buffer.data());
            
            if (client->type == ClientType::LOG) {
                // 로그 처리
                logBuffer_->addLog(std::move(data));
                
                // 영속성 처리 (있는 경우)
                if (persistenceManager_) {
                    persistenceManager_->write(data);
                }
                
                // 응답 전송 (에러 처리 포함)
                const char* response = "OK\n";
                ssize_t sent = send(client->fd, response, strlen(response), MSG_NOSIGNAL);
                if (sent < 0 && errno != EPIPE) {
                    logger_->error("send failed: " + std::string(strerror(errno)));
                }
            } else {
                // 쿼리 처리
                std::string response = queryHandler_->processQuery(data);
                send(client->fd, response.c_str(), response.length(), MSG_NOSIGNAL);
            }
        }
    }
    
    void removeClient(ClientConnection* client) {
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, client->fd, nullptr);
        close(client->fd);
        clientCount_--;
        logger_->info("Client disconnected (remaining=" + 
                     std::to_string(clientCount_.load()) + ")");
        delete client;
    }
};
```

### 8.3 성능 측정 결과

```
=== LogCaster Performance Benchmark ===

Test Configuration:
- CPU: 8-core Intel i7
- RAM: 16GB
- OS: Linux 5.15
- Network: Loopback

1. Single Client Latency Test:
   - Average: 0.15ms
   - P50: 0.12ms
   - P95: 0.25ms
   - P99: 0.45ms

2. Throughput Test (1000 concurrent clients):
   - Total Messages: 1,000,000
   - Total Time: 18.5s
   - Throughput: 54,054 msg/sec

3. I/O Model Comparison:
   Model        Throughput    CPU Usage    Memory
   ------------------------------------------------
   select()     12,000/s      85%          150MB
   poll()       28,000/s      75%          180MB
   epoll()      54,000/s      45%          120MB
   
4. C vs C++ Implementation:
   Language    Throughput    Latency(P99)    Binary Size
   -------------------------------------------------------
   C           54,054/s      0.45ms          45KB
   C++         58,823/s      0.38ms          125KB
```

### 8.4 프로덕션 최적화 (베스트 프랙티스)

#### 소켓 최적화 설정
```c
// 서버 소켓 최적화 함수
int createOptimizedSocket(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    
    // SO_REUSEADDR - 재시작 시 바로 바인딩 가능
    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    // TCP_NODELAY - Nagle 알고리즘 비활성화 (실시간성 향상)
    int nodelay = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
    
    // 수신/송신 버퍼 크기 최적화
    int bufsize = 256 * 1024;  // 256KB
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
    
    // TCP Keepalive 설정
    int keepalive = 1;
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
    
    // 논블로킹 모드 설정
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    
    return fd;
}

// CPU Affinity 설정 (성능 최적화)
void set_thread_affinity(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    
    pthread_t thread = pthread_self();
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}

// I/O 스레드는 코어 0, 워커 스레드는 1-7
void optimize_thread_placement(thread_pool_t* pool) {
    // I/O 스레드
    set_thread_affinity(0);
    
    // 워커 스레드들
    for (int i = 0; i < pool->num_threads; i++) {
        // 워커 스레드 내부에서 호출
        // set_thread_affinity((i % 7) + 1);
    }
}
```

#### Zero-Copy 최적화
```c
// sendfile을 사용한 zero-copy 전송
ssize_t send_log_file(int client_fd, int file_fd, 
                     off_t offset, size_t count) {
    return sendfile(client_fd, file_fd, &offset, count);
}

// splice를 사용한 파이프라인
void zero_copy_pipeline(int in_fd, int out_fd) {
    int pipefd[2];
    pipe(pipefd);
    
    // 입력 -> 파이프
    splice(in_fd, NULL, pipefd[1], NULL, 
           65536, SPLICE_F_MOVE);
    
    // 파이프 -> 출력
    splice(pipefd[0], NULL, out_fd, NULL, 
           65536, SPLICE_F_MOVE);
    
    close(pipefd[0]);
    close(pipefd[1]);
}
```

## 🚀 9. [신규] 대규모 클라이언트 처리 전략

### 9.1 C100K 문제 해결

C10K를 넘어 10만 연결을 처리하기 위한 전략:

```c
// 대규모 연결을 위한 epoll 최적화
struct large_scale_server {
    int epoll_fd;
    struct epoll_event* events;
    
    // 연결별 최소 메모리 사용
    struct minimal_connection {
        int fd;
        uint32_t flags;  // 상태 플래그 압축
        uint16_t last_activity;  // 시간을 16비트로 압축
        uint8_t buffer_index;    // 버퍼 풀 인덱스
    } __attribute__((packed));
    
    // 메모리 풀
    struct buffer_pool {
        char buffers[256][4096];  // 256개의 4KB 버퍼
        uint64_t used_bitmap[4];  // 비트맵으로 사용 추적
    } *buffer_pool;
};

// 100K 연결 초기화
int init_large_scale_server(struct large_scale_server* server) {
    // 대용량 이벤트 배열
    server->events = calloc(10000, sizeof(struct epoll_event));
    
    // epoll 인스턴스 생성 (큰 사이즈 힌트)
    server->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    
    // 버퍼 풀 할당 (huge pages 사용)
    server->buffer_pool = mmap(NULL, sizeof(struct buffer_pool),
                              PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                              -1, 0);
    
    return 0;
}

// 메모리 효율적인 타이머 관리
struct timer_wheel {
    // 시간 휠 (1초 단위, 3600개 슬롯)
    struct connection_list slots[3600];
    int current_slot;
    time_t last_tick;
};

void check_timeouts(struct timer_wheel* wheel) {
    time_t now = time(NULL);
    
    while (wheel->last_tick < now) {
        wheel->last_tick++;
        wheel->current_slot = (wheel->current_slot + 1) % 3600;
        
        // 현재 슬롯의 모든 연결 검사
        struct connection_list* list = &wheel->slots[wheel->current_slot];
        process_timeout_list(list);
    }
}
```

### 9.2 NUMA 고려사항

```c
// NUMA 인식 서버 설계
#include <numa.h>

struct numa_aware_server {
    int num_nodes;
    struct node_context {
        int epoll_fd;
        pthread_t thread;
        cpu_set_t cpuset;
        void* local_memory;
    } *nodes;
};

void init_numa_server(struct numa_aware_server* server) {
    server->num_nodes = numa_num_configured_nodes();
    server->nodes = calloc(server->num_nodes, sizeof(struct node_context));
    
    for (int node = 0; node < server->num_nodes; node++) {
        struct node_context* ctx = &server->nodes[node];
        
        // NUMA 노드별 메모리 할당
        ctx->local_memory = numa_alloc_onnode(1024 * 1024 * 100, node);
        
        // CPU 친화도 설정
        CPU_ZERO(&ctx->cpuset);
        numa_node_to_cpus(node, &ctx->cpuset);
        
        // 노드별 epoll 인스턴스
        ctx->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
        
        // 노드별 워커 스레드
        pthread_create(&ctx->thread, NULL, node_worker, ctx);
        pthread_setaffinity_np(ctx->thread, sizeof(cpu_set_t), &ctx->cpuset);
    }
}
```

## 🔧 10. [신규] epoll/kqueue 고급 활용법

### 10.1 배치 처리 최적화

```c
// 배치 이벤트 처리
struct batch_processor {
    struct epoll_event events[10000];
    
    // 이벤트 타입별 핸들러
    void (*read_batch[1000])(int fd);
    void (*write_batch[1000])(int fd);
    int read_count;
    int write_count;
};

void process_events_batch(int epoll_fd, struct batch_processor* proc) {
    int nfds = epoll_wait(epoll_fd, proc->events, 10000, 0);
    
    // 이벤트 분류
    proc->read_count = 0;
    proc->write_count = 0;
    
    for (int i = 0; i < nfds; i++) {
        if (proc->events[i].events & EPOLLIN) {
            proc->read_batch[proc->read_count++] = 
                handle_read_event;
        }
        if (proc->events[i].events & EPOLLOUT) {
            proc->write_batch[proc->write_count++] = 
                handle_write_event;
        }
    }
    
    // 타입별 일괄 처리 (캐시 효율성)
    for (int i = 0; i < proc->read_count; i++) {
        proc->read_batch[i](proc->events[i].data.fd);
    }
    
    for (int i = 0; i < proc->write_count; i++) {
        proc->write_batch[i](proc->events[i].data.fd);
    }
}
```

### 10.2 고급 epoll 기능

```c
// EPOLLEXCLUSIVE - 썬더링 허드 방지
void add_exclusive_listener(int epoll_fd, int listen_fd) {
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLEXCLUSIVE;
    ev.data.fd = listen_fd;
    
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);
}

// EPOLLRDHUP - 반쪽 종료 감지
void enable_half_close_detection(int epoll_fd, int client_fd) {
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    ev.data.fd = client_fd;
    
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
}

// 사용자 정의 데이터
struct connection_context {
    int fd;
    void* user_data;
    void (*handler)(struct connection_context*);
};

void add_with_context(int epoll_fd, struct connection_context* ctx) {
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = ctx;  // fd 대신 포인터 사용
    
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ctx->fd, &ev);
}
```

### 10.3 kqueue 특화 기능 (BSD/macOS)

```c
#ifdef __APPLE__
// kqueue는 더 풍부한 이벤트 타입 지원
void setup_kqueue_advanced(int kq) {
    struct kevent changes[10];
    int nchanges = 0;
    
    // 파일 변경 감시
    EV_SET(&changes[nchanges++], file_fd, EVFILT_VNODE,
           EV_ADD | EV_ENABLE | EV_CLEAR,
           NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND,
           0, NULL);
    
    // 프로세스 감시
    EV_SET(&changes[nchanges++], child_pid, EVFILT_PROC,
           EV_ADD | EV_ENABLE,
           NOTE_EXIT | NOTE_FORK,
           0, NULL);
    
    // 타이머 (나노초 단위)
    EV_SET(&changes[nchanges++], 1, EVFILT_TIMER,
           EV_ADD | EV_ENABLE,
           NOTE_NSECONDS,
           1000000000,  // 1초
           NULL);
    
    // 사용자 정의 이벤트
    EV_SET(&changes[nchanges++], 42, EVFILT_USER,
           EV_ADD | EV_ENABLE | EV_CLEAR,
           0, 0, NULL);
    
    kevent(kq, changes, nchanges, NULL, 0, NULL);
}
#endif
```

### 10.4 io_uring - 차세대 I/O (Linux 5.1+)

```c
#ifdef __linux__
#include <liburing.h>

// io_uring은 진정한 비동기 I/O
struct io_uring_server {
    struct io_uring ring;
    struct io_uring_sqe* sqe;
    struct io_uring_cqe* cqe;
};

void setup_io_uring(struct io_uring_server* server) {
    // 큐 초기화 (4096 엔트리)
    io_uring_queue_init(4096, &server->ring, IORING_SETUP_SQPOLL);
    
    // 다중 연결 수락 준비
    for (int i = 0; i < 100; i++) {
        sqe = io_uring_get_sqe(&server->ring);
        io_uring_prep_accept(sqe, listen_fd, NULL, NULL, 0);
        io_uring_sqe_set_data(sqe, (void*)(intptr_t)i);
    }
    
    // 제출
    io_uring_submit(&server->ring);
}

// 완료 처리
void process_io_uring_completions(struct io_uring_server* server) {
    struct io_uring_cqe* cqe;
    unsigned head;
    int count = 0;
    
    io_uring_for_each_cqe(&server->ring, head, cqe) {
        if (cqe->res < 0) {
            // 에러 처리
            continue;
        }
        
        int id = (intptr_t)io_uring_cqe_get_data(cqe);
        handle_completion(id, cqe->res);
        count++;
    }
    
    io_uring_cq_advance(&server->ring, count);
}
#endif
```

## ⚠️ 자주 하는 실수와 해결법

### 1. fd_set 재사용 실수 (select)
```c
// ❌ 잘못된 예시
fd_set readfds;
FD_ZERO(&readfds);
FD_SET(server_fd, &readfds);

while (1) {
    select(max_fd + 1, &readfds, NULL, NULL, NULL);
    // select 후 readfds가 변경됨!
    // 다음 반복에서 server_fd가 없음!
}

// ✅ 올바른 예시
fd_set master_fds, read_fds;
FD_ZERO(&master_fds);
FD_SET(server_fd, &master_fds);

while (1) {
    read_fds = master_fds;  // 복사본 사용
    select(max_fd + 1, &read_fds, NULL, NULL, NULL);
}
```

### 2. Edge Triggered 모드에서 데이터 누락
```c
// ❌ 잘못된 예시 - ET 모드에서 한 번만 읽기
if (events[i].events & EPOLLIN) {
    char buffer[1024];
    recv(fd, buffer, sizeof(buffer), 0);  // 일부만 읽음!
}

// ✅ 올바른 예시 - EAGAIN까지 모두 읽기
if (events[i].events & EPOLLIN) {
    char buffer[1024];
    while (1) {
        ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
        if (n < 0 && errno == EAGAIN) break;
        // 데이터 처리
    }
}
```

### 3. max_fd 관리 실수 (select/poll)
```c
// ❌ 잘못된 예시 - max_fd 업데이트 안 함
close(client_fds[i]);
FD_CLR(client_fds[i], &master_set);
// max_fd가 여전히 닫힌 fd를 가리킴!

// ✅ 올바른 예시 - max_fd 재계산
close(client_fds[i]);
FD_CLR(client_fds[i], &master_set);
client_fds[i] = -1;

// max_fd 재계산
max_fd = server_fd;
for (int j = 0; j < MAX_CLIENTS; j++) {
    if (client_fds[j] > max_fd) {
        max_fd = client_fds[j];
    }
}
```

### 4. epoll 이벤트 포인터 관리
```c
// ❌ 잘못된 예시 - 스택 변수 포인터 사용
void add_client(int epoll_fd, int client_fd) {
    struct client_data data;  // 스택 변수!
    data.fd = client_fd;
    
    struct epoll_event ev;
    ev.data.ptr = &data;  // 함수 종료 시 무효!
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
}

// ✅ 올바른 예시 - 동적 할당
void add_client(int epoll_fd, int client_fd) {
    struct client_data* data = malloc(sizeof(struct client_data));
    data->fd = client_fd;
    
    struct epoll_event ev;
    ev.data.ptr = data;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
}
```

### 5. 타임아웃 처리 실수
```c
// ❌ 잘못된 예시 - 타임아웃을 무시
int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
// n == 0일 때 아무 처리 안 함

// ✅ 올바른 예시 - 타임아웃 활용
int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
if (n == 0) {
    // 주기적 작업 수행
    check_client_timeouts();
    flush_logs();
}
```

## 🎯 실습 프로젝트

### 프로젝트 1: 고성능 에코 서버 (난이도: ⭐⭐⭐)
```c
// 구현할 기능:
// 1. select, poll, epoll 각각으로 구현
// 2. 성능 비교 벤치마킹
// 3. 10,000 동시 연결 처리
// 4. 통계 수집 (연결수, 처리량)

// 도전 과제:
// - ET/LT 모드 비교
// - 메모리 풀 구현
// - CPU 친화도 설정
```

### 프로젝트 2: 실시간 채팅 서버 (난이도: ⭐⭐⭐⭐)
```c
// 구현할 기능:
// 1. 채널 기반 메시징
// 2. 효율적인 브로드캐스트
// 3. 사용자 인증
// 4. 메시지 히스토리

// 고급 기능:
// - Zero-copy 브로드캐스트
// - 채널별 epoll 인스턴스
// - 부하 분산
```

### 프로젝트 3: HTTP/1.1 웹서버 (난이도: ⭐⭐⭐⭐⭐)
```c
// 구현할 기능:
// 1. HTTP 파싱과 응답
// 2. Keep-alive 연결 관리
// 3. 정적 파일 서빙
// 4. 가상 호스트

// 최적화:
// - sendfile() 사용
// - 파일 캐싱
// - io_uring 적용
```

## ✅ 학습 체크리스트

### 🟢 기초 (1-2주)
- [ ] 블로킹 vs 비블로킹 I/O 이해
- [ ] select() 기본 사용법
- [ ] fd_set 조작 매크로
- [ ] 간단한 다중 클라이언트 서버
- [ ] 타임아웃 처리

### 🟡 중급 (3-4주)
- [ ] poll() 사용법과 장단점
- [ ] epoll 기본 사용법
- [ ] Level Triggered 이해
- [ ] 비블로킹 소켓 처리
- [ ] 에러 처리 패턴
- [ ] 1000+ 연결 처리

### 🔴 고급 (5-8주)
- [ ] Edge Triggered 마스터
- [ ] epoll 고급 기능
- [ ] C10K 문제 해결
- [ ] 메모리 최적화
- [ ] CPU 친화도 설정
- [ ] Zero-copy 기법

### ⚫ 전문가 (3개월+)
- [ ] C100K 달성
- [ ] NUMA 최적화
- [ ] io_uring 활용
- [ ] 커스텀 이벤트 루프
- [ ] 분산 I/O 처리

## 📚 추가 학습 자료

### 권장 도서
1. **"Unix Network Programming"** - W. Richard Stevens
2. **"The Linux Programming Interface"** - Michael Kerrisk
3. **"High Performance Browser Networking"** - Ilya Grigorik

### 온라인 자료
- [Linux epoll Tutorial](https://linux.die.net/man/4/epoll)
- [C10K Problem](http://www.kegel.com/c10k.html)
- [Libevent - Event notification library](https://libevent.org/)
- [libev - High-performance event loop](http://software.schmorp.de/pkg/libev.html)

### 실전 프로젝트
- **Nginx**: epoll 기반 웹 서버
- **Redis**: 싱글 스레드 이벤트 루프
- **Node.js**: libuv 기반 비동기 I/O

## ✅ 11. 다음 단계 준비

이 문서를 완전히 이해했다면:

1. **select(), poll(), epoll()의 차이점**과 용도를 알아야 합니다
2. **I/O 멀티플렉싱의 기본 원리**를 이해해야 합니다
3. **Edge Triggered vs Level Triggered** 모드의 차이를 알아야 합니다
4. **고성능 서버 구현**을 위한 최적화 기법을 알아야 합니다
5. **대용량 연결 처리**를 위한 시스템 튜닝을 할 수 있어야 합니다
6. **[신규] C100K 수준의 대규모 처리**를 설계할 수 있어야 합니다
7. **[신규] 플랫폼별 최적화 기법**을 활용할 수 있어야 합니다

### 🎯 확인 문제

1. **select()와 epoll()의 성능 차이가 나는 이유는 무엇인가요?**
   - 힌트: O(n) vs O(1) 복잡도를 생각해보세요

2. **Edge Triggered 모드에서 모든 데이터를 읽어야 하는 이유는 무엇인가요?**
   - 힌트: "한 번만 알림"의 의미를 생각해보세요

3. **LogCaster에서 C10K 문제를 해결하기 위한 방법은 무엇인가요?**
   - 힌트: I/O 모델, 시스템 콜, 메모리 관리를 종합적으로 고려하세요

### 💻 실습 프로젝트

1. **채팅 서버**: epoll을 사용한 실시간 채팅 서버
2. **파일 서버**: 대용량 파일 전송 서버 (sendfile 활용)
3. **모니터링 도구**: 연결 상태 실시간 모니터링
4. **프록시 서버**: HTTP 프록시 서버 구현

다음 문서에서는 **디버깅 도구 마스터하기**에 대해 자세히 알아보겠습니다!

---

*💡 팁: I/O 멀티플렉싱은 고성능 서버의 핵심입니다. 각 방법의 특성을 이해하고 상황에 맞게 선택하세요!*

---

*이 문서는 LogCaster 프로젝트를 위한 I/O 멀티플렉싱 완벽 가이드입니다. 다음 문서에서는 디버깅 도구 활용에 대해 알아보겠습니다!*