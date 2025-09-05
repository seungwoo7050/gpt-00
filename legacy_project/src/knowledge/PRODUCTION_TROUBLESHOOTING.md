# LogCaster 프로덕션 트러블슈팅 가이드

## 🎯 이 문서의 목적

실제 운영 환경에서 LogCaster를 운영하면서 발생할 수 있는 문제들과 해결 방법을 실제 사례를 바탕으로 정리했습니다.

## 🚀 성능 관련 이슈

### Case 1: 높은 CPU 사용률 (100% 고정)

**증상:**
- CPU 사용률이 지속적으로 100%
- 로그 처리 속도 저하
- 클라이언트 연결 타임아웃 발생

**원인 분석:**
```bash
# top으로 스레드별 CPU 사용률 확인
top -H -p $(pgrep logcaster)

# strace로 시스템 콜 패턴 확인
strace -c -p $(pgrep logcaster)
```

**발견된 문제:** Busy waiting in select loop
```c
// 문제가 된 코드
while (server->running) {
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);
    
    // timeout이 NULL이라 블로킹되지 않고 계속 루프
    if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) > 0) {
        // ...
    }
}
```

**해결 방법:**
```c
// 수정된 코드
struct timeval timeout;
while (server->running) {
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);
    
    // 100ms 타임아웃 설정
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    
    if (select(max_fd + 1, &read_fds, NULL, NULL, &timeout) > 0) {
        // ...
    }
}
```

### Case 2: 메모리 사용량 급증

**증상:**
- 시간이 지날수록 메모리 사용량 증가
- OOM Killer에 의한 프로세스 종료

**모니터링:**
```bash
# 실시간 메모리 사용량 모니터링
watch -n 1 'ps aux | grep logcaster'

# 메모리 맵 확인
pmap -x $(pgrep logcaster)
```

**발견된 문제:** 검색 결과 메모리 누수
```c
// 문제가 된 코드
char** search_logs(const char* pattern, int* count) {
    char** results = malloc(sizeof(char*) * MAX_RESULTS);
    // ... 검색 로직 ...
    return results;  // 호출자가 free하는 것을 잊음
}
```

**해결 방법:**
1. **명확한 메모리 소유권 정의**
```c
// 함수 주석에 메모리 해제 책임 명시
/**
 * @return 검색 결과 배열 (호출자가 free_search_results()로 해제해야 함)
 */
char** search_logs(const char* pattern, int* count);

void free_search_results(char** results, int count) {
    for (int i = 0; i < count; i++) {
        free(results[i]);
    }
    free(results);
}
```

2. **메모리 풀 사용**
```c
typedef struct {
    char* buffer;
    size_t size;
    size_t used;
} memory_pool_t;

// 재사용 가능한 메모리 풀로 할당/해제 오버헤드 감소
```

### Case 3: 로그 버퍼 오버플로우

**증상:**
- 특정 시간대에 로그 유실 발생
- "Buffer full" 에러 메시지

**분석 도구:**
```bash
# 로그 유입량 모니터링
tcpdump -i lo -n port 9999 | pv -l -i 1 > /dev/null
```

**해결 방법:**

1. **적응형 버퍼 크기**
```c
typedef struct {
    size_t current_size;
    size_t max_size;
    size_t high_water_mark;
    time_t last_resize;
} adaptive_buffer_t;

void adjust_buffer_size(adaptive_buffer_t* buf, size_t current_load) {
    if (current_load > buf->high_water_mark * 0.8) {
        // 버퍼 크기 증가
        resize_buffer(buf, buf->current_size * 1.5);
    } else if (current_load < buf->high_water_mark * 0.3) {
        // 버퍼 크기 감소 (메모리 절약)
        resize_buffer(buf, buf->current_size * 0.7);
    }
}
```

2. **백프레셔(Backpressure) 구현**
```c
// 클라이언트에게 속도 조절 신호 전송
if (buffer_usage() > 90) {
    send(client_fd, "SLOW_DOWN\n", 10, 0);
}
```

## 🔒 동시성 관련 이슈

### Case 4: 데드락 발생

**증상:**
- 특정 조건에서 서버 응답 없음
- 스레드가 mutex_lock에서 블록됨

**디버깅:**
```bash
# GDB로 데드락 상태 확인
gdb -p $(pgrep logcaster)
(gdb) info threads
(gdb) thread apply all bt
```

**발견된 패턴:**
```
Thread 1: holds mutex_A, waiting for mutex_B
Thread 2: holds mutex_B, waiting for mutex_A
```

**해결 방법:**

1. **Lock 순서 일관성 유지**
```c
// 전역 lock 순서 정의
enum lock_order {
    LOCK_ORDER_BUFFER = 1,
    LOCK_ORDER_CLIENT = 2,
    LOCK_ORDER_STATS = 3
};

// 항상 낮은 번호부터 획득
void safe_dual_lock(pthread_mutex_t* m1, int order1,
                    pthread_mutex_t* m2, int order2) {
    if (order1 < order2) {
        pthread_mutex_lock(m1);
        pthread_mutex_lock(m2);
    } else {
        pthread_mutex_lock(m2);
        pthread_mutex_lock(m1);
    }
}
```

2. **Try-lock 패턴 사용**
```c
int try_acquire_locks(pthread_mutex_t* m1, pthread_mutex_t* m2) {
    if (pthread_mutex_trylock(m1) == 0) {
        if (pthread_mutex_trylock(m2) == 0) {
            return 1;  // 성공
        }
        pthread_mutex_unlock(m1);
    }
    return 0;  // 실패, 나중에 재시도
}
```

### Case 5: 경쟁 상태로 인한 데이터 손실

**증상:**
- 가끔씩 로그 순서가 뒤바뀜
- 카운터 값이 정확하지 않음

**해결 방법:**

1. **원자적 연산 사용**
```c
#include <stdatomic.h>

// 기존 코드
int log_count = 0;
log_count++;  // 스레드 안전하지 않음

// 개선된 코드
atomic_int log_count = 0;
atomic_fetch_add(&log_count, 1);  // 스레드 안전
```

2. **Lock-free 데이터 구조**
```c
typedef struct node {
    void* data;
    struct node* next;
} node_t;

typedef struct {
    _Atomic(node_t*) head;
    _Atomic(node_t*) tail;
} lockfree_queue_t;

// Compare-and-swap을 사용한 lock-free enqueue
void lockfree_enqueue(lockfree_queue_t* q, void* data) {
    node_t* new_node = malloc(sizeof(node_t));
    new_node->data = data;
    new_node->next = NULL;
    
    node_t* prev_tail;
    do {
        prev_tail = atomic_load(&q->tail);
    } while (!atomic_compare_exchange_weak(&q->tail, &prev_tail, new_node));
    
    prev_tail->next = new_node;
}
```

## 🌐 네트워크 관련 이슈

### Case 6: TIME_WAIT 소켓 누적

**증상:**
- netstat에서 수천 개의 TIME_WAIT 상태 소켓
- 새로운 연결 실패

**확인:**
```bash
netstat -an | grep TIME_WAIT | wc -l
```

**해결 방법:**

1. **SO_REUSEADDR 설정**
```c
int opt = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

2. **커널 파라미터 튜닝**
```bash
# /etc/sysctl.conf
net.ipv4.tcp_tw_reuse = 1
net.ipv4.tcp_tw_recycle = 1
net.ipv4.tcp_fin_timeout = 30
```

### Case 7: 대량 연결 시 성능 저하

**증상:**
- 연결 수가 1000개를 넘으면 급격한 성능 저하
- select() 호출이 병목

**해결 방법:** epoll 사용 (Linux)
```c
int epoll_fd = epoll_create1(0);

struct epoll_event ev;
ev.events = EPOLLIN | EPOLLET;  // Edge-triggered
ev.data.fd = server_fd;
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

struct epoll_event events[MAX_EVENTS];
while (running) {
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    for (int i = 0; i < nfds; i++) {
        if (events[i].data.fd == server_fd) {
            // 새 연결 처리
        } else {
            // 데이터 처리
        }
    }
}
```

## 📊 모니터링 및 진단

### 실시간 모니터링 스크립트

```bash
#!/bin/bash
# monitor_logcaster.sh

PID=$(pgrep logcaster)

while true; do
    clear
    echo "=== LogCaster Monitor ==="
    echo "Time: $(date)"
    echo
    
    # CPU & Memory
    ps -p $PID -o pid,ppid,cmd,%cpu,%mem,vsz,rss
    echo
    
    # Thread count
    echo "Threads: $(ls /proc/$PID/task | wc -l)"
    echo
    
    # File descriptors
    echo "Open files: $(ls /proc/$PID/fd | wc -l)"
    echo
    
    # Network connections
    echo "Connections: $(netstat -anp 2>/dev/null | grep $PID | wc -l)"
    echo
    
    # Log processing rate
    echo "Logs/sec: $(calculate_rate)"
    
    sleep 1
done
```

### 성능 프로파일링

```c
// 간단한 성능 측정 매크로
#define MEASURE_TIME(name, code) do { \
    struct timespec start, end; \
    clock_gettime(CLOCK_MONOTONIC, &start); \
    code \
    clock_gettime(CLOCK_MONOTONIC, &end); \
    double elapsed = (end.tv_sec - start.tv_sec) + \
                    (end.tv_nsec - start.tv_nsec) / 1e9; \
    log_performance(name, elapsed); \
} while(0)

// 사용 예
MEASURE_TIME("buffer_insert", {
    insert_log_entry(buffer, entry);
});
```

## 🛡️ 예방적 조치

### 1. Health Check 엔드포인트
```c
void* health_check_thread(void* arg) {
    int health_fd = create_socket(8080);
    
    while (running) {
        int client = accept(health_fd, NULL, NULL);
        if (client > 0) {
            const char* status = get_health_status();
            send(client, status, strlen(status), 0);
            close(client);
        }
    }
}

const char* get_health_status() {
    static char status[1024];
    snprintf(status, sizeof(status),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n\r\n"
        "{\"status\":\"healthy\","
        "\"uptime\":%ld,"
        "\"connections\":%d,"
        "\"logs_processed\":%ld}\r\n",
        uptime, connection_count, total_logs);
    return status;
}
```

### 2. 자동 복구 메커니즘
```c
void install_crash_handler() {
    struct sigaction sa;
    sa.sa_handler = crash_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
}

void crash_handler(int sig) {
    // 스택 트레이스 저장
    void* array[50];
    int size = backtrace(array, 50);
    
    // 로그 파일에 기록
    int fd = open("crash.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    backtrace_symbols_fd(array, size, fd);
    close(fd);
    
    // 깨끗한 종료 시도
    cleanup_resources();
    exit(1);
}
```

## 📝 체크리스트

프로덕션 배포 전 확인사항:

- [ ] 메모리 누수 테스트 완료 (24시간 이상)
- [ ] 부하 테스트 완료 (예상 트래픽의 2배)
- [ ] 모니터링 설정 완료
- [ ] 로그 로테이션 설정
- [ ] 자동 재시작 설정 (systemd/supervisor)
- [ ] 백업 및 복구 절차 문서화
- [ ] 성능 기준선(baseline) 측정
- [ ] 알림 임계값 설정

---

💡 **Remember**: 문제는 항상 발생합니다. 중요한 것은 빠르게 감지하고 대응하는 것입니다!