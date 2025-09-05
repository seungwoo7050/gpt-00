# 13. 디버깅 도구 🐛
## GDB, Valgrind로 버그 잡기 완벽 가이드

---

## 📋 문서 정보

- **난이도**: 중급-고급 (Intermediate-Advanced)
- **예상 학습 시간**: 12-15시간
- **필요 선수 지식**: 
  - C/C++ 기본 문법
  - 포인터와 메모리 관리
  - 기본적인 Linux 명령어
  - 멀티스레딩 개념
- **실습 환경**: Linux/Unix 환경 (GDB, Valgrind 필수)

---

## 🎯 이 문서에서 배울 내용

디버깅은 개발자의 가장 중요한 기술 중 하나입니다. LogCaster처럼 복잡한 멀티스레드 서버를 개발할 때는 더욱 중요하죠. 전문가들이 사용하는 강력한 디버깅 도구들을 마스터해보겠습니다:

🔹 **GDB**: 코드 한 줄씩 실행하며 문제 찾기
🔹 **Valgrind**: 메모리 누수와 오류 검출
🔹 **성능 프로파일링**: 병목점 찾아서 최적화
🔹 **로그 기반 디버깅**: 운영 환경에서의 문제 해결

### 🎮 LogCaster 디버깅 시나리오
```
🐛 LogCaster에서 발생할 수 있는 문제들
├── 💥 세그멘테이션 폴트 → GDB로 크래시 지점 찾기
├── 🕳️ 메모리 누수 → Valgrind로 leak 검출
├── 🐌 성능 저하 → perf로 CPU 사용률 분석
└── 🔄 데드락 → 스레드 상태 분석으로 해결
```

---

## 🔍 1. GDB (GNU Debugger) - 코드 탐정

### 🕵️ GDB란?

GDB는 프로그램을 **한 줄씩 실행하면서 내부 상태를 관찰**할 수 있는 강력한 디버거입니다. 마치 프로그램 내부에 CCTV를 설치하는 것과 같죠!

```
💡 실생활 비유: GDB = 코드 속 타임머신
- 브레이크포인트: "여기서 멈춰!"
- 스텝 실행: "한 발자국씩 천천히"
- 변수 관찰: "지금 이 값이 뭐지?"
- 백트레이스: "어떻게 여기까지 왔지?"
```

### 🔧 디버그 정보 포함해서 컴파일

```bash
# [SEQUENCE: 1] C 컴파일 (디버그 정보 포함)
gcc -g -O0 -Wall -pthread -o logcaster_debug src/*.c

# [SEQUENCE: 2] C++ 컴파일 (디버그 정보 포함)
g++ -g -O0 -Wall -std=c++17 -pthread -o logcaster_cpp_debug src/*.cpp

# 플래그 설명:
# -g: 디버그 정보 포함
# -O0: 최적화 끄기 (디버깅을 위해)
# -Wall: 모든 경고 메시지 출력
```

### 🎯 기본 GDB 명령어들

```bash
# [SEQUENCE: 3] GDB 시작
gdb ./logcaster_debug

# 기본 명령어들
(gdb) help                    # 도움말
(gdb) run                     # 프로그램 실행
(gdb) run -p 9999            # 인자와 함께 실행
(gdb) quit                    # GDB 종료

# 브레이크포인트 설정
(gdb) break main              # main 함수에 브레이크포인트
(gdb) break server.c:42       # server.c 파일 42번째 줄
(gdb) break handle_client     # handle_client 함수
(gdb) info breakpoints        # 브레이크포인트 목록 보기
(gdb) delete 1                # 1번 브레이크포인트 삭제

# 실행 제어
(gdb) continue                # 계속 실행 (c로 축약 가능)
(gdb) step                    # 한 줄 실행 (함수 안으로 들어감, s)
(gdb) next                    # 한 줄 실행 (함수 건너뜀, n)
(gdb) finish                  # 현재 함수 끝까지 실행

# 정보 조회
(gdb) print variable_name     # 변수 값 출력 (p)
(gdb) info locals             # 지역 변수들 출력
(gdb) info args               # 함수 인자들 출력
(gdb) backtrace               # 함수 호출 스택 (bt)
(gdb) where                   # 현재 위치
```

### 🛠️ LogCaster 디버깅 실전 예제

LogCaster 서버에서 세그멘테이션 폴트가 발생했다고 가정해봅시다:

```c
// [SEQUENCE: 4] 버그가 있는 LogCaster 코드 (의도적)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* message;
    int timestamp;
} log_entry_t;

typedef struct {
    log_entry_t* entries;
    int capacity;
    int count;
} log_buffer_t;

// 버그: NULL 포인터 역참조 가능성
void add_log_entry(log_buffer_t* buffer, const char* message) {
    if (buffer->count >= buffer->capacity) {
        printf("Buffer full!\n");
        return;
    }
    
    // 🐛 BUG: buffer가 NULL이거나 entries가 NULL일 수 있음
    log_entry_t* entry = &buffer->entries[buffer->count];
    
    // 🐛 BUG: message가 NULL일 수 있음
    entry->message = malloc(strlen(message) + 1);
    strcpy(entry->message, message);
    entry->timestamp = time(NULL);
    
    buffer->count++;
}

int main() {
    log_buffer_t* buffer = NULL;  // 🐛 BUG: NULL 포인터!
    
    // 이 호출이 세그폴트 발생
    add_log_entry(buffer, "Test message");
    
    return 0;
}
```

**GDB로 디버깅하기:**

```bash
# [SEQUENCE: 5] GDB 디버깅 세션
$ gcc -g -O0 -o buggy_logcaster buggy_logcaster.c
$ gdb ./buggy_logcaster

(gdb) run
Starting program: ./buggy_logcaster

Program received signal SIGSEGV, Segmentation fault.
0x0000000100000f42 in add_log_entry (buffer=0x0, message=0x100000f9a "Test message") at buggy_logcaster.c:23
23      if (buffer->count >= buffer->capacity) {

# 어디서 크래시가 났는지 확인
(gdb) where
#0  0x0000000100000f42 in add_log_entry (buffer=0x0, message=0x100000f9a "Test message") at buggy_logcaster.c:23
#1  0x0000000100000f7a in main () at buggy_logcaster.c:42

# 변수 값 확인
(gdb) print buffer
$1 = (log_buffer_t *) 0x0

(gdb) print message
$2 = 0x100000f9a "Test message"

# 소스 코드 확인
(gdb) list
18      }
19      
20      // 버그: NULL 포인터 역참조 가능성
21      void add_log_entry(log_buffer_t* buffer, const char* message) {
22          if (buffer->count >= buffer->capacity) {
23              printf("Buffer full!\n");
24              return;
25          }
26          
27          // 🐛 BUG: buffer가 NULL이거나 entries가 NULL일 수 있음

# 스택 프레임 이동해서 main 함수 확인
(gdb) frame 1
#1  0x0000000100000f7a in main () at buggy_logcaster.c:42
42      add_log_entry(buffer, "Test message");

(gdb) list
37      }
38      
39      int main() {
40          log_buffer_t* buffer = NULL;  // 🐛 BUG: NULL 포인터!
41          
42          add_log_entry(buffer, "Test message");
43          
44          return 0;
45      }

# 문제 발견: buffer가 NULL!
```

### 🔧 멀티스레드 디버깅

LogCaster는 멀티스레드 프로그램이므로 스레드별 디버깅이 중요합니다:

```bash
# [SEQUENCE: 6] 멀티스레드 GDB 명령어들
(gdb) info threads            # 모든 스레드 목록
(gdb) thread 2                # 2번 스레드로 전환
(gdb) thread apply all bt     # 모든 스레드의 백트레이스
(gdb) thread apply all info locals  # 모든 스레드의 지역변수

# 특정 스레드에만 브레이크포인트
(gdb) break handle_client thread 3

# 스레드 생성/종료 감지
(gdb) catch syscall clone     # 새 스레드 생성 감지
(gdb) catch syscall exit      # 스레드 종료 감지
```

### 📝 GDB 설정 파일 (.gdbinit)

자주 사용하는 명령어들을 자동화할 수 있습니다:

```bash
# [SEQUENCE: 7] ~/.gdbinit 파일 생성
cat > ~/.gdbinit << 'EOF'
# 예쁘게 출력
set print pretty on
set print array on
set print address on

# 히스토리 저장
set history save on
set history size 10000
set history filename ~/.gdb_history

# LogCaster 전용 명령어 정의
define logcaster-status
    printf "=== LogCaster Debug Status ===\n"
    info threads
    printf "\n=== Current Variables ===\n"
    info locals
    printf "\n=== Call Stack ===\n"
    bt
end

# 자동 브레이크포인트 설정
define logcaster-setup
    break main
    break handle_client
    break add_log_entry
    printf "LogCaster breakpoints set!\n"
end

# 단축 명령어
alias ll = list
alias p = print
alias bt = backtrace
EOF
```

---

## 🔍 2. Valgrind - 메모리 탐정

### 🕵️ Valgrind란?

Valgrind는 **메모리 오류를 찾아주는 메모리 탐정**입니다. 메모리 누수, 잘못된 메모리 접근, 초기화되지 않은 메모리 사용 등을 찾아줍니다.

```
💡 실생활 비유: Valgrind = 메모리 보안관
- 메모리 누수: "이 메모리 주인 없이 떠돌고 있어!"
- 잘못된 접근: "여기 들어가면 안 돼!"
- 이중 해제: "이미 반납한 메모리야!"
```

### 🔧 Valgrind 기본 사용법

```bash
# [SEQUENCE: 8] Valgrind 설치 (Ubuntu/Debian)
sudo apt-get install valgrind

# [SEQUENCE: 9] Valgrind 설치 (macOS)
brew install valgrind

# [SEQUENCE: 10] 메모리 검사 실행
valgrind --tool=memcheck --leak-check=full --track-origins=yes --show-leak-kinds=all ./logcaster

# 플래그 설명:
# --tool=memcheck: 메모리 검사 도구 사용
# --leak-check=full: 상세한 누수 검사
# --track-origins=yes: 초기화되지 않은 메모리 추적
# --show-leak-kinds=all: 모든 종류의 누수 표시
```

### 🐛 메모리 누수 예제

```c
// [SEQUENCE: 11] 메모리 누수가 있는 LogCaster 코드
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct log_entry {
    char* message;
    struct log_entry* next;
} log_entry_t;

log_entry_t* create_log_entry(const char* message) {
    log_entry_t* entry = malloc(sizeof(log_entry_t));
    if (!entry) return NULL;
    
    // 🐛 LEAK: message를 위한 메모리 할당하지만 해제하지 않음
    entry->message = malloc(strlen(message) + 1);
    strcpy(entry->message, message);
    entry->next = NULL;
    
    return entry;
}

void free_log_entry(log_entry_t* entry) {
    if (entry) {
        // 🐛 LEAK: message 메모리를 해제하지 않음!
        // free(entry->message);  // 이 줄이 누락됨
        free(entry);
    }
}

int main() {
    // 여러 로그 엔트리 생성
    for (int i = 0; i < 100; i++) {
        char message[64];
        snprintf(message, sizeof(message), "Log message %d", i);
        
        log_entry_t* entry = create_log_entry(message);
        
        // 사용 후 해제 (하지만 message 메모리는 누수!)
        free_log_entry(entry);
    }
    
    // 🐛 LEAK: 할당한 메모리 일부를 해제하지 않음
    char* never_freed = malloc(1024);
    strcpy(never_freed, "This will never be freed!");
    
    return 0;
}
```

**Valgrind 실행 결과:**

```bash
# [SEQUENCE: 12] Valgrind 출력 분석
$ gcc -g -O0 -o leaky_logcaster leaky_logcaster.c
$ valgrind --tool=memcheck --leak-check=full ./leaky_logcaster

==12345== Memcheck, a memory error detector
==12345== HEAP SUMMARY:
==12345==     in use at exit: 2,324 bytes in 101 blocks
==12345==   total heap usage: 201 allocs, 100 frees, 3,348 bytes allocated
==12345== 
==12345== 1,300 bytes in 100 blocks are definitely lost in loss record 1 of 2
==12345==    at 0x4C29BE3: malloc (vg_replace_malloc.c:299)
==12345==    by 0x400567: create_log_entry (leaky_logcaster.c:13)
==12345==    by 0x4005B4: main (leaky_logcaster.c:28)
==12345== 
==12345== 1,024 bytes in 1 blocks are definitely lost in loss record 2 of 2
==12345==    at 0x4C29BE3: malloc (vg_replace_malloc.c:299)
==12345==    by 0x4005E1: main (leaky_logcaster.c:34)
==12345== 
==12345== LEAK SUMMARY:
==12345==     definitely lost: 2,324 bytes in 101 blocks
==12345==     indirectly lost: 0 bytes in 0 blocks
==12345==       possibly lost: 0 bytes in 0 blocks
==12345==     still reachable: 0 bytes in 0 blocks
==12345==          suppressed: 0 bytes in 0 blocks
```

**해석:**
- `definitely lost`: 확실히 누수된 메모리 (접근 불가능)
- `indirectly lost`: 간접적으로 누수된 메모리 
- `possibly lost`: 누수 가능성이 있는 메모리
- `still reachable`: 프로그램 종료 시점에 접근 가능한 메모리

### 🔧 초기화되지 않은 메모리 검출

```c
// [SEQUENCE: 13] 초기화되지 않은 메모리 사용 예제
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int count;
    int capacity;
    char data[100];
} log_buffer_t;

int main() {
    log_buffer_t* buffer = malloc(sizeof(log_buffer_t));
    
    // 🐛 BUG: buffer를 초기화하지 않고 사용
    if (buffer->count > 0) {  // 초기화되지 않은 값 사용!
        printf("Buffer has %d entries\n", buffer->count);
    }
    
    // 🐛 BUG: data 배열을 초기화하지 않고 출력
    printf("First character: %c\n", buffer->data[0]);
    
    free(buffer);
    return 0;
}
```

**Valgrind 출력:**
```bash
==12346== Conditional jump or move depends on uninitialised value(s)
==12346==    at 0x400537: main (uninit_example.c:15)
==12346== 
==12346== Use of uninitialised value of size 8
==12346==    at 0x400545: main (uninit_example.c:16)
```

### ⚡ Valgrind 고급 기능들

```bash
# [SEQUENCE: 14] 다양한 Valgrind 도구들

# 1. 캐시 프로파일러 (성능 분석)
valgrind --tool=cachegrind ./logcaster
# 결과 분석
cg_annotate cachegrind.out.12345

# 2. 힙 프로파일러 (메모리 사용량 분석)
valgrind --tool=massif ./logcaster
# 결과 시각화
ms_print massif.out.12345

# 3. 스레드 에러 검출 (데이터 경합)
valgrind --tool=helgrind ./logcaster

# 4. DRD (또 다른 스레드 에러 검출기)
valgrind --tool=drd ./logcaster
```

### 📊 Valgrind 출력 해석 가이드

```bash
# [SEQUENCE: 15] Valgrind 오류 메시지 해석
==PID== Invalid read of size 4
==PID==    at 0x40052A: function_name (file.c:42)
==PID==  Address 0x520A040 is 0 bytes after a block of size 40 alloc'd

# 해석:
# - Invalid read: 잘못된 메모리 읽기
# - size 4: 4바이트 읽기 시도
# - 0x520A040: 문제가 된 메모리 주소
# - 0 bytes after: 할당된 블록 끝에서 0바이트 뒤 (즉, 범위 밖)
# - block of size 40: 40바이트 크기의 블록

==PID== Invalid write of size 8
# - Invalid write: 잘못된 메모리 쓰기

==PID== Invalid free() / delete / delete[]
# - 이미 해제된 메모리를 다시 해제하려고 시도

==PID== Mismatched free() / delete / delete []
# - malloc/free vs new/delete 불일치
```

---

## 📊 3. 성능 프로파일링 - 병목점 찾기

### ⚡ 성능 분석이 중요한 이유

LogCaster 서버가 느려진다면? 어디가 문제인지 **추측하지 말고 측정**해야 합니다!

```
💡 성능 최적화의 황금 법칙
1. 측정 없이는 최적화 없다
2. 80%의 시간이 20%의 코드에서 소모된다 (파레토 법칙)  
3. 가장 의외한 곳에서 병목이 발생한다
```

### 🔧 Linux perf 도구

```bash
# [SEQUENCE: 16] perf 설치 및 기본 사용법
sudo apt-get install linux-tools-common linux-tools-generic

# CPU 사용률 프로파일링
perf record -g ./logcaster
perf report

# 시스템 전체 모니터링 (10초간)
perf top -p $(pgrep logcaster) -d 10

# 함수별 호출 횟수 측정
perf stat -e cycles,instructions,cache-misses ./logcaster

# 메모리 접근 패턴 분석
perf record -e cache-misses,cache-references ./logcaster
```

### 📊 gprof를 사용한 프로파일링

```bash
# [SEQUENCE: 17] gprof 프로파일링
# 컴파일 시 -pg 플래그 추가
gcc -g -pg -O0 -o logcaster_prof src/*.c

# 프로그램 실행 (gmon.out 파일 생성)
./logcaster_prof

# 프로파일링 결과 분석
gprof ./logcaster_prof gmon.out > profile_report.txt

# 함수별 시간 분석
gprof -b ./logcaster_prof gmon.out | head -20
```

### 🔍 커스텀 성능 측정 코드

때로는 직접 시간을 측정하는 것이 가장 정확합니다:

```c
// [SEQUENCE: 18] C 버전 성능 측정
#include <time.h>
#include <sys/time.h>

typedef struct {
    struct timespec start_time;
    struct timespec end_time;
    const char* operation_name;
} performance_timer_t;

// 타이머 시작
void start_timer(performance_timer_t* timer, const char* operation) {
    timer->operation_name = operation;
    clock_gettime(CLOCK_MONOTONIC, &timer->start_time);
}

// 타이머 종료 및 결과 출력
void end_timer(performance_timer_t* timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->end_time);
    
    double elapsed = (timer->end_time.tv_sec - timer->start_time.tv_sec) +
                    (timer->end_time.tv_nsec - timer->start_time.tv_nsec) / 1e9;
    
    printf("[PERF] %s took %.6f seconds\n", timer->operation_name, elapsed);
}

// [SEQUENCE: 19] LogCaster 성능 측정 예제
void benchmark_log_operations() {
    performance_timer_t timer;
    
    // 로그 추가 성능 측정
    start_timer(&timer, "Adding 10000 logs");
    for (int i = 0; i < 10000; i++) {
        char message[256];
        snprintf(message, sizeof(message), "Benchmark log message %d", i);
        add_log_entry(&log_buffer, message);
    }
    end_timer(&timer);
    
    // 로그 검색 성능 측정
    start_timer(&timer, "Searching 10000 logs");
    search_logs(&log_buffer, "benchmark");
    end_timer(&timer);
}
```

```cpp
// [SEQUENCE: 20] C++ 버전 성능 측정 (더 편리함)
#include <chrono>

class PerformanceTimer {
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    std::string operation_name_;
    
public:
    PerformanceTimer(const std::string& operation) 
        : operation_name_(operation)
        , start_time_(std::chrono::high_resolution_clock::now()) {
    }
    
    ~PerformanceTimer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time_);
        
        std::cout << "[PERF] " << operation_name_ 
                  << " took " << duration.count() << " microseconds" << std::endl;
    }
};

// [SEQUENCE: 21] RAII를 활용한 자동 측정
void logcaster_performance_test() {
    {
        PerformanceTimer timer("LogBuffer creation and 10000 insertions");
        
        LogBuffer buffer(10000);
        for (int i = 0; i < 10000; i++) {
            buffer.addLog("Performance test message " + std::to_string(i));
        }
    }  // 여기서 자동으로 시간 측정 결과 출력
    
    {
        PerformanceTimer timer("Regex search in 10000 logs");
        // 검색 로직...
    }
}
```

### 📊 메모리 사용량 모니터링

```c
// [SEQUENCE: 22] 메모리 사용량 측정
#include <sys/resource.h>

void print_memory_usage(const char* label) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    
    printf("[MEMORY] %s - RSS: %ld KB, Peak: %ld KB\n",
           label,
           usage.ru_maxrss,      // 현재 메모리 사용량
           usage.ru_maxrss);     // 최대 메모리 사용량
}

// [SEQUENCE: 23] 파일에서 상세한 메모리 정보 읽기 (Linux)
void print_detailed_memory_info() {
    FILE* status_file = fopen("/proc/self/status", "r");
    if (!status_file) {
        perror("fopen /proc/self/status");
        return;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), status_file)) {
        if (strncmp(line, "VmRSS:", 6) == 0 ||
            strncmp(line, "VmSize:", 7) == 0 ||
            strncmp(line, "VmPeak:", 7) == 0) {
            printf("[MEMORY] %s", line);
        }
    }
    
    fclose(status_file);
}
```

---

## 🧵 4. 멀티스레드 디버깅 - 동시성 문제 해결

### 🔄 데드락 (Deadlock) 검출

LogCaster에서 가장 흔한 멀티스레드 문제 중 하나는 데드락입니다:

```c
// [SEQUENCE: 24] 데드락이 발생할 수 있는 코드
#include <pthread.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread1_func(void* arg) {
    printf("Thread 1: Locking mutex1\n");
    pthread_mutex_lock(&mutex1);
    
    usleep(100000);  // 100ms 대기 (데드락 유발용)
    
    printf("Thread 1: Trying to lock mutex2\n");
    pthread_mutex_lock(&mutex2);  // 데드락 발생 지점!
    
    printf("Thread 1: Got both mutexes\n");
    
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    return NULL;
}

void* thread2_func(void* arg) {
    printf("Thread 2: Locking mutex2\n");
    pthread_mutex_lock(&mutex2);
    
    usleep(100000);  // 100ms 대기
    
    printf("Thread 2: Trying to lock mutex1\n");
    pthread_mutex_lock(&mutex1);  // 데드락 발생 지점!
    
    printf("Thread 2: Got both mutexes\n");
    
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    return NULL;
}
```

**Helgrind로 데드락 검출:**

```bash
# [SEQUENCE: 25] Helgrind를 사용한 데드락 검출
valgrind --tool=helgrind ./deadlock_example

==12347== Possible data race during read of size 4 at 0x60104C by thread #3
==12347== Locks held: none
==12347==    at 0x400B2C: thread2_func (deadlock.c:25)
==12347== 
==12347== This conflicts with a previous write of size 4 by thread #2
==12347== Locks held: 1, at address 0x601080
==12347==    at 0x400A8F: thread1_func (deadlock.c:15)
==12347== 
==12347== Thread #2 was created
==12347==    at 0x4C30E90: clone (vg_replace_malloc.c:301)
==12347==    by 0x4011893: pthread_create@@GLIBC_2.2.5 (pthread_create.c:512)
==12347==    by 0x400C1A: main (deadlock.c:45)
```

### 🔍 GDB를 사용한 데드락 디버깅

```bash
# [SEQUENCE: 26] 데드락 발생 시 GDB 디버깅
(gdb) run
# 프로그램이 멈추면 Ctrl+C

(gdb) info threads
  Id   Target Id         Frame 
  3    Thread 0x7ffff6ff3700 (LWP 12349) 0x7ffff78d4bcd in __lll_lock_wait ()
  2    Thread 0x7ffff77f4700 (LWP 12348) 0x7ffff78d4bcd in __lll_lock_wait ()
* 1    Thread 0x7ffff7fe3740 (LWP 12347) 0x7ffff78d1344 in pthread_join ()

# 각 스레드에서 무엇을 기다리고 있는지 확인
(gdb) thread 2
(gdb) bt
#0  0x7ffff78d4bcd in __lll_lock_wait ()
#1  0x7ffff78d0e4f in _L_lock_870 ()
#2  0x7ffff78d0cdf in pthread_mutex_lock ()
#3  0x0000000000400a9f in thread1_func (arg=0x0) at deadlock.c:18

(gdb) thread 3  
(gdb) bt
#0  0x7ffff78d4bcd in __lll_lock_wait ()
#1  0x7ffff78d0e4f in _L_lock_870 ()
#2  0x7ffff78d0cdf in pthread_mutex_lock ()
#3  0x0000000000400b3c in thread2_func (arg=0x0) at deadlock.c:30

# 두 스레드 모두 mutex_lock에서 대기 중 → 데드락!
```

### 🛡️ 데드락 방지 전략

```c
// [SEQUENCE: 27] 데드락 방지 - 순서 지정 방식
void safe_dual_lock() {
    // 항상 mutex1을 먼저, mutex2를 나중에 잠금
    pthread_mutex_lock(&mutex1);
    pthread_mutex_lock(&mutex2);
    
    // 중요한 작업 수행
    
    // 역순으로 해제
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
}

// [SEQUENCE: 28] 타임아웃을 사용한 데드락 방지
int safe_timed_lock(pthread_mutex_t* mutex, int timeout_ms) {
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_nsec += timeout_ms * 1000000;
    
    int result = pthread_mutex_timedlock(mutex, &timeout);
    if (result == ETIMEDOUT) {
        printf("Lock timeout - possible deadlock avoided!\n");
        return -1;
    }
    
    return result;
}
```

---

## 🏥 5. 로그 기반 디버깅 - 운영 환경의 문제 해결

### 📝 효과적인 로그 메시지 작성

운영 환경에서는 GDB나 Valgrind를 사용할 수 없습니다. 로그가 유일한 디버깅 수단이죠!

```c
// [SEQUENCE: 29] 좋은 로그 메시지 vs 나쁜 로그 메시지

// ❌ 나쁜 로그
printf("Error occurred\n");
printf("Client connected\n");
printf("Processing request\n");

// ✅ 좋은 로그
printf("[ERROR] Database connection failed: %s (errno=%d) at %s:%d\n", 
       strerror(errno), errno, __FILE__, __LINE__);
printf("[INFO] Client connected from %s:%d (socket_fd=%d)\n", 
       client_ip, client_port, client_fd);
printf("[DEBUG] Processing %s request (size=%zu bytes, client_id=%d)\n", 
       request_type, request_size, client_id);
```

### 🎯 구조화된 로그 시스템

```c
// [SEQUENCE: 30] LogCaster용 구조화된 로그 시스템
typedef enum {
    LOG_TRACE,    // 상세한 실행 흐름
    LOG_DEBUG,    // 디버깅 정보
    LOG_INFO,     // 일반 정보
    LOG_WARN,     // 경고
    LOG_ERROR,    // 오류
    LOG_FATAL     // 치명적 오류
} log_level_t;

const char* LOG_LEVEL_NAMES[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

// [SEQUENCE: 31] 로그 레벨별 출력 함수
void logcaster_log(log_level_t level, const char* file, int line, 
                   const char* func, const char* fmt, ...) {
    // 현재 시간
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    
    // 스레드 ID
    pthread_t thread_id = pthread_self();
    
    // 로그 헤더 출력
    printf("[%04d-%02d-%02d %02d:%02d:%02d] [%s] [Thread-%lu] [%s:%d in %s()] ",
           tm_info->tm_year + 1900,
           tm_info->tm_mon + 1,
           tm_info->tm_mday,
           tm_info->tm_hour,
           tm_info->tm_min,
           tm_info->tm_sec,
           LOG_LEVEL_NAMES[level],
           (unsigned long)thread_id,
           file,
           line,
           func);
    
    // 실제 로그 메시지 출력
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    
    printf("\n");
    fflush(stdout);
}

// [SEQUENCE: 32] 편리한 매크로 정의
#define LOG_TRACE(...) logcaster_log(LOG_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_DEBUG(...) logcaster_log(LOG_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(...)  logcaster_log(LOG_INFO,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARN(...)  logcaster_log(LOG_WARN,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...) logcaster_log(LOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_FATAL(...) logcaster_log(LOG_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

// [SEQUENCE: 33] 사용 예제
void handle_client_connection(int client_fd, const char* client_ip) {
    LOG_INFO("New client connection from %s (fd=%d)", client_ip, client_fd);
    
    char buffer[1024];
    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read < 0) {
        LOG_ERROR("Failed to receive data from client %s: %s", 
                  client_ip, strerror(errno));
        return;
    }
    
    if (bytes_read == 0) {
        LOG_INFO("Client %s closed connection", client_ip);
        return;
    }
    
    buffer[bytes_read] = '\0';
    LOG_DEBUG("Received %zd bytes from %s: '%.50s%s'", 
              bytes_read, client_ip, buffer, 
              strlen(buffer) > 50 ? "..." : "");
    
    // 로그 처리...
    LOG_TRACE("Processing log message from %s", client_ip);
    
    if (process_log_message(buffer) != 0) {
        LOG_ERROR("Failed to process log message from %s", client_ip);
    } else {
        LOG_DEBUG("Successfully processed log from %s", client_ip);
    }
}
```

### 📊 성능 모니터링 로그

```c
// [SEQUENCE: 34] 성능 지표를 로그로 기록
typedef struct {
    unsigned long total_connections;
    unsigned long active_connections;
    unsigned long total_logs_processed;
    unsigned long total_bytes_received;
    time_t start_time;
} server_stats_t;

server_stats_t g_server_stats = {0};

void log_server_statistics() {
    time_t current_time = time(NULL);
    double uptime = difftime(current_time, g_server_stats.start_time);
    
    double logs_per_second = g_server_stats.total_logs_processed / uptime;
    double bytes_per_second = g_server_stats.total_bytes_received / uptime;
    
    LOG_INFO("=== Server Statistics ===");
    LOG_INFO("Uptime: %.0f seconds", uptime);
    LOG_INFO("Total connections: %lu", g_server_stats.total_connections);
    LOG_INFO("Active connections: %lu", g_server_stats.active_connections); 
    LOG_INFO("Total logs processed: %lu", g_server_stats.total_logs_processed);
    LOG_INFO("Total bytes received: %lu", g_server_stats.total_bytes_received);
    LOG_INFO("Average logs/sec: %.2f", logs_per_second);
    LOG_INFO("Average bytes/sec: %.2f", bytes_per_second);
    
    // 메모리 사용량도 함께 기록
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    LOG_INFO("Memory usage (RSS): %ld KB", usage.ru_maxrss);
}

// [SEQUENCE: 35] 주기적 통계 출력 (별도 스레드에서)
void* statistics_thread(void* arg) {
    while (server_running) {
        sleep(60);  // 1분마다
        log_server_statistics();
    }
    return NULL;
}
```

---

## 🔥 6. 흔한 실수와 해결방법

### 6.1 GDB 관련 실수

#### [SEQUENCE: 36] 최적화된 코드 디버깅
```bash
# ❌ 잘못된 예: 최적화 플래그와 함께 컴파일
gcc -O2 -g -o program program.c
# 변수가 최적화되어 사라지거나 함수가 인라인됨

# ✅ 올바른 예: 디버깅용 빌드
gcc -O0 -g -o program_debug program.c
```

### 6.2 Valgrind 관련 실수

#### [SEQUENCE: 37] 불완전한 메모리 검사
```bash
# ❌ 잘못된 예: 기본 옵션만 사용
valgrind ./program

# ✅ 올바른 예: 상세한 검사 옵션
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
         --track-origins=yes --verbose ./program
```

### 6.3 프로파일링 실수

#### [SEQUENCE: 38] 대표성 없는 테스트
```c
// ❌ 잘못된 예: 실제와 다른 작은 데이터셋
void benchmark() {
    for (int i = 0; i < 10; i++) {  // 너무 작은 반복
        process_log("test");
    }
}

// ✅ 올바른 예: 실제 워크로드 시뮬레이션
void realistic_benchmark() {
    // 실제 로그 파일 로드
    char** logs = load_real_logs("production_logs.txt");
    
    // 충분한 횟수 반복 (통계적 유의성)
    for (int i = 0; i < 100000; i++) {
        process_log(logs[i % log_count]);
    }
}
```

---

## 7. 실습 프로젝트

### 프로젝트 1: 메모리 누수 탐정 (기초)
**목표**: 의도적으로 만든 메모리 누수를 찾고 수정

**구현 사항**:
- 다양한 메모리 누수 패턴 생성
- Valgrind로 누수 탐지
- 누수 수정 및 검증
- 자동화된 메모리 검사 스크립트

### 프로젝트 2: 성능 병목점 분석기 (중급)
**목표**: LogCaster의 성능 병목점 식별 및 최적화

**구현 사항**:
- CPU 프로파일링
- 메모리 액세스 패턴 분석
- I/O 병목점 검출
- 최적화 전후 비교 리포트

### 프로젝트 3: 멀티스레드 디버거 도구 (고급)
**목표**: 데드락과 레이스 컨디션 자동 검출 도구 개발

**구현 사항**:
- 스레드 모니터링 시스템
- 락 순서 분석
- 데드락 예측 알고리즘
- 시각화된 스레드 상태 표시

---

## 8. 학습 체크리스트

### 기초 레벨 ✅
- [ ] GDB 기본 명령어 숙지
- [ ] 브레이크포인트 설정과 스텝 실행
- [ ] Valgrind로 메모리 누수 검출
- [ ] 기본적인 성능 측정

### 중급 레벨 📚
- [ ] 조건부 브레이크포인트 활용
- [ ] 멀티스레드 디버깅
- [ ] 메모리 오류 패턴 이해
- [ ] 프로파일링 결과 분석

### 고급 레벨 🚀
- [ ] 코어 덤프 분석
- [ ] 원격 디버깅
- [ ] 커스텀 Valgrind 도구 작성
- [ ] 시스템 레벨 성능 분석

### 전문가 레벨 🏆
- [ ] 어셈블리 레벨 디버깅
- [ ] 커널 디버깅 기법
- [ ] 분산 시스템 디버깅
- [ ] 프로덕션 환경 문제 해결

---

## 9. 추가 학습 자료

### 추천 도서
- "The Art of Debugging with GDB, DDD, and Eclipse" - Norman Matloff
- "Valgrind 3.3 - Advanced Debugging and Profiling" - J. Seward
- "Systems Performance" - Brendan Gregg

### 온라인 리소스
- [GDB Documentation](https://www.gnu.org/software/gdb/documentation/)
- [Valgrind User Manual](https://valgrind.org/docs/manual/manual.html)
- [Linux Performance](http://www.brendangregg.com/linuxperf.html)

### 실습 도구
- rr (Record and Replay debugger)
- AddressSanitizer (ASan)
- ThreadSanitizer (TSan)
- Intel VTune Profiler

---

## ✅ 6. 다음 단계 준비

이 문서를 완전히 이해했다면:

1. **GDB**로 세그폴트와 논리 오류를 디버깅할 수 있어야 합니다
2. **Valgrind**로 메모리 누수와 메모리 오류를 찾을 수 있어야 합니다  
3. **성능 프로파일링**으로 병목점을 식별하고 최적화할 수 있어야 합니다
4. **로그 기반 디버깅**으로 운영 환경의 문제를 진단할 수 있어야 합니다

### 🎯 확인 문제

1. LogCaster 서버에서 데드락이 발생했다면 GDB를 사용해서 어떻게 원인을 찾을 수 있을까요?

2. Valgrind에서 "definitely lost" vs "still reachable" 메모리의 차이점은 무엇인가요?

3. LogCaster 서버의 처리량이 갑자기 떨어졌다면 어떤 순서로 성능 분석을 진행해야 할까요?

다음 문서에서는 **객체 지향 프로그래밍**에 대해 자세히 알아보겠습니다!

---

*💡 팁: 디버깅은 탐정 작업입니다. 단서를 모으고, 가설을 세우고, 실험으로 검증하세요. 도구는 여러분의 든든한 파트너입니다!*