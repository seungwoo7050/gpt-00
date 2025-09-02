# 7. 네트워킹 기초 🌐
## TCP/IP와 소켓 프로그래밍
*LogCaster 프로젝트를 위한 완벽 가이드*

---

## 📋 문서 정보
- **난이도**: 🟡 중급
- **예상 학습 시간**: 8-10시간
- **전제 조건**: 
  - C 기본 문법
  - 포인터와 구조체 이해
  - 기본적인 운영체제 개념
  - 파일 디스크립터 개념
- **실습 환경**: Linux/macOS, GCC/G++, 네트워크 연결

## 🎯 이 문서에서 배울 내용

LogCaster는 네트워크를 통해 로그 데이터를 수집하고 조회하는 서버입니다. 이 문서에서는 TCP/IP 프로토콜의 기본 개념부터 실제 소켓 프로그래밍까지, 네트워킹의 모든 기초를 다룹니다. 완전한 초보자도 네트워크 프로그래밍을 이해하고 LogCaster를 구현할 수 있도록 단계별로 설명합니다.

### 학습 목표
- TCP/IP 프로토콜의 작동 원리 이해
- 소켓 프로그래밍의 기본 개념 습득
- 클라이언트-서버 통신 구현
- 안전하고 효율적인 네트워크 프로그램 작성
- LogCaster의 네트워킹 요구사항 구현
- **[신규] 멀티 포트 서버 관리 및 프로토콜 멀티플렉싱**
- **[신규] IRC 프로토콜 통합을 위한 네트워킹 패턴**

---

## 🌐 1. TCP/IP 프로토콜의 이해

### 1.1 네트워킹이란?

네트워킹을 일상생활에 비유하면 우편 시스템과 유사합니다:

```
현실 세계                    네트워크 세계
-----------                  -------------
편지 봉투                 →  패킷(Packet)
수신자 주소              →  IP 주소
우편번호                 →  포트 번호
우체국                   →  라우터
우편배달부               →  네트워크 프로토콜
```

#### 실생활 예시로 이해하기

```c
// 편지를 보내는 과정
1. 편지 작성 (데이터 생성)
2. 봉투에 넣기 (패킷화)
3. 주소 작성 (IP:Port 지정)
4. 우체국에 전달 (네트워크 전송)
5. 수신자가 받음 (데이터 수신)

// 네트워크 통신
1. 메시지 생성: "Hello, Server!"
2. TCP 패킷으로 포장
3. 192.168.1.100:9999로 전송
4. 네트워크를 통해 라우팅
5. 서버가 메시지 수신
```

### 1.2 네트워크 계층 구조

#### OSI 7계층 vs TCP/IP 4계층

```
┌─────────────────────────────────────────────────────────┐
│                   실제 데이터의 여행                      │
├─────────────────────────────────────────────────────────┤
│  응용 프로그램 (LogCaster)                              │
│      ↓ "로그 메시지 전송해줘"                           │
│  응용 계층 (HTTP, FTP, SSH...)                          │
│      ↓ 프로토콜 형식으로 포장                           │
│  전송 계층 (TCP/UDP)                                     │
│      ↓ 포트 번호 추가, 순서 보장                        │
│  인터넷 계층 (IP)                                        │
│      ↓ IP 주소 추가, 경로 결정                          │
│  네트워크 접근 계층 (Ethernet, WiFi...)                  │
│      ↓ 물리적 신호로 변환                               │
│  물리적 네트워크 (케이블, 전파...)                       │
└─────────────────────────────────────────────────────────┘
```

#### 각 계층의 역할 상세 설명

```c
// 4. 응용 계층 (Application Layer)
// - 사용자가 직접 사용하는 프로그램
// - LogCaster가 여기에 해당
char* log_message = "ERROR: Database connection failed";

// 3. 전송 계층 (Transport Layer)
// - TCP: 신뢰성 있는 전송 (편지는 반드시 도착!)
// - UDP: 빠른 전송 (편지가 없어져도 괜찮아)
struct tcp_header {
    uint16_t source_port;      // 보내는 포트
    uint16_t destination_port; // 받는 포트
    uint32_t sequence_number;  // 순서 번호
    // ... 더 많은 필드들
};

// 2. 인터넷 계층 (Internet Layer)
// - IP 주소로 목적지 찾기
// - 라우터가 길을 안내
struct ip_header {
    uint32_t source_ip;      // 보내는 IP
    uint32_t destination_ip; // 받는 IP
    // ... 더 많은 필드들
};

// 1. 네트워크 접근 계층 (Network Access Layer)
// - 실제 물리적 전송
// - 이더넷, WiFi 등
```

### 1.3 TCP vs UDP 심화 이해

#### TCP (Transmission Control Protocol)

TCP는 전화 통화와 같습니다:

```c
// TCP 연결 과정을 코드로 표현
void tcp_connection_example() {
    // 1. 전화 걸기 (3-way handshake)
    call_number("010-1234-5678");  // SYN
    wait_for_answer();              // SYN+ACK
    say_hello();                    // ACK
    
    // 2. 대화하기 (데이터 전송)
    send_message("안녕하세요?");
    receive_confirmation("네, 들렸습니다");
    
    // 3. 전화 끊기 (연결 종료)
    say_goodbye();                  // FIN
    wait_for_goodbye();             // ACK
}
```

**TCP의 특징을 실제 코드로 이해하기:**

```c
// TCP의 신뢰성 보장 메커니즘
typedef struct {
    uint32_t seq_num;      // 순서 번호
    uint32_t ack_num;      // 확인 번호
    uint16_t window_size;  // 수신 가능한 데이터 크기
    uint16_t checksum;     // 오류 검사
} tcp_reliability_t;

// 예시: TCP가 데이터를 보내는 과정
void tcp_send_with_reliability(int socket, const char* data) {
    // 1. 데이터를 작은 조각(세그먼트)으로 나누기
    // 2. 각 조각에 순서 번호 부여
    // 3. 전송 후 ACK 대기
    // 4. ACK가 오지 않으면 재전송
    // 5. 모든 데이터가 전송될 때까지 반복
}
```

```
클라이언트          서버
    |               |
    |---SYN-------->|  1. 연결 요청
    |<--SYN+ACK-----|  2. 연결 수락 + 확인
    |---ACK-------->|  3. 최종 확인
    |               |
    |===데이터 전송===|
    |               |
    |---FIN-------->|  4. 연결 종료
    |<--ACK---------|  5. 종료 확인
```

#### UDP (User Datagram Protocol)
**특징:**
- **비연결형**: 사전 연결 설정 없음
- **빠른 속도**: 오버헤드가 적음
- **신뢰성 없음**: 데이터 손실 가능
- **간단함**: 최소한의 기능만 제공

### 1.4 IP 주소와 포트의 이해

#### IP 주소 체계

```c
// IPv4 주소 구조 (32비트 = 4바이트)
// 예: 192.168.1.100
struct ipv4_address {
    uint8_t octet1;  // 192
    uint8_t octet2;  // 168
    uint8_t octet3;  // 1
    uint8_t octet4;  // 100
};

// 특수 IP 주소들의 의미
#define LOCALHOST     "127.0.0.1"    // 자기 자신 (루프백)
#define ANY_ADDR      "0.0.0.0"      // 모든 인터페이스
#define BROADCAST     "255.255.255.255" // 브로드캐스트

// 사설 IP 대역 (인터넷에 직접 연결 안 됨)
// 10.0.0.0    ~ 10.255.255.255   (클래스 A)
// 172.16.0.0  ~ 172.31.255.255   (클래스 B)
// 192.168.0.0 ~ 192.168.255.255  (클래스 C)
```

#### 포트 번호의 이해

포트는 아파트의 호수와 같습니다:

```c
// 포트 번호 체계 (16비트 = 0~65535)
typedef enum {
    // Well-known ports (0-1023)
    PORT_FTP_DATA    = 20,
    PORT_FTP_CONTROL = 21,
    PORT_SSH         = 22,
    PORT_TELNET      = 23,
    PORT_SMTP        = 25,
    PORT_HTTP        = 80,
    PORT_HTTPS       = 443,
    
    // Registered ports (1024-49151)
    PORT_MYSQL       = 3306,
    PORT_POSTGRESQL  = 5432,
    PORT_REDIS       = 6379,
    
    // Dynamic/Private ports (49152-65535)
    PORT_LOGCRAFTER_LOG   = 9999,  // 로그 수집
    PORT_LOGCRAFTER_QUERY = 9998   // 로그 조회
} common_ports_t;

// 실제 사용 예시
void explain_ports() {
    printf("IP 주소 = 아파트 건물 주소\n");
    printf("포트 번호 = 아파트 호수\n");
    printf("\n");
    printf("192.168.1.100:80   = 건물에서 80호 (웹서버)\n");
    printf("192.168.1.100:3306 = 건물에서 3306호 (MySQL)\n");
    printf("192.168.1.100:9999 = 건물에서 9999호 (LogCaster)\n");
}
```

---

## 🔌 2. 소켓 프로그래밍 기초

### 2.1 소켓이란?

소켓을 일상생활에 비유하면:

```c
// 소켓 = 전화기
// IP 주소 = 전화번호
// 포트 = 내선번호
// bind() = 전화기 설치
// listen() = 전화 대기
// accept() = 전화 받기
// connect() = 전화 걸기
// send()/recv() = 대화하기
// close() = 전화 끊기
```

```
프로세스 A                    프로세스 B
    |                            |
[소켓 A] ←----- 네트워크 ----→ [소켓 B]
    |                            |
  IP:Port                      IP:Port
192.168.1.100:9999         192.168.1.101:8080
```

### 2.2 소켓의 종류와 생성

```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// 소켓 생성 함수 상세 설명
int create_socket_explained() {
    // socket(도메인, 타입, 프로토콜)
    // 도메인: AF_INET (IPv4), AF_INET6 (IPv6), AF_UNIX (로컬)
    // 타입: SOCK_STREAM (TCP), SOCK_DGRAM (UDP)
    // 프로토콜: 0 (자동 선택), IPPROTO_TCP, IPPROTO_UDP
    
    // TCP 소켓 생성
    int tcp_socket = socket(AF_INET,     // IPv4 사용
                           SOCK_STREAM,  // TCP (스트림)
                           0);           // 프로토콜 자동
    if (tcp_socket == -1) {
        perror("TCP 소켓 생성 실패");
        return -1;
    }
    printf("TCP 소켓 생성 성공! 파일 디스크립터: %d\n", tcp_socket);
    
    // UDP 소켓 생성
    int udp_socket = socket(AF_INET,     // IPv4 사용
                           SOCK_DGRAM,   // UDP (데이터그램)
                           0);           // 프로토콜 자동
    if (udp_socket == -1) {
        perror("UDP 소켓 생성 실패");
        return -1;
    }
    printf("UDP 소켓 생성 성공! 파일 디스크립터: %d\n", udp_socket);
    
    // 소켓도 파일처럼 닫아야 함
    close(tcp_socket);
    close(udp_socket);
    
    return 0;
}
```

### 2.3 소켓 주소 구조체 완벽 이해

```c
#include <arpa/inet.h>
#include <string.h>

// IPv4 주소 구조체 상세 분석
void understand_sockaddr_in() {
    // sockaddr_in 구조체는 IPv4 주소 정보를 담는 그릇
    struct sockaddr_in addr;
    
    // 1. 구조체 초기화 (쓰레기값 방지)
    memset(&addr, 0, sizeof(addr));
    
    // 2. 주소 체계 설정 (AF_INET = IPv4)
    addr.sin_family = AF_INET;
    
    // 3. 포트 설정 (네트워크 바이트 순서로 변환)
    addr.sin_port = htons(9999);
    // htons = Host TO Network Short
    // 내 컴퓨터의 바이트 순서 → 네트워크 바이트 순서
    
    // 4. IP 주소 설정 (여러 방법)
    
    // 방법 1: 모든 인터페이스에서 접속 허용
    addr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0
    
    // 방법 2: 특정 IP 주소 지정 (문자열 → 이진)
    inet_pton(AF_INET, "192.168.1.100", &addr.sin_addr);
    
    // 방법 3: 로컬호스트만 허용
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    
    // 구조체 내용 출력
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);
    printf("설정된 주소: %s:%d\n", ip_str, ntohs(addr.sin_port));
}

// 바이트 순서 변환 이해하기
void understand_byte_order() {
    // 컴퓨터마다 바이트 저장 순서가 다름!
    // Little Endian: Intel x86 (낮은 바이트부터)
    // Big Endian: 네트워크 (높은 바이트부터)
    
    uint16_t port = 9999;  // 0x270F
    
    printf("원본 포트: %d (0x%04X)\n", port, port);
    
    // 메모리에서의 표현 (Little Endian 시스템)
    uint8_t* bytes = (uint8_t*)&port;
    printf("메모리: [0x%02X][0x%02X]\n", bytes[0], bytes[1]);
    // 출력: [0x0F][0x27] - 거꾸로 저장됨!
    
    // 네트워크 바이트 순서로 변환
    uint16_t net_port = htons(port);
    bytes = (uint8_t*)&net_port;
    printf("네트워크: [0x%02X][0x%02X]\n", bytes[0], bytes[1]);
    // 출력: [0x27][0x0F] - 올바른 순서
    
    // 변환 함수들
    // htons(): uint16_t host → network (포트용)
    // htonl(): uint32_t host → network (IP용)
    // ntohs(): uint16_t network → host
    // ntohl(): uint32_t network → host
}
```

### 바이트 순서 변환

네트워크는 빅 엔디안(Big Endian)을 사용하므로 바이트 순서 변환이 필요합니다.

```c
#include <arpa/inet.h>

// 호스트 → 네트워크 바이트 순서
uint16_t htons(uint16_t hostshort);    // short (포트)
uint32_t htonl(uint32_t hostlong);     // long (IP)

// 네트워크 → 호스트 바이트 순서
uint16_t ntohs(uint16_t netshort);
uint32_t ntohl(uint32_t netlong);

// IP 주소 문자열 ↔ 이진 변환
int inet_pton(int af, const char *src, void *dst);      // 문자열 → 이진
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);  // 이진 → 문자열

// 예시
struct sockaddr_in addr;
addr.sin_port = htons(9999);                    // 포트 설정
inet_pton(AF_INET, "192.168.1.100", &addr.sin_addr);  // IP 설정

char ip_str[INET_ADDRSTRLEN];
inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);
printf("IP: %s, Port: %d\n", ip_str, ntohs(addr.sin_port));
```

---

## 🖥️ 3. 클라이언트-서버 모델

### 서버 구현 과정

#### 1. 기본 TCP 서버 구조

```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_server_socket(int port) {
    int server_fd;
    struct sockaddr_in server_addr;
    int opt = 1;
    
    // 1. 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        return -1;
    }
    
    // 2. 소켓 옵션 설정 (포트 재사용)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        return -1;
    }
    
    // 3. 주소 구조체 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // 모든 인터페이스
    server_addr.sin_port = htons(port);
    
    // 4. 주소에 바인딩
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        return -1;
    }
    
    // 5. 연결 대기 상태로 전환
    if (listen(server_fd, 10) < 0) {  // 백로그 큐 크기: 10
        perror("listen failed");
        close(server_fd);
        return -1;
    }
    
    printf("Server listening on port %d\n", port);
    return server_fd;
}

// 클라이언트 연결 처리
void handle_client(int client_fd, struct sockaddr_in* client_addr) {
    char buffer[1024];
    char client_ip[INET_ADDRSTRLEN];
    
    // 클라이언트 IP 주소 출력
    inet_ntop(AF_INET, &client_addr->sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("Client connected from %s:%d\n", client_ip, ntohs(client_addr->sin_port));
    
    // 메시지 수신
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Received: %s\n", buffer);
        
        // 응답 전송
        const char* response = "Log received successfully\n";
        send(client_fd, response, strlen(response), 0);
    }
    
    close(client_fd);
}

// 메인 서버 루프
int run_server(int port) {
    int server_fd = create_server_socket(port);
    if (server_fd == -1) {
        return -1;
    }
    
    printf("LogCaster server started on port %d\n", port);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // 클라이언트 연결 수락
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("accept failed");
            continue;
        }
        
        // 클라이언트 처리 (단일 스레드 버전)
        handle_client(client_fd, &client_addr);
    }
    
    close(server_fd);
    return 0;
}
```

#### 2. LogCaster 서버 예시

```c
#include <time.h>

typedef struct LogEntry {
    char message[256];
    time_t timestamp;
    char client_ip[INET_ADDRSTRLEN];
    int client_port;
} LogEntry;

// 로그 저장을 위한 간단한 배열 (나중에 연결 리스트로 개선)
#define MAX_LOGS 1000
LogEntry log_storage[MAX_LOGS];
int log_count = 0;

void handle_log_client(int client_fd, struct sockaddr_in* client_addr) {
    char buffer[256];
    char client_ip[INET_ADDRSTRLEN];
    
    inet_ntop(AF_INET, &client_addr->sin_addr, client_ip, INET_ADDRSTRLEN);
    
    while (1) {
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            break;  // 연결 종료 또는 오류
        }
        
        buffer[bytes_received] = '\0';
        
        // 로그 엔트리 생성 및 저장
        if (log_count < MAX_LOGS) {
            LogEntry* entry = &log_storage[log_count];
            strncpy(entry->message, buffer, sizeof(entry->message) - 1);
            entry->message[sizeof(entry->message) - 1] = '\0';
            entry->timestamp = time(NULL);
            strncpy(entry->client_ip, client_ip, sizeof(entry->client_ip) - 1);
            entry->client_port = ntohs(client_addr->sin_port);
            
            log_count++;
            
            printf("[LOG] %s:%d - %s", client_ip, ntohs(client_addr->sin_port), buffer);
            
            // 확인 응답
            const char* ack = "ACK\n";
            send(client_fd, ack, strlen(ack), 0);
        } else {
            // 로그 저장소가 가득 참
            const char* error = "ERROR: Log storage full\n";
            send(client_fd, error, strlen(error), 0);
        }
    }
    
    printf("Client disconnected: %s:%d\n", client_ip, ntohs(client_addr->sin_port));
    close(client_fd);
}
```

### 클라이언트 구현

#### 1. 기본 TCP 클라이언트

```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int connect_to_server(const char* server_ip, int port) {
    int client_fd;
    struct sockaddr_in server_addr;
    
    // 1. 소켓 생성
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket failed");
        return -1;
    }
    
    // 2. 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(client_fd);
        return -1;
    }
    
    // 3. 서버에 연결
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(client_fd);
        return -1;
    }
    
    printf("Connected to server %s:%d\n", server_ip, port);
    return client_fd;
}

// 로그 전송 클라이언트
void send_log_message(const char* server_ip, int port, const char* message) {
    int client_fd = connect_to_server(server_ip, port);
    if (client_fd == -1) {
        return;
    }
    
    // 메시지 전송
    if (send(client_fd, message, strlen(message), 0) == -1) {
        perror("send failed");
    } else {
        printf("Sent: %s", message);
        
        // 서버 응답 수신
        char response[256];
        ssize_t bytes_received = recv(client_fd, response, sizeof(response) - 1, 0);
        if (bytes_received > 0) {
            response[bytes_received] = '\0';
            printf("Server response: %s", response);
        }
    }
    
    close(client_fd);
}

// 테스트 클라이언트
int main() {
    const char* server_ip = "127.0.0.1";
    int port = 9999;
    
    // 여러 로그 메시지 전송
    send_log_message(server_ip, port, "Server started\n");
    send_log_message(server_ip, port, "User login: admin\n");
    send_log_message(server_ip, port, "Database connection established\n");
    send_log_message(server_ip, port, "Processing request #12345\n");
    
    return 0;
}
```

#### 2. 지속적인 연결을 위한 클라이언트

```c
// 연결을 유지하며 여러 메시지를 전송하는 클라이언트
void interactive_log_client(const char* server_ip, int port) {
    int client_fd = connect_to_server(server_ip, port);
    if (client_fd == -1) {
        return;
    }
    
    char message[256];
    char response[256];
    
    printf("Connected to LogCaster server. Type 'quit' to exit.\n");
    
    while (1) {
        printf("Enter log message: ");
        if (fgets(message, sizeof(message), stdin) == NULL) {
            break;
        }
        
        // 'quit' 명령어 확인
        if (strncmp(message, "quit", 4) == 0) {
            break;
        }
        
        // 메시지 전송
        if (send(client_fd, message, strlen(message), 0) == -1) {
            perror("send failed");
            break;
        }
        
        // 서버 응답 수신
        ssize_t bytes_received = recv(client_fd, response, sizeof(response) - 1, 0);
        if (bytes_received <= 0) {
            printf("Server disconnected\n");
            break;
        }
        
        response[bytes_received] = '\0';
        printf("Server: %s", response);
    }
    
    close(client_fd);
    printf("Disconnected from server\n");
}
```

---

## 🔧 4. 고급 소켓 프로그래밍

### 소켓 옵션 설정

```c
#include <sys/socket.h>

int set_socket_options(int socket_fd) {
    int opt = 1;
    
    // 1. 주소 재사용 (SO_REUSEADDR)
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("SO_REUSEADDR failed");
        return -1;
    }
    
    // 2. 포트 재사용 (SO_REUSEPORT) - Linux 3.9+
    #ifdef SO_REUSEPORT
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("SO_REUSEPORT failed");
        return -1;
    }
    #endif
    
    // 3. Keep-Alive 설정
    if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0) {
        perror("SO_KEEPALIVE failed");
        return -1;
    }
    
    // 4. 수신 버퍼 크기 설정
    int recv_buffer_size = 64 * 1024;  // 64KB
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &recv_buffer_size, sizeof(recv_buffer_size)) < 0) {
        perror("SO_RCVBUF failed");
        return -1;
    }
    
    // 5. 송신 버퍼 크기 설정
    int send_buffer_size = 64 * 1024;  // 64KB
    if (setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, &send_buffer_size, sizeof(send_buffer_size)) < 0) {
        perror("SO_SNDBUF failed");
        return -1;
    }
    
    // 6. TCP_NODELAY (Nagle 알고리즘 비활성화)
    #ifdef TCP_NODELAY
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0) {
        perror("TCP_NODELAY failed");
        return -1;
    }
    #endif
    
    return 0;
}
```

### 비블로킹 소켓

```c
#include <fcntl.h>
#include <errno.h>

// 소켓을 비블로킹 모드로 설정
int set_nonblocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        return -1;
    }
    
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL O_NONBLOCK");
        return -1;
    }
    
    return 0;
}

// 비블로킹 accept
int nonblocking_accept(int server_fd, struct sockaddr_in* client_addr, socklen_t* client_len) {
    int client_fd = accept(server_fd, (struct sockaddr*)client_addr, client_len);
    
    if (client_fd == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 연결 대기 중인 클라이언트가 없음
            return -1;
        } else {
            perror("accept failed");
            return -2;  // 실제 오류
        }
    }
    
    return client_fd;
}

// 비블로킹 recv
ssize_t nonblocking_recv(int client_fd, char* buffer, size_t buffer_size) {
    ssize_t bytes_received = recv(client_fd, buffer, buffer_size, 0);
    
    if (bytes_received == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 수신할 데이터가 없음
            return 0;
        } else {
            perror("recv failed");
            return -1;  // 실제 오류
        }
    }
    
    return bytes_received;
}
```

### 에러 처리 및 재연결

```c
#include <signal.h>

// SIGPIPE 시그널 무시 (연결이 끊어진 소켓에 쓰기 시 발생)
void ignore_sigpipe() {
    signal(SIGPIPE, SIG_IGN);
}

// 안전한 recv (부분 수신 처리)
ssize_t safe_recv(int socket_fd, void* buffer, size_t length) {
    char* ptr = (char*)buffer;
    size_t total_received = 0;
    
    while (total_received < length) {
        ssize_t bytes_received = recv(socket_fd, ptr + total_received, 
                                    length - total_received, 0);
        
        if (bytes_received == -1) {
            if (errno == EINTR) {
                continue;  // 시그널에 의한 중단, 재시도
            }
            return -1;  // 실제 오류
        } else if (bytes_received == 0) {
            break;  // 연결 종료
        }
        
        total_received += bytes_received;
    }
    
    return total_received;
}

// 안전한 send (부분 전송 처리)
ssize_t safe_send(int socket_fd, const void* buffer, size_t length) {
    const char* ptr = (const char*)buffer;
    size_t total_sent = 0;
    
    while (total_sent < length) {
        ssize_t bytes_sent = send(socket_fd, ptr + total_sent, 
                                length - total_sent, 0);
        
        if (bytes_sent == -1) {
            if (errno == EINTR) {
                continue;  // 시그널에 의한 중단, 재시도
            }
            return -1;  // 실제 오류
        }
        
        total_sent += bytes_sent;
    }
    
    return total_sent;
}

// 자동 재연결 클라이언트
int robust_connect(const char* server_ip, int port, int max_retries, int retry_delay) {
    int client_fd;
    
    for (int retry = 0; retry < max_retries; retry++) {
        client_fd = connect_to_server(server_ip, port);
        if (client_fd != -1) {
            return client_fd;  // 연결 성공
        }
        
        printf("Connection failed, retrying in %d seconds... (%d/%d)\n", 
               retry_delay, retry + 1, max_retries);
        sleep(retry_delay);
    }
    
    printf("Failed to connect after %d retries\n", max_retries);
    return -1;
}
```

---

## 🎯 5. LogCaster 프로젝트에서의 네트워킹 활용

### 프로토콜 설계

LogCaster에서 사용할 간단한 텍스트 기반 프로토콜:

```
로그 전송 프로토콜:
CLIENT -> SERVER: "LOG:<timestamp>:<level>:<message>\n"
SERVER -> CLIENT: "ACK\n" 또는 "ERROR:<error_message>\n"

로그 조회 프로토콜:
CLIENT -> SERVER: "QUERY:<keyword>\n"
SERVER -> CLIENT: "RESULT:<count>\n<log1>\n<log2>\n...\nEND\n"

예시:
CLIENT: "LOG:1640995200:INFO:Server started\n"
SERVER: "ACK\n"

CLIENT: "QUERY:ERROR\n"
SERVER: "RESULT:2\nLOG:1640995300:ERROR:Database connection failed\nLOG:1640995301:ERROR:File not found\nEND\n"
```

### 멀티포트 서버 구현

```c
#include <sys/select.h>

// 두 개의 포트를 동시에 처리하는 서버
int run_multiport_server(int log_port, int query_port) {
    int log_server_fd, query_server_fd;
    int max_fd;
    fd_set read_fds;
    
    // 로그 수집 서버 소켓 생성
    log_server_fd = create_server_socket(log_port);
    if (log_server_fd == -1) {
        return -1;
    }
    
    // 로그 조회 서버 소켓 생성
    query_server_fd = create_server_socket(query_port);
    if (query_server_fd == -1) {
        close(log_server_fd);
        return -1;
    }
    
    printf("LogCaster server started:\n");
    printf("  Log collection port: %d\n", log_port);
    printf("  Log query port: %d\n", query_port);
    
    max_fd = (log_server_fd > query_server_fd) ? log_server_fd : query_server_fd;
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(log_server_fd, &read_fds);
        FD_SET(query_server_fd, &read_fds);
        
        // select를 사용하여 여러 소켓 모니터링
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select failed");
            break;
        }
        
        // 로그 수집 포트에 새 연결
        if (FD_ISSET(log_server_fd, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_fd = accept(log_server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd != -1) {
                printf("New log client connected\n");
                handle_log_client(client_fd, &client_addr);
            }
        }
        
        // 로그 조회 포트에 새 연결
        if (FD_ISSET(query_server_fd, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_fd = accept(query_server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd != -1) {
                printf("New query client connected\n");
                handle_query_client(client_fd, &client_addr);
            }
        }
    }
    
    close(log_server_fd);
    close(query_server_fd);
    return 0;
}

// 로그 조회 클라이언트 처리
void handle_query_client(int client_fd, struct sockaddr_in* client_addr) {
    char buffer[256];
    char client_ip[INET_ADDRSTRLEN];
    
    inet_ntop(AF_INET, &client_addr->sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("Query client connected: %s:%d\n", client_ip, ntohs(client_addr->sin_port));
    
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        
        // "QUERY:" 프리픽스 확인
        if (strncmp(buffer, "QUERY:", 6) == 0) {
            char* keyword = buffer + 6;
            // 개행 문자 제거
            char* newline = strchr(keyword, '\n');
            if (newline) *newline = '\0';
            
            // 로그 검색 및 결과 전송
            search_and_send_logs(client_fd, keyword);
        } else {
            const char* error = "ERROR:Invalid query format\n";
            send(client_fd, error, strlen(error), 0);
        }
    }
    
    close(client_fd);
    printf("Query client disconnected: %s:%d\n", client_ip, ntohs(client_addr->sin_port));
}

// 로그 검색 및 전송
void search_and_send_logs(int client_fd, const char* keyword) {
    char response[4096];
    char temp[512];
    int match_count = 0;
    
    // 검색 결과 구성
    strcpy(response, "RESULT:");
    
    // 저장된 로그에서 키워드 검색
    for (int i = 0; i < log_count; i++) {
        if (strstr(log_storage[i].message, keyword) != NULL) {
            match_count++;
        }
    }
    
    // 결과 개수 추가
    sprintf(temp, "%d\n", match_count);
    strcat(response, temp);
    
    // 매칭된 로그 추가
    for (int i = 0; i < log_count; i++) {
        if (strstr(log_storage[i].message, keyword) != NULL) {
            sprintf(temp, "LOG:%ld:%s:%s", 
                   log_storage[i].timestamp,
                   log_storage[i].client_ip,
                   log_storage[i].message);
            strcat(response, temp);
        }
    }
    
    strcat(response, "END\n");
    
    // 결과 전송
    send(client_fd, response, strlen(response), 0);
}
```

---

## 🔧 5. 고급 네트워킹 기법

### 5.1 비블로킹 소켓과 타임아웃

```c
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>

// 소켓을 비블로킹 모드로 설정
int set_socket_nonblocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        return -1;
    }
    
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL O_NONBLOCK");
        return -1;
    }
    
    printf("소켓 %d를 비블로킹 모드로 설정했습니다.\n", socket_fd);
    return 0;
}

// 타임아웃이 있는 connect
int connect_with_timeout(const char* server_ip, int port, int timeout_sec) {
    int client_fd;
    struct sockaddr_in server_addr;
    fd_set write_fds;
    struct timeval timeout;
    
    // 소켓 생성
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        return -1;
    }
    
    // 비블로킹 모드 설정
    set_socket_nonblocking(client_fd);
    
    // 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
    
    // connect 시도 (비블로킹이므로 즉시 반환)
    int result = connect(client_fd, (struct sockaddr*)&server_addr, 
                        sizeof(server_addr));
    
    if (result == 0) {
        // 즉시 연결 성공 (로컬 연결의 경우)
        return client_fd;
    }
    
    if (errno != EINPROGRESS) {
        // 실제 에러
        perror("connect");
        close(client_fd);
        return -1;
    }
    
    // select로 타임아웃 대기
    FD_ZERO(&write_fds);
    FD_SET(client_fd, &write_fds);
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;
    
    result = select(client_fd + 1, NULL, &write_fds, NULL, &timeout);
    
    if (result == 0) {
        // 타임아웃
        fprintf(stderr, "연결 타임아웃 (%d초)\n", timeout_sec);
        close(client_fd);
        return -1;
    }
    
    if (result < 0) {
        // select 에러
        perror("select");
        close(client_fd);
        return -1;
    }
    
    // 연결 상태 확인
    int error = 0;
    socklen_t len = sizeof(error);
    if (getsockopt(client_fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        perror("getsockopt");
        close(client_fd);
        return -1;
    }
    
    if (error != 0) {
        // 연결 실패
        fprintf(stderr, "연결 실패: %s\n", strerror(error));
        close(client_fd);
        return -1;
    }
    
    // 연결 성공! 블로킹 모드로 되돌리기
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags & ~O_NONBLOCK);
    
    return client_fd;
}

// 타임아웃이 있는 recv
ssize_t recv_with_timeout(int socket_fd, void* buffer, size_t length, 
                         int timeout_sec) {
    fd_set read_fds;
    struct timeval timeout;
    
    FD_ZERO(&read_fds);
    FD_SET(socket_fd, &read_fds);
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;
    
    int result = select(socket_fd + 1, &read_fds, NULL, NULL, &timeout);
    
    if (result == 0) {
        // 타임아웃
        errno = ETIMEDOUT;
        return -1;
    }
    
    if (result < 0) {
        // select 에러
        return -1;
    }
    
    // 데이터 수신
    return recv(socket_fd, buffer, length, 0);
}
```

### 5.2 안전한 데이터 전송

```c
// 정확한 바이트 수만큼 전송 보장
ssize_t send_all(int socket_fd, const void* buffer, size_t length) {
    const char* ptr = (const char*)buffer;
    size_t remaining = length;
    
    while (remaining > 0) {
        ssize_t sent = send(socket_fd, ptr, remaining, MSG_NOSIGNAL);
        
        if (sent == -1) {
            if (errno == EINTR) {
                // 시그널에 의한 중단, 재시도
                continue;
            }
            // 실제 에러
            return -1;
        }
        
        ptr += sent;
        remaining -= sent;
    }
    
    return length;
}

// 정확한 바이트 수만큼 수신 보장
ssize_t recv_all(int socket_fd, void* buffer, size_t length) {
    char* ptr = (char*)buffer;
    size_t remaining = length;
    
    while (remaining > 0) {
        ssize_t received = recv(socket_fd, ptr, remaining, 0);
        
        if (received == -1) {
            if (errno == EINTR) {
                // 시그널에 의한 중단, 재시도
                continue;
            }
            // 실제 에러
            return -1;
        }
        
        if (received == 0) {
            // 연결 종료
            break;
        }
        
        ptr += received;
        remaining -= received;
    }
    
    return length - remaining;
}

// 메시지 프레이밍 (길이-데이터 프로토콜)
typedef struct {
    uint32_t length;  // 메시지 길이 (네트워크 바이트 순서)
    char data[0];     // 가변 길이 데이터
} message_frame_t;

// 프레임 단위로 메시지 전송
int send_message(int socket_fd, const char* message) {
    size_t msg_len = strlen(message);
    uint32_t net_len = htonl(msg_len);
    
    // 길이 전송
    if (send_all(socket_fd, &net_len, sizeof(net_len)) != sizeof(net_len)) {
        return -1;
    }
    
    // 데이터 전송
    if (send_all(socket_fd, message, msg_len) != msg_len) {
        return -1;
    }
    
    return 0;
}

// 프레임 단위로 메시지 수신
char* recv_message(int socket_fd) {
    uint32_t net_len;
    
    // 길이 수신
    if (recv_all(socket_fd, &net_len, sizeof(net_len)) != sizeof(net_len)) {
        return NULL;
    }
    
    uint32_t msg_len = ntohl(net_len);
    
    // 보안: 메시지 크기 제한
    if (msg_len > 1024 * 1024) {  // 1MB 제한
        fprintf(stderr, "메시지가 너무 큽니다: %u bytes\n", msg_len);
        return NULL;
    }
    
    // 메모리 할당
    char* message = (char*)malloc(msg_len + 1);
    if (!message) {
        perror("malloc");
        return NULL;
    }
    
    // 데이터 수신
    if (recv_all(socket_fd, message, msg_len) != msg_len) {
        free(message);
        return NULL;
    }
    
    message[msg_len] = '\0';
    return message;
}
```

### 5.3 멀티포트 서버 (select 사용)

```c
#include <sys/select.h>

// 여러 포트를 동시에 처리하는 서버
int run_multiport_server(int log_port, int query_port) {
    int log_fd, query_fd;
    int max_fd;
    fd_set master_fds, read_fds;
    
    // 두 개의 서버 소켓 생성
    log_fd = create_server_socket(log_port);
    if (log_fd == -1) return -1;
    
    query_fd = create_server_socket(query_port);
    if (query_fd == -1) {
        close(log_fd);
        return -1;
    }
    
    printf("📊 멀티포트 서버 시작\n");
    printf("   로그 수집: %d\n", log_port);
    printf("   로그 조회: %d\n", query_port);
    
    // fd_set 초기화
    FD_ZERO(&master_fds);
    FD_SET(log_fd, &master_fds);
    FD_SET(query_fd, &master_fds);
    max_fd = (log_fd > query_fd) ? log_fd : query_fd;
    
    while (1) {
        // master_fds 복사 (select가 수정하므로)
        read_fds = master_fds;
        
        // 이벤트 대기
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select");
            break;
        }
        
        // 로그 수집 포트 확인
        if (FD_ISSET(log_fd, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_fd = accept(log_fd, (struct sockaddr*)&client_addr, 
                                 &client_len);
            if (client_fd != -1) {
                printf("📝 새 로그 클라이언트\n");
                // 실제로는 스레드나 프로세스로 처리
                handle_log_collection_client(client_fd, &client_addr);
            }
        }
        
        // 로그 조회 포트 확인
        if (FD_ISSET(query_fd, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_fd = accept(query_fd, (struct sockaddr*)&client_addr, 
                                 &client_len);
            if (client_fd != -1) {
                printf("🔍 새 조회 클라이언트\n");
                // 실제로는 스레드나 프로세스로 처리
                handle_log_query_client(client_fd, &client_addr);
            }
        }
    }
    
    close(log_fd);
    close(query_fd);
    return 0;
}
```

---

## 🖐️ 6. 네트워크 프로그래밍 보안 고려사항

### 6.1 입력 검증

```c
// 안전한 문자열 처리
int validate_log_message(const char* message) {
    size_t len = strlen(message);
    
    // 길이 제한
    if (len == 0 || len > 1024) {
        return 0;
    }
    
    // 위험한 문자 검사
    for (size_t i = 0; i < len; i++) {
        // 제어 문자 거부 (개행 제외)
        if (message[i] < 32 && message[i] != '\n' && message[i] != '\t') {
            return 0;
        }
    }
    
    return 1;
}

// 버퍼 오버플로우 방지
void safe_string_copy(char* dest, const char* src, size_t dest_size) {
    if (dest_size == 0) return;
    
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}
```

### 6.2 DoS 공격 방지

```c
// 연결 제한
typedef struct {
    char ip[INET_ADDRSTRLEN];
    int connection_count;
    time_t first_connection;
    time_t last_connection;
} client_info_t;

#define MAX_CONNECTIONS_PER_IP 10
#define CONNECTION_WINDOW 60  // 60초

int check_connection_limit(const char* client_ip) {
    static client_info_t clients[100];
    static int client_count = 0;
    time_t now = time(NULL);
    
    // 기존 클라이언트 찾기
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].ip, client_ip) == 0) {
            // 시간 창 확인
            if (now - clients[i].first_connection > CONNECTION_WINDOW) {
                // 새 창 시작
                clients[i].connection_count = 1;
                clients[i].first_connection = now;
                clients[i].last_connection = now;
                return 1;
            }
            
            // 연결 수 확인
            if (clients[i].connection_count >= MAX_CONNECTIONS_PER_IP) {
                return 0;  // 제한 초과
            }
            
            clients[i].connection_count++;
            clients[i].last_connection = now;
            return 1;
        }
    }
    
    // 새 클라이언트
    if (client_count < 100) {
        strcpy(clients[client_count].ip, client_ip);
        clients[client_count].connection_count = 1;
        clients[client_count].first_connection = now;
        clients[client_count].last_connection = now;
        client_count++;
        return 1;
    }
    
    return 0;  // 테이블 가득 참
}
```

---

## 🎯 7. LogCaster 프로토콜 설계

### 7.1 텍스트 기반 프로토콜

```
로그 수집 프로토콜:
==================
요청: LOG|<level>|<timestamp>|<message>\n
응답: OK|<log_id>\n 또는 ERROR|<error_message>\n
예시:
C: LOG|ERROR|1640995200|Database connection failed\n
S: OK|12345\n
로그 조회 프로토콜:
==================
요청: QUERY|<type>|<parameter>\n
응답: RESULT|<count>\n
      <log1>\n
      <log2>\n
      END\n
예시:
C: QUERY|KEYWORD|error\n
S: RESULT|2\n
S: 12345|ERROR|1640995200|Database connection failed\n
S: 12346|ERROR|1640995300|File not found\n
S: END\n
```

### 7.2 바이너리 프로토콜 (고급)

```c
// 효율적인 바이너리 프로토콜
typedef enum {
    CMD_LOG = 0x01,
    CMD_QUERY = 0x02,
    CMD_PING = 0x03,
    CMD_STATS = 0x04
} command_type_t;

typedef struct {
    uint8_t version;      // 프로토콜 버전
    uint8_t command;      // 명령 타입
    uint16_t length;      // 페이로드 길이
    uint32_t sequence;    // 시퀀스 번호
} protocol_header_t;

typedef struct {
    protocol_header_t header;
    uint8_t level;        // 로그 레벨
    uint32_t timestamp;   // Unix 타임스탬프
    uint16_t msg_length;  // 메시지 길이
    char message[0];      // 가변 길이 메시지
} log_packet_t;

// 패킷 생성
log_packet_t* create_log_packet(log_level_t level, const char* message) {
    size_t msg_len = strlen(message);
    size_t packet_size = sizeof(log_packet_t) + msg_len;
    
    log_packet_t* packet = (log_packet_t*)malloc(packet_size);
    if (!packet) return NULL;
    
    packet->header.version = 1;
    packet->header.command = CMD_LOG;
    packet->header.length = htons(sizeof(uint8_t) + sizeof(uint32_t) + 
                                 sizeof(uint16_t) + msg_len);
    packet->header.sequence = htonl(0);  // 나중에 구현
    
    packet->level = level;
    packet->timestamp = htonl(time(NULL));
    packet->msg_length = htons(msg_len);
    memcpy(packet->message, message, msg_len);
    
    return packet;
}
```

---

## 🔄 8. epoll과 kqueue - 고성능 I/O 멀티플렉싱

### 8.1 epoll (Linux)

#### 기본 사용법
```c
#include <sys/epoll.h>

// epoll 인스턴스 생성
int epoll_fd = epoll_create1(0);
if (epoll_fd < 0) {
    perror("epoll_create1");
    return -1;
}

// 소켓을 epoll에 추가
struct epoll_event event;
event.events = EPOLLIN | EPOLLET;  // Edge-triggered
event.data.fd = socket_fd;

if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) < 0) {
    perror("epoll_ctl");
    return -1;
}

// 이벤트 대기
struct epoll_event events[MAX_EVENTS];
int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout_ms);

for (int i = 0; i < nfds; i++) {
    if (events[i].events & EPOLLIN) {
        // 읽기 가능
        handle_read(events[i].data.fd);
    }
    if (events[i].events & EPOLLOUT) {
        // 쓰기 가능
        handle_write(events[i].data.fd);
    }
}
```

#### Edge-Triggered vs Level-Triggered
```c
// Edge-Triggered (ET) - 상태 변화 시에만 알림
event.events = EPOLLIN | EPOLLET;

// Level-Triggered (LT) - 처리할 데이터가 있는 동안 계속 알림
event.events = EPOLLIN;  // 기본값

// ET 모드에서는 반드시 EAGAIN까지 읽어야 함
while (1) {
    ssize_t count = read(fd, buffer, sizeof(buffer));
    if (count < 0) {
        if (errno == EAGAIN) {
            // 더 이상 읽을 데이터 없음
            break;
        }
        perror("read");
        return -1;
    }
    // 데이터 처리...
}
```

### 8.2 kqueue (BSD/macOS)

```c
#include <sys/event.h>

// kqueue 생성
int kq = kqueue();
if (kq < 0) {
    perror("kqueue");
    return -1;
}

// 이벤트 등록
struct kevent change;
EV_SET(&change, socket_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

if (kevent(kq, &change, 1, NULL, 0, NULL) < 0) {
    perror("kevent");
    return -1;
}

// 이벤트 대기
struct kevent events[MAX_EVENTS];
int nev = kevent(kq, NULL, 0, events, MAX_EVENTS, &timeout);

for (int i = 0; i < nev; i++) {
    if (events[i].filter == EVFILT_READ) {
        handle_read(events[i].ident);
    }
}
```

### 8.3 select/poll vs epoll/kqueue 성능 비교

| 특성 | select | poll | epoll | kqueue |
|------|--------|------|-------|---------|
| 최대 FD 수 | 1024 (FD_SETSIZE) | 제한 없음 | 제한 없음 | 제한 없음 |
| 시간 복잡도 | O(n) | O(n) | O(1) | O(1) |
| 메모리 사용 | 고정 | fd 수에 비례 | 효율적 | 효율적 |
| 플랫폼 | 모든 Unix | 모든 Unix | Linux | BSD/macOS |
| Edge-triggered | 불가 | 불가 | 가능 | 가능 |

## 🔌 9. Connection Pooling

### 9.1 연결 풀의 필요성

매번 새로운 연결을 생성하는 것은 비용이 많이 듭니다:
- TCP 3-way handshake 시간
- 소켓 생성/해제 오버헤드
- 시스템 자원 소비

### 9.2 기본 연결 풀 구현

```c
typedef struct connection {
    int fd;
    int in_use;
    time_t last_used;
    struct sockaddr_in addr;
} connection_t;

typedef struct connection_pool {
    connection_t* connections;
    int pool_size;
    int active_count;
    pthread_mutex_t mutex;
} connection_pool_t;

// 연결 풀 초기화
connection_pool_t* pool_create(int size) {
    connection_pool_t* pool = malloc(sizeof(connection_pool_t));
    pool->connections = calloc(size, sizeof(connection_t));
    pool->pool_size = size;
    pool->active_count = 0;
    pthread_mutex_init(&pool->mutex, NULL);
    
    // 미리 연결 생성
    for (int i = 0; i < size; i++) {
        pool->connections[i].fd = -1;
        pool->connections[i].in_use = 0;
    }
    
    return pool;
}

// 연결 획득
int pool_get_connection(connection_pool_t* pool, 
                       struct sockaddr_in* addr) {
    pthread_mutex_lock(&pool->mutex);
    
    // 기존 연결 재사용 시도
    for (int i = 0; i < pool->pool_size; i++) {
        connection_t* conn = &pool->connections[i];
        if (!conn->in_use && conn->fd >= 0 &&
            memcmp(&conn->addr, addr, sizeof(*addr)) == 0) {
            // 연결 상태 확인
            char buf;
            int error = 0;
            socklen_t len = sizeof(error);
            if (getsockopt(conn->fd, SOL_SOCKET, SO_ERROR, 
                          &error, &len) == 0 && error == 0) {
                conn->in_use = 1;
                conn->last_used = time(NULL);
                pthread_mutex_unlock(&pool->mutex);
                return conn->fd;
            }
            // 연결이 끊어진 경우 정리
            close(conn->fd);
            conn->fd = -1;
        }
    }
    
    // 새 연결 생성
    for (int i = 0; i < pool->pool_size; i++) {
        connection_t* conn = &pool->connections[i];
        if (!conn->in_use && conn->fd < 0) {
            int fd = socket(AF_INET, SOCK_STREAM, 0);
            if (fd < 0) {
                pthread_mutex_unlock(&pool->mutex);
                return -1;
            }
            
            // 논블로킹 모드 설정
            int flags = fcntl(fd, F_GETFL, 0);
            fcntl(fd, F_SETFL, flags | O_NONBLOCK);
            
            if (connect(fd, (struct sockaddr*)addr, 
                       sizeof(*addr)) < 0) {
                if (errno != EINPROGRESS) {
                    close(fd);
                    pthread_mutex_unlock(&pool->mutex);
                    return -1;
                }
            }
            
            conn->fd = fd;
            conn->in_use = 1;
            conn->last_used = time(NULL);
            conn->addr = *addr;
            pool->active_count++;
            
            pthread_mutex_unlock(&pool->mutex);
            return fd;
        }
    }
    
    pthread_mutex_unlock(&pool->mutex);
    return -1;  // 풀이 가득 참
}

// 연결 반환
void pool_release_connection(connection_pool_t* pool, int fd) {
    pthread_mutex_lock(&pool->mutex);
    
    for (int i = 0; i < pool->pool_size; i++) {
        if (pool->connections[i].fd == fd) {
            pool->connections[i].in_use = 0;
            pool->connections[i].last_used = time(NULL);
            break;
        }
    }
    
    pthread_mutex_unlock(&pool->mutex);
}
```

## 🎚️ 10. TCP 튜닝

### 10.1 고급 TCP 튜닝 옵션

#### TCP 혼잡 제어 알고리즘
```c
// Linux에서 사용 가능한 혼잡 제어 알고리즘 확인
// cat /proc/sys/net/ipv4/tcp_available_congestion_control

// 현재 알고리즘 확인
char buffer[256];
socklen_t len = sizeof(buffer);
getsockopt(socket_fd, IPPROTO_TCP, TCP_CONGESTION, buffer, &len);
printf("Current congestion control: %s\n", buffer);

// BBR 알고리즘으로 변경 (Linux 4.9+)
if (setsockopt(socket_fd, IPPROTO_TCP, TCP_CONGESTION, 
               "bbr", 4) < 0) {
    perror("Failed to set BBR");
}
```

#### TCP 성능 파라미터
```c
// TCP_USER_TIMEOUT - 전송 실패 시 타임아웃 (밀리초)
unsigned int timeout = 30000;  // 30초
setsockopt(socket_fd, IPPROTO_TCP, TCP_USER_TIMEOUT, 
           &timeout, sizeof(timeout));

// TCP_QUICKACK - 지연 ACK 비활성화
int quickack = 1;
setsockopt(socket_fd, IPPROTO_TCP, TCP_QUICKACK, 
           &quickack, sizeof(quickack));

// TCP_CORK - 작은 패킷들을 모아서 전송
int cork = 1;
setsockopt(socket_fd, IPPROTO_TCP, TCP_CORK, &cork, sizeof(cork));
// 데이터 쓰기...
cork = 0;  // Cork 해제하여 전송
setsockopt(socket_fd, IPPROTO_TCP, TCP_CORK, &cork, sizeof(cork));
```

### 10.2 시스템 레벨 TCP 튜닝

#### sysctl 파라미터 (Linux)
```bash
# TCP 백로그 크기 증가
echo 4096 > /proc/sys/net/core/somaxconn

# TCP 버퍼 크기 튜닝
# min default max (bytes)
echo "4096 87380 67108864" > /proc/sys/net/ipv4/tcp_rmem
echo "4096 65536 67108864" > /proc/sys/net/ipv4/tcp_wmem

# TIME_WAIT 소켓 재사용
echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse

# TCP keepalive 파라미터
echo 60 > /proc/sys/net/ipv4/tcp_keepalive_time    # 60초
echo 10 > /proc/sys/net/ipv4/tcp_keepalive_intvl   # 10초
echo 6 > /proc/sys/net/ipv4/tcp_keepalive_probes    # 6회
```

## ⚠️ 11. 일반적인 문제와 해결책

### 11.1 "Address already in use" 오류

```c
// 문제: 프로그램 종료 후 즉시 재시작할 때 발생
// 원인: TIME_WAIT 상태의 소켓
// 해결: SO_REUSEADDR 옵션 사용

int reuse = 1;
if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, 
               &reuse, sizeof(reuse)) < 0) {
    perror("setsockopt");
}
```

### 11.2 "Connection refused" 오류

```c
// 문제: 클라이언트가 연결할 수 없음
// 가능한 원인들:

// 1. 서버가 실행 중이지 않음
// 해결: 서버 실행 확인
system("netstat -tln | grep 9999");

// 2. 방화벽이 차단
// 해결: 방화벽 규칙 확인

// 3. 잘못된 IP/포트
// 해결: 주소 확인
```

### 8.3 데이터가 잘려서 오는 문제

```c
// 문제: send()한 데이터가 한 번에 오지 않음
// 원인: TCP 스트림 특성
// 해결: 메시지 경계 구분

// 방법 1: 고정 길이 메시지
typedef struct {
    char data[256];  // 항상 256바이트
} fixed_message_t;

// 방법 2: 구분자 사용
// 메시지 끝에 '\n' 추가

// 방법 3: 길이 프리픽스
// [4바이트 길이][가변 데이터]
```

---

## ✅ 9. 실습 과제

### 과제 1: 에코 서버 개선
- 여러 클라이언트 동시 처리 (select 사용)
- 클라이언트별 통계 출력
- 우아한 종료 처리

### 과제 2: 채팅 서버 구현
- 여러 사용자 간 메시지 브로드캐스트
- 사용자 이름 관리
- 개인 메시지 기능

### 과제 3: 파일 전송 프로토콜
- 대용량 파일 전송
- 진행률 표시
- 재개 기능

---

## 🚀 10. [신규] 멀티 포트 서버 관리

### 10.1 여러 포트에서 동시 리스닝

LogCaster-IRC는 3개의 포트를 동시에 관리해야 합니다:

```c
// 멀티 포트 서버 구조체
typedef struct {
    int log_port;      // 9999: 로그 수집
    int query_port;    // 9998: 쿼리 인터페이스
    int irc_port;      // 6667: IRC 서버
    
    int log_fd;
    int query_fd;
    int irc_fd;
    
    fd_set master_set;
    int max_fd;
} multi_port_server_t;

// 멀티 포트 서버 초기화
int init_multi_port_server(multi_port_server_t* server) {
    FD_ZERO(&server->master_set);
    
    // 각 포트별 리스너 생성
    server->log_fd = create_listener(server->log_port);
    server->query_fd = create_listener(server->query_port);
    server->irc_fd = create_listener(server->irc_port);
    
    // fd_set에 추가
    FD_SET(server->log_fd, &server->master_set);
    FD_SET(server->query_fd, &server->master_set);
    FD_SET(server->irc_fd, &server->master_set);
    
    // 최대 fd 계산
    server->max_fd = max(server->log_fd, 
                        max(server->query_fd, server->irc_fd));
    
    return 0;
}

// 이벤트 루프에서 처리
void handle_multi_port_events(multi_port_server_t* server) {
    fd_set read_fds = server->master_set;
    
    if (select(server->max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
        return;
    }
    
    // 각 포트별로 다른 처리
    if (FD_ISSET(server->log_fd, &read_fds)) {
        handle_log_connection(server->log_fd);
    }
    
    if (FD_ISSET(server->query_fd, &read_fds)) {
        handle_query_connection(server->query_fd);
    }
    
    if (FD_ISSET(server->irc_fd, &read_fds)) {
        handle_irc_connection(server->irc_fd);
    }
}
```

### 10.2 프로토콜 멀티플렉싱

동일한 포트에서 여러 프로토콜 지원:

```c
// 프로토콜 식별
typedef enum {
    PROTO_LOGCRAFTER,
    PROTO_IRC,
    PROTO_HTTP      // 향후 웹 인터페이스용
} protocol_type_t;

// 프로토콜 감지
protocol_type_t detect_protocol(const char* data, size_t len) {
    // IRC는 NICK 또는 USER로 시작
    if (strncmp(data, "NICK ", 5) == 0 || 
        strncmp(data, "USER ", 5) == 0) {
        return PROTO_IRC;
    }
    
    // HTTP는 GET/POST로 시작
    if (strncmp(data, "GET ", 4) == 0 ||
        strncmp(data, "POST ", 5) == 0) {
        return PROTO_HTTP;
    }
    
    // 기본은 LogCaster 프로토콜
    return PROTO_LOGCRAFTER;
}

// 프로토콜별 핸들러 디스패치
void dispatch_by_protocol(int client_fd, const char* data, size_t len) {
    protocol_type_t proto = detect_protocol(data, len);
    
    switch (proto) {
    case PROTO_IRC:
        handle_irc_client(client_fd, data, len);
        break;
    case PROTO_HTTP:
        handle_http_request(client_fd, data, len);
        break;
    default:
        handle_logcaster_client(client_fd, data, len);
    }
}
```

### 10.3 포트별 보안 정책

```c
// 포트별 접근 제어
typedef struct {
    int port;
    char allowed_ips[MAX_IPS][INET_ADDRSTRLEN];
    int ip_count;
    bool require_auth;
    int rate_limit;  // 초당 최대 연결 수
} port_security_t;

// 포트별 보안 설정
port_security_t port_policies[] = {
    {9999, {{"0.0.0.0"}}, 1, false, 100},  // 로그: 모든 IP, 인증 불필요
    {9998, {{"127.0.0.1"}}, 1, true, 10},  // 쿼리: 로컬만, 인증 필요
    {6667, {{"0.0.0.0"}}, 1, true, 50}     // IRC: 모든 IP, 인증 필요
};

bool check_port_access(int port, struct sockaddr_in* client_addr) {
    // 포트별 정책 찾기
    port_security_t* policy = find_port_policy(port);
    if (!policy) return false;
    
    // IP 확인
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr->sin_addr, client_ip, sizeof(client_ip));
    
    if (!is_ip_allowed(policy, client_ip)) {
        log_warning("Rejected connection from %s to port %d", client_ip, port);
        return false;
    }
    
    // Rate limiting
    if (!check_rate_limit(policy, client_ip)) {
        log_warning("Rate limit exceeded for %s on port %d", client_ip, port);
        return false;
    }
    
    return true;
}
```

## 🔄 11. [신규] IRC 통합을 위한 네트워킹 패턴

### 11.1 텍스트 기반 프로토콜 처리

```c
// 라인 기반 프로토콜 버퍼
typedef struct {
    char buffer[4096];
    size_t offset;
    
    // 파싱된 라인들
    char* lines[MAX_LINES];
    int line_count;
} line_buffer_t;

// IRC 메시지 파싱
int parse_lines(line_buffer_t* lb, const char* data, size_t len) {
    // 기존 버퍼에 추가
    size_t space = sizeof(lb->buffer) - lb->offset;
    size_t to_copy = (len < space) ? len : space;
    
    memcpy(lb->buffer + lb->offset, data, to_copy);
    lb->offset += to_copy;
    
    // 라인 단위로 분리
    lb->line_count = 0;
    char* start = lb->buffer;
    char* end = lb->buffer + lb->offset;
    
    while (start < end && lb->line_count < MAX_LINES) {
        char* crlf = strstr(start, "\r\n");
        if (!crlf) break;
        
        *crlf = '\0';  // null 종료
        lb->lines[lb->line_count++] = start;
        start = crlf + 2;  // \r\n 건너뛰기
    }
    
    // 처리되지 않은 데이터 앞으로 이동
    if (start < end) {
        size_t remaining = end - start;
        memmove(lb->buffer, start, remaining);
        lb->offset = remaining;
    } else {
        lb->offset = 0;
    }
    
    return lb->line_count;
}
```

### 11.2 비동기 메시지 브로드캐스트

```c
// 채널 멤버들에게 브로드캐스트
typedef struct {
    int* client_fds;
    int client_count;
    pthread_mutex_t mutex;
} channel_t;

// 효율적인 브로드캐스트
void broadcast_to_channel(channel_t* channel, const char* message) {
    pthread_mutex_lock(&channel->mutex);
    
    // 모든 클라이언트에게 전송
    for (int i = 0; i < channel->client_count; i++) {
        int fd = channel->client_fds[i];
        
        // Non-blocking 전송
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        
        ssize_t sent = send(fd, message, strlen(message), MSG_NOSIGNAL);
        
        if (sent < 0 && errno != EAGAIN) {
            // 연결 끊김 - 제거 표시
            channel->client_fds[i] = -1;
        }
        
        fcntl(fd, F_SETFL, flags);  // 원래 플래그 복원
    }
    
    // 끊긴 연결 정리
    compact_client_list(channel);
    
    pthread_mutex_unlock(&channel->mutex);
}
```

### 11.3 프로토콜 브리지

```c
// LogCaster 로그를 IRC 메시지로 변환
void bridge_log_to_irc(const log_entry_t* log, channel_t* irc_channel) {
    char irc_message[512];
    
    // IRC PRIVMSG 형식으로 변환
    snprintf(irc_message, sizeof(irc_message),
             ":LogBot!log@system PRIVMSG #logs-%s :[%s] %s\r\n",
             log->level,
             format_timestamp(log->timestamp),
             log->message);
    
    // 해당 채널에 브로드캐스트
    broadcast_to_channel(irc_channel, irc_message);
}

// IRC 명령을 LogCaster 쿼리로 변환
query_result_t* bridge_irc_to_query(const char* irc_command) {
    // "!query COUNT level:ERROR" -> LogCaster 쿼리로 변환
    if (strncmp(irc_command, "!query ", 7) == 0) {
        const char* query = irc_command + 7;
        return execute_log_query(query);
    }
    
    return NULL;
}
```

## ⚠️ 자주 하는 실수와 해결법

### 1. bind() 전에 SO_REUSEADDR 설정 안 함
```c
// ❌ 잘못된 예시
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
// "Address already in use" 에러 발생!

// ✅ 올바른 예시
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
int opt = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
```

### 2. 네트워크 바이트 순서 변환 누락
```c
// ❌ 잘못된 예시
addr.sin_port = 9999;  // 호스트 바이트 순서!

// ✅ 올바른 예시
addr.sin_port = htons(9999);  // 네트워크 바이트 순서
```

### 3. recv() 반환값 체크 안 함
```c
// ❌ 잘못된 예시
char buffer[1024];
recv(client_fd, buffer, sizeof(buffer), 0);
printf("%s", buffer);  // 문자열이 아닐 수 있음!

// ✅ 올바른 예시
char buffer[1024];
ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
if (bytes > 0) {
    buffer[bytes] = '\0';  // null 종료
    printf("%s", buffer);
} else if (bytes == 0) {
    // 연결 종료
} else {
    // 에러 처리
}
```

### 4. TCP 스트림 특성 무시
```c
// ❌ 잘못된 예시 - 한 번에 모든 데이터가 온다고 가정
send(fd, "Hello World", 11, 0);
// 수신 측
recv(fd, buffer, 11, 0);  // 11바이트가 안 올 수도 있음!

// ✅ 올바른 예시 - 메시지 경계 처리
// 1. 길이 프리픽스 사용
uint32_t len = htonl(11);
send(fd, &len, sizeof(len), 0);
send(fd, "Hello World", 11, 0);

// 2. 구분자 사용 (예: \n)
send(fd, "Hello World\n", 12, 0);
```

### 5. accept() 후 파일 디스크립터 누수
```c
// ❌ 잘못된 예시
while (1) {
    int client_fd = accept(server_fd, NULL, NULL);
    handle_client(client_fd);
    // close(client_fd) 누락!
}

// ✅ 올바른 예시
while (1) {
    int client_fd = accept(server_fd, NULL, NULL);
    handle_client(client_fd);
    close(client_fd);  // 반드시 닫기
}
```

## 🎯 실습 프로젝트

### 프로젝트 1: 멀티프로토콜 채팅 서버 (난이도: ⭐⭐⭐)
```c
// 구현할 기능:
// 1. TCP와 UDP 동시 지원
// 2. 여러 채팅방 관리
// 3. 사용자 인증
// 4. 메시지 히스토리

// 도전 과제:
// - select/poll/epoll 사용
// - 비블로킹 I/O
// - 프로토콜 설계
```

### 프로젝트 2: HTTP 프록시 서버 (난이도: ⭐⭐⭐⭐)
```c
// 구현할 기능:
// 1. HTTP 요청 파싱
// 2. 대상 서버 연결
// 3. 캐싱 기능
// 4. 로깅 및 필터링

// 고급 기능:
// - HTTPS 터널링
// - 로드 밸런싱
// - 압축 지원
```

### 프로젝트 3: 분산 로그 수집 시스템 (난이도: ⭐⭐⭐⭐⭐)
```c
// 구현할 기능:
// 1. 다중 서버 연결
// 2. 로그 집계 및 검색
// 3. 실시간 스트리밍
// 4. 장애 복구

// 확장 기능:
// - 샤딩과 복제
// - 압축 전송
// - 보안 연결 (TLS)
```

## ✅ 학습 체크리스트

### 🟢 기초 (1-2주)
- [ ] TCP/IP 계층 구조 이해
- [ ] 소켓 생성과 연결
- [ ] 기본 서버/클라이언트 구현
- [ ] 네트워크 바이트 순서
- [ ] 간단한 에코 서버

### 🟡 중급 (3-4주)
- [ ] 다중 클라이언트 처리
- [ ] select/poll 사용법
- [ ] 비블로킹 I/O
- [ ] 타임아웃 처리
- [ ] 프로토콜 설계
- [ ] 에러 처리 패턴

### 🔴 고급 (5-8주)
- [ ] epoll/kqueue 활용
- [ ] 연결 풀링
- [ ] 고성능 서버 설계
- [ ] 보안 고려사항
- [ ] 프로토콜 멀티플렉싱
- [ ] 로드 밸런싱

### ⚫ 전문가 (3개월+)
- [ ] Zero-copy 기법
- [ ] Kernel bypass (DPDK)
- [ ] RDMA 프로그래밍
- [ ] 커스텀 프로토콜 스택
- [ ] 고가용성 시스템 설계

## 📚 추가 학습 자료

### 권장 도서
1. **"Unix Network Programming"** - W. Richard Stevens
2. **"TCP/IP Illustrated"** - W. Richard Stevens
3. **"High Performance Browser Networking"** - Ilya Grigorik

### 온라인 리소스
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [Linux Socket Programming](https://www.linuxhowtos.org/C_C++/socket.htm)
- [RFC 793 - TCP](https://tools.ietf.org/html/rfc793)

### 실습 도구
- **Wireshark**: 패킷 분석
- **netcat**: 네트워크 디버깅
- **iperf**: 대역폭 측정
- **tcpdump**: 커맨드라인 패킷 캡처

## 🎯 12. 다음 단계 준비

이 문서를 완전히 이해했다면:

1. **TCP/IP의 작동 원리**를 설명할 수 있어야 합니다
2. **소켓 API**를 사용하여 서버와 클라이언트를 구현할 수 있어야 합니다
3. **네트워크 바이트 순서**의 중요성을 이해해야 합니다
4. **안전한 네트워크 프로그래밍**의 기본을 알아야 합니다
5. **프로토콜 설계**의 기초를 이해해야 합니다
6. **[신규] 멀티 포트 서버**를 구현할 수 있어야 합니다
7. **[신규] 프로토콜 멀티플렉싱**을 이해해야 합니다

### 확인 문제

1. TCP와 UDP의 차이점은 무엇이며, LogCaster에 TCP가 적합한 이유는?
2. `bind()` 함수의 역할과 필요성은?
3. 네트워크 바이트 순서 변환이 필요한 이유는?
4. `send()`가 요청한 바이트보다 적게 전송할 수 있는 이유는?
5. **[신규] 여러 포트를 동시에 관리하는 방법은?**
6. **[신규] 텍스트 기반 프로토콜의 라인 파싱 방법은?**

다음 문서에서는 **시스템 프로그래밍 심화**에 대해 자세히 알아보겠습니다!

---

*💡 팁: 네트워크 디버깅 도구들을 활용하세요!*
- `netstat -tln`: 열린 포트 확인
- `tcpdump`: 패킷 캡처
- `telnet`: 간단한 TCP 테스트
- `nc (netcat)`: 범용 네트워크 도구