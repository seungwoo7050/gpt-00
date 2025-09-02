# LogCaster 연습문제와 미니 프로젝트

## 🎯 이 문서의 목적

각 학습 단계별로 실력을 점검하고 향상시킬 수 있는 연습문제와 미니 프로젝트를 제공합니다. 난이도별로 구성되어 있으니 차근차근 도전해보세요!

## 📝 연습문제

### 🟢 초급 - 기본 문법과 개념

#### 문제 1: 간단한 로그 구조체 만들기
**요구사항:**
- 로그 메시지, 시간, 로그 레벨을 저장할 수 있는 구조체 정의
- 로그를 출력하는 함수 구현
- 동적 메모리 할당 사용

**시작 코드:**
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// TODO: log_entry 구조체 정의

// TODO: create_log_entry 함수 구현
// 힌트: malloc 사용, 메시지 복사

// TODO: print_log_entry 함수 구현

// TODO: free_log_entry 함수 구현

int main() {
    // 테스트 코드
    log_entry* log = create_log_entry("Server started", "INFO");
    print_log_entry(log);
    free_log_entry(log);
    return 0;
}
```

**기대 출력:**
```
[2025-07-26 10:30:45] [INFO] Server started
```

#### 문제 2: 파일 디스크립터 이해하기
**요구사항:**
- 파일을 열고 파일 디스크립터 번호 출력
- 표준 입출력의 파일 디스크립터 확인
- 최대 파일 디스크립터 개수 확인

**힌트:**
```c
#include <fcntl.h>
#include <unistd.h>
// open(), close() 사용
// STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO
```

### 🟡 중급 - 네트워킹과 동시성

#### 문제 3: 간단한 TCP 에코 서버
**요구사항:**
- 포트 12345에서 리스닝
- 받은 메시지를 그대로 돌려줌
- 단일 클라이언트만 처리

**뼈대 코드:**
```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE] = {0};
    
    // TODO: 소켓 생성
    
    // TODO: 주소 설정
    
    // TODO: bind
    
    // TODO: listen
    
    printf("Echo server listening on port %d\n", PORT);
    
    // TODO: accept
    
    // TODO: 메시지 수신 및 에코
    
    // TODO: 정리
    
    return 0;
}
```

**테스트 방법:**
```bash
# 서버 실행
./echo_server

# 다른 터미널에서
telnet localhost 12345
# 또는
echo "Hello Server" | nc localhost 12345
```

#### 문제 4: 생산자-소비자 패턴
**요구사항:**
- 원형 버퍼 구현 (크기: 10)
- 생산자 스레드: 1초마다 숫자 추가
- 소비자 스레드: 2초마다 숫자 제거
- 동기화 필수

**구조:**
```c
typedef struct {
    int buffer[10];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer_t;
```

### 🔴 고급 - 실전 응용

#### 문제 5: 멀티스레드 로그 수집기
**요구사항:**
- 여러 클라이언트 동시 처리
- 스레드 풀 사용 (워커 스레드 4개)
- 로그를 메모리 버퍼에 저장
- 간단한 통계 기능 (총 로그 수, 레벨별 카운트)

**평가 기준:**
- 스레드 안전성
- 메모리 누수 없음
- 동시 접속 10개 이상 처리

## 🚀 미니 프로젝트

### 프로젝트 1: 간단한 채팅 서버 (1-2일)
**목표:** TCP 서버와 멀티스레딩 이해

**기능 요구사항:**
1. 여러 클라이언트 동시 접속
2. 한 클라이언트의 메시지를 모든 클라이언트에게 전달
3. 닉네임 설정 기능
4. 클라이언트 목록 확인 (`/list` 명령)
5. 개인 메시지 (`/msg 닉네임 메시지`)

**구현 힌트:**
```c
typedef struct client {
    int fd;
    char nickname[32];
    struct client* next;
} client_t;

typedef struct {
    client_t* head;
    pthread_mutex_t mutex;
    int count;
} client_list_t;
```

### 프로젝트 2: 파일 모니터링 시스템 (2-3일)
**목표:** 파일 시스템 이벤트와 비동기 처리 이해

**기능 요구사항:**
1. 지정된 디렉토리의 파일 변경 감지
2. 변경 내역을 로그로 기록
3. 실시간 통계 표시 (변경 횟수, 파일 종류별 통계)
4. 설정 파일을 통한 모니터링 규칙 정의

**Linux에서 inotify 사용:**
```c
#include <sys/inotify.h>

int fd = inotify_init();
int wd = inotify_add_watch(fd, "/path/to/watch", 
                          IN_CREATE | IN_DELETE | IN_MODIFY);
```

### 프로젝트 3: 간단한 부하 분산기 (3-4일)
**목표:** 네트워크 프록시와 부하 분산 알고리즘 이해

**기능 요구사항:**
1. 프론트엔드 포트에서 연결 수락
2. 여러 백엔드 서버로 요청 분산
3. 라운드 로빈 알고리즘 구현
4. 백엔드 서버 상태 확인 (Health Check)
5. 실패한 서버 자동 제외

**아키텍처:**
```
클라이언트 → [부하 분산기:8080] → 백엔드1:9001
                               → 백엔드2:9002
                               → 백엔드3:9003
```

### 프로젝트 4: 로그 분석 도구 (1주일)
**목표:** LogCaster의 확장 버전 구현

**기능 요구사항:**
1. 대용량 로그 파일 처리 (100GB+)
2. 정규식 기반 패턴 매칭
3. 시간대별 집계
4. 실시간 알림 (특정 패턴 발견 시)
5. 웹 인터페이스 (선택사항)

**성능 목표:**
- 초당 100,000 라인 처리
- 메모리 사용량 < 1GB
- 검색 응답시간 < 100ms

## 📊 자가 평가 체크리스트

### 각 문제를 완료한 후 확인하세요:

#### 코드 품질
- [ ] 컴파일 경고 없음 (`-Wall -Wextra`)
- [ ] 메모리 누수 없음 (Valgrind 확인)
- [ ] 에러 처리 구현
- [ ] 의미 있는 변수명 사용

#### 기능 완성도
- [ ] 모든 요구사항 구현
- [ ] 예외 상황 처리
- [ ] 테스트 케이스 통과
- [ ] 문서화 (주석)

#### 성능
- [ ] CPU 사용률 적절함
- [ ] 메모리 사용량 합리적
- [ ] 응답 시간 만족
- [ ] 동시성 처리 올바름

## 🎓 학습 팁

### 문제 해결 접근법
1. **문제 이해**: 요구사항을 정확히 파악
2. **계획 수립**: 구현 전에 설계
3. **단계별 구현**: 작은 부분부터 시작
4. **테스트**: 각 단계마다 확인
5. **개선**: 동작 후 최적화

### 디버깅 전략
```c
// 디버그 출력 매크로
#ifdef DEBUG
#define DBG_PRINT(fmt, ...) \
    fprintf(stderr, "[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#define DBG_PRINT(fmt, ...)
#endif

// 사용 예
DBG_PRINT("Client connected: fd=%d", client_fd);
```

### 코드 리뷰 포인트
1. **가독성**: 다른 사람이 이해할 수 있는가?
2. **효율성**: 불필요한 연산은 없는가?
3. **안정성**: 모든 에러 케이스를 처리하는가?
4. **확장성**: 기능 추가가 쉬운가?

## 🏆 도전 과제

### Expert Level: Lock-Free 로그 버퍼
- Compare-and-Swap을 사용한 lock-free 구현
- 멀티코어 환경에서 성능 측정
- 기존 mutex 버전과 성능 비교

### Expert Level: 커스텀 프로토콜 설계
- 바이너리 프로토콜 설계
- 압축 알고리즘 적용
- 암호화 통신 구현

---

💡 **Remember**: 
- 완벽한 해답은 없습니다. 여러 방법을 시도해보세요!
- 에러는 배움의 기회입니다.
- 코드를 작성하는 것보다 이해하는 것이 중요합니다.