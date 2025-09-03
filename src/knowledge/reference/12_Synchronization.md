# 6. 동기화 메커니즘 🔒
## 여러 스레드가 사이좋게 일하는 법
*LogCaster 프로젝트를 위한 완벽 가이드*

---

## 📋 문서 정보
- **난이도**: 🟡 중급 → 🔴 고급
- **예상 학습 시간**: 10-12시간
- **전제 조건**: 
  - C/C++ 기본 문법
  - 멀티스레딩 기초 이해
  - 포인터와 메모리 관리
  - 기본 자료구조
- **실습 환경**: Linux/macOS, GCC/G++ (pthread 지원), C11 이상 (atomic 지원)

## 🎯 이 문서에서 배울 내용

**"여러 사람이 같은 공간을 사용할 때 어떻게 해야 할까요? 🤔"**

> 📝 **전제 조건**: 이 문서를 읽기 전에 [5. Multithreading.md](./5.%20Multithreading.md)에서 스레드의 기본 개념을 이해하는 것을 권장합니다.

동기화 메커니즘을 **공용 주방**에 비유해보세요:

🍳 **문제 상황**: 여러 사람이 같은 주방에서 요리하려고 한다면?
- A가 냉장고를 열어서 우유를 꺼내는데...
- 동시에 B도 냉장고를 열어서 우유를 꺼내려고 하면?
- 결과: 충돌! 혹은 우유가 떨어지거나, 둘 다 우유를 못 마실 수도...

🔒 **해결책**: 주방 사용 규칙을 만들자!
- 냉장고를 사용할 때는 반드시 열쇠를 잠그기
- 사용 후에는 반드시 열쇠 푽기
- 다른 사람이 사용 중이면 기다리기

LogCaster에서도 마찬가지예요! 여러 스레드가 동시에 로그 데이터에 접근하려고 하면 충돌이 일어날 수 있어요. 그래서 **동기화 메커니즘**이 필요한 거죠! 

### 학습 목표
- 🔑 **Mutex(락)**: 공용 자원의 '열쇠' 사용법 완벽 마스터
- 📚 **Reader-Writer Lock**: 독자는 여러 명, 작가는 한 명만!
- ⚡ **Lock-Free 프로그래밍**: 열쇠 없이도 안전하게!
- 🛡️ **Thread-Safe 자료구조**: 누가 사용해도 안전한 컴퓨터
- 🏠 **스레드 풀**: 직원들을 효율적으로 관리하는 비법

### 이 문서의 특징
- 🎨 **시각적인 비유**: 복잡한 개념을 일상생활 예시로!
- 🔨 **실제 동작하는 코드**: 복사해서 바로 사용 가능!
- 📊 **성능 비교**: 어떤 방법이 더 빠른지 숫자로 증명!
- 🐛 **실수 방지**: 초보자가 자주 하는 실수와 해결법!

---

## 🔐 1. Mutex와 Lock 심화 - 화장실 열쇠의 모든 것

### 1.1 Mutex를 화장실에 비유하면?

**"가장 이해하기 쉽운 비유로 시작해보세요! 🚪"**

학교나 집에서 화장실을 사용할 때를 생각해보세요:

```
🚪 화장실 = 공유 자원 (로그 데이터)
🔑 열쇠 = Mutex
👥 사람들 = 스레드

📋 화장실 사용 규칙:
1. 화장실에 들어가려면 열쇠가 필요함 (mutex_lock)
2. 한 번에 한 사람만 사용 가능 (상호 배제)
3. 사용 후 열쇠 반납 (mutex_unlock)
4. 열쇠가 없으면 기다림 (대기 상태)

🤔 **어긴다면?**
- 누가 화장실을 사용 중이면 기다리죠 (상식적!)
- 두 명이 동시에 들어가면 난리나죠 (충돌 발생!)
- LogCaster에서도 마찬가지예요!
```

💡 **실제 LogCaster 예시**:
```
📝 로그 데이터 = 화장실
🧵 스레드 A = "새 로그를 추가하고 싶어!"
🧵 스레드 B = "나도 새 로그를 추가하고 싶어!"

동기화 없이: 두 스레드가 동시에 데이터를 수정 → 데이터 손상!
동기화 있음: 한 스레드가 끝날 때까지 다른 스레드 대기 → 안전!
```

### 1.2 Mutex의 내부 동작 원리

Mutex(Mutual Exclusion)는 **원자적 연산(Atomic Operation)**을 기반으로 작동합니다.

#### 원자적 연산이란?
원자적 연산은 중간에 끊기지 않고 한 번에 완료되는 연산입니다.

```
일반 연산 (3단계로 나뉨):          원자적 연산 (한 번에):
counter = counter + 1              atomic_increment(&counter)
1. temp = counter   (읽기)         → 한 번에 증가!
2. temp = temp + 1  (계산)
3. counter = temp   (쓰기)

문제: 스레드 A가 1번을 하는 중에
      스레드 B도 1번을 하면?
      → 둘 다 같은 값을 읽음!
```

```c
// Mutex의 기본 구조 (단순화된 버전)
typedef struct {
    int locked;        // 0: 🔓 잠금 해제, 1: 🔒 잠금
    int owner_tid;     // 👤 소유자 스레드 ID
    int wait_count;    // 🚶 대기 중인 스레드 수
} simple_mutex_t;

// Test-and-Set 원자적 연산 (어셈블리 수준)
// "문 잠그고 이전 상태 확인" - 한 번에!
int test_and_set(int* target) {
    // 이 3줄이 실제로는 CPU의 단일 명령어로 실행됨
    int old_value = *target;  // 현재 값 읽기
    *target = 1;              // 1로 설정 (잠금)
    return old_value;         // 이전 값 반환
}

// 단순한 스핀락 구현 (계속 돌면서 확인)
void simple_lock(simple_mutex_t* mutex) {
    // test_and_set이 0을 반환할 때까지 계속 시도
    // 0 반환 = 이전에 잠겨있지 않았음 = 내가 잠금 성공!
    while (test_and_set(&mutex->locked) == 1) {
        // 🔄 계속 시도 (CPU 사이클 소모)
        // 문제: CPU를 100% 사용하면서 기다림
    }
    mutex->owner_tid = get_current_thread_id();
}

void simple_unlock(simple_mutex_t* mutex) {
    mutex->owner_tid = 0;
    mutex->locked = 0;  // 🔓 원자적 해제
}

/* 스핀락 vs 일반 뮤텍스
스핀락: 🏃 계속 뛰면서 확인 (짧은 대기에 유리)
뮤텍스: 😴 잠들어서 기다림 (긴 대기에 유리)
*/
```

### 1.3 다양한 Mutex 타입 - 상황별 열쇠 선택하기

```
🔑 일반 뮤텍스: 기본 열쇠 (재진입 불가)
🔑🔑 재귀 뮤텍스: 마스터키 (같은 사람은 여러 번 가능)
🔑❗ 에러체크 뮤텍스: 똑똑한 열쇠 (실수 방지)
⏰ 타임드 뮤텍스: 시간제한 열쇠
```

#### 1. 일반 Mutex (PTHREAD_MUTEX_NORMAL) - 기본 열쇠

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t normal_mutex = PTHREAD_MUTEX_INITIALIZER;

void* normal_mutex_test(void* arg) {
    int thread_id = *(int*)arg;
    
    printf("Thread %d: Trying to lock\n", thread_id);
    
    pthread_mutex_lock(&normal_mutex);
    printf("Thread %d: Got lock, working...\n", thread_id);
    
    // 중요: 같은 스레드가 다시 lock을 시도하면 데드락
    // pthread_mutex_lock(&normal_mutex);  // 절대 하지 말 것!
    
    sleep(2);  // 작업 시뮬레이션
    
    printf("Thread %d: Releasing lock\n", thread_id);
    pthread_mutex_unlock(&normal_mutex);
    
    return NULL;
}
```

#### 2. 재귀 Mutex (PTHREAD_MUTEX_RECURSIVE) - 마스터키

재귀 뮤텍스는 같은 스레드가 여러 번 잠글 수 있습니다.

```
일반 뮤텍스:                    재귀 뮤텍스:
🔒 lock()                      🔒 lock() - count: 1
🔒 lock() → 💀 데드락!          🔒 lock() - count: 2
                               🔓 unlock() - count: 1
                               🔓 unlock() - count: 0 (해제)
```

```c
pthread_mutex_t recursive_mutex;

void init_recursive_mutex() {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&recursive_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

void recursive_function(int depth) {
    pthread_mutex_lock(&recursive_mutex);
    
    printf("Recursion depth: %d\n", depth);
    
    if (depth > 0) {
        recursive_function(depth - 1);  // 같은 스레드가 다시 lock
    }
    
    pthread_mutex_unlock(&recursive_mutex);
}

void* recursive_test(void* arg) {
    recursive_function(3);
    return NULL;
}
```

#### 3. 에러 체크 Mutex (PTHREAD_MUTEX_ERRORCHECK) - 똑똑한 열쇠

프로그래머의 실수를 방지해주는 안전한 뮤텍스입니다.

```c
pthread_mutex_t errorcheck_mutex;

void init_errorcheck_mutex() {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&errorcheck_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

void* errorcheck_test(void* arg) {
    int result;
    
    // 정상적인 lock
    result = pthread_mutex_lock(&errorcheck_mutex);
    printf("First lock result: %d\n", result);  // 0 (성공)
    
    // 같은 스레드가 다시 lock 시도
    result = pthread_mutex_lock(&errorcheck_mutex);
    printf("Second lock result: %d\n", result);  // EDEADLK (35)
    
    pthread_mutex_unlock(&errorcheck_mutex);
    
    // 이미 unlock된 mutex를 다시 unlock
    result = pthread_mutex_unlock(&errorcheck_mutex);
    printf("Double unlock result: %d\n", result);  // EPERM (1)
    
    return NULL;
}
```

#### 4. 타임드 Mutex - 시간제한 열쇠

```c
// 타임드 뮤텍스 사용 예제
pthread_mutex_t timed_mutex = PTHREAD_MUTEX_INITIALIZER;

void* timed_lock_example(void* arg) {
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 5;  // 5초 제한
    
    int result = pthread_mutex_timedlock(&timed_mutex, &timeout);
    
    if (result == 0) {
        printf("🔓 Lock acquired!\n");
        // 작업 수행...
        pthread_mutex_unlock(&timed_mutex);
    } else if (result == ETIMEDOUT) {
        printf("⏰ Timeout! Could not acquire lock in 5 seconds\n");
    } else {
        printf("❌ Error: %s\n", strerror(result));
    }
    
    return NULL;
}
```

### 1.4 LogCaster용 Thread-Safe 데이터 구조

#### 1. Thread-Safe 연결 리스트 - 안전한 로그 저장소

여러 스레드가 동시에 로그를 추가/삭제할 수 있는 안전한 리스트입니다.

```c
#include <pthread.h>
#include <time.h>

typedef struct LogNode {
    char message[256];
    time_t timestamp;
    char client_ip[16];
    int client_port;
    struct LogNode* next;
} LogNode;

typedef struct ThreadSafeLogList {
    LogNode* head;
    LogNode* tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;  // 리스트가 비어있지 않음을 알림
} ThreadSafeLogList;

// 리스트 초기화
ThreadSafeLogList* create_log_list() {
    ThreadSafeLogList* list = malloc(sizeof(ThreadSafeLogList));
    if (list == NULL) return NULL;
    
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    
    pthread_mutex_init(&list->mutex, NULL);
    pthread_cond_init(&list->not_empty, NULL);
    
    return list;
}

// 로그 추가 (앞쪽에 삽입)
int log_list_push_front(ThreadSafeLogList* list, const char* message, 
                       const char* client_ip, int client_port) {
    LogNode* new_node = malloc(sizeof(LogNode));
    if (new_node == NULL) return -1;
    
    // 데이터 설정
    strncpy(new_node->message, message, sizeof(new_node->message) - 1);
    new_node->message[sizeof(new_node->message) - 1] = '\0';
    new_node->timestamp = time(NULL);
    strncpy(new_node->client_ip, client_ip, sizeof(new_node->client_ip) - 1);
    new_node->client_ip[sizeof(new_node->client_ip) - 1] = '\0';
    new_node->client_port = client_port;
    
    pthread_mutex_lock(&list->mutex);
    
    new_node->next = list->head;
    list->head = new_node;
    
    if (list->tail == NULL) {
        list->tail = new_node;
    }
    
    list->count++;
    
    // 대기 중인 소비자에게 알림
    pthread_cond_signal(&list->not_empty);
    
    pthread_mutex_unlock(&list->mutex);
    
    return 0;
}

// 로그 추가 (뒤쪽에 삽입)
int log_list_push_back(ThreadSafeLogList* list, const char* message,
                      const char* client_ip, int client_port) {
    LogNode* new_node = malloc(sizeof(LogNode));
    if (new_node == NULL) return -1;
    
    // 데이터 설정
    strncpy(new_node->message, message, sizeof(new_node->message) - 1);
    new_node->message[sizeof(new_node->message) - 1] = '\0';
    new_node->timestamp = time(NULL);
    strncpy(new_node->client_ip, client_ip, sizeof(new_node->client_ip) - 1);
    new_node->client_ip[sizeof(new_node->client_ip) - 1] = '\0';
    new_node->client_port = client_port;
    new_node->next = NULL;
    
    pthread_mutex_lock(&list->mutex);
    
    if (list->tail == NULL) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        list->tail->next = new_node;
        list->tail = new_node;
    }
    
    list->count++;
    
    pthread_cond_signal(&list->not_empty);
    
    pthread_mutex_unlock(&list->mutex);
    
    return 0;
}

// 로그 제거 (앞쪽에서)
LogNode* log_list_pop_front(ThreadSafeLogList* list) {
    pthread_mutex_lock(&list->mutex);
    
    while (list->head == NULL) {
        pthread_cond_wait(&list->not_empty, &list->mutex);
    }
    
    LogNode* node = list->head;
    list->head = node->next;
    
    if (list->head == NULL) {
        list->tail = NULL;
    }
    
    list->count--;
    
    pthread_mutex_unlock(&list->mutex);
    
    node->next = NULL;  // 연결 끊기
    return node;
}

// 비블로킹 pop (즉시 반환)
LogNode* log_list_try_pop_front(ThreadSafeLogList* list) {
    pthread_mutex_lock(&list->mutex);
    
    if (list->head == NULL) {
        pthread_mutex_unlock(&list->mutex);
        return NULL;
    }
    
    LogNode* node = list->head;
    list->head = node->next;
    
    if (list->head == NULL) {
        list->tail = NULL;
    }
    
    list->count--;
    
    pthread_mutex_unlock(&list->mutex);
    
    node->next = NULL;
    return node;
}

// 리스트 크기 조회
int log_list_size(ThreadSafeLogList* list) {
    pthread_mutex_lock(&list->mutex);
    int size = list->count;
    pthread_mutex_unlock(&list->mutex);
    return size;
}

// 키워드로 검색
int log_list_search(ThreadSafeLogList* list, const char* keyword, 
                   LogNode** results, int max_results) {
    int found_count = 0;
    
    pthread_mutex_lock(&list->mutex);
    
    LogNode* current = list->head;
    while (current != NULL && found_count < max_results) {
        if (strstr(current->message, keyword) != NULL) {
            // 결과 복사
            results[found_count] = malloc(sizeof(LogNode));
            memcpy(results[found_count], current, sizeof(LogNode));
            results[found_count]->next = NULL;
            found_count++;
        }
        current = current->next;
    }
    
    pthread_mutex_unlock(&list->mutex);
    
    return found_count;
}

// 리스트 정리
void destroy_log_list(ThreadSafeLogList* list) {
    if (list == NULL) return;
    
    pthread_mutex_lock(&list->mutex);
    
    LogNode* current = list->head;
    while (current != NULL) {
        LogNode* next = current->next;
        free(current);
        current = next;
    }
    
    pthread_mutex_unlock(&list->mutex);
    
    pthread_mutex_destroy(&list->mutex);
    pthread_cond_destroy(&list->not_empty);
    free(list);
}
```

#### 2. Thread-Safe 원형 버퍼 - 효율적인 로그 큐

고정 크기의 원형 버퍼로 메모리 효율적인 로그 관리가 가능합니다.

```
원형 버퍼 동작 원리:

초기 상태:        가득 찬 상태:      순환 상태:
[_][_][_][_]     [A][B][C][D]      [E][F][C][D]
 ^head,tail       ^tail  ^head      ^tail  ^head

head: 읽기 위치
tail: 쓰기 위치
```

```c
#define BUFFER_SIZE 1000

typedef struct CircularLogBuffer {
    LogNode* buffer[BUFFER_SIZE];
    int head;           // 읽기 위치
    int tail;           // 쓰기 위치
    int count;          // 현재 아이템 수
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} CircularLogBuffer;

CircularLogBuffer* create_circular_buffer() {
    CircularLogBuffer* cb = malloc(sizeof(CircularLogBuffer));
    if (cb == NULL) return NULL;
    
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    
    pthread_mutex_init(&cb->mutex, NULL);
    pthread_cond_init(&cb->not_empty, NULL);
    pthread_cond_init(&cb->not_full, NULL);
    
    return cb;
}

// 버퍼에 로그 추가
int circular_buffer_put(CircularLogBuffer* cb, LogNode* node) {
    pthread_mutex_lock(&cb->mutex);
    
    // 버퍼가 가득 찰 때까지 대기
    while (cb->count == BUFFER_SIZE) {
        pthread_cond_wait(&cb->not_full, &cb->mutex);
    }
    
    cb->buffer[cb->tail] = node;
    cb->tail = (cb->tail + 1) % BUFFER_SIZE;
    cb->count++;
    
    pthread_cond_signal(&cb->not_empty);
    
    pthread_mutex_unlock(&cb->mutex);
    
    return 0;
}

// 버퍼에서 로그 가져오기
LogNode* circular_buffer_get(CircularLogBuffer* cb) {
    pthread_mutex_lock(&cb->mutex);
    
    // 버퍼가 비어있을 때까지 대기
    while (cb->count == 0) {
        pthread_cond_wait(&cb->not_empty, &cb->mutex);
    }
    
    LogNode* node = cb->buffer[cb->head];
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count--;
    
    pthread_cond_signal(&cb->not_full);
    
    pthread_mutex_unlock(&cb->mutex);
    
    return node;
}

// 타임아웃이 있는 get
LogNode* circular_buffer_get_timeout(CircularLogBuffer* cb, int timeout_ms) {
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += timeout_ms / 1000;
    timeout.tv_nsec += (timeout_ms % 1000) * 1000000;
    
    pthread_mutex_lock(&cb->mutex);
    
    while (cb->count == 0) {
        int result = pthread_cond_timedwait(&cb->not_empty, &cb->mutex, &timeout);
        if (result == ETIMEDOUT) {
            pthread_mutex_unlock(&cb->mutex);
            return NULL;
        }
    }
    
    LogNode* node = cb->buffer[cb->head];
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count--;
    
    pthread_cond_signal(&cb->not_full);
    
    pthread_mutex_unlock(&cb->mutex);
    
    return node;
}
```

---

## 🔄 2. Reader-Writer Lock 심화 - 도서관의 열람실과 편집실

### 2.1 Reader-Writer Lock 이해하기

```
📚 도서관 비유:
- 📖 Reader = 책을 읽는 사람 (여러 명 동시 가능)
- ✏️ Writer = 책을 수정하는 사람 (한 명만 가능)

규칙:
1. 여러 Reader는 동시에 읽기 가능
2. Writer가 쓰는 동안은 아무도 못 들어옴
3. Reader가 읽는 동안 Writer는 대기
```

### 2.2 Reader-Writer Lock의 공정성 문제

일반적인 Reader-Writer Lock은 **Reader 선호** 또는 **Writer 선호** 방식을 가집니다.

```
Reader 선호 문제:
R1 읽는 중... R2 들어옴... R3 들어옴... W1 대기...
R4 들어옴... R5 들어옴... W1 계속 대기... 😢

Writer 기아 상태 발생!
```

#### 2.3 Reader 선호 방식의 문제점 시뮬레이션

```c
#include <pthread.h>

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
int shared_data = 0;

// Reader가 계속 들어오면 Writer가 기아 상태에 빠질 수 있음
void* continuous_reader(void* arg) {
    int reader_id = *(int*)arg;
    
    for (int i = 0; i < 100; i++) {
        pthread_rwlock_rdlock(&rwlock);
        
        printf("Reader %d reading: %d\n", reader_id, shared_data);
        usleep(10000);  // 10ms 읽기 작업
        
        pthread_rwlock_unlock(&rwlock);
        
        usleep(1000);   // 1ms 대기 후 다시 읽기
    }
    
    return NULL;
}

void* writer(void* arg) {
    int writer_id = *(int*)arg;
    
    for (int i = 0; i < 10; i++) {
        printf("Writer %d waiting to write...\n", writer_id);
        
        pthread_rwlock_wrlock(&rwlock);
        
        shared_data++;
        printf("Writer %d wrote: %d\n", writer_id, shared_data);
        usleep(50000);  // 50ms 쓰기 작업
        
        pthread_rwlock_unlock(&rwlock);
        
        sleep(1);
    }
    
    return NULL;
}
```

#### 2.4 공정한 Reader-Writer Lock 구현

대기 중인 Writer가 있으면 새로운 Reader도 대기하도록 하여 공정성을 보장합니다.

```c
typedef struct FairRWLock {
    pthread_mutex_t mutex;
    pthread_cond_t readers_cond;
    pthread_cond_t writers_cond;
    int readers_count;
    int writers_count;
    int writing;
    int waiting_writers;
} FairRWLock;

void fair_rwlock_init(FairRWLock* lock) {
    pthread_mutex_init(&lock->mutex, NULL);
    pthread_cond_init(&lock->readers_cond, NULL);
    pthread_cond_init(&lock->writers_cond, NULL);
    lock->readers_count = 0;
    lock->writers_count = 0;
    lock->writing = 0;
    lock->waiting_writers = 0;
}

void fair_rwlock_rdlock(FairRWLock* lock) {
    pthread_mutex_lock(&lock->mutex);
    
    // Writer가 대기 중이면 Reader도 대기
    while (lock->writing || lock->waiting_writers > 0) {
        pthread_cond_wait(&lock->readers_cond, &lock->mutex);
    }
    
    lock->readers_count++;
    
    pthread_mutex_unlock(&lock->mutex);
}

void fair_rwlock_wrlock(FairRWLock* lock) {
    pthread_mutex_lock(&lock->mutex);
    
    lock->waiting_writers++;
    
    while (lock->writing || lock->readers_count > 0) {
        pthread_cond_wait(&lock->writers_cond, &lock->mutex);
    }
    
    lock->waiting_writers--;
    lock->writing = 1;
    
    pthread_mutex_unlock(&lock->mutex);
}

void fair_rwlock_rdunlock(FairRWLock* lock) {
    pthread_mutex_lock(&lock->mutex);
    
    lock->readers_count--;
    
    if (lock->readers_count == 0 && lock->waiting_writers > 0) {
        pthread_cond_signal(&lock->writers_cond);
    }
    
    pthread_mutex_unlock(&lock->mutex);
}

void fair_rwlock_wrunlock(FairRWLock* lock) {
    pthread_mutex_lock(&lock->mutex);
    
    lock->writing = 0;
    
    if (lock->waiting_writers > 0) {
        pthread_cond_signal(&lock->writers_cond);
    } else {
        pthread_cond_broadcast(&lock->readers_cond);
    }
    
    pthread_mutex_unlock(&lock->mutex);
}
```

### 2.5 LogCaster에서 Reader-Writer Lock 활용

통계는 자주 읽지만 가끔 업데이트되므로 Reader-Writer Lock이 적합합니다.

```c
// 로그 통계 정보
typedef struct LogStatistics {
    int total_logs;
    int error_count;
    int warning_count;
    int info_count;
    time_t first_log_time;
    time_t last_log_time;
    char most_active_client[16];
    int most_active_count;
} LogStatistics;

LogStatistics global_stats = {0};
pthread_rwlock_t stats_lock = PTHREAD_RWLOCK_INITIALIZER;

// 통계 업데이트 (Writer)
void update_log_statistics(const char* message, const char* client_ip) {
    pthread_rwlock_wrlock(&stats_lock);
    
    global_stats.total_logs++;
    global_stats.last_log_time = time(NULL);
    
    if (global_stats.first_log_time == 0) {
        global_stats.first_log_time = global_stats.last_log_time;
    }
    
    // 로그 레벨 분석
    if (strstr(message, "ERROR") != NULL) {
        global_stats.error_count++;
    } else if (strstr(message, "WARNING") != NULL) {
        global_stats.warning_count++;
    } else if (strstr(message, "INFO") != NULL) {
        global_stats.info_count++;
    }
    
    // 가장 활발한 클라이언트 업데이트 (단순화된 버전)
    static char last_client[16] = "";
    static int current_count = 0;
    
    if (strcmp(last_client, client_ip) == 0) {
        current_count++;
    } else {
        strcpy(last_client, client_ip);
        current_count = 1;
    }
    
    if (current_count > global_stats.most_active_count) {
        strcpy(global_stats.most_active_client, client_ip);
        global_stats.most_active_count = current_count;
    }
    
    pthread_rwlock_unlock(&stats_lock);
}

// 통계 조회 (Reader)
void get_log_statistics(LogStatistics* stats) {
    pthread_rwlock_rdlock(&stats_lock);
    
    memcpy(stats, &global_stats, sizeof(LogStatistics));
    
    pthread_rwlock_unlock(&stats_lock);
}

// 통계 출력 (Reader)
void print_log_statistics() {
    LogStatistics stats;
    get_log_statistics(&stats);
    
    printf("=== Log Statistics ===\n");
    printf("Total logs: %d\n", stats.total_logs);
    printf("Errors: %d, Warnings: %d, Info: %d\n", 
           stats.error_count, stats.warning_count, stats.info_count);
    printf("First log: %s", ctime(&stats.first_log_time));
    printf("Last log: %s", ctime(&stats.last_log_time));
    printf("Most active client: %s (%d logs)\n", 
           stats.most_active_client, stats.most_active_count);
}

// 여러 Reader가 동시에 통계를 읽을 수 있음
void* statistics_monitor(void* arg) {
    while (1) {
        print_log_statistics();
        sleep(5);  // 5초마다 통계 출력
    }
    return NULL;
}
```

---

## ⚡ 3. 고성능 Lock-Free 프로그래밍 - 신호등 없는 로터리

### 3.1 Lock-Free의 개념

```
일반 교차로 (Lock):           로터리 (Lock-Free):
🚦 신호등 필요                🔄 신호등 없음
⏱️ 대기 시간 발생             🚗 계속 진행
🚶 한 방향씩 진행             🔄 동시 진행 가능
```

### 3.2 Atomic Operations

Lock을 사용하지 않고 원자적 연산으로 동기화하는 방법입니다.

#### 원자적 연산의 종류

```c
// 1. Load (읽기)
int value = atomic_load(&atomic_var);

// 2. Store (쓰기)  
atomic_store(&atomic_var, 42);

// 3. Exchange (교환)
int old = atomic_exchange(&atomic_var, new_value);

// 4. Compare-And-Swap (CAS)
int expected = 10;
int desired = 20;
// atomic_var이 expected와 같으면 desired로 변경
atomic_compare_exchange(&atomic_var, &expected, desired);

// 5. Fetch-And-Add (가져오고 더하기)
int old = atomic_fetch_add(&atomic_var, 5);
```

```c
#include <stdatomic.h>

// 원자적 카운터
typedef struct AtomicCounter {
    atomic_int value;
} AtomicCounter;

void atomic_counter_init(AtomicCounter* counter, int initial_value) {
    atomic_store(&counter->value, initial_value);
}

int atomic_counter_increment(AtomicCounter* counter) {
    return atomic_fetch_add(&counter->value, 1) + 1;
}

int atomic_counter_decrement(AtomicCounter* counter) {
    return atomic_fetch_sub(&counter->value, 1) - 1;
}

int atomic_counter_get(AtomicCounter* counter) {
    return atomic_load(&counter->value);
}

// Compare-And-Swap를 이용한 Lock-Free 스택
typedef struct LockFreeNode {
    void* data;
    struct LockFreeNode* next;
} LockFreeNode;

typedef struct LockFreeStack {
    atomic_uintptr_t head;  // LockFreeNode* 포인터를 원자적으로 저장
} LockFreeStack;

void lockfree_stack_init(LockFreeStack* stack) {
    atomic_store(&stack->head, (uintptr_t)NULL);
}

void lockfree_stack_push(LockFreeStack* stack, void* data) {
    LockFreeNode* new_node = malloc(sizeof(LockFreeNode));
    new_node->data = data;
    
    uintptr_t old_head;
    do {
        old_head = atomic_load(&stack->head);
        new_node->next = (LockFreeNode*)old_head;
    } while (!atomic_compare_exchange_weak(&stack->head, &old_head, (uintptr_t)new_node));
}

void* lockfree_stack_pop(LockFreeStack* stack) {
    uintptr_t old_head;
    LockFreeNode* new_head;
    
    do {
        old_head = atomic_load(&stack->head);
        if (old_head == (uintptr_t)NULL) {
            return NULL;  // 스택이 비어있음
        }
        new_head = ((LockFreeNode*)old_head)->next;
    } while (!atomic_compare_exchange_weak(&stack->head, &old_head, (uintptr_t)new_head));
    
    void* data = ((LockFreeNode*)old_head)->data;
    free((LockFreeNode*)old_head);
    return data;
}
```

### 3.3 LogCaster용 고성능 카운터

Lock 없이 통계를 수집하여 성능을 극대화합니다.

```c
// 성능 측정을 위한 원자적 카운터들
typedef struct PerformanceCounters {
    atomic_long total_connections;
    atomic_long active_connections;
    atomic_long total_messages;
    atomic_long bytes_received;
    atomic_long bytes_sent;
    atomic_long errors;
} PerformanceCounters;

PerformanceCounters perf_counters = {0};

// 연결 시작
void perf_connection_start() {
    atomic_fetch_add(&perf_counters.total_connections, 1);
    atomic_fetch_add(&perf_counters.active_connections, 1);
}

// 연결 종료
void perf_connection_end() {
    atomic_fetch_sub(&perf_counters.active_connections, 1);
}

// 메시지 수신
void perf_message_received(size_t bytes) {
    atomic_fetch_add(&perf_counters.total_messages, 1);
    atomic_fetch_add(&perf_counters.bytes_received, bytes);
}

// 응답 전송
void perf_response_sent(size_t bytes) {
    atomic_fetch_add(&perf_counters.bytes_sent, bytes);
}

// 에러 발생
void perf_error_occurred() {
    atomic_fetch_add(&perf_counters.errors, 1);
}

// 성능 통계 출력
void print_performance_stats() {
    printf("=== Performance Statistics ===\n");
    printf("Total connections: %ld\n", atomic_load(&perf_counters.total_connections));
    printf("Active connections: %ld\n", atomic_load(&perf_counters.active_connections));
    printf("Total messages: %ld\n", atomic_load(&perf_counters.total_messages));
    printf("Bytes received: %ld\n", atomic_load(&perf_counters.bytes_received));
    printf("Bytes sent: %ld\n", atomic_load(&perf_counters.bytes_sent));
    printf("Errors: %ld\n", atomic_load(&perf_counters.errors));
}

// 클라이언트 처리 함수에 성능 측정 추가
void* handle_client_with_perf(void* arg) {
    ClientInfo* client_info = (ClientInfo*)arg;
    int client_fd = client_info->client_fd;
    char buffer[256];
    
    perf_connection_start();
    
    while (1) {
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received < 0) {
                perf_error_occurred();
            }
            break;
        }
        
        buffer[bytes_received] = '\0';
        perf_message_received(bytes_received);
        
        // 로그 처리...
        
        const char* ack = "ACK\n";
        ssize_t bytes_sent = send(client_fd, ack, strlen(ack), 0);
        
        if (bytes_sent > 0) {
            perf_response_sent(bytes_sent);
        } else {
            perf_error_occurred();
        }
    }
    
    perf_connection_end();
    close(client_fd);
    free(client_info);
    
    return NULL;
}
```

---

## 🏭 4. 스레드 풀 패턴 고도화 - 스마트한 일꾼 관리

### 4.1 스레드 풀의 진화

```
기본 스레드 풀:              동적 스레드 풀:
👷👷👷 (고정)                👷👷👷 ~ 👷👷👷👷👷 (가변)
일이 없어도 대기             일이 없으면 일부 퇴근
일이 많아도 그대로           일이 많으면 추가 고용
```

### 4.2 동적 크기 조절 스레드 풀

```c
typedef struct DynamicThreadPool {
    pthread_t* threads;
    int core_threads;        // 기본 스레드 수
    int max_threads;         // 최대 스레드 수
    int current_threads;     // 현재 스레드 수
    int active_threads;      // 작업 중인 스레드 수
    
    Task* task_queue_head;
    Task* task_queue_tail;
    int task_count;
    int max_queue_size;      // 최대 큐 크기
    
    pthread_mutex_t pool_mutex;
    pthread_cond_t task_available;
    pthread_cond_t task_completed;
    
    bool shutdown;
    time_t last_scale_time;  // 마지막 스케일링 시간
} DynamicThreadPool;

// 워커 스레드 함수 (동적 종료 지원)
void* dynamic_worker_thread(void* arg) {
    DynamicThreadPool* pool = (DynamicThreadPool*)arg;
    time_t idle_start = 0;
    const int MAX_IDLE_TIME = 60;  // 60초 유휴 시간 후 종료
    
    while (1) {
        pthread_mutex_lock(&pool->pool_mutex);
        
        // 작업 대기
        while (pool->task_count == 0 && !pool->shutdown) {
            struct timespec timeout;
            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_sec += 5;  // 5초 타임아웃
            
            int wait_result = pthread_cond_timedwait(&pool->task_available, 
                                                   &pool->pool_mutex, &timeout);
            
            if (wait_result == ETIMEDOUT) {
                // 타임아웃 발생
                if (idle_start == 0) {
                    idle_start = time(NULL);
                } else if (time(NULL) - idle_start > MAX_IDLE_TIME && 
                          pool->current_threads > pool->core_threads) {
                    // 코어 스레드보다 많고 오랜 시간 유휴 상태면 종료
                    pool->current_threads--;
                    pthread_mutex_unlock(&pool->pool_mutex);
                    return NULL;
                }
            }
        }
        
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->pool_mutex);
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
            pool->active_threads++;
            idle_start = 0;  // 유휴 시간 리셋
        }
        
        pthread_mutex_unlock(&pool->pool_mutex);
        
        // 작업 수행
        if (task != NULL) {
            task->function(task->arg);
            free(task);
            
            pthread_mutex_lock(&pool->pool_mutex);
            pool->active_threads--;
            pthread_cond_signal(&pool->task_completed);
            pthread_mutex_unlock(&pool->pool_mutex);
        }
    }
    
    pthread_mutex_lock(&pool->pool_mutex);
    pool->current_threads--;
    pthread_mutex_unlock(&pool->pool_mutex);
    
    return NULL;
}

// 스레드 풀 자동 스케일링
void scale_thread_pool(DynamicThreadPool* pool) {
    time_t current_time = time(NULL);
    
    // 최소 10초 간격으로 스케일링
    if (current_time - pool->last_scale_time < 10) {
        return;
    }
    
    pthread_mutex_lock(&pool->pool_mutex);
    
    // 작업 큐가 많이 쌓이고 모든 스레드가 바쁘면 스레드 추가
    if (pool->task_count > pool->current_threads * 2 && 
        pool->active_threads == pool->current_threads &&
        pool->current_threads < pool->max_threads) {
        
        // 새 스레드 추가
        pool->threads = realloc(pool->threads, 
                               sizeof(pthread_t) * (pool->current_threads + 1));
        
        if (pthread_create(&pool->threads[pool->current_threads], NULL, 
                          dynamic_worker_thread, pool) == 0) {
            pool->current_threads++;
            printf("Scaled up: %d threads\n", pool->current_threads);
        }
    }
    
    pool->last_scale_time = current_time;
    
    pthread_mutex_unlock(&pool->pool_mutex);
}

// 작업 추가 (큐 크기 제한)
int add_task_with_limit(DynamicThreadPool* pool, void (*function)(void*), void* arg) {
    pthread_mutex_lock(&pool->pool_mutex);
    
    // 큐가 가득 찬 경우 거부
    if (pool->task_count >= pool->max_queue_size) {
        pthread_mutex_unlock(&pool->pool_mutex);
        return -1;  // 큐 가득 찬 오류
    }
    
    Task* new_task = malloc(sizeof(Task));
    if (new_task == NULL) {
        pthread_mutex_unlock(&pool->pool_mutex);
        return -2;  // 메모리 할당 오류
    }
    
    new_task->function = function;
    new_task->arg = arg;
    new_task->next = NULL;
    
    if (pool->task_queue_tail == NULL) {
        pool->task_queue_head = new_task;
        pool->task_queue_tail = new_task;
    } else {
        pool->task_queue_tail->next = new_task;
        pool->task_queue_tail = new_task;
    }
    
    pool->task_count++;
    
    pthread_cond_signal(&pool->task_available);
    
    pthread_mutex_unlock(&pool->pool_mutex);
    
    // 자동 스케일링 검사
    scale_thread_pool(pool);
    
    return 0;
}

### 4.3 우선순위 기반 스레드 풀

중요한 작업을 먼저 처리하는 스마트한 스레드 풀입니다.

```
우선순위 큐:
[긴급:10] → [중요:7] → [일반:5] → [낮음:2]
    ↓
  먼저 처리
```

// 우선순위 작업 큐
typedef struct PriorityTask {
    void (*function)(void* arg);
    void* arg;
    int priority;  // 높을수록 우선순위 높음
    struct PriorityTask* next;
} PriorityTask;

typedef struct PriorityThreadPool {
    pthread_t* threads;
    int thread_count;
    
    PriorityTask* task_queue_head;
    int task_count;
    
    pthread_mutex_t pool_mutex;
    pthread_cond_t task_available;
    
    bool shutdown;
} PriorityThreadPool;

// 우선순위에 따른 작업 삽입
int add_priority_task(PriorityThreadPool* pool, void (*function)(void*), 
                     void* arg, int priority) {
    PriorityTask* new_task = malloc(sizeof(PriorityTask));
    if (new_task == NULL) return -1;
    
    new_task->function = function;
    new_task->arg = arg;
    new_task->priority = priority;
    
    pthread_mutex_lock(&pool->pool_mutex);
    
    // 우선순위에 따라 삽입 위치 결정
    if (pool->task_queue_head == NULL || 
        pool->task_queue_head->priority < priority) {
        // 맨 앞에 삽입
        new_task->next = pool->task_queue_head;
        pool->task_queue_head = new_task;
    } else {
        // 적절한 위치에 삽입
        PriorityTask* current = pool->task_queue_head;
        while (current->next != NULL && current->next->priority >= priority) {
            current = current->next;
        }
        new_task->next = current->next;
        current->next = new_task;
    }
    
    pool->task_count++;
    
    pthread_cond_signal(&pool->task_available);
    
    pthread_mutex_unlock(&pool->pool_mutex);
    
    return 0;
}
```

---

## 📊 5. 동기화 성능 측정과 최적화

### 5.1 Lock Contention 측정

```c
// Lock 경합 측정 도구
typedef struct LockStats {
    pthread_mutex_t* mutex;
    atomic_long wait_count;      // 대기 횟수
    atomic_long wait_time_ns;    // 총 대기 시간
    atomic_long hold_time_ns;    // 총 보유 시간
    atomic_long acquisition_count; // 획득 횟수
} LockStats;

void lock_with_stats(LockStats* stats) {
    struct timespec start, acquired, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Lock 시도
    pthread_mutex_lock(stats->mutex);
    
    clock_gettime(CLOCK_MONOTONIC, &acquired);
    
    // 대기 시간 계산
    long wait_ns = (acquired.tv_sec - start.tv_sec) * 1000000000L +
                   (acquired.tv_nsec - start.tv_nsec);
    
    atomic_fetch_add(&stats->wait_count, 1);
    atomic_fetch_add(&stats->wait_time_ns, wait_ns);
    atomic_fetch_add(&stats->acquisition_count, 1);
}

void unlock_with_stats(LockStats* stats) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    
    // 보유 시간은 lock_with_stats에서 계산
    pthread_mutex_unlock(stats->mutex);
}

void print_lock_stats(LockStats* stats, const char* name) {
    long acquisitions = atomic_load(&stats->acquisition_count);
    if (acquisitions == 0) return;
    
    long total_wait = atomic_load(&stats->wait_time_ns);
    long avg_wait = total_wait / acquisitions;
    
    printf("=== Lock Statistics: %s ===\n", name);
    printf("Acquisitions: %ld\n", acquisitions);
    printf("Average wait time: %.2f μs\n", avg_wait / 1000.0);
    printf("Total wait time: %.2f ms\n", total_wait / 1000000.0);
    
    // 경합도 계산 (대기 시간 비율)
    double contention = (double)atomic_load(&stats->wait_count) / acquisitions;
    printf("Contention rate: %.2f%%\n", contention * 100);
}
```

### 5.2 동기화 최적화 전략

#### 1. Lock 세분화 (Fine-Grained Locking)

```c
// 나쁜 예: 하나의 큰 Lock
typedef struct {
    pthread_mutex_t mutex;
    LogList logs;
    Statistics stats;
    Configuration config;
} BadServer;

// 좋은 예: 여러 개의 작은 Lock
typedef struct {
    pthread_mutex_t log_mutex;    // 로그 전용
    pthread_mutex_t stats_mutex;  // 통계 전용
    pthread_mutex_t config_mutex; // 설정 전용
    LogList logs;
    Statistics stats;
    Configuration config;
} GoodServer;
```

#### 2. Lock 회피 기법

```c
// Thread-Local Storage 활용
__thread LogBuffer local_buffer = {0};
__thread int buffer_count = 0;

void log_message_optimized(const char* message) {
    // 로컬 버퍼에 먼저 저장
    if (buffer_count < LOCAL_BUFFER_SIZE) {
        strcpy(local_buffer[buffer_count++], message);
        return;
    }
    
    // 버퍼가 가득 차면 한 번에 flush
    pthread_mutex_lock(&global_log_mutex);
    for (int i = 0; i < buffer_count; i++) {
        add_to_global_log(local_buffer[i]);
    }
    pthread_mutex_unlock(&global_log_mutex);
    
    buffer_count = 0;
}
```

## 🐛 6. 일반적인 동기화 버그와 해결책

### 6.1 데드락 체크리스트

```
✅ 항상 같은 순서로 Lock 획득
✅ Lock을 잡은 채로 blocking 함수 호출 금지
✅ 가능하면 try_lock 사용
✅ Lock 보유 시간 최소화
✅ 순환 대기 조건 제거
```

### 6.2 Race Condition 방지

```c
// 위험한 코드
if (shared_ptr != NULL) {     // 체크
    use_resource(shared_ptr);  // 사용 - 이 사이에 NULL이 될 수 있음!
}

// 안전한 코드
pthread_mutex_lock(&mutex);
if (shared_ptr != NULL) {
    use_resource(shared_ptr);
}
pthread_mutex_unlock(&mutex);
```

## ✅ 7. 다음 단계 준비

이 문서를 완전히 이해했다면:

1. **다양한 Mutex 타입**의 차이점과 용도를 알아야 합니다
2. **Reader-Writer Lock**의 동작 원리와 공정성 문제를 이해해야 합니다
3. **Atomic Operations**을 활용한 Lock-Free 프로그래밍을 할 수 있어야 합니다
4. **Thread-Safe 데이터 구조**를 설계하고 구현할 수 있어야 합니다
5. **고급 스레드 풀 패턴**을 구현할 수 있어야 합니다

### 🎯 확인 문제

1. **Reader-Writer Lock에서 Reader 선호 방식의 문제점은 무엇이며, 어떻게 해결할 수 있을까요?**
   - 힌트: Writer가 계속 기다리는 상황을 생각해보세요

2. **Lock-Free 프로그래밍에서 ABA 문제란 무엇이고, 어떻게 방지할 수 있을까요?**
   - 힌트: A→B→A로 값이 변해도 변화를 감지 못하는 문제

3. **LogCaster에서 로그 통계 정보에 Reader-Writer Lock을 사용하는 이유는 무엇인가요?**
   - 힌트: 읽기와 쓰기의 빈도 차이를 생각해보세요

### 💻 실습 프로젝트

1. **Lock 성능 비교기**: 다양한 Lock의 성능을 측정하고 비교
2. **Thread-Safe 캐시**: LRU 캐시를 thread-safe하게 구현
3. **Lock-Free 큐**: Compare-And-Swap을 이용한 큐 구현
4. **동적 스레드 풀**: 부하에 따라 크기가 조절되는 스레드 풀

다음 문서에서는 **데이터 구조**에 대해 자세히 알아보겠습니다!

---

*💡 팁: 동기화는 성능과 안전성의 균형입니다. 항상 필요한 만큼만 동기화하세요!*

## ⚠️ 자주 하는 실수와 해결법

### 1. Lock 순서 실수로 인한 Deadlock
```c
// ❌ 잘못된 예시 - 순서가 다름
void* thread1(void* arg) {
    pthread_mutex_lock(&mutex_A);
    pthread_mutex_lock(&mutex_B);  // A → B
    // ...
}

void* thread2(void* arg) {
    pthread_mutex_lock(&mutex_B);
    pthread_mutex_lock(&mutex_A);  // B → A (Deadlock!)
    // ...
}

// ✅ 올바른 예시 - 일관된 순서
// 항상 낮은 주소의 mutex를 먼저 잠금
void safe_double_lock(pthread_mutex_t* m1, pthread_mutex_t* m2) {
    if (m1 < m2) {
        pthread_mutex_lock(m1);
        pthread_mutex_lock(m2);
    } else {
        pthread_mutex_lock(m2);
        pthread_mutex_lock(m1);
    }
}
```

### 2. 조건 변수 Signal 놓침
```c
// ❌ 잘못된 예시 - signal을 놓칠 수 있음
void producer() {
    item = produce_item();
    pthread_cond_signal(&cond);  // mutex 없이 signal!
}

void consumer() {
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);  // signal을 놓침!
    // ...
}

// ✅ 올바른 예시
void producer() {
    pthread_mutex_lock(&mutex);
    item = produce_item();
    item_ready = true;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void consumer() {
    pthread_mutex_lock(&mutex);
    while (!item_ready) {  // while 루프 사용!
        pthread_cond_wait(&cond, &mutex);
    }
    // ...
}
```

### 3. Reader-Writer Lock에서 Writer 기아
```c
// ❌ 문제가 있는 구현 - Writer 기아 가능
void reader() {
    pthread_rwlock_rdlock(&rwlock);
    // 긴 작업...
    sleep(10);  // Reader가 너무 오래 잡고 있음
    pthread_rwlock_unlock(&rwlock);
}

// ✅ 개선된 구현
void reader_improved() {
    pthread_rwlock_rdlock(&rwlock);
    // 데이터 복사
    data_copy = copy_data();
    pthread_rwlock_unlock(&rwlock);  // 즉시 해제
    
    // 긴 작업은 lock 밖에서
    process_data(data_copy);
}
```

### 4. Atomic 연산 메모리 순서 실수
```c
// ❌ 잘못된 예시 - 잘못된 메모리 순서
atomic_store(&ready, 1);  // relaxed 순서 (기본값)
atomic_store(&data, 42);  // data가 ready보다 먼저 보일 수 있음!

// ✅ 올바른 예시
atomic_store_explicit(&data, 42, memory_order_release);
atomic_store_explicit(&ready, 1, memory_order_release);
```

### 5. Lock 해제 누락
```c
// ❌ 잘못된 예시
void process() {
    pthread_mutex_lock(&mutex);
    if (error_condition) {
        return;  // unlock 누락!
    }
    pthread_mutex_unlock(&mutex);
}

// ✅ 올바른 예시 - RAII 패턴 사용
typedef struct {
    pthread_mutex_t* mutex;
} auto_lock_t;

void auto_lock_init(auto_lock_t* lock, pthread_mutex_t* mutex) {
    lock->mutex = mutex;
    pthread_mutex_lock(mutex);
}

void auto_lock_cleanup(auto_lock_t* lock) {
    pthread_mutex_unlock(lock->mutex);
}

void process_safe() {
    auto_lock_t lock;
    auto_lock_init(&lock, &mutex);
    pthread_cleanup_push((void*)auto_lock_cleanup, &lock);
    
    if (error_condition) {
        pthread_cleanup_pop(1);  // unlock 자동 실행
        return;
    }
    
    pthread_cleanup_pop(1);
}
```

## 🎯 실습 프로젝트

### 프로젝트 1: Multi-Level Lock Manager (난이도: ⭐⭐⭐)
```c
// 구현할 기능:
// 1. 계층적 락 관리 (데이터베이스처럼)
// 2. 데드락 감지 알고리즘
// 3. 락 타임아웃 처리
// 4. 락 통계 수집

typedef struct {
    pthread_mutex_t* locks[MAX_LOCKS];
    int lock_order[MAX_LOCKS];
    int lock_count;
    // 데드락 감지를 위한 wait-for 그래프
} LockManager;

// 구현 목표:
// - 여러 개의 락을 안전하게 관리
// - 데드락 자동 감지 및 해결
// - 성능 모니터링
```

### 프로젝트 2: Lock-Free 메시지 큐 (난이도: ⭐⭐⭐⭐)
```c
// 구현할 기능:
// 1. CAS 기반 lock-free 큐
// 2. ABA 문제 해결
// 3. 메모리 순서 보장
// 4. 성능 벤치마킹

// 도전 과제:
// - Hazard Pointer 구현
// - 백프레셔 메커니즘
// - Wait-free 보장
```

### 프로젝트 3: 적응형 동기화 시스템 (난이도: ⭐⭐⭐⭐⭐)
```c
// 구현할 기능:
// 1. 경합도에 따른 동적 전략 변경
// 2. Spin → Yield → Sleep 전환
// 3. Reader-Writer 비율 분석
// 4. 자동 최적화

// 고급 기능:
// - 머신러닝 기반 예측
// - NUMA 인식 최적화
// - 실시간 성능 조정
```

## ✅ 학습 체크리스트

### 🟢 기초 (1-2주)
- [ ] Mutex의 기본 사용법
- [ ] 조건 변수 이해
- [ ] Reader-Writer Lock 사용
- [ ] 간단한 생산자-소비자 패턴
- [ ] 데드락 개념 이해

### 🟡 중급 (3-4주)
- [ ] 다양한 Mutex 타입 활용
- [ ] 공정한 Reader-Writer Lock 구현
- [ ] Thread-safe 자료구조 설계
- [ ] Lock 경합 측정과 분석
- [ ] 타임아웃 처리
- [ ] Lock 세분화 기법

### 🔴 고급 (5-8주)
- [ ] Atomic 연산과 메모리 순서
- [ ] Lock-free 자료구조 구현
- [ ] ABA 문제 해결
- [ ] Wait-free 알고리즘
- [ ] 성능 최적화 기법
- [ ] 동적 스레드 풀 구현

### ⚫ 전문가 (3개월+)
- [ ] 커스텀 동기화 프리미티브
- [ ] Transactional Memory
- [ ] RCU (Read-Copy-Update)
- [ ] Hazard Pointer 구현
- [ ] 분산 락 알고리즘
- [ ] 형식 검증과 모델 체킹

## 📚 추가 학습 자료

### 권장 도서
1. **"The Art of Multiprocessor Programming"** - 고급 동기화 기법
2. **"C++ Concurrency in Action"** - 현대적인 동시성 프로그래밍
3. **"Java Concurrency in Practice"** - 동시성 패턴과 안티패턴

### 온라인 자료
- [Lock-Free Programming Tutorial](https://www.1024cores.net/)
- [Concurrency Kit Library](https://github.com/concurrencykit/ck)
- [Intel Threading Building Blocks](https://www.threadingbuildingblocks.org/)

### 도구와 라이브러리
- **ThreadSanitizer**: Race condition 감지
- **Helgrind**: Deadlock 감지 (Valgrind)
- **Intel VTune**: 성능 프로파일링
- **Concurrency Visualizer**: 동시성 시각화

---

*이 문서는 LogCaster 프로젝트를 위한 동기화 메커니즘 완벽 가이드입니다. 다음 문서에서는 효율적인 데이터 구조에 대해 알아보겠습니다!*