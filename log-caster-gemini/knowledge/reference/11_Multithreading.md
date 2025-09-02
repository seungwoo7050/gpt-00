# 5. 멀티스레딩과 동시성 프로그래밍 🧵
## 하나의 프로그램이 여러 일을 동시에!

*LogCaster 프로젝트를 위한 완벽 가이드*

---

## 📋 문서 정보
- **난이도**: 🟡 중급 → 🔴 고급
- **예상 학습 시간**: 8-10시간
- **전제 조건**: 
  - C/C++ 기본 문법
  - 메모리 관리 이해
  - 프로세스 개념 이해
- **실습 환경**: Linux/macOS, GCC/G++ (pthread 지원), C++11 이상

## 🎯 이 문서에서 배울 내용

**"여러분, 멀티스레딩이 뭐라고 생각하세요? 🤔"**

간단합니다! 마치 **요리를 할 때 여러 사람이 동시에 다른 일을 하는 것**처럼, 프로그램도 여러 작업을 동시에 처리할 수 있어요!

🍳 **요리 비유**:
- 한 사람이 야채 썬기 (스레드 1)
- 다른 사람이 고기 구워기 (스레드 2)
- 또 다른 사람이 밥 짓기 (스레드 3)

결과? **요리 시간이 훨씬 단축!** 🚀

LogCaster 또한 마찬가지입니다. 여러 클라이언트가 동시에 로그를 보내도, 서버는 모두를 빠르게 처리할 수 있는 **고성능 멀티태스킹 서버**가 되어야 해요!

### 학습 목표
- 🤼 프로세스와 스레드의 근본적인 차이 이해
- 🛠️ pthreads를 사용한 스레드 생성과 관리
- ⚠️ 동시성 문제(Race Condition, Deadlock) 해결
- 🔒 동기화 메커니즘(Mutex, Condition Variable) 활용
- 🏠 효율적인 스레드 풀 설계와 구현
- **[신규] 채널별 스레드 풀 전략**
- **[신규] 브로드캐스트 동시성 패턴**

---

## 🔄 1. 프로세스 vs 스레드의 진짜 차이

**"어떤 게 더 좋은지 가장 쉽게 알아보세요! 🤔"**

### 1.1 일상생활로 이해하기

프로세스와 스레드를 **회사와 직원**에 비유해보면 딱 이해되어요:

```
🏢 프로세스 = 독립된 회사
- 각자의 사무실 (메모리 공간)
- 각자의 직원들 (스레드)
- 각자의 자원 (파일, 소켓 등)

👨‍💼 스레드 = 같은 회사의 직원들
- 같은 사무실 공유 (메모리 공유)
- 각자의 책상만 별도 (스택)
- 회사 자원 공동 사용
```

💡 **즉, 스레드는 더 빠르게 소통하지만, 서로 영향을 줄 수 있어요!**

### 1.2 프로세스 (Process)

**"독립적이지만 무거운 존재 🏭"**

**특징:**
- 🚪 **독립성**: 각각 별도의 메모리 공간 소유
- 🛡️ **안전성**: 한 프로세스가 죽어도 다른 프로세스에 영향 없음
- 💰 **비용**: 생성과 전환에 많은 자원 필요
- 💬 **통신**: IPC(프로세스 간 통신) 방식 사용 (파이프, 소켓 등)

### 1.3 스레드 (Thread)

**"빠르고 유연하지만 주의가 필요한 존재 ⚡"**

**특징:**
- 🚀 **빠른 생성**: 프로세스보다 100배 빠른 생성
- 📝 **메모리 공유**: 같은 프로세스 내 메모리 공유
- ⚠️ **위험성**: 한 스레드의 문제가 전체에 영향
- 💬 **빠른 통신**: 메모리를 직접 공유하므로 빠름

### 1.4 언제 뭐를 사용할까?

| 상황 | 추천 | 이유 |
|------|-----|------|
| 🌐 웹 서버 | 스레드 | 빠른 응답, 많은 동시 연결 |
| 📊 데이터베이스 | 프로세스 | 안정성, 장애 격리 |
| 🎮 게임 | 스레드 | 실시간 처리, 빠른 반응 |
| 🏦 백킹 시스템 | 프로세스 | 보안, 안정성 |

**LogCaster는?** 스레드! 많은 클라이언트를 빠르게 처리해야 하니까요. 🚀

### 1.5 실제 비교 예제

**🕰️ 속도 비교** (생성 속도 기준):
```
프로세스 생성: 10,000μs (10밀리초)
스레드 생성:     100μs (0.1밀리초)

차이: 100배! 🚀
```

**💾 메모리 사용량**:
```
프로세스: 8MB 기본 사용
스레드:   8KB 기본 사용 (스택만)

차이: 1000배 효율적! 💪
```
- **독립적인 메모리 공간**: 각 프로세스는 별도의 가상 주소 공간을 가짐
- **높은 안정성**: 한 프로세스의 오류가 다른 프로세스에 영향 없음
- **높은 오버헤드**: 생성/전환 비용이 큼
- **IPC 필요**: 프로세스 간 통신을 위해 특별한 메커니즘 필요

```
프로세스 A          프로세스 B
┌─────────────┐    ┌─────────────┐
│   코드영역   │    │   코드영역   │
├─────────────┤    ├─────────────┤
│  데이터영역  │    │  데이터영역  │
├─────────────┤    ├─────────────┤
│    힙       │    │    힙       │
├─────────────┤    ├─────────────┤
│   스택      │    │   스택      │
└─────────────┘    └─────────────┘
```

### 스레드 (Thread)

**특징:**
- **공유 메모리 공간**: 같은 프로세스의 스레드들은 메모리를 공유
- **빠른 생성/전환**: 컨텍스트 스위칭 비용이 적음
- **효율적인 통신**: 메모리 공유를 통한 빠른 데이터 교환
- **동기화 필요**: 공유 자원 접근 시 동기화 메커니즘 필요

```
프로세스
┌─────────────────────────────────┐
│         코드영역 (공유)          │
├─────────────────────────────────┤
│        데이터영역 (공유)         │
├─────────────────────────────────┤
│          힙 (공유)              │
├─────────────┬───────────────────┤
│스레드1 스택 │  스레드2 스택     │
│             │                   │
└─────────────┴───────────────────┘
```

### LogCaster에서 스레드를 선택하는 이유

**"왜 LogCaster는 스레드를 선택했을까요? 🤔"**

학교 급식실을 생각해보세요. 한 명이 모든 학생에게 음식을 주는 것보다, **여러 명이 동시에 서빙**하는 게 훨씬 빠르죠?

1. 🚀 **빠른 응답**: 클라이언트 연결마다 빠른 스레드 생성
   - 마치 여러 직원이 동시에 손님을 서빙하는 레스토랑처럼!
   
2. 💰 **효율적인 자원 사용**: 메모리 공유로 낮은 오버헤드
   - 같은 주방에서 요리사들이 재료를 공유하는 것처럼!
   
3. 📝 **로그 데이터 공유**: 모든 스레드가 같은 로그 저장소에 접근
   - 마치 모든 직원이 같은 주문 장부를 보는 것처럼!

---

## 🧵 2. C의 pthreads 라이브러리

**"스레드를 만드는 방법을 배워보세요! 🛠️"**

### pthreads 기본 개념

POSIX Threads(pthreads)는 **UNIX 계열 시스템의 스레드 만드는 도구상자**예요! 마치 **레고 블록을 조립하는 설명서**처럼, 스레드를 만들고 관리하는 방법을 알려주죠.

```c
#include <pthread.h>  // 🧵 스레드를 위한 도구들
#include <stdio.h>    // 📝 입출력 함수들
#include <stdlib.h>   // 💾 메모리 관리 함수들
#include <unistd.h>   // ⏰ sleep 같은 시스템 함수들

// 🔧 컴파일 방법: gcc -pthread program.c -o program
// -pthread 플래그가 중요해요! 이게 있어야 스레드를 사용할 수 있어요.
```

💡 **중요한 점**: `-pthread` 플래그를 기억하세요! 이건 컴파일러에게 "이 프로그램은 스레드를 사용해요!"라고 알려주는 거예요.

### 2.2 스레드 생성과 종료

#### 기본 스레드 생성 - 단계별 이해

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 스레드 함수 - 스레드가 실행할 코드
// 반드시 void* 타입을 반환하고, void* 인자를 받아야 함
void* thread_function(void* arg) {
    int thread_num = *(int*)arg;  // void* → int* 변환
    
    printf("👶 Thread %d: 태어났습니다!\n", thread_num);
    
    // 스레드 작업 수행
    for (int i = 0; i < 5; i++) {
        printf("   Thread %d: 작업 중... (%d/5)\n", thread_num, i + 1);
        sleep(1);  // 1초 대기 (실제 작업 시뮬레이션)
    }
    
    printf("🎯 Thread %d: 작업 완료!\n", thread_num);
    
    // 반환값 준비 (동적 할당 필요)
    int* result = malloc(sizeof(int));
    *result = thread_num * 10;  // 스레드 번호 * 10을 결과로
    
    return result;  // pthread_join()에서 받을 수 있음
}

int main() {
    pthread_t threads[3];  // 스레드 ID 저장 배열
    int thread_args[3] = {1, 2, 3};  // 각 스레드에 전달할 인자
    
    printf("🚀 메인 스레드: 프로그램 시작\n\n");
    
    // 스레드 생성
    for (int i = 0; i < 3; i++) {
        // pthread_create(스레드 ID, 속성, 스레드 함수, 인자)
        int ret = pthread_create(&threads[i], NULL, thread_function, &thread_args[i]);
        
        if (ret != 0) {
            fprintf(stderr, "❌ 스레드 %d 생성 실패\n", i);
            exit(1);
        }
        printf("🏭 메인: 스레드 %d 생성 완료\n", i + 1);
    }
    
    // 메인 스레드도 동시에 작업 가능
    printf("\n💼 메인 스레드: 내 작업 수행 중...\n");
    sleep(2);
    printf("💼 메인 스레드: 내 작업 완료\n\n");
    
    // 모든 스레드가 완료될 때까지 대기
    printf("🕰️ 메인: 모든 스레드 종료 대기 중...\n\n");
    
    for (int i = 0; i < 3; i++) {
        void* result;
        // pthread_join: 스레드 종료 대기 및 결과 수신
        int ret = pthread_join(threads[i], &result);
        
        if (ret == 0) {
            printf("✅ 스레드 %d 종료, 결과: %d\n", 
                   i + 1, *(int*)result);
            free(result);  // malloc으로 할당한 메모리 해제
        }
    }
    
    printf("\n🎆 모든 스레드 종료 완료!\n");
    return 0;
}

/* 실행 결과 예시:
🚀 메인 스레드: 프로그램 시작

🏭 메인: 스레드 1 생성 완료
👶 Thread 1: 태어났습니다!
🏭 메인: 스레드 2 생성 완료
   Thread 1: 작업 중... (1/5)
👶 Thread 2: 태어났습니다!
🏭 메인: 스레드 3 생성 완료
   Thread 2: 작업 중... (1/5)
👶 Thread 3: 태어났습니다!

💼 메인 스레드: 내 작업 수행 중...
   Thread 3: 작업 중... (1/5)
...
*/
```

#### 스레드 분리 (Detached Thread)

**"때로는 스레드를 '자유롭게' 떠나보내야 할 때가 있어요! 🕊️"**

일반적으로는 자식이 집에 오기를 기다리죠(`pthread_join`). 하지만 때로는 **"알아서 해라, 나는 기다리지 않을게!"**라고 할 때가 있어요. 그게 바로 **Detached Thread**입니다.

```c
// 🆓 스레드를 분리 상태로 생성 (자동으로 정리됨)
void create_detached_thread() {
    pthread_t thread;
    pthread_attr_t attr;  // 스레드 속성 (마치 설정집처럼)
    
    // 🛠️ 스레드 속성 초기화
    pthread_attr_init(&attr);
    
    // 🔓 분리 상태로 설정 - "자유롭게 살아라!"
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    // 🚀 스레드 생성
    pthread_create(&thread, &attr, thread_function, NULL);
    
    // 🧹 속성 객체 정리 (안 하면 메모리 누수!)
    pthread_attr_destroy(&attr);
    
    // ✨ pthread_join 불필요! 스레드가 알아서 자신을 정리해요
}

// 🔄 또는 이미 실행 중인 스레드를 나중에 분리
void detach_running_thread(pthread_t thread) {
    pthread_detach(thread);  // "이제부터 자유롭게 살아!"
}
```

🤔 **언제 Detached Thread를 사용할까요?**
- 🌐 웹 서버: 클라이언트 요청 처리 후 결과를 기다리지 않을 때
- 📰 로그 수집: 로그만 기록하고 끝날 때
- 📧 백그라운드 작업: 정리나 모니터링 작업

### LogCaster용 클라이언트 처리 스레드

**"이제 진짜 LogCaster에서 쓰일 코드를 만들어보세요! 🛠️"**

각 클라이언트가 연결되면, **전담 직원을 배정**해서 그 클라이언트만 담당하게 할 거예요. 이게 바로 LogCaster의 핵심 아이디어입니다!

```c
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// 📄 클라이언트 정보 구조체 - 각 클라이언트의 '신분증'
typedef struct {
    int client_fd;                    // 클라이언트와 연결된 소켓 번호
    struct sockaddr_in client_addr;   // 클라이언트의 IP 주소와 포트
    int thread_id;                    // 이 클라이언트를 담당하는 스레드 번호
} ClientInfo;

// 📃 로그 엔트리 구조체 - 하나의 '로그 사건'
typedef struct LogEntry {
    char message[256];                // 로그 메시지 내용
    time_t timestamp;                 // 언제 발생했는지
    char client_ip[INET_ADDRSTRLEN];  // 어느 클라이언트에서 보냈는지
    int client_port;                  // 클라이언트 포트 번호
    struct LogEntry* next;            // 다음 로그를 가리키는 포인터 (연결 리스트)
} LogEntry;

// 🌍 전역 로그 리스트 (모든 스레드가 공유하는 '공용 장부')
LogEntry* log_head = NULL;  // 처음 로그를 가리키는 포인터
int total_logs = 0;         // 지금까지 수집된 로그 총 개수

// ⚠️ 주의: 이 전역 변수들은 모든 스레드가 동시에 접근하므로
//       나중에 동기화(락) 처리가 필요해요!

// 클라이언트 처리 스레드 함수
void* handle_client_thread(void* arg) {
    ClientInfo* client_info = (ClientInfo*)arg;
    int client_fd = client_info->client_fd;
    char client_ip[INET_ADDRSTRLEN];
    char buffer[256];
    
    // 클라이언트 IP 주소 문자열로 변환
    inet_ntop(AF_INET, &client_info->client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    
    printf("[Thread %d] Client connected: %s:%d\n", 
           client_info->thread_id, client_ip, ntohs(client_info->client_addr.sin_port));
    
    // 클라이언트로부터 메시지 수신 및 처리
    while (1) {
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            // 연결 종료 또는 오류
            break;
        }
        
        buffer[bytes_received] = '\0';
        
        printf("[Thread %d] Received: %s", client_info->thread_id, buffer);
        
        // 로그 엔트리 생성 (동기화는 나중에 추가 예정)
        LogEntry* new_entry = malloc(sizeof(LogEntry));
        strncpy(new_entry->message, buffer, sizeof(new_entry->message) - 1);
        new_entry->message[sizeof(new_entry->message) - 1] = '\0';
        new_entry->timestamp = time(NULL);
        strncpy(new_entry->client_ip, client_ip, sizeof(new_entry->client_ip) - 1);
        new_entry->client_port = ntohs(client_info->client_addr.sin_port);
        
        // 리스트 앞에 추가 (현재는 thread-unsafe)
        new_entry->next = log_head;
        log_head = new_entry;
        total_logs++;
        
        // 확인 응답 전송
        const char* ack = "ACK\n";
        send(client_fd, ack, strlen(ack), 0);
    }
    
    printf("[Thread %d] Client disconnected: %s:%d\n", 
           client_info->thread_id, client_ip, ntohs(client_info->client_addr.sin_port));
    
    close(client_fd);
    free(client_info);  // 클라이언트 정보 메모리 해제
    
    return NULL;
}

// 멀티스레드 서버 메인 루프
int run_multithreaded_server(int port) {
    int server_fd = create_server_socket(port);
    if (server_fd == -1) {
        return -1;
    }
    
    printf("Multithreaded LogCaster server started on port %d\n", port);
    
    int thread_counter = 0;
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // 클라이언트 연결 수락
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("accept failed");
            continue;
        }
        
        // 클라이언트 정보 구조체 생성
        ClientInfo* client_info = malloc(sizeof(ClientInfo));
        client_info->client_fd = client_fd;
        client_info->client_addr = client_addr;
        client_info->thread_id = ++thread_counter;
        
        // 새 스레드에서 클라이언트 처리
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client_thread, client_info) != 0) {
            perror("pthread_create failed");
            close(client_fd);
            free(client_info);
            continue;
        }
        
        // 스레드를 분리 상태로 설정 (자동 정리)
        pthread_detach(client_thread);
        
        printf("Created thread %d for new client\n", thread_counter);
    }
    
    close(server_fd);
    return 0;
}
```

---

## 🔒 3. 동시성 문제와 해결방법 - 스레드의 덤짜

### 3.1 Race Condition (경쟁 상태) - 누가 먼저?

여러 스레드가 동시에 공유 자원에 접근할 때 발생하는 문제입니다.

#### 일상생활 비유
```
은행 계좌 예시:
- 남편과 아내가 같은 계좌에서 동시에 돈을 인출
- 잔액: 100만원

남편: 잔액 확인 (100만원) → 50만원 인출 → 잔액 업데이트
아내: 잔액 확인 (100만원) → 70만원 인출 → 잔액 업데이트

결과: 잘못된 잔액 (둘 다 100만원을 기준으로 계산)
```

#### 문제 상황 코드

```c
#include <pthread.h>
#include <stdio.h>

int shared_counter = 0;  // 공유 변수 (위험!)

void* increment_counter(void* arg) {
    int thread_id = *(int*)arg;
    
    for (int i = 0; i < 100000; i++) {
        // 위험한 코드: Race Condition 발생 가능
        shared_counter++;  
        // 이 한 줄이 실제로는:
        // 1. temp = shared_counter  (현재 값 읽기)
        // 2. temp = temp + 1       (증가)
        // 3. shared_counter = temp (다시 쓰기)
        // 
        // 문제: 여러 스레드가 동시에 1번을 수행하면?
    }
    
    printf("Thread %d finished\n", thread_id);
    return NULL;
}

/* Race Condition 발생 예:
시간  Thread 1              Thread 2              shared_counter
----  -------------------   -------------------   --------------
  1   temp = counter (0)                          0
  2                         temp = counter (0)     0
  3   temp = temp + 1 (1)                         0
  4                         temp = temp + 1 (1)   0
  5   counter = temp (1)                          1
  6                         counter = temp (1)    1  ← 2가 되어야 하는데!
*/

int main() {
    pthread_t threads[4];
    int thread_ids[4] = {1, 2, 3, 4};
    
    // 4개의 스레드가 동시에 카운터 증가
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, increment_counter, &thread_ids[i]);
    }
    
    // 모든 스레드 완료 대기
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Expected: 400000, Actual: %d\n", shared_counter);
    // 실제 결과는 400000보다 작을 가능성이 높음
    
    return 0;
}
```

### 3.2 Deadlock (교착 상태) - 서로를 기다리는 스레드들

두 개 이상의 스레드가 서로가 가진 자원을 기다리며 무한히 대기하는 상황입니다.

#### 일상생활 비유
```
식당에서:
- 철수: 젖가락을 들고 숙가락을 기다림
- 영희: 숙가락을 들고 젖가락을 기다림
→ 둘 다 영원히 기다림 (Deadlock!)
```

#### Deadlock 발생 예시

```c
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread1_function(void* arg) {
    printf("Thread 1: Acquiring mutex1\n");
    pthread_mutex_lock(&mutex1);
    
    sleep(1);  // 의도적 지연
    
    printf("Thread 1: Acquiring mutex2\n");
    pthread_mutex_lock(&mutex2);  // Thread 2가 이미 가지고 있음
    
    // 작업 수행...
    
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    
    return NULL;
}

void* thread2_function(void* arg) {
    printf("Thread 2: Acquiring mutex2\n");
    pthread_mutex_lock(&mutex2);
    
    sleep(1);  // 의도적 지연
    
    printf("Thread 2: Acquiring mutex1\n");
    pthread_mutex_lock(&mutex1);  // Thread 1이 이미 가지고 있음
    
    // 작업 수행...
    
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    
    return NULL;
}

// Deadlock 방지법: 항상 같은 순서로 뮤텍스 획득
void* safe_thread1(void* arg) {
    pthread_mutex_lock(&mutex1);  // 항상 mutex1을 먼저
    pthread_mutex_lock(&mutex2);  // 그 다음 mutex2
    
    // 작업 수행...
    
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    
    return NULL;
}

void* safe_thread2(void* arg) {
    pthread_mutex_lock(&mutex1);  // 항상 mutex1을 먼저
    pthread_mutex_lock(&mutex2);  // 그 다음 mutex2
    
    // 작업 수행...
    
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    
    return NULL;
}
```

---

## 🔐 4. 동기화 메커니즘 - 스레드들의 교통 정리

### 4.1 Mutex (뮤텍스) - 화장실 열쇠

Mutual Exclusion의 줄임말로, 한 번에 하나의 스레드만 임계 영역에 접근할 수 있도록 보장합니다.

#### 일상생활 비유
```
화장실 사용:
1. 문 잠금 (mutex_lock)     - 다른 사람 못 들어옴
2. 화장실 사용             - 안전하게 혼자 사용
3. 문 열기 (mutex_unlock)   - 다음 사람이 사용 가능
```

#### 기본 뮤텍스 사용법

```c
#include <pthread.h>

// 뮤텍스 선언 방법 1: 정적 초기화
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// 뮤텍스 선언 방법 2: 동적 초기화
pthread_mutex_t log_mutex;
pthread_mutex_init(&log_mutex, NULL);

// 뮤텍스를 사용한 안전한 함수
void safe_add_log(const char* message) {
    // 1. 화장실 문 잠금 (다른 스레드 대기)
    pthread_mutex_lock(&log_mutex);
    
    // 2. 임계 영역 (Critical Section) - 혼자만 실행
    // 여기서 공유 자원에 안전하게 접근
    LogEntry* new_entry = malloc(sizeof(LogEntry));
    strcpy(new_entry->message, message);
    new_entry->timestamp = time(NULL);
    new_entry->next = log_head;
    log_head = new_entry;
    total_logs++;
    
    // 3. 화장실 문 열기 (다음 스레드 사용 가능)
    pthread_mutex_unlock(&log_mutex);
}

// 프로그램 종료 시 뮤텍스 정리
void cleanup_mutex() {
    pthread_mutex_destroy(&log_mutex);
}

/* 뮤텍스 사용 규칙:
1. lock하면 반드시 unlock해야 함
2. 임계 영역은 최대한 짧게
3. 중첩 lock 금지 (데드락 위험)
*/
```

#### LogCaster에서 Thread-Safe 로그 관리

```c
#include <pthread.h>
#include <time.h>
#include <string.h>

// 전역 변수들
LogEntry* log_head = NULL;
int total_logs = 0;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread-safe 로그 추가
void thread_safe_add_log(const char* message, const char* client_ip, int client_port) {
    // 로그 엔트리 미리 생성 (뮤텍스 외부에서)
    LogEntry* new_entry = malloc(sizeof(LogEntry));
    if (new_entry == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    // 데이터 설정
    strncpy(new_entry->message, message, sizeof(new_entry->message) - 1);
    new_entry->message[sizeof(new_entry->message) - 1] = '\0';
    new_entry->timestamp = time(NULL);
    strncpy(new_entry->client_ip, client_ip, sizeof(new_entry->client_ip) - 1);
    new_entry->client_ip[sizeof(new_entry->client_ip) - 1] = '\0';
    new_entry->client_port = client_port;
    
    // 임계 영역: 리스트에 추가
    pthread_mutex_lock(&log_mutex);
    
    new_entry->next = log_head;
    log_head = new_entry;
    total_logs++;
    
    pthread_mutex_unlock(&log_mutex);
    
    printf("Log added. Total logs: %d\n", total_logs);
}

// Thread-safe 로그 검색
int thread_safe_search_logs(const char* keyword, LogEntry** results, int max_results) {
    int found_count = 0;
    
    pthread_mutex_lock(&log_mutex);
    
    LogEntry* current = log_head;
    while (current != NULL && found_count < max_results) {
        if (strstr(current->message, keyword) != NULL) {
            // 결과 복사 (포인터만 저장하면 뮤텍스 해제 후 위험)
            results[found_count] = malloc(sizeof(LogEntry));
            memcpy(results[found_count], current, sizeof(LogEntry));
            results[found_count]->next = NULL;  // 링크 끊기
            found_count++;
        }
        current = current->next;
    }
    
    pthread_mutex_unlock(&log_mutex);
    
    return found_count;
}

// Thread-safe 로그 출력
void thread_safe_print_all_logs() {
    pthread_mutex_lock(&log_mutex);
    
    printf("=== All Logs (%d entries) ===\n", total_logs);
    
    LogEntry* current = log_head;
    int count = 0;
    while (current != NULL) {
        printf("[%d] %s from %s:%d at %s", 
               ++count,
               current->message,
               current->client_ip,
               current->client_port,
               ctime(&current->timestamp));
        current = current->next;
    }
    
    pthread_mutex_unlock(&log_mutex);
}

// 업데이트된 클라이언트 처리 함수
void* handle_client_thread_safe(void* arg) {
    ClientInfo* client_info = (ClientInfo*)arg;
    int client_fd = client_info->client_fd;
    char client_ip[INET_ADDRSTRLEN];
    char buffer[256];
    
    inet_ntop(AF_INET, &client_info->client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_info->client_addr.sin_port);
    
    printf("[Thread %d] Client connected: %s:%d\n", 
           client_info->thread_id, client_ip, client_port);
    
    while (1) {
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            break;
        }
        
        buffer[bytes_received] = '\0';
        
        // Thread-safe 로그 추가
        thread_safe_add_log(buffer, client_ip, client_port);
        
        // 확인 응답
        const char* ack = "ACK\n";
        send(client_fd, ack, strlen(ack), 0);
    }
    
    printf("[Thread %d] Client disconnected: %s:%d\n", 
           client_info->thread_id, client_ip, client_port);
    
    close(client_fd);
    free(client_info);
    
    return NULL;
}
```

### Reader-Writer Lock

읽기 작업은 동시에 허용하되, 쓰기 작업은 독점적으로 수행하는 동기화 메커니즘입니다.

```c
#include <pthread.h>

pthread_rwlock_t log_rwlock = PTHREAD_RWLOCK_INITIALIZER;

// 읽기 전용 작업 (여러 스레드가 동시 실행 가능)
void read_logs() {
    pthread_rwlock_rdlock(&log_rwlock);
    
    // 로그 읽기 작업
    LogEntry* current = log_head;
    while (current != NULL) {
        printf("Log: %s\n", current->message);
        current = current->next;
    }
    
    pthread_rwlock_unlock(&log_rwlock);
}

// 쓰기 작업 (독점적 실행)
void write_log(const char* message) {
    pthread_rwlock_wrlock(&log_rwlock);
    
    // 로그 추가 작업
    LogEntry* new_entry = malloc(sizeof(LogEntry));
    strcpy(new_entry->message, message);
    new_entry->next = log_head;
    log_head = new_entry;
    
    pthread_rwlock_unlock(&log_rwlock);
}

// 정리
void cleanup_rwlock() {
    pthread_rwlock_destroy(&log_rwlock);
}
```

### Condition Variable (조건 변수)

특정 조건이 만족될 때까지 스레드를 대기시키는 동기화 메커니즘입니다.

```c
#include <pthread.h>

// 생산자-소비자 패턴 예시
typedef struct {
    char** buffer;
    int head;
    int tail;
    int count;
    int capacity;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;  // 버퍼가 비어있지 않음
    pthread_cond_t not_full;   // 버퍼가 가득 차지 않음
} LogBuffer;

// 버퍼 초기화
LogBuffer* create_log_buffer(int capacity) {
    LogBuffer* buffer = malloc(sizeof(LogBuffer));
    buffer->buffer = malloc(sizeof(char*) * capacity);
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;
    buffer->capacity = capacity;
    
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
    
    return buffer;
}

// 생산자: 로그 추가
void buffer_add_log(LogBuffer* buffer, const char* message) {
    pthread_mutex_lock(&buffer->mutex);
    
    // 버퍼가 가득 찰 때까지 대기
    while (buffer->count == buffer->capacity) {
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }
    
    // 로그 추가
    buffer->buffer[buffer->tail] = malloc(strlen(message) + 1);
    strcpy(buffer->buffer[buffer->tail], message);
    buffer->tail = (buffer->tail + 1) % buffer->capacity;
    buffer->count++;
    
    // 소비자에게 알림
    pthread_cond_signal(&buffer->not_empty);
    
    pthread_mutex_unlock(&buffer->mutex);
}

// 소비자: 로그 가져오기
char* buffer_get_log(LogBuffer* buffer) {
    pthread_mutex_lock(&buffer->mutex);
    
    // 버퍼가 비어있을 때까지 대기
    while (buffer->count == 0) {
        pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
    }
    
    // 로그 가져오기
    char* message = buffer->buffer[buffer->head];
    buffer->head = (buffer->head + 1) % buffer->capacity;
    buffer->count--;
    
    // 생산자에게 알림
    pthread_cond_signal(&buffer->not_full);
    
    pthread_mutex_unlock(&buffer->mutex);
    
    return message;
}

// 로그 처리 워커 스레드
void* log_processor_thread(void* arg) {
    LogBuffer* buffer = (LogBuffer*)arg;
    
    while (1) {
        char* message = buffer_get_log(buffer);
        
        // 로그 처리 (예: 파일에 쓰기, 네트워크 전송 등)
        printf("Processing log: %s", message);
        
        // 처리 시뮬레이션
        usleep(100000);  // 100ms 지연
        
        free(message);
    }
    
    return NULL;
}
```

---

## 🏭 5. 스레드 풀 패턴

스레드를 미리 생성해두고 재사용하는 패턴으로, 스레드 생성/소멸 오버헤드를 줄입니다.

### 기본 스레드 풀 구현

```c
#include <pthread.h>
#include <stdbool.h>

// 작업 큐의 노드
typedef struct Task {
    void (*function)(void* arg);  // 수행할 함수
    void* arg;                    // 함수 인자
    struct Task* next;            // 다음 작업
} Task;

// 스레드 풀 구조체
typedef struct ThreadPool {
    pthread_t* threads;           // 워커 스레드 배열
    int thread_count;             // 스레드 개수
    
    Task* task_queue_head;        // 작업 큐 헤드
    Task* task_queue_tail;        // 작업 큐 테일
    int task_count;               // 대기 중인 작업 수
    
    pthread_mutex_t queue_mutex;  // 큐 접근 뮤텍스
    pthread_cond_t queue_cond;    // 작업 대기 조건 변수
    
    bool shutdown;                // 종료 플래그
} ThreadPool;

// 워커 스레드 함수
void* worker_thread(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;
    
    while (1) {
        pthread_mutex_lock(&pool->queue_mutex);
        
        // 작업이 있을 때까지 대기
        while (pool->task_count == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
        }
        
        // 종료 신호 확인
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }
        
        // 작업 가져오기
        Task* task = pool->task_queue_head;
        if (task != NULL) {
            pool->task_queue_head = task->next;
            if (pool->task_queue_head == NULL) {
                pool->task_queue_tail = NULL;
            }
            pool->task_count--;
        }
        
        pthread_mutex_unlock(&pool->queue_mutex);
        
        // 작업 수행
        if (task != NULL) {
            task->function(task->arg);
            free(task);
        }
    }
    
    return NULL;
}

// 스레드 풀 생성
ThreadPool* create_thread_pool(int thread_count) {
    ThreadPool* pool = malloc(sizeof(ThreadPool));
    if (pool == NULL) return NULL;
    
    pool->threads = malloc(sizeof(pthread_t) * thread_count);
    pool->thread_count = thread_count;
    pool->task_queue_head = NULL;
    pool->task_queue_tail = NULL;
    pool->task_count = 0;
    pool->shutdown = false;
    
    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->queue_cond, NULL);
    
    // 워커 스레드 생성
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            // 오류 처리
            destroy_thread_pool(pool);
            return NULL;
        }
    }
    
    return pool;
}

// 작업 추가
int add_task_to_pool(ThreadPool* pool, void (*function)(void*), void* arg) {
    if (pool->shutdown) {
        return -1;  // 이미 종료됨
    }
    
    Task* new_task = malloc(sizeof(Task));
    if (new_task == NULL) {
        return -1;
    }
    
    new_task->function = function;
    new_task->arg = arg;
    new_task->next = NULL;
    
    pthread_mutex_lock(&pool->queue_mutex);
    
    // 큐에 작업 추가
    if (pool->task_queue_tail == NULL) {
        pool->task_queue_head = new_task;
        pool->task_queue_tail = new_task;
    } else {
        pool->task_queue_tail->next = new_task;
        pool->task_queue_tail = new_task;
    }
    
    pool->task_count++;
    
    // 대기 중인 워커 스레드에 알림
    pthread_cond_signal(&pool->queue_cond);
    
    pthread_mutex_unlock(&pool->queue_mutex);
    
    return 0;
}

// 스레드 풀 종료
void destroy_thread_pool(ThreadPool* pool) {
    if (pool == NULL) return;
    
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = true;
    pthread_cond_broadcast(&pool->queue_cond);  // 모든 워커 깨우기
    pthread_mutex_unlock(&pool->queue_mutex);
    
    // 모든 스레드가 종료될 때까지 대기
    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    // 남은 작업들 정리
    Task* current = pool->task_queue_head;
    while (current != NULL) {
        Task* next = current->next;
        free(current);
        current = next;
    }
    
    // 자원 정리
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_cond);
    free(pool->threads);
    free(pool);
}

// LogCaster에서 스레드 풀 사용
void handle_client_task(void* arg) {
    ClientInfo* client_info = (ClientInfo*)arg;
    
    // 기존 handle_client_thread_safe 함수의 내용
    handle_client_thread_safe(client_info);
}

// 스레드 풀 기반 서버
int run_thread_pool_server(int port, int pool_size) {
    ThreadPool* pool = create_thread_pool(pool_size);
    if (pool == NULL) {
        fprintf(stderr, "Failed to create thread pool\n");
        return -1;
    }
    
    int server_fd = create_server_socket(port);
    if (server_fd == -1) {
        destroy_thread_pool(pool);
        return -1;
    }
    
    printf("Thread pool server started (pool size: %d, port: %d)\n", pool_size, port);
    
    int thread_counter = 0;
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("accept failed");
            continue;
        }
        
        // 클라이언트 정보 생성
        ClientInfo* client_info = malloc(sizeof(ClientInfo));
        client_info->client_fd = client_fd;
        client_info->client_addr = client_addr;
        client_info->thread_id = ++thread_counter;
        
        // 스레드 풀에 작업 추가
        if (add_task_to_pool(pool, handle_client_task, client_info) != 0) {
            fprintf(stderr, "Failed to add task to pool\n");
            close(client_fd);
            free(client_info);
        }
    }
    
    close(server_fd);
    destroy_thread_pool(pool);
    return 0;
}
```

---

## 🆚 6. C++의 std::thread - 더 편한 스레드 프로그래밍

### 6.1 C++ 스레드의 장점

```cpp
#include <thread>
#include <iostream>
#include <mutex>
#include <vector>

// C++ 스레드의 장점:
// 1. RAII (Resource Acquisition Is Initialization)
// 2. 람다 함수 지원
// 3. move 의미론
// 4. 타입 안전성

void simple_thread_example() {
    // 1. 간단한 스레드 생성
    std::thread t1([]() {
        std::cout << "Hello from thread!" << std::endl;
    });
    
    // 2. 인자가 있는 스레드
    int value = 42;
    std::thread t2([](int x) {
        std::cout << "Value: " << x << std::endl;
    }, value);
    
    // 3. 스레드 종료 대기
    t1.join();
    t2.join();
}
```

### 6.2 LogCaster C++ 버전

```cpp
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <chrono>
#include <iostream>
#include <sstream>

// C++에서의 로그 엔트리
struct LogEntry {
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    std::string client_ip;
    int client_port;
    
    LogEntry(const std::string& msg, const std::string& ip, int port)
        : message(msg), client_ip(ip), client_port(port),
          timestamp(std::chrono::system_clock::now()) {}
};

// Thread-Safe 로그 매니저
class LogManager {
private:
    std::vector<std::unique_ptr<LogEntry>> logs;
    mutable std::shared_mutex log_mutex;  // C++17 reader-writer lock
    
public:
    // 로그 추가 (쓰기 작업)
    void add_log(const std::string& message, const std::string& client_ip, int port) {
        std::unique_lock<std::shared_mutex> lock(log_mutex);
        logs.push_back(std::make_unique<LogEntry>(message, client_ip, port));
    }
    
    // 로그 검색 (읽기 작업 - 여러 스레드 동시 가능)
    std::vector<LogEntry> search_logs(const std::string& keyword) const {
        std::shared_lock<std::shared_mutex> lock(log_mutex);  // 읽기 잠금
        std::vector<LogEntry> results;
        
        for (const auto& log : logs) {
            if (log->message.find(keyword) != std::string::npos) {
                results.push_back(*log);  // 복사본 반환
            }
        }
        
        return results;
    }
    
    // 전체 로그 수 (읽기 작업)
    size_t get_log_count() const {
        std::shared_lock<std::shared_mutex> lock(log_mutex);
        return logs.size();
    }
};

// 완전한 LogCaster 서버 구현
class LogCasterServer {
private:
    // 스레드 풀 관련
    std::vector<std::thread> worker_threads;
    std::queue<std::pair<int, sockaddr_in>> client_queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::atomic<bool> shutdown{false};
    
    // 로그 관리
    LogManager log_manager;
    
    // 통계
    std::atomic<int> active_connections{0};
    std::atomic<uint64_t> total_connections{0};
    
public:
    LogCasterServer(int num_threads) {
        std::cout << "🚀 Starting LogCaster server with " 
                  << num_threads << " worker threads\n";
        
        // 워커 스레드 풀 생성
        for (int i = 0; i < num_threads; ++i) {
            worker_threads.emplace_back([this, i] {
                worker_thread_function(i);
            });
        }
    }
    
    ~LogCasterServer() {
        std::cout << "🛑 Shutting down server...\n";
        
        // 종료 신호
        shutdown = true;
        cv.notify_all();
        
        // 모든 스레드 종료 대기
        for (auto& t : worker_threads) {
            if (t.joinable()) {
                t.join();
            }
        }
        
        std::cout << "✅ Server shutdown complete\n";
        std::cout << "📊 Total connections handled: " << total_connections << "\n";
        std::cout << "📊 Total logs collected: " << log_manager.get_log_count() << "\n";
    }
    
    // 새 클라이언트 연결 추가
    void add_client(int client_fd, sockaddr_in addr) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            client_queue.push({client_fd, addr});
        }
        cv.notify_one();  // 하나의 워커 깨우기
        
        total_connections++;
    }
    
    // 로그 검색 인터페이스
    std::vector<LogEntry> search_logs(const std::string& keyword) {
        return log_manager.search_logs(keyword);
    }
    
    // 서버 상태 출력
    void print_status() {
        std::cout << "\n=== Server Status ===\n";
        std::cout << "Active connections: " << active_connections << "\n";
        std::cout << "Total connections: " << total_connections << "\n";
        std::cout << "Total logs: " << log_manager.get_log_count() << "\n";
        std::cout << "==================\n\n";
    }
    
private:
    void worker_thread_function(int thread_id) {
        std::cout << "[Thread " << thread_id << "] Worker started\n";
        
        while (!shutdown) {
            std::unique_lock<std::mutex> lock(queue_mutex);
            
            // 클라이언트 대기 (조건 변수 사용)
            cv.wait(lock, [this] { 
                return !client_queue.empty() || shutdown; 
            });
            
            if (shutdown) break;
            
            // C++17 structured binding
            auto [client_fd, addr] = client_queue.front();
            client_queue.pop();
            lock.unlock();  // 큐 접근 완료, 락 해제
            
            // 클라이언트 처리
            handle_client(thread_id, client_fd, addr);
        }
        
        std::cout << "[Thread " << thread_id << "] Worker stopped\n";
    }
    
    void handle_client(int thread_id, int client_fd, sockaddr_in& addr) {
        active_connections++;
        
        // 클라이언트 IP 주소 추출
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);
        std::string client_ip(ip_str);
        int client_port = ntohs(addr.sin_port);
        
        std::cout << "[Thread " << thread_id << "] Client connected: " 
                  << client_ip << ":" << client_port << "\n";
        
        // 메시지 수신 및 처리
        char buffer[1024];
        while (true) {
            ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes <= 0) {
                break;  // 연결 종료
            }
            
            buffer[bytes] = '\0';
            std::string message(buffer);
            
            // 로그 저장
            log_manager.add_log(message, client_ip, client_port);
            
            // 응답 전송
            std::string response = "ACK: " + std::to_string(bytes) + " bytes received\n";
            send(client_fd, response.c_str(), response.length(), 0);
        }
        
        close(client_fd);
        active_connections--;
        
        std::cout << "[Thread " << thread_id << "] Client disconnected: " 
                  << client_ip << ":" << client_port << "\n";
    }
};

// 사용 예시
int main() {
    const int PORT = 8080;
    const int THREAD_POOL_SIZE = 4;
    
    // 서버 인스턴스 생성
    LogCasterServer server(THREAD_POOL_SIZE);
    
    // 서버 소켓 생성
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }
    
    // SO_REUSEADDR 옵션 설정
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 바인드
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }
    
    // 리슨
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }
    
    std::cout << "📡 Server listening on port " << PORT << "\n\n";
    
    // 상태 출력 스레드 (선택적)
    std::thread status_thread([&server] {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            server.print_status();
        }
    });
    status_thread.detach();
    
    // 메인 accept 루프
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cerr << "Accept failed\n";
            continue;
        }
        
        // 스레드 풀에 클라이언트 추가
        server.add_client(client_fd, client_addr);
    }
    
    close(server_fd);
    return 0;
}
```

### 6.3 C++ 고급 동시성 기능

#### std::async와 Future

```cpp
#include <future>
#include <chrono>

// 비동기 로그 분석 예시
class LogAnalyzer {
public:
    // 비동기로 로그 분석 시작
    std::future<int> analyze_logs_async(const std::vector<LogEntry>& logs) {
        return std::async(std::launch::async, [logs] {
            int error_count = 0;
            
            for (const auto& log : logs) {
                if (log.message.find("ERROR") != std::string::npos) {
                    error_count++;
                }
                
                // CPU 집약적인 작업 시뮬레이션
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            return error_count;
        });
    }
    
    // 여러 분석 작업을 병렬로 수행
    void parallel_analysis() {
        std::vector<std::future<int>> futures;
        
        // 여러 비동기 작업 시작
        for (int i = 0; i < 4; ++i) {
            futures.push_back(std::async(std::launch::async, [i] {
                std::cout << "Analysis " << i << " started\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                return i * 10;
            }));
        }
        
        // 모든 결과 수집
        for (auto& future : futures) {
            int result = future.get();  // 블로킹
            std::cout << "Result: " << result << "\n";
        }
    }
};
```

#### std::atomic - Lock-Free 프로그래밍

```cpp
#include <atomic>

// Lock-free 통계 수집기
class Statistics {
private:
    std::atomic<uint64_t> total_logs{0};
    std::atomic<uint64_t> error_logs{0};
    std::atomic<uint64_t> warning_logs{0};
    
public:
    // Lock-free 증가
    void increment_total() {
        total_logs.fetch_add(1, std::memory_order_relaxed);
    }
    
    void increment_errors() {
        error_logs.fetch_add(1, std::memory_order_relaxed);
    }
    
    void increment_warnings() {
        warning_logs.fetch_add(1, std::memory_order_relaxed);
    }
    
    // 스냅샷 가져오기
    void get_snapshot(uint64_t& total, uint64_t& errors, uint64_t& warnings) {
        // memory_order_acquire로 읽기
        total = total_logs.load(std::memory_order_acquire);
        errors = error_logs.load(std::memory_order_acquire);
        warnings = warning_logs.load(std::memory_order_acquire);
    }
};

// Lock-free 큐 (단순화된 버전)
template<typename T>
class LockFreeQueue {
private:
    struct Node {
        std::atomic<T*> data;
        std::atomic<Node*> next;
        
        Node() : data(nullptr), next(nullptr) {}
    };
    
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    
public:
    LockFreeQueue() {
        Node* dummy = new Node;
        head.store(dummy);
        tail.store(dummy);
    }
    
    void push(T item) {
        Node* new_node = new Node;
        T* data = new T(std::move(item));
        new_node->data.store(data);
        
        Node* prev_tail = tail.exchange(new_node);
        prev_tail->next.store(new_node);
    }
    
    bool pop(T& result) {
        Node* head_node = head.load();
        Node* next = head_node->next.load();
        
        if (next == nullptr) {
            return false;  // 큐가 비어있음
        }
        
        T* data = next->data.load();
        if (data == nullptr) {
            return false;
        }
        
        result = std::move(*data);
        head.store(next);
        
        delete head_node;
        delete data;
        
        return true;
    }
};
```

## 🎨 7. 멀티스레딩 디자인 패턴

### 7.1 생산자-소비자 패턴

```c
// 완전한 생산자-소비자 구현
typedef struct {
    void** items;           // 아이템 배열
    int capacity;           // 최대 용량
    int size;              // 현재 크기
    int head;              // 시작 인덱스
    int tail;              // 끝 인덱스
    pthread_mutex_t mutex;  // 뮤텍스
    pthread_cond_t not_full;   // 가득 차지 않음
    pthread_cond_t not_empty;  // 비어있지 않음
} BoundedBuffer;

// 버퍼 생성
BoundedBuffer* create_buffer(int capacity) {
    BoundedBuffer* buffer = malloc(sizeof(BoundedBuffer));
    buffer->items = malloc(sizeof(void*) * capacity);
    buffer->capacity = capacity;
    buffer->size = 0;
    buffer->head = 0;
    buffer->tail = 0;
    
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);
    
    return buffer;
}

// 생산자: 아이템 추가
void produce(BoundedBuffer* buffer, void* item) {
    pthread_mutex_lock(&buffer->mutex);
    
    // 버퍼가 가득 찰 때까지 대기
    while (buffer->size == buffer->capacity) {
        printf("Buffer full, producer waiting...\n");
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }
    
    // 아이템 추가
    buffer->items[buffer->tail] = item;
    buffer->tail = (buffer->tail + 1) % buffer->capacity;
    buffer->size++;
    
    printf("Produced item. Buffer size: %d/%d\n", 
           buffer->size, buffer->capacity);
    
    // 소비자에게 신호
    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);
}

// 소비자: 아이템 가져오기
void* consume(BoundedBuffer* buffer) {
    pthread_mutex_lock(&buffer->mutex);
    
    // 버퍼가 빌 때까지 대기
    while (buffer->size == 0) {
        printf("Buffer empty, consumer waiting...\n");
        pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
    }
    
    // 아이템 가져오기
    void* item = buffer->items[buffer->head];
    buffer->head = (buffer->head + 1) % buffer->capacity;
    buffer->size--;
    
    printf("Consumed item. Buffer size: %d/%d\n", 
           buffer->size, buffer->capacity);
    
    // 생산자에게 신호
    pthread_cond_signal(&buffer->not_full);
    pthread_mutex_unlock(&buffer->mutex);
    
    return item;
}
```

### 7.2 읽기-쓰기 패턴 (Multiple Readers, Single Writer)

```c
typedef struct {
    int data;
    pthread_rwlock_t rwlock;
    int reader_count;
    int writer_waiting;
} SharedData;

// 읽기 작업 (여러 스레드 동시 가능)
void* reader_thread(void* arg) {
    SharedData* shared = (SharedData*)arg;
    int thread_id = *(int*)arg;
    
    for (int i = 0; i < 5; i++) {
        pthread_rwlock_rdlock(&shared->rwlock);
        
        printf("Reader %d: Reading data = %d\n", 
               thread_id, shared->data);
        
        // 읽기 작업 시뮬레이션
        usleep(100000);  // 100ms
        
        pthread_rwlock_unlock(&shared->rwlock);
        
        usleep(50000);  // 다음 읽기까지 대기
    }
    
    return NULL;
}

// 쓰기 작업 (독점적)
void* writer_thread(void* arg) {
    SharedData* shared = (SharedData*)arg;
    int thread_id = *(int*)arg;
    
    for (int i = 0; i < 3; i++) {
        pthread_rwlock_wrlock(&shared->rwlock);
        
        int old_value = shared->data;
        shared->data = rand() % 100;
        
        printf("Writer %d: Changed data from %d to %d\n", 
               thread_id, old_value, shared->data);
        
        pthread_rwlock_unlock(&shared->rwlock);
        
        usleep(200000);  // 다음 쓰기까지 대기
    }
    
    return NULL;
}
```

## 🔍 8. 멀티스레딩 디버깅과 문제 해결

### 8.1 일반적인 문제와 해결법

#### 1. Race Condition 탐지

```c
// ThreadSanitizer를 사용한 컴파일
// gcc -fsanitize=thread -g program.c -o program -pthread

// Helgrind (Valgrind 도구) 사용
// valgrind --tool=helgrind ./program

// 디버깅을 위한 로깅 매크로
#define DEBUG_LOG(fmt, ...) do { \
    pthread_mutex_lock(&debug_mutex); \
    fprintf(stderr, "[Thread %ld] " fmt "\n", \
            pthread_self(), ##__VA_ARGS__); \
    pthread_mutex_unlock(&debug_mutex); \
} while(0)

pthread_mutex_t debug_mutex = PTHREAD_MUTEX_INITIALIZER;
```

#### 2. Deadlock 진단

```c
// Deadlock 감지 도구
typedef struct {
    pthread_mutex_t* mutex;
    pthread_t owner;
    const char* location;
    struct timespec lock_time;
} MutexInfo;

// 뮤텍스 추적 시스템
void tracked_lock(pthread_mutex_t* mutex, const char* location) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    if (pthread_mutex_trylock(mutex) != 0) {
        printf("[WAIT] Thread %ld waiting for mutex at %s\n", 
               pthread_self(), location);
        
        pthread_mutex_lock(mutex);
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        double wait_time = (end.tv_sec - start.tv_sec) + 
                          (end.tv_nsec - start.tv_nsec) / 1e9;
        
        if (wait_time > 1.0) {
            printf("[WARN] Thread %ld waited %.2fs for mutex at %s\n", 
                   pthread_self(), wait_time, location);
        }
    }
    
    // 뮤텍스 소유자 기록
    printf("[LOCK] Thread %ld acquired mutex at %s\n", 
           pthread_self(), location);
}

void tracked_unlock(pthread_mutex_t* mutex, const char* location) {
    printf("[UNLOCK] Thread %ld released mutex at %s\n", 
           pthread_self(), location);
    pthread_mutex_unlock(mutex);
}

// 사용 예
#define LOCK(m) tracked_lock(&(m), __FILE__ ":" #__LINE__)
#define UNLOCK(m) tracked_unlock(&(m), __FILE__ ":" #__LINE__)
```

### 8.2 성능 프로파일링

```c
// 스레드별 성능 측정
typedef struct {
    pthread_t thread_id;
    uint64_t operations;
    double total_time;
    double min_time;
    double max_time;
} ThreadPerformance;

// 성능 측정 매크로
#define MEASURE_TIME(code, perf) do { \
    struct timespec start, end; \
    clock_gettime(CLOCK_MONOTONIC, &start); \
    code; \
    clock_gettime(CLOCK_MONOTONIC, &end); \
    double elapsed = (end.tv_sec - start.tv_sec) + \
                    (end.tv_nsec - start.tv_nsec) / 1e9; \
    update_performance(perf, elapsed); \
} while(0)

void update_performance(ThreadPerformance* perf, double time) {
    perf->operations++;
    perf->total_time += time;
    if (time < perf->min_time || perf->min_time == 0) {
        perf->min_time = time;
    }
    if (time > perf->max_time) {
        perf->max_time = time;
    }
}

void print_performance(ThreadPerformance* perf) {
    printf("Thread %ld Performance:\n", perf->thread_id);
    printf("  Operations: %lu\n", perf->operations);
    printf("  Total time: %.2fs\n", perf->total_time);
    printf("  Avg time: %.4fms\n", 
           (perf->total_time / perf->operations) * 1000);
    printf("  Min time: %.4fms\n", perf->min_time * 1000);
    printf("  Max time: %.4fms\n", perf->max_time * 1000);
}
```

## 💡 9. 실전 팁과 베스트 프랙티스

### 9.1 스레드 안전성 체크리스트

```c
/*
✅ 스레드 안전성 체크리스트:

1. 모든 공유 변수에 대한 동기화 확인
2. 전역 변수 최소화
3. 함수의 재진입성(Reentrancy) 보장
4. 정적 변수 사용 주의
5. 시그널 핸들러의 스레드 안전성
6. 라이브러리 함수의 스레드 안전성 확인
*/

// 스레드 안전한 함수 예시
char* thread_safe_format_time(time_t timestamp) {
    // 정적 변수 대신 스레드 로컬 저장소 사용
    static __thread char buffer[64];
    
    struct tm tm_info;
    localtime_r(&timestamp, &tm_info);  // thread-safe 버전 사용
    
    strftime(buffer, sizeof(buffer), 
             "%Y-%m-%d %H:%M:%S", &tm_info);
    
    return buffer;
}

// 스레드 안전하지 않은 함수 → 스레드 안전하게 변경
static int global_counter = 0;
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

int increment_counter_unsafe() {
    return ++global_counter;  // Race condition!
}

int increment_counter_safe() {
    pthread_mutex_lock(&counter_mutex);
    int result = ++global_counter;
    pthread_mutex_unlock(&counter_mutex);
    return result;
}
```

### 9.2 확장성 있는 설계

```c
// CPU 코어 수에 따른 스레드 수 결정
int get_optimal_thread_count() {
    int cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    
    // I/O 바운드 작업: CPU 수의 2배
    // CPU 바운드 작업: CPU 수와 동일
    
    return cpu_count * 2;  // I/O 중심 서버의 경우
}

// 동적 스레드 풀 크기 조절
typedef struct {
    pthread_t* threads;
    int min_threads;
    int max_threads;
    int current_threads;
    pthread_mutex_t resize_mutex;
} DynamicThreadPool;

void adjust_thread_pool_size(DynamicThreadPool* pool, 
                           int pending_tasks) {
    pthread_mutex_lock(&pool->resize_mutex);
    
    // 대기 작업이 많으면 스레드 추가
    if (pending_tasks > pool->current_threads * 2 && 
        pool->current_threads < pool->max_threads) {
        // 스레드 추가 로직
        printf("Adding thread. New count: %d\n", 
               pool->current_threads + 1);
    }
    // 대기 작업이 적으면 스레드 감소
    else if (pending_tasks < pool->current_threads / 2 && 
             pool->current_threads > pool->min_threads) {
        // 스레드 감소 로직
        printf("Removing thread. New count: %d\n", 
               pool->current_threads - 1);
    }
    
    pthread_mutex_unlock(&pool->resize_mutex);
}
```

## 🔄 10. Thread-Local Storage (TLS)

### 10.1 TLS 개념과 필요성

Thread-Local Storage는 각 스레드가 자신만의 전역 변수를 가질 수 있게 하는 메커니즘입니다.

#### 왜 필요한가?
- 전역 변수의 편리함 + 스레드 안전성
- 동기화 없이 스레드별 데이터 저장
- 성능 향상 (락 불필요)

### 10.2 C에서의 TLS (pthread)

```c
#include <pthread.h>

// TLS 키 생성
pthread_key_t tls_key;

// 소멸자 함수
void tls_destructor(void* value) {
    if (value) {
        free(value);
    }
}

// 초기화
void init_tls() {
    pthread_key_create(&tls_key, tls_destructor);
}

// 스레드별 데이터 구조체
typedef struct {
    int thread_id;
    char buffer[1024];
    size_t processed_count;
    double total_time;
} thread_data_t;

// 스레드 함수
void* worker_thread(void* arg) {
    // 스레드별 데이터 할당
    thread_data_t* data = malloc(sizeof(thread_data_t));
    data->thread_id = *(int*)arg;
    data->processed_count = 0;
    data->total_time = 0.0;
    
    // TLS에 저장
    pthread_setspecific(tls_key, data);
    
    // 작업 수행
    while (1) {
        // TLS에서 데이터 가져오기
        thread_data_t* my_data = pthread_getspecific(tls_key);
        
        // 스레드별 버퍼 사용 (동기화 불필요)
        sprintf(my_data->buffer, "Thread %d processing...", 
                my_data->thread_id);
        
        my_data->processed_count++;
        // ... 작업 수행
    }
    
    return NULL;
}
```

### 10.3 C++11 thread_local

```cpp
#include <thread>
#include <chrono>

// thread_local 변수 선언
thread_local int tl_counter = 0;
thread_local std::string tl_name;
thread_local std::chrono::steady_clock::time_point tl_start_time;

// 스레드별 통계 수집
class ThreadStats {
private:
    thread_local static size_t requests_handled_;
    thread_local static size_t bytes_processed_;
    thread_local static double total_latency_;
    
public:
    static void recordRequest(size_t bytes, double latency) {
        requests_handled_++;
        bytes_processed_ += bytes;
        total_latency_ += latency;
    }
    
    static void printStats() {
        std::cout << "Thread " << std::this_thread::get_id() 
                  << " Stats:\n"
                  << "  Requests: " << requests_handled_ << "\n"
                  << "  Bytes: " << bytes_processed_ << "\n"
                  << "  Avg Latency: " 
                  << (total_latency_ / requests_handled_) << "ms\n";
    }
};

// Static member 초기화
thread_local size_t ThreadStats::requests_handled_ = 0;
thread_local size_t ThreadStats::bytes_processed_ = 0;
thread_local double ThreadStats::total_latency_ = 0.0;
```

## 🚀 11. 비동기 프로그래밍 패턴

### 11.1 C++11 std::async와 std::future

```cpp
#include <future>
#include <vector>

// 비동기 작업 실행
std::future<int> async_computation(int value) {
    return std::async(std::launch::async, [value]() {
        // 복잡한 계산
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return value * value;
    });
}

// 여러 비동기 작업 관리
void process_multiple_async() {
    std::vector<std::future<int>> futures;
    
    // 10개의 비동기 작업 시작
    for (int i = 0; i < 10; i++) {
        futures.push_back(async_computation(i));
    }
    
    // 결과 수집
    for (auto& fut : futures) {
        try {
            int result = fut.get();  // 블로킹
            std::cout << "Result: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}
```

### 11.2 Promise와 Future 패턴

```cpp
// Producer-Consumer with promise/future
class AsyncQueue {
private:
    std::queue<std::promise<std::string>> promises_;
    std::queue<std::string> values_;
    std::mutex mutex_;
    
public:
    std::future<std::string> pop_async() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!values_.empty()) {
            // 즉시 값 반환
            std::promise<std::string> p;
            p.set_value(std::move(values_.front()));
            values_.pop();
            return p.get_future();
        }
        
        // 나중에 값 설정
        promises_.emplace();
        return promises_.back().get_future();
    }
    
    void push(std::string value) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!promises_.empty()) {
            // 대기 중인 promise에 값 설정
            promises_.front().set_value(std::move(value));
            promises_.pop();
        } else {
            // 큐에 저장
            values_.push(std::move(value));
        }
    }
};
```

### 11.3 Callback 기반 비동기 패턴

```cpp
template<typename Result>
class AsyncOperation {
public:
    using Callback = std::function<void(Result)>;
    using ErrorCallback = std::function<void(std::exception_ptr)>;
    
private:
    std::thread worker_;
    
public:
    void execute_async(std::function<Result()> operation,
                      Callback on_success,
                      ErrorCallback on_error) {
        worker_ = std::thread([=]() {
            try {
                Result result = operation();
                on_success(std::move(result));
            } catch (...) {
                on_error(std::current_exception());
            }
        });
        worker_.detach();
    }
};
```

## 🌟 12. 코루틴 (C++20)

### 12.1 기본 코루틴 구조

```cpp
#include <coroutine>

// 간단한 Generator 코루틴
template<typename T>
class Generator {
public:
    struct promise_type {
        T current_value;
        
        Generator get_return_object() {
            return Generator{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }
        
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        
        std::suspend_always yield_value(T value) {
            current_value = value;
            return {};
        }
        
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
    
private:
    std::coroutine_handle<promise_type> handle_;
    
public:
    explicit Generator(std::coroutine_handle<promise_type> h) 
        : handle_(h) {}
    
    ~Generator() {
        if (handle_) {
            handle_.destroy();
        }
    }
    
    bool move_next() {
        handle_.resume();
        return !handle_.done();
    }
    
    T current_value() {
        return handle_.promise().current_value;
    }
};

// 사용 예제
Generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;
        auto tmp = a;
        a = b;
        b = tmp + b;
    }
}
```

### 12.2 비동기 코루틴

```cpp
// Awaitable 타입
template<typename T>
struct Task {
    struct promise_type {
        T value_;
        std::exception_ptr exception_;
        
        Task get_return_object() {
            return Task{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }
        
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        
        void return_value(T value) {
            value_ = std::move(value);
        }
        
        void unhandled_exception() {
            exception_ = std::current_exception();
        }
    };
    
    std::coroutine_handle<promise_type> handle_;
    
    // Awaiter interface
    bool await_ready() { return handle_.done(); }
    void await_suspend(std::coroutine_handle<> h) {
        // 나중에 h.resume() 호출
    }
    T await_resume() {
        if (handle_.promise().exception_) {
            std::rethrow_exception(handle_.promise().exception_);
        }
        return handle_.promise().value_;
    }
};
```

## ✅ 13. 다음 단계 준비

이 문서를 완전히 이해했다면:

1. **프로세스와 스레드의 차이점**을 명확히 설명할 수 있어야 합니다
2. **pthreads 라이브러리**를 사용하여 멀티스레드 프로그램을 작성할 수 있어야 합니다
3. **Race Condition과 Deadlock**을 이해하고 방지할 수 있어야 합니다
4. **Mutex, Condition Variable** 등의 동기화 메커니즘을 활용할 수 있어야 합니다
5. **스레드 풀 패턴**을 구현하고 사용할 수 있어야 합니다

### 🎯 확인 문제

1. 다음 코드에서 Race Condition이 발생할 수 있는 이유는?
```c
int global_counter = 0;
void* increment(void* arg) {
    for (int i = 0; i < 1000; i++) {
        global_counter++;
    }
    return NULL;
}
```

2. Deadlock을 방지하기 위한 일반적인 방법들은 무엇인가요?

3. LogCaster에서 스레드 풀을 사용하는 이유와 장점은 무엇인가요?

## 🚀 10. [신규] 채널별 스레드 풀 전략

### 10.1 계층적 스레드 풀 아키텍처

IRC 채널마다 다른 우선순위와 처리량이 필요합니다:

```cpp
// 채널별 스레드 풀 관리
class HierarchicalThreadPoolManager {
private:
    struct ChannelThreadPool {
        std::string channelName;
        std::unique_ptr<ThreadPool> pool;
        int priority;
        size_t workerCount;
        std::atomic<size_t> taskCount{0};
    };
    
    std::unordered_map<std::string, ChannelThreadPool> channelPools_;
    std::shared_ptr<ThreadPool> defaultPool_;
    std::mutex managerMutex_;
    
public:
    // 채널별 스레드 풀 생성
    void createChannelPool(const std::string& channel, 
                          size_t workers, int priority) {
        std::lock_guard lock(managerMutex_);
        
        auto& channelPool = channelPools_[channel];
        channelPool.channelName = channel;
        channelPool.priority = priority;
        channelPool.workerCount = workers;
        channelPool.pool = std::make_unique<ThreadPool>(workers);
        
        // 우선순위 설정
        channelPool.pool->setPriority(priority);
    }
    
    // 동적 스레드 조정
    void adjustThreads() {
        std::lock_guard lock(managerMutex_);
        
        for (auto& [channel, pool] : channelPools_) {
            size_t queueSize = pool.pool->getQueueSize();
            size_t activeWorkers = pool.pool->getActiveWorkers();
            
            // 부하에 따라 스레드 수 조정
            if (queueSize > activeWorkers * 10) {
                // 스레드 추가
                pool.pool->addWorkers(2);
                logInfo("Added 2 workers to channel {}", channel);
            } else if (queueSize == 0 && activeWorkers > pool.workerCount) {
                // 스레드 감소
                pool.pool->removeWorkers(1);
                logInfo("Removed 1 worker from channel {}", channel);
            }
        }
    }
    
    // 작업 제출
    template<typename F>
    void submitToChannel(const std::string& channel, F&& task) {
        auto it = channelPools_.find(channel);
        if (it != channelPools_.end()) {
            it->second.taskCount++;
            it->second.pool->enqueue(std::forward<F>(task));
        } else {
            // 기본 풀로 폴백
            defaultPool_->enqueue(std::forward<F>(task));
        }
    }
};

// 사용 예시
HierarchicalThreadPoolManager poolManager;

// 중요한 채널은 더 많은 스레드
poolManager.createChannelPool("#logs-error", 8, PRIORITY_HIGH);
poolManager.createChannelPool("#logs-warning", 4, PRIORITY_MEDIUM);
poolManager.createChannelPool("#logs-info", 2, PRIORITY_LOW);

// 에러 로그 브로드캐스트 (높은 우선순위)
poolManager.submitToChannel("#logs-error", [&]() {
    broadcastCriticalLog(errorLog);
});
```

### 10.2 작업 훔치기(Work Stealing) 패턴

```cpp
// 작업 훔치기 스레드 풀
class WorkStealingThreadPool {
private:
    struct WorkerThread {
        std::deque<std::function<void()>> localQueue;
        std::mutex queueMutex;
        std::condition_variable cv;
        std::thread thread;
        std::atomic<bool> running{true};
        WorkStealingThreadPool* pool;
        
        void run() {
            while (running) {
                std::function<void()> task;
                
                // 1. 로컬 큐에서 작업 가져오기
                {
                    std::unique_lock lock(queueMutex);
                    if (!localQueue.empty()) {
                        task = std::move(localQueue.front());
                        localQueue.pop_front();
                    }
                }
                
                // 2. 로컬 큐가 비었으면 다른 워커에서 훔치기
                if (!task) {
                    task = pool->stealWork(this);
                }
                
                // 3. 그래도 없으면 전역 큐 확인
                if (!task) {
                    task = pool->getFromGlobalQueue();
                }
                
                // 4. 작업 실행
                if (task) {
                    task();
                } else {
                    // 대기
                    std::unique_lock lock(queueMutex);
                    cv.wait_for(lock, std::chrono::milliseconds(10));
                }
            }
        }
    };
    
    std::vector<std::unique_ptr<WorkerThread>> workers_;
    std::queue<std::function<void()>> globalQueue_;
    std::mutex globalMutex_;
    
public:
    // 다른 워커에서 작업 훔치기
    std::function<void()> stealWork(WorkerThread* thief) {
        for (auto& worker : workers_) {
            if (worker.get() == thief) continue;
            
            std::unique_lock lock(worker->queueMutex, std::try_to_lock);
            if (lock.owns_lock() && !worker->localQueue.empty()) {
                // 뒤에서 훔치기 (캐시 친화적)
                auto task = std::move(worker->localQueue.back());
                worker->localQueue.pop_back();
                return task;
            }
        }
        return nullptr;
    }
    
    // 작업 제출 (로컬 큐 우선)
    void submit(std::function<void()> task) {
        // 현재 스레드가 워커 스레드인지 확인
        auto thisId = std::this_thread::get_id();
        
        for (auto& worker : workers_) {
            if (worker->thread.get_id() == thisId) {
                // 로컬 큐에 추가
                std::lock_guard lock(worker->queueMutex);
                worker->localQueue.push_back(std::move(task));
                worker->cv.notify_one();
                return;
            }
        }
        
        // 워커 스레드가 아니면 전역 큐에 추가
        {
            std::lock_guard lock(globalMutex_);
            globalQueue_.push(std::move(task));
        }
        
        // 랜덤 워커 깨우기
        workers_[rand() % workers_.size()]->cv.notify_one();
    }
};
```

## 🔄 11. [신규] 브로드캐스트 동시성 패턴

### 11.1 Fan-Out 패턴

```cpp
// 효율적인 브로드캐스트를 위한 Fan-Out
class FanOutBroadcaster {
private:
    static constexpr size_t CHUNK_SIZE = 100;
    
    struct BroadcastTask {
        std::vector<std::shared_ptr<IRCClient>> clients;
        std::string message;
        std::promise<void> completion;
    };
    
public:
    // 병렬 브로드캐스트
    std::future<void> broadcast(const std::vector<std::shared_ptr<IRCClient>>& clients,
                                const std::string& message) {
        BroadcastTask task{clients, message};
        auto future = task.completion.get_future();
        
        // 클라이언트를 청크로 분할
        size_t numChunks = (clients.size() + CHUNK_SIZE - 1) / CHUNK_SIZE;
        std::atomic<size_t> completedChunks{0};
        
        for (size_t i = 0; i < numChunks; ++i) {
            size_t start = i * CHUNK_SIZE;
            size_t end = std::min(start + CHUNK_SIZE, clients.size());
            
            // 각 청크를 별도 스레드에서 처리
            threadPool.enqueue([this, &task, start, end, &completedChunks, numChunks]() {
                for (size_t j = start; j < end; ++j) {
                    try {
                        task.clients[j]->send(task.message);
                    } catch (const std::exception& e) {
                        logError("Broadcast failed for client: {}", e.what());
                    }
                }
                
                // 모든 청크 완료 확인
                if (++completedChunks == numChunks) {
                    task.completion.set_value();
                }
            });
        }
        
        return future;
    }
};
```

### 11.2 Pipeline 패턴

```cpp
// 메시지 처리 파이프라인
class MessagePipeline {
private:
    // 파이프라인 스테이지
    enum Stage {
        PARSE,
        VALIDATE,
        TRANSFORM,
        BROADCAST
    };
    
    // 각 스테이지별 스레드 풀
    std::array<std::unique_ptr<ThreadPool>, 4> stagePools_;
    
    // 스테이지 간 큐
    std::array<ConcurrentQueue<MessageTask>, 3> stageQueues_;
    
public:
    MessagePipeline() {
        // 각 스테이지별로 적절한 스레드 수 할당
        stagePools_[PARSE] = std::make_unique<ThreadPool>(2);
        stagePools_[VALIDATE] = std::make_unique<ThreadPool>(4);
        stagePools_[TRANSFORM] = std::make_unique<ThreadPool>(4);
        stagePools_[BROADCAST] = std::make_unique<ThreadPool>(8);
        
        // 파이프라인 시작
        startPipeline();
    }
    
    void processMessage(const std::string& rawMessage) {
        MessageTask task{rawMessage};
        stageQueues_[0].push(std::move(task));
    }
    
private:
    void startPipeline() {
        // Parse 스테이지
        for (int i = 0; i < 2; ++i) {
            stagePools_[PARSE]->enqueue([this]() {
                while (running_) {
                    MessageTask task;
                    if (stageQueues_[0].try_pop(task)) {
                        task.parsed = parseIRCMessage(task.raw);
                        stageQueues_[1].push(std::move(task));
                    } else {
                        std::this_thread::sleep_for(1ms);
                    }
                }
            });
        }
        
        // Validate 스테이지
        for (int i = 0; i < 4; ++i) {
            stagePools_[VALIDATE]->enqueue([this]() {
                while (running_) {
                    MessageTask task;
                    if (stageQueues_[1].try_pop(task)) {
                        if (validateMessage(task.parsed)) {
                            stageQueues_[2].push(std::move(task));
                        }
                    } else {
                        std::this_thread::sleep_for(1ms);
                    }
                }
            });
        }
        
        // Transform & Broadcast 스테이지도 유사하게...
    }
};
```

### 11.3 동시성 제어 패턴

```cpp
// 적응형 동시성 제어
class AdaptiveConcurrencyControl {
private:
    struct Metrics {
        std::atomic<double> avgLatency{0.0};
        std::atomic<size_t> throughput{0};
        std::atomic<size_t> errorRate{0};
    };
    
    Metrics metrics_;
    std::atomic<size_t> maxConcurrency_{100};
    std::atomic<size_t> currentConcurrency_{0};
    
public:
    // 동시성 토큰 획득
    class ConcurrencyToken {
        AdaptiveConcurrencyControl* control_;
    public:
        explicit ConcurrencyToken(AdaptiveConcurrencyControl* control) 
            : control_(control) {
            control_->currentConcurrency_++;
        }
        
        ~ConcurrencyToken() {
            control_->currentConcurrency_--;
        }
    };
    
    std::optional<ConcurrencyToken> tryAcquire() {
        size_t current = currentConcurrency_.load();
        size_t max = maxConcurrency_.load();
        
        if (current >= max) {
            return std::nullopt;
        }
        
        return ConcurrencyToken(this);
    }
    
    // 메트릭 기반 동시성 조정
    void adjustConcurrency() {
        double latency = metrics_.avgLatency.load();
        size_t throughput = metrics_.throughput.load();
        size_t errors = metrics_.errorRate.load();
        
        // Little's Law: L = λ * W
        // L: 평균 시스템 내 요청 수
        // λ: 평균 도착률 (throughput)
        // W: 평균 응답시간 (latency)
        
        if (errors > throughput * 0.01) {  // 1% 이상 에러
            // 동시성 감소
            maxConcurrency_ = maxConcurrency_ * 0.9;
        } else if (latency < 10.0 && errors == 0) {  // 성능 여유
            // 동시성 증가
            maxConcurrency_ = maxConcurrency_ * 1.1;
        }
        
        // 범위 제한
        maxConcurrency_ = std::clamp(maxConcurrency_.load(), 
                                    size_t(10), size_t(1000));
    }
};
```

다음 문서에서는 **동기화 메커니즘**에 대해 더 자세히 알아보겠습니다!

---

*💡 팁: 멀티스레딩은 강력하지만 복잡합니다. 항상 동기화를 염두에 두고 코딩하세요!*

## ⚠️ 자주 하는 실수와 해결법

### 1. Race Condition
```c
// ❌ 잘못된 예시
int counter = 0;
void* increment(void* arg) {
    for (int i = 0; i < 1000000; i++) {
        counter++;  // Race condition!
    }
    return NULL;
}

// ✅ 올바른 예시
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

void* increment(void* arg) {
    for (int i = 0; i < 1000000; i++) {
        pthread_mutex_lock(&mutex);
        counter++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}
```

### 2. Deadlock
```c
// ❌ 잘못된 예시 - 순환 대기
void* thread1(void* arg) {
    pthread_mutex_lock(&mutex1);
    sleep(1);  // 타이밍 문제 유발
    pthread_mutex_lock(&mutex2);  // Deadlock!
    // ...
}

void* thread2(void* arg) {
    pthread_mutex_lock(&mutex2);
    sleep(1);
    pthread_mutex_lock(&mutex1);  // Deadlock!
    // ...
}

// ✅ 올바른 예시 - 순서 정하기
void* thread1(void* arg) {
    pthread_mutex_lock(&mutex1);  // 항상 mutex1 먼저
    pthread_mutex_lock(&mutex2);
    // ...
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
}
```

### 3. 메모리 누수
```c
// ❌ 잘못된 예시
void* worker(void* arg) {
    char* buffer = malloc(1024);
    // pthread_exit(NULL);  // free 없이 종료!
    return NULL;
}

// ✅ 올바른 예시
void* worker(void* arg) {
    char* buffer = malloc(1024);
    
    // cleanup handler 등록
    pthread_cleanup_push(free, buffer);
    
    // 작업 수행...
    
    pthread_cleanup_pop(1);  // cleanup 실행
    return NULL;
}
```

### 4. Detached 스레드 잘못 사용
```c
// ❌ 잘못된 예시
void create_worker() {
    pthread_t thread;
    int local_data = 42;
    pthread_create(&thread, NULL, worker, &local_data);
    pthread_detach(thread);
    // 함수 종료 - local_data 소멸!
}

// ✅ 올바른 예시
void create_worker() {
    pthread_t thread;
    int* data = malloc(sizeof(int));
    *data = 42;
    pthread_create(&thread, NULL, worker, data);
    pthread_detach(thread);
    // worker에서 free(data) 책임
}
```

### 5. 조건 변수 Spurious Wakeup
```c
// ❌ 잘못된 예시
pthread_cond_wait(&cond, &mutex);
// 조건 확인 없이 진행!

// ✅ 올바른 예시
while (!condition) {  // while 루프 사용!
    pthread_cond_wait(&cond, &mutex);
}
```

## 🎯 실습 프로젝트

### 프로젝트 1: Thread Pool 기반 작업 스케줄러 (난이도: ⭐⭐⭐)
```c
// 구현할 기능:
// 1. 동적 크기 조절 가능한 스레드 풀
// 2. 우선순위 큐
// 3. 작업 취소 기능
// 4. 통계 수집 (처리 시간, 대기 시간)

typedef struct {
    void (*function)(void*);
    void* arg;
    int priority;
    bool cancelable;
} PriorityTask;

// 구현 목표:
// - 우선순위 기반 작업 처리
// - 작업 취소 메커니즘
// - 실시간 성능 모니터링
```

### 프로젝트 2: Producer-Consumer 로그 시스템 (난이도: ⭐⭐⭐⭐)
```c
// 구현할 기능:
// 1. 다중 생산자/소비자
// 2. 환형 버퍼
// 3. 백프레셔 처리
// 4. 로그 레벨별 필터링

// 도전 과제:
// - Lock-free 환형 버퍼 구현
// - 성능 벤치마킹
// - 로그 압축/회전
```

### 프로젝트 3: 병렬 파일 검색 도구 (난이도: ⭐⭐⭐⭐⭐)
```c
// 구현할 기능:
// 1. 디렉토리 트리 병렬 탐색
// 2. 정규식 패턴 매칭
// 3. Work stealing 알고리즘
// 4. 진행률 표시

// 고급 기능:
// - 메모리 맵 파일 사용
// - 캐시 친화적 설계
// - 중단/재개 기능
```

## ✅ 학습 체크리스트

### 🟢 기초 (1-2주)
- [ ] 프로세스 vs 스레드 이해
- [ ] pthread_create/join 사용
- [ ] 기본 mutex 사용
- [ ] 간단한 멀티스레드 프로그램 작성

### 🟡 중급 (3-4주)
- [ ] Race condition 이해와 해결
- [ ] Deadlock 예방 기법
- [ ] Condition variable 사용
- [ ] Reader-Writer lock 구현
- [ ] Thread-local storage 활용
- [ ] 스레드 풀 패턴 구현

### 🔴 고급 (5-8주)
- [ ] Lock-free 프로그래밍 기초
- [ ] Memory ordering 이해
- [ ] Work stealing 구현
- [ ] 성능 프로파일링과 최적화
- [ ] C++20 코루틴 사용
- [ ] 분산 시스템 동기화

### ⚫ 전문가 (3개월+)
- [ ] 커스텀 동기화 프리미티브 구현
- [ ] 고성능 동시성 데이터 구조
- [ ] NUMA 아키텍처 최적화
- [ ] 실시간 시스템 프로그래밍
- [ ] 형식 검증 도구 사용

## 📚 추가 학습 자료

### 권장 도서
1. **"Programming with POSIX Threads"** - David Butenhof
2. **"C++ Concurrency in Action"** - Anthony Williams
3. **"The Art of Multiprocessor Programming"** - Maurice Herlihy

### 온라인 리소스
- [POSIX Threads Programming](https://computing.llnl.gov/tutorials/pthreads/)
- [C++ Reference - Thread support library](https://en.cppreference.com/w/cpp/thread)
- [Intel Threading Building Blocks](https://www.threadingbuildingblocks.org/)

### 실습 프로젝트 아이디어
1. **멀티스레드 웹 크롤러**: 여러 URL을 동시에 다운로드
2. **병렬 이미지 처리기**: 여러 이미지를 동시에 변환
3. **실시간 채팅 서버**: 스레드 풀 기반 채팅 서버
4. **로그 분석 도구**: 대용량 로그를 병렬로 분석

---

*이 문서는 LogCaster 프로젝트를 위한 멀티스레딩 완벽 가이드입니다. 다음 문서에서는 동기화 메커니즘을 더 깊이 다루겠습니다!*