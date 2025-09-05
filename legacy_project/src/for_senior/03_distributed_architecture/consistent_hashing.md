# 일관된 해싱 (Consistent Hashing)

## 학습 목표
- 일관된 해싱의 원리와 구현
- 가상 노드를 통한 로드 밸런싱
- 노드 추가/제거시 최소 재배치
- 분산 로그 시스템의 샤딩 전략

## 1. 현재 문제 상황

### 1.1 단순 해싱의 문제점
```c
// [SEQUENCE: 175] 나이브한 샤딩
typedef struct {
    server_t* servers;
    size_t num_servers;
} simple_shard_t;

int get_server_simple(simple_shard_t* shard, const char* key) {
    uint32_t hash = hash_function(key);
    return hash % shard->num_servers;  // 문제!
}

// 문제 상황:
// 3개 서버: hash % 3
// - key1 (hash=10) -> server 1
// - key2 (hash=11) -> server 2  
// - key3 (hash=12) -> server 0

// 서버 추가시 (4개):
// - key1 (hash=10) -> server 2 (이동!)
// - key2 (hash=11) -> server 3 (이동!)
// - key3 (hash=12) -> server 0 (유지)

// 결과: 75%의 데이터가 재배치됨!
```

### 1.2 로드 불균형
```c
// [SEQUENCE: 176] 불균등한 분포
void measure_load_distribution() {
    simple_shard_t shard = {.num_servers = 5};
    int counts[5] = {0};
    
    // 100만개 키 분포 테스트
    for (int i = 0; i < 1000000; i++) {
        char key[32];
        snprintf(key, sizeof(key), "log_%d", i);
        int server = get_server_simple(&shard, key);
        counts[server]++;
    }
    
    // 결과: 심각한 불균형
    // Server 0: 180,234 (18%)
    // Server 1: 243,122 (24%) <- 과부하
    // Server 2: 198,445 (20%)
    // Server 3: 156,234 (16%) <- 부족
    // Server 4: 221,965 (22%)
}
```

## 2. 일관된 해싱 구현

### 2.1 기본 구조
```c
// [SEQUENCE: 177] 일관된 해싱 링
typedef struct hash_node {
    uint32_t hash;
    char server_id[64];
    void* data;
    struct hash_node* next;
} hash_node_t;

typedef struct {
    hash_node_t* ring;     // 정렬된 해시 링
    size_t node_count;
    pthread_rwlock_t lock;
    
    // 가상 노드
    int virtual_nodes;     // 서버당 가상 노드 수
    
    // 통계
    atomic_uint64_t lookups;
    atomic_uint64_t rebalances;
} consistent_hash_t;

// [SEQUENCE: 178] 해시 링 초기화
consistent_hash_t* consistent_hash_create(int virtual_nodes) {
    consistent_hash_t* ch = calloc(1, sizeof(consistent_hash_t));
    ch->virtual_nodes = virtual_nodes;
    pthread_rwlock_init(&ch->lock, NULL);
    return ch;
}

// [SEQUENCE: 179] 서버 추가
void consistent_hash_add_server(consistent_hash_t* ch, const char* server_id) {
    pthread_rwlock_wrlock(&ch->lock);
    
    // 각 가상 노드에 대해
    for (int i = 0; i < ch->virtual_nodes; i++) {
        char virtual_key[128];
        snprintf(virtual_key, sizeof(virtual_key), "%s#%d", server_id, i);
        
        uint32_t hash = hash_function(virtual_key);
        
        // 정렬된 위치에 삽입
        hash_node_t* node = malloc(sizeof(hash_node_t));
        node->hash = hash;
        strcpy(node->server_id, server_id);
        node->data = NULL;
        
        insert_sorted(&ch->ring, node);
        ch->node_count++;
    }
    
    pthread_rwlock_unlock(&ch->lock);
    
    log_info("Added server %s with %d virtual nodes", 
             server_id, ch->virtual_nodes);
}

// [SEQUENCE: 180] 정렬된 삽입
void insert_sorted(hash_node_t** head, hash_node_t* new_node) {
    if (*head == NULL || (*head)->hash > new_node->hash) {
        new_node->next = *head;
        *head = new_node;
        return;
    }
    
    hash_node_t* current = *head;
    while (current->next && current->next->hash < new_node->hash) {
        current = current->next;
    }
    
    new_node->next = current->next;
    current->next = new_node;
}

// [SEQUENCE: 181] 키에 대한 서버 찾기
const char* consistent_hash_get_server(consistent_hash_t* ch, const char* key) {
    pthread_rwlock_rdlock(&ch->lock);
    
    uint32_t hash = hash_function(key);
    
    // 이진 탐색으로 다음 노드 찾기
    hash_node_t* node = find_successor(ch->ring, hash);
    
    // 링의 끝이면 첫 노드로
    if (node == NULL) {
        node = ch->ring;
    }
    
    const char* server = node ? node->server_id : NULL;
    
    pthread_rwlock_unlock(&ch->lock);
    
    atomic_fetch_add(&ch->lookups, 1);
    
    return server;
}

// [SEQUENCE: 182] 후속 노드 찾기
hash_node_t* find_successor(hash_node_t* ring, uint32_t hash) {
    hash_node_t* current = ring;
    
    while (current) {
        if (current->hash >= hash) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;  // 링의 끝
}
```

### 2.2 가상 노드 최적화
```c
// [SEQUENCE: 183] 가중치 기반 가상 노드
typedef struct {
    char server_id[64];
    int weight;        // 서버 가중치 (성능/용량)
    int virtual_count; // 실제 가상 노드 수
} weighted_server_t;

void add_weighted_server(consistent_hash_t* ch, 
                        const char* server_id, 
                        int weight) {
    // 가중치에 비례한 가상 노드 수
    int virtual_nodes = ch->virtual_nodes * weight;
    
    pthread_rwlock_wrlock(&ch->lock);
    
    for (int i = 0; i < virtual_nodes; i++) {
        char virtual_key[128];
        
        // 더 나은 분포를 위한 해시 체인
        if (i == 0) {
            snprintf(virtual_key, sizeof(virtual_key), "%s", server_id);
        } else {
            // 이전 해시를 기반으로 다음 해시 생성
            uint32_t prev_hash = hash_function(virtual_key);
            snprintf(virtual_key, sizeof(virtual_key), "%s#%u", 
                    server_id, prev_hash);
        }
        
        uint32_t hash = hash_function(virtual_key);
        
        hash_node_t* node = malloc(sizeof(hash_node_t));
        node->hash = hash;
        strcpy(node->server_id, server_id);
        node->data = (void*)(intptr_t)weight;
        
        insert_sorted(&ch->ring, node);
    }
    
    ch->node_count += virtual_nodes;
    pthread_rwlock_unlock(&ch->lock);
}

// [SEQUENCE: 184] Jump Consistent Hash (구글)
int32_t jump_consistent_hash(uint64_t key, int32_t num_buckets) {
    int64_t b = -1;
    int64_t j = 0;
    
    while (j < num_buckets) {
        b = j;
        key = key * 2862933555777941757ULL + 1;
        j = (b + 1) * ((double)(1LL << 31) / (double)((key >> 33) + 1));
    }
    
    return b;
}
```

### 2.3 서버 제거와 재배치
```c
// [SEQUENCE: 185] 서버 제거
typedef struct {
    char key[256];
    char old_server[64];
    char new_server[64];
} migration_task_t;

migration_task_t* consistent_hash_remove_server(consistent_hash_t* ch, 
                                               const char* server_id,
                                               size_t* task_count) {
    pthread_rwlock_wrlock(&ch->lock);
    
    // 영향받는 키 범위 찾기
    migration_task_t* tasks = NULL;
    size_t capacity = 1000;
    *task_count = 0;
    tasks = malloc(capacity * sizeof(migration_task_t));
    
    // 제거할 노드들 찾기
    hash_node_t** current = &ch->ring;
    while (*current) {
        if (strcmp((*current)->server_id, server_id) == 0) {
            hash_node_t* to_remove = *current;
            uint32_t start_hash = to_remove->hash;
            
            // 이전 노드의 해시 찾기
            uint32_t prev_hash = find_predecessor_hash(ch->ring, start_hash);
            
            // 다음 노드 찾기
            hash_node_t* next = to_remove->next;
            if (!next) next = ch->ring;  // 순환
            
            // 영향받는 키 범위: (prev_hash, start_hash]
            // 이 범위의 키들은 next->server_id로 이동
            
            if (*task_count < capacity) {
                migration_task_t* task = &tasks[(*task_count)++];
                snprintf(task->key, sizeof(task->key), 
                        "range:(%u,%u]", prev_hash, start_hash);
                strcpy(task->old_server, server_id);
                strcpy(task->new_server, next->server_id);
            }
            
            // 노드 제거
            *current = to_remove->next;
            free(to_remove);
            ch->node_count--;
        } else {
            current = &(*current)->next;
        }
    }
    
    pthread_rwlock_unlock(&ch->lock);
    
    atomic_fetch_add(&ch->rebalances, 1);
    
    return tasks;
}

// [SEQUENCE: 186] 이전 노드 해시 찾기
uint32_t find_predecessor_hash(hash_node_t* ring, uint32_t hash) {
    if (!ring) return 0;
    
    uint32_t prev = 0;
    hash_node_t* current = ring;
    
    // 마지막 노드 찾기 (순환을 위해)
    while (current->next) {
        current = current->next;
    }
    prev = current->hash;
    
    // 다시 처음부터 찾기
    current = ring;
    while (current && current->hash < hash) {
        prev = current->hash;
        current = current->next;
    }
    
    return prev;
}
```

## 3. 분산 로그 시스템 적용

### 3.1 로그 샤딩
```c
// [SEQUENCE: 187] 로그 샤딩 시스템
typedef struct {
    consistent_hash_t* ch;
    
    // 로컬 로그 버퍼 (각 서버)
    struct {
        char server_id[64];
        circular_buffer_t* buffer;
        pthread_t worker;
    } local_shards[MAX_SHARDS];
    
    size_t num_shards;
    
    // 라우팅 캐시
    struct {
        char key[256];
        char server[64];
        time_t timestamp;
    } route_cache[ROUTE_CACHE_SIZE];
    
    size_t cache_index;
    pthread_mutex_t cache_lock;
} log_sharding_system_t;

// [SEQUENCE: 188] 로그 라우팅
void route_log_entry(log_sharding_system_t* system, 
                    const char* log_key,
                    const log_entry_t* entry) {
    // 캐시 확인
    const char* server = check_route_cache(system, log_key);
    
    if (!server) {
        // 일관된 해싱으로 서버 결정
        server = consistent_hash_get_server(system->ch, log_key);
        
        // 캐시 업데이트
        update_route_cache(system, log_key, server);
    }
    
    // 해당 서버의 버퍼로 전송
    for (size_t i = 0; i < system->num_shards; i++) {
        if (strcmp(system->local_shards[i].server_id, server) == 0) {
            add_to_buffer(system->local_shards[i].buffer, entry);
            return;
        }
    }
    
    // 원격 서버로 전송
    send_to_remote_server(server, entry);
}

// [SEQUENCE: 189] 키 생성 전략
typedef enum {
    KEY_BY_TIME,      // 시간 기반
    KEY_BY_SOURCE,    // 소스 기반
    KEY_BY_TYPE,      // 로그 타입 기반
    KEY_BY_HASH       // 내용 해시 기반
} key_strategy_t;

char* generate_log_key(const log_entry_t* entry, key_strategy_t strategy) {
    static char key[256];
    
    switch (strategy) {
        case KEY_BY_TIME:
            // 시간 윈도우별 그룹화
            snprintf(key, sizeof(key), "logs_%ld", 
                    entry->timestamp / 3600);  // 시간당
            break;
            
        case KEY_BY_SOURCE:
            // 소스별 그룹화
            snprintf(key, sizeof(key), "logs_%s", entry->source);
            break;
            
        case KEY_BY_TYPE:
            // 타입별 그룹화
            snprintf(key, sizeof(key), "logs_%s", entry->type);
            break;
            
        case KEY_BY_HASH:
            // 균등 분포
            uint32_t hash = hash_function(entry->message);
            snprintf(key, sizeof(key), "logs_%u", hash);
            break;
    }
    
    return key;
}
```

### 3.2 리밸런싱
```c
// [SEQUENCE: 190] 점진적 데이터 마이그레이션
typedef struct {
    consistent_hash_t* ch;
    
    struct migration_state {
        bool active;
        char source[64];
        char target[64];
        uint32_t start_hash;
        uint32_t end_hash;
        uint32_t current_hash;
        
        atomic_uint64_t migrated_count;
        atomic_uint64_t total_count;
    } migrations[MAX_MIGRATIONS];
    
    size_t num_migrations;
    pthread_t migration_thread;
} rebalancer_t;

// [SEQUENCE: 191] 마이그레이션 워커
void* migration_worker(void* arg) {
    rebalancer_t* rb = (rebalancer_t*)arg;
    
    while (running) {
        for (size_t i = 0; i < rb->num_migrations; i++) {
            struct migration_state* ms = &rb->migrations[i];
            
            if (!ms->active) continue;
            
            // 배치 단위로 마이그레이션
            size_t batch_size = 1000;
            size_t migrated = 0;
            
            while (migrated < batch_size && 
                   ms->current_hash <= ms->end_hash) {
                
                // 해당 범위의 데이터 읽기
                log_entry_t* entries = read_range(
                    ms->source, 
                    ms->current_hash,
                    MIN(ms->current_hash + 100, ms->end_hash)
                );
                
                // 타겟으로 전송
                if (entries) {
                    send_batch_to_server(ms->target, entries);
                    
                    // 소스에서 삭제 표시
                    mark_for_deletion(ms->source, 
                                    ms->current_hash,
                                    ms->current_hash + 100);
                    
                    migrated += 100;
                    atomic_fetch_add(&ms->migrated_count, 100);
                }
                
                ms->current_hash += 100;
            }
            
            // 완료 확인
            if (ms->current_hash > ms->end_hash) {
                ms->active = false;
                log_info("Migration completed: %s -> %s (%lu entries)",
                        ms->source, ms->target, ms->migrated_count);
            }
        }
        
        usleep(10000);  // 10ms - 부하 제어
    }
    
    return NULL;
}

// [SEQUENCE: 192] 스마트 리밸런싱
void smart_rebalance(log_sharding_system_t* system) {
    // 각 서버의 부하 측정
    typedef struct {
        char server_id[64];
        uint64_t load;
        double capacity;
    } server_load_t;
    
    server_load_t loads[MAX_SHARDS];
    size_t num_servers = collect_server_loads(loads);
    
    // 평균 부하 계산
    uint64_t total_load = 0;
    for (size_t i = 0; i < num_servers; i++) {
        total_load += loads[i].load;
    }
    uint64_t avg_load = total_load / num_servers;
    
    // 불균형 감지
    for (size_t i = 0; i < num_servers; i++) {
        if (loads[i].load > avg_load * 1.5) {
            // 과부하 서버
            log_warn("Server %s overloaded: %lu (avg: %lu)",
                    loads[i].server_id, loads[i].load, avg_load);
            
            // 가상 노드 수 조정
            adjust_virtual_nodes(system->ch, loads[i].server_id, -10);
            
        } else if (loads[i].load < avg_load * 0.5) {
            // 저활용 서버
            log_info("Server %s underutilized: %lu (avg: %lu)",
                    loads[i].server_id, loads[i].load, avg_load);
            
            // 가상 노드 수 증가
            adjust_virtual_nodes(system->ch, loads[i].server_id, +10);
        }
    }
}
```

## 4. 고급 기능

### 4.1 복제와 일관된 해싱
```c
// [SEQUENCE: 193] 복제본 배치
typedef struct {
    consistent_hash_t* ch;
    int replication_factor;
    
    // 복제 전략
    enum {
        REPL_NEXT_N,      // 링에서 다음 N개 노드
        REPL_SPREAD,      // 분산 배치
        REPL_ZONE_AWARE   // 가용 영역 인식
    } strategy;
} replicated_hash_t;

// [SEQUENCE: 194] 복제본 위치 결정
void get_replica_nodes(replicated_hash_t* rh,
                      const char* key,
                      char servers[][64],
                      size_t* count) {
    *count = 0;
    
    pthread_rwlock_rdlock(&rh->ch->lock);
    
    uint32_t hash = hash_function(key);
    hash_node_t* start = find_successor(rh->ch->ring, hash);
    
    if (!start) start = rh->ch->ring;
    
    switch (rh->strategy) {
        case REPL_NEXT_N: {
            // 다음 N개의 고유 서버
            hash_node_t* current = start;
            char seen[MAX_SHARDS][64] = {0};
            int seen_count = 0;
            
            while (*count < rh->replication_factor && current) {
                // 중복 제거
                bool duplicate = false;
                for (int i = 0; i < seen_count; i++) {
                    if (strcmp(seen[i], current->server_id) == 0) {
                        duplicate = true;
                        break;
                    }
                }
                
                if (!duplicate) {
                    strcpy(servers[*count], current->server_id);
                    strcpy(seen[seen_count++], current->server_id);
                    (*count)++;
                }
                
                current = current->next;
                if (!current) current = rh->ch->ring;  // 순환
            }
            break;
        }
        
        case REPL_ZONE_AWARE: {
            // 다른 가용 영역에 복제
            get_zone_aware_replicas(rh, start, servers, count);
            break;
        }
    }
    
    pthread_rwlock_unlock(&rh->ch->lock);
}

// [SEQUENCE: 195] 가용 영역 인식 복제
typedef struct {
    char server_id[64];
    char zone[32];
    char region[32];
} server_location_t;

void get_zone_aware_replicas(replicated_hash_t* rh,
                            hash_node_t* primary,
                            char servers[][64],
                            size_t* count) {
    server_location_t* locations = get_server_locations();
    
    // 주 서버의 위치
    server_location_t* primary_loc = find_location(
        locations, primary->server_id
    );
    
    strcpy(servers[(*count)++], primary->server_id);
    
    // 다른 존의 서버 찾기
    hash_node_t* current = primary->next;
    if (!current) current = rh->ch->ring;
    
    while (*count < rh->replication_factor && current != primary) {
        server_location_t* loc = find_location(
            locations, current->server_id
        );
        
        // 다른 존이면 추가
        if (strcmp(loc->zone, primary_loc->zone) != 0) {
            strcpy(servers[(*count)++], current->server_id);
        }
        
        current = current->next;
        if (!current) current = rh->ch->ring;
    }
}
```

### 4.2 부하 인식 라우팅
```c
// [SEQUENCE: 196] 부하 기반 일관된 해싱
typedef struct {
    consistent_hash_t* ch;
    
    // 서버별 부하 정보
    struct {
        char server_id[64];
        atomic_uint64_t active_connections;
        atomic_uint64_t request_rate;
        atomic_uint64_t response_time_us;
        double load_factor;
    } server_loads[MAX_SHARDS];
    
    size_t num_servers;
    pthread_t monitor_thread;
} load_aware_hash_t;

// [SEQUENCE: 197] 부하 인식 서버 선택
const char* get_server_with_load_balancing(load_aware_hash_t* lah,
                                          const char* key) {
    // 기본 서버와 대체 서버들
    char candidates[5][64];
    size_t num_candidates = 0;
    
    get_replica_nodes(lah, key, candidates, &num_candidates);
    
    // 가장 부하가 적은 서버 선택
    const char* best_server = candidates[0];
    double min_load = DBL_MAX;
    
    for (size_t i = 0; i < num_candidates; i++) {
        double load = get_server_load(lah, candidates[i]);
        
        if (load < min_load * 0.8) {  // 20% 이상 낮으면
            min_load = load;
            best_server = candidates[i];
        }
    }
    
    return best_server;
}

// [SEQUENCE: 198] 동적 가중치 조정
void* load_monitor_thread(void* arg) {
    load_aware_hash_t* lah = (load_aware_hash_t*)arg;
    
    while (running) {
        // 각 서버의 메트릭 수집
        for (size_t i = 0; i < lah->num_servers; i++) {
            collect_server_metrics(&lah->server_loads[i]);
            
            // 부하 계수 계산
            double load = calculate_load_factor(&lah->server_loads[i]);
            lah->server_loads[i].load_factor = load;
            
            // 과부하시 가상 노드 감소
            if (load > 0.8) {
                reduce_virtual_nodes(lah->ch, 
                                   lah->server_loads[i].server_id);
            }
        }
        
        sleep(5);  // 5초마다
    }
    
    return NULL;
}
```

## 5. 최적화와 성능

### 5.1 해시 함수 선택
```c
// [SEQUENCE: 199] 고품질 해시 함수
uint32_t murmur3_32(const void* key, size_t len, uint32_t seed) {
    const uint8_t* data = (const uint8_t*)key;
    const int nblocks = len / 4;
    uint32_t h1 = seed;
    
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    
    // Body
    const uint32_t* blocks = (const uint32_t*)(data + nblocks*4);
    for(int i = -nblocks; i; i++) {
        uint32_t k1 = blocks[i];
        
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= c2;
        
        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> 19);
        h1 = h1*5 + 0xe6546b64;
    }
    
    // Tail
    const uint8_t* tail = (const uint8_t*)(data + nblocks*4);
    uint32_t k1 = 0;
    
    switch(len & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1; k1 = (k1 << 15) | (k1 >> 17); k1 *= c2;
                h1 ^= k1;
    }
    
    // Finalization
    h1 ^= len;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    
    return h1;
}

// [SEQUENCE: 200] xxHash (더 빠른 대안)
uint32_t xxhash32(const void* data, size_t len, uint32_t seed) {
    // xxHash는 MurmurHash보다 2-3배 빠름
    // 구현 생략...
    return XXH32(data, len, seed);
}
```

### 5.2 캐싱과 최적화
```c
// [SEQUENCE: 201] 라우팅 캐시
typedef struct {
    struct cache_entry {
        uint64_t key_hash;
        char server[64];
        time_t timestamp;
        atomic_int hits;
    } entries[CACHE_SIZE];
    
    atomic_size_t index;
    
    // 통계
    atomic_uint64_t hits;
    atomic_uint64_t misses;
} routing_cache_t;

const char* cached_get_server(routing_cache_t* cache,
                            consistent_hash_t* ch,
                            const char* key) {
    uint64_t key_hash = xxhash64(key, strlen(key), 0);
    
    // 캐시 조회
    size_t slot = key_hash % CACHE_SIZE;
    struct cache_entry* entry = &cache->entries[slot];
    
    if (entry->key_hash == key_hash &&
        time(NULL) - entry->timestamp < CACHE_TTL) {
        atomic_fetch_add(&entry->hits, 1);
        atomic_fetch_add(&cache->hits, 1);
        return entry->server;
    }
    
    // 캐시 미스
    atomic_fetch_add(&cache->misses, 1);
    
    // 일관된 해싱으로 찾기
    const char* server = consistent_hash_get_server(ch, key);
    
    // 캐시 업데이트
    entry->key_hash = key_hash;
    strcpy(entry->server, server);
    entry->timestamp = time(NULL);
    entry->hits = 1;
    
    return server;
}
```

## 6. 모니터링과 디버깅

### 6.1 분포 분석
```c
// [SEQUENCE: 202] 키 분포 분석
typedef struct {
    char server_id[64];
    uint32_t hash_start;
    uint32_t hash_end;
    uint64_t key_count;
    uint64_t total_size;
    double ownership_percent;
} server_distribution_t;

void analyze_distribution(consistent_hash_t* ch,
                         server_distribution_t* dist,
                         size_t* count) {
    pthread_rwlock_rdlock(&ch->lock);
    
    *count = 0;
    hash_node_t* current = ch->ring;
    uint32_t prev_hash = 0;
    
    // 각 서버의 해시 범위 계산
    while (current && *count < MAX_SHARDS) {
        // 같은 서버의 범위 병합
        bool found = false;
        for (size_t i = 0; i < *count; i++) {
            if (strcmp(dist[i].server_id, current->server_id) == 0) {
                // 범위 확장
                uint32_t range = current->hash - prev_hash;
                dist[i].ownership_percent += 
                    (double)range / UINT32_MAX * 100;
                found = true;
                break;
            }
        }
        
        if (!found) {
            strcpy(dist[*count].server_id, current->server_id);
            dist[*count].hash_start = prev_hash;
            dist[*count].hash_end = current->hash;
            dist[*count].ownership_percent = 
                (double)(current->hash - prev_hash) / UINT32_MAX * 100;
            (*count)++;
        }
        
        prev_hash = current->hash;
        current = current->next;
    }
    
    pthread_rwlock_unlock(&ch->lock);
}

// [SEQUENCE: 203] 시각화
void visualize_hash_ring(consistent_hash_t* ch) {
    printf("=== Consistent Hash Ring ===\n");
    printf("Total nodes: %zu\n", ch->node_count);
    printf("Virtual nodes per server: %d\n\n", ch->virtual_nodes);
    
    server_distribution_t dist[MAX_SHARDS];
    size_t count;
    analyze_distribution(ch, dist, &count);
    
    printf("Server Distribution:\n");
    for (size_t i = 0; i < count; i++) {
        printf("  %s: %.2f%% ownership\n",
               dist[i].server_id,
               dist[i].ownership_percent);
        
        // 간단한 바 차트
        int bars = (int)(dist[i].ownership_percent / 2);
        printf("    [");
        for (int j = 0; j < bars; j++) printf("=");
        printf("]\n");
    }
    
    // 표준편차 계산
    double avg = 100.0 / count;
    double variance = 0;
    for (size_t i = 0; i < count; i++) {
        double diff = dist[i].ownership_percent - avg;
        variance += diff * diff;
    }
    double stddev = sqrt(variance / count);
    
    printf("\nBalance Score: %.2f%% (lower is better)\n", 
           stddev / avg * 100);
}
```

## 마무리

일관된 해싱으로 달성한 개선사항:
1. **최소 재배치**: 노드 추가/제거시 K/N 데이터만 이동
2. **균등 분포**: 가상 노드로 부하 균형
3. **확장성**: O(1) 라우팅 결정
4. **유연성**: 가중치 기반 노드 용량 처리

이는 분산 시스템의 핵심 로드 밸런싱 기술입니다.