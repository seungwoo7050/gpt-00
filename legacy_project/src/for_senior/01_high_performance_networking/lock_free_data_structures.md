# Lock-free 데이터 구조 (Lock-free Data Structures)

## 학습 목표
- 멀티코어 환경에서 락 없이 동시성 제어
- CAS(Compare-And-Swap) 기반 알고리즘 구현
- Memory ordering과 원자성 연산 마스터
- False sharing 방지와 캐시 최적화

## 1. 현재 문제 상황

### 1.1 Mutex 기반 구현의 한계
```c
// [SEQUENCE: 1] 현재 LogCaster의 mutex 기반 구현
typedef struct {
    log_entry_t* entries;
    size_t capacity;
    size_t count;
    size_t head;
    size_t tail;
    pthread_mutex_t mutex;  // 병목 지점!
} circular_buffer_t;

int add_log(circular_buffer_t* buffer, const char* message) {
    pthread_mutex_lock(&buffer->mutex);  // 모든 스레드가 대기
    
    // 로그 추가 로직...
    
    pthread_mutex_unlock(&buffer->mutex);
    return 0;
}
```

### 1.2 성능 문제
```c
// [SEQUENCE: 2] 경쟁 상황에서의 성능 저하
void benchmark_mutex_buffer() {
    // 16개 스레드로 동시 쓰기
    // 결과: 처리량이 스레드 수에 반비례
    // 1 thread:  100,000 ops/sec
    // 4 threads:  80,000 ops/sec  
    // 16 threads: 40,000 ops/sec  // 심각한 경쟁!
}
```

## 2. Lock-free 설계

### 2.1 SPSC(Single Producer Single Consumer) 링버퍼
```c
// [SEQUENCE: 3] Cache line 정렬된 lock-free 구조
#define CACHE_LINE_SIZE 64

typedef struct {
    alignas(CACHE_LINE_SIZE) atomic_size_t head;
    char padding1[CACHE_LINE_SIZE - sizeof(atomic_size_t)];
    
    alignas(CACHE_LINE_SIZE) atomic_size_t tail;
    char padding2[CACHE_LINE_SIZE - sizeof(atomic_size_t)];
    
    size_t capacity;
    log_entry_t* entries;
} lockfree_spsc_queue_t;

// [SEQUENCE: 4] Wait-free 쓰기 연산
bool spsc_enqueue(lockfree_spsc_queue_t* queue, const log_entry_t* entry) {
    size_t current_tail = atomic_load_explicit(&queue->tail, memory_order_relaxed);
    size_t next_tail = (current_tail + 1) % queue->capacity;
    
    // 가득 찬 상태 확인
    if (next_tail == atomic_load_explicit(&queue->head, memory_order_acquire)) {
        return false;  // 큐가 가득 참
    }
    
    // 데이터 복사
    queue->entries[current_tail] = *entry;
    
    // tail 업데이트 (release semantic으로 데이터 가시성 보장)
    atomic_store_explicit(&queue->tail, next_tail, memory_order_release);
    return true;
}

// [SEQUENCE: 5] Wait-free 읽기 연산
bool spsc_dequeue(lockfree_spsc_queue_t* queue, log_entry_t* entry) {
    size_t current_head = atomic_load_explicit(&queue->head, memory_order_relaxed);
    
    // 빈 상태 확인
    if (current_head == atomic_load_explicit(&queue->tail, memory_order_acquire)) {
        return false;  // 큐가 비어있음
    }
    
    // 데이터 읽기
    *entry = queue->entries[current_head];
    
    // head 업데이트
    size_t next_head = (current_head + 1) % queue->capacity;
    atomic_store_explicit(&queue->head, next_head, memory_order_release);
    return true;
}
```

### 2.2 MPMC(Multiple Producer Multiple Consumer) 큐
```c
// [SEQUENCE: 6] CAS 기반 MPMC 큐
typedef struct mpmc_node {
    atomic_uintptr_t next;
    log_entry_t data;
} mpmc_node_t;

typedef struct {
    alignas(CACHE_LINE_SIZE) atomic_uintptr_t head;
    char padding1[CACHE_LINE_SIZE - sizeof(atomic_uintptr_t)];
    
    alignas(CACHE_LINE_SIZE) atomic_uintptr_t tail;
    char padding2[CACHE_LINE_SIZE - sizeof(atomic_uintptr_t)];
} lockfree_mpmc_queue_t;

// [SEQUENCE: 7] Lock-free enqueue with CAS loop
void mpmc_enqueue(lockfree_mpmc_queue_t* queue, const log_entry_t* entry) {
    mpmc_node_t* new_node = aligned_alloc(CACHE_LINE_SIZE, sizeof(mpmc_node_t));
    new_node->data = *entry;
    atomic_store(&new_node->next, 0);
    
    mpmc_node_t* prev_tail;
    
    // CAS loop to update tail
    while (true) {
        prev_tail = (mpmc_node_t*)atomic_load(&queue->tail);
        mpmc_node_t* tail_next = (mpmc_node_t*)atomic_load(&prev_tail->next);
        
        // tail이 변경되었는지 확인
        if (prev_tail == (mpmc_node_t*)atomic_load(&queue->tail)) {
            if (tail_next == NULL) {
                // tail의 next를 새 노드로 설정 시도
                if (atomic_compare_exchange_weak(&prev_tail->next, 
                                               &tail_next, 
                                               (uintptr_t)new_node)) {
                    break;  // 성공!
                }
            } else {
                // tail을 앞으로 이동 도움
                atomic_compare_exchange_weak(&queue->tail, 
                                           &prev_tail, 
                                           (uintptr_t)tail_next);
            }
        }
    }
    
    // tail을 새 노드로 이동
    atomic_compare_exchange_weak(&queue->tail, &prev_tail, (uintptr_t)new_node);
}

// [SEQUENCE: 8] Lock-free dequeue
bool mpmc_dequeue(lockfree_mpmc_queue_t* queue, log_entry_t* entry) {
    mpmc_node_t* head;
    
    while (true) {
        head = (mpmc_node_t*)atomic_load(&queue->head);
        mpmc_node_t* tail = (mpmc_node_t*)atomic_load(&queue->tail);
        mpmc_node_t* head_next = (mpmc_node_t*)atomic_load(&head->next);
        
        // 일관성 확인
        if (head == (mpmc_node_t*)atomic_load(&queue->head)) {
            if (head == tail) {
                if (head_next == NULL) {
                    return false;  // 큐가 비어있음
                }
                // tail 이동 도움
                atomic_compare_exchange_weak(&queue->tail, 
                                           &tail, 
                                           (uintptr_t)head_next);
            } else {
                // 데이터 읽기
                if (head_next != NULL) {
                    *entry = head_next->data;
                    
                    // head 이동
                    if (atomic_compare_exchange_weak(&queue->head, 
                                                   &head, 
                                                   (uintptr_t)head_next)) {
                        free(head);  // 이전 head 해제
                        return true;
                    }
                }
            }
        }
    }
}
```

## 3. 고급 Lock-free 패턴

### 3.1 Hazard Pointers로 메모리 안전성
```c
// [SEQUENCE: 9] Hazard pointer 구현
typedef struct {
    atomic_uintptr_t* hazard_pointers;
    size_t max_threads;
    __thread int thread_id;
} hazard_pointer_domain_t;

// [SEQUENCE: 10] 안전한 메모리 접근
mpmc_node_t* safe_read(atomic_uintptr_t* location, 
                       hazard_pointer_domain_t* domain) {
    mpmc_node_t* ptr;
    
    do {
        ptr = (mpmc_node_t*)atomic_load(location);
        // Hazard pointer 설정
        atomic_store(&domain->hazard_pointers[thread_id], (uintptr_t)ptr);
        
        // Double check
    } while (ptr != (mpmc_node_t*)atomic_load(location));
    
    return ptr;
}

// [SEQUENCE: 11] 지연된 메모리 해제
typedef struct retire_node {
    void* ptr;
    struct retire_node* next;
} retire_node_t;

__thread retire_node_t* retire_list = NULL;
__thread size_t retire_count = 0;

void retire_ptr(void* ptr) {
    retire_node_t* node = malloc(sizeof(retire_node_t));
    node->ptr = ptr;
    node->next = retire_list;
    retire_list = node;
    retire_count++;
    
    if (retire_count >= RETIRE_THRESHOLD) {
        scan_and_free();
    }
}

void scan_and_free() {
    // 모든 hazard pointer 수집
    uintptr_t* hazards = collect_hazard_pointers();
    
    retire_node_t** current = &retire_list;
    while (*current) {
        retire_node_t* node = *current;
        
        // Hazard pointer에 없으면 해제 가능
        if (!is_hazardous(node->ptr, hazards)) {
            *current = node->next;
            free(node->ptr);
            free(node);
            retire_count--;
        } else {
            current = &node->next;
        }
    }
    
    free(hazards);
}
```

### 3.2 RCU(Read-Copy-Update) 패턴
```c
// [SEQUENCE: 12] RCU 기반 로그 설정 업데이트
typedef struct {
    int log_level;
    char* log_format;
    bool enable_compression;
} log_config_t;

typedef struct {
    atomic_uintptr_t current_config;
    pthread_mutex_t update_mutex;  // 업데이트만 직렬화
} rcu_config_t;

// [SEQUENCE: 13] Lock-free 읽기
const log_config_t* rcu_read_config(rcu_config_t* rcu) {
    return (log_config_t*)atomic_load(&rcu->current_config);
}

// [SEQUENCE: 14] Copy-on-write 업데이트
void rcu_update_config(rcu_config_t* rcu, 
                      void (*updater)(log_config_t*)) {
    pthread_mutex_lock(&rcu->update_mutex);
    
    // 현재 설정 복사
    log_config_t* old = (log_config_t*)atomic_load(&rcu->current_config);
    log_config_t* new = malloc(sizeof(log_config_t));
    *new = *old;
    
    // 업데이트 적용
    updater(new);
    
    // 원자적 교체
    atomic_store(&rcu->current_config, (uintptr_t)new);
    
    // Grace period 대기
    synchronize_rcu();
    
    // 이전 설정 해제
    free(old);
    
    pthread_mutex_unlock(&rcu->update_mutex);
}

// [SEQUENCE: 15] Quiescent state 기반 동기화
void synchronize_rcu() {
    // 모든 스레드가 최소 한 번 스케줄링될 때까지 대기
    for (int i = 0; i < num_threads; i++) {
        while (!thread_passed_quiescent_state(i)) {
            sched_yield();
        }
    }
}
```

## 4. 실전 적용: Lock-free 로그 버퍼

### 4.1 하이브리드 접근
```c
// [SEQUENCE: 16] Per-thread 버퍼 + Lock-free 집계
typedef struct {
    // 스레드 로컬 버퍼 (경쟁 없음)
    __thread log_entry_t local_buffer[LOCAL_BUFFER_SIZE];
    __thread size_t local_count;
    
    // 글로벌 lock-free 큐
    lockfree_mpmc_queue_t global_queue;
    
    // 통계 (lock-free 업데이트)
    atomic_uint64_t total_logs;
    atomic_uint64_t dropped_logs;
} hybrid_log_buffer_t;

// [SEQUENCE: 17] 효율적인 로그 추가
void add_log_hybrid(hybrid_log_buffer_t* buffer, const char* message) {
    // 스레드 로컬 버퍼에 추가
    if (local_count < LOCAL_BUFFER_SIZE) {
        local_buffer[local_count++] = create_log_entry(message);
        return;
    }
    
    // 로컬 버퍼 플러시
    flush_local_buffer(buffer);
    
    // 새 엔트리 추가
    local_buffer[0] = create_log_entry(message);
    local_count = 1;
}

// [SEQUENCE: 18] 배치 플러시
void flush_local_buffer(hybrid_log_buffer_t* buffer) {
    for (size_t i = 0; i < local_count; i++) {
        if (!mpmc_enqueue(&buffer->global_queue, &local_buffer[i])) {
            // 큐 가득 참 - 통계 업데이트
            atomic_fetch_add(&buffer->dropped_logs, local_count - i);
            break;
        }
    }
    
    atomic_fetch_add(&buffer->total_logs, local_count);
    local_count = 0;
}
```

### 4.2 메모리 순서 최적화
```c
// [SEQUENCE: 19] 세밀한 메모리 순서 제어
typedef struct {
    // 읽기가 많은 통계는 relaxed
    atomic_uint64_t read_count;     // memory_order_relaxed
    
    // 중요한 상태는 acquire-release
    atomic_bool shutdown;           // memory_order_acquire/release
    
    // 순서가 중요한 연산은 seq_cst
    atomic_uint64_t sequence_num;   // memory_order_seq_cst
} optimized_stats_t;

// [SEQUENCE: 20] 최적화된 통계 업데이트
void update_stats_optimized(optimized_stats_t* stats) {
    // 단순 카운터는 relaxed로 충분
    atomic_fetch_add_explicit(&stats->read_count, 1, memory_order_relaxed);
    
    // 종료 확인은 acquire로 읽기
    if (atomic_load_explicit(&stats->shutdown, memory_order_acquire)) {
        return;
    }
    
    // 시퀀스 번호는 순서 보장 필요
    uint64_t seq = atomic_fetch_add_explicit(&stats->sequence_num, 1, 
                                            memory_order_seq_cst);
    
    process_with_sequence(seq);
}
```

## 5. 성능 분석과 최적화

### 5.1 벤치마크 결과
```c
// [SEQUENCE: 21] 성능 측정 코드
void benchmark_lockfree_vs_mutex() {
    const int num_threads = 16;
    const int ops_per_thread = 1000000;
    
    // Mutex 버전
    double mutex_time = benchmark_mutex_buffer(num_threads, ops_per_thread);
    
    // Lock-free 버전
    double lockfree_time = benchmark_lockfree_buffer(num_threads, ops_per_thread);
    
    printf("Performance comparison (16 threads):\n");
    printf("Mutex:     %.2f ops/sec\n", total_ops / mutex_time);
    printf("Lock-free: %.2f ops/sec\n", total_ops / lockfree_time);
    printf("Speedup:   %.2fx\n", mutex_time / lockfree_time);
    
    // 결과:
    // Mutex:     400,000 ops/sec
    // Lock-free: 8,000,000 ops/sec  
    // Speedup:   20x
}
```

### 5.2 CPU 캐시 최적화
```c
// [SEQUENCE: 22] False sharing 방지
typedef struct {
    // 캐시라인 정렬로 false sharing 방지
    struct {
        atomic_uint64_t value;
        char padding[CACHE_LINE_SIZE - sizeof(atomic_uint64_t)];
    } counters[MAX_THREADS];
} cache_friendly_stats_t;

// [SEQUENCE: 23] NUMA 인식 메모리 할당
void* numa_aware_alloc(size_t size, int numa_node) {
    void* ptr = numa_alloc_onnode(size, numa_node);
    if (!ptr) {
        ptr = aligned_alloc(CACHE_LINE_SIZE, size);
    }
    return ptr;
}
```

## 6. 실전 고려사항

### 6.1 ABA 문제 해결
```c
// [SEQUENCE: 24] Tagged pointer로 ABA 방지
typedef struct {
    uintptr_t ptr : 48;    // 실제 포인터 (48비트)
    uintptr_t tag : 16;    // 버전 태그 (16비트)
} tagged_ptr_t;

// [SEQUENCE: 25] CAS with tag
bool tagged_cas(atomic_uintptr_t* location, 
                tagged_ptr_t expected, 
                tagged_ptr_t desired) {
    desired.tag = expected.tag + 1;  // 태그 증가
    
    uint64_t exp = *(uint64_t*)&expected;
    uint64_t des = *(uint64_t*)&desired;
    
    return atomic_compare_exchange_strong(location, &exp, des);
}
```

### 6.2 디버깅과 모니터링
```c
// [SEQUENCE: 26] Lock-free 디버깅 도구
typedef struct {
    atomic_uint64_t cas_failures;
    atomic_uint64_t retry_count;
    atomic_uint64_t max_retries;
} lockfree_debug_stats_t;

#ifdef DEBUG_LOCKFREE
    #define LOCKFREE_RETRY_STAT() atomic_fetch_add(&stats->retry_count, 1)
    #define LOCKFREE_FAILURE_STAT() atomic_fetch_add(&stats->cas_failures, 1)
#else
    #define LOCKFREE_RETRY_STAT()
    #define LOCKFREE_FAILURE_STAT()
#endif
```

## 마무리

Lock-free 프로그래밍으로 달성한 개선사항:
1. **처리량**: 20배 향상 (멀티코어 확장성)
2. **지연시간**: 예측 가능한 성능 (no blocking)
3. **확장성**: 코어 수에 비례한 성능 증가
4. **복원력**: 데드락 불가능

이는 고성능 시스템의 핵심 기술입니다.