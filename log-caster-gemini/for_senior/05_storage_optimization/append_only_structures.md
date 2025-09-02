# Append-Only 구조 (Append-Only Structures)

## 학습 목표
- Append-only 데이터 구조의 설계와 구현
- 불변성과 동시성 이점 활용
- 시간 여행 쿼리와 버전 관리
- 효율적인 가비지 컬렉션

## 1. 현재 문제 상황

### 1.1 변경 가능한 구조의 문제점
```c
// [SEQUENCE: 359] 전통적인 가변 로그 버퍼의 문제
typedef struct {
    log_entry_t* entries;
    size_t capacity;
    size_t head;
    size_t tail;
    pthread_mutex_t lock;
} mutable_log_buffer_t;

void update_log_entry(mutable_log_buffer_t* buffer, size_t index, 
                     const char* new_message) {
    pthread_mutex_lock(&buffer->lock);
    
    // 문제 1: 기존 데이터 덮어쓰기
    free(buffer->entries[index].message);
    buffer->entries[index].message = strdup(new_message);
    buffer->entries[index].timestamp = time(NULL);
    
    // 문제 2: 읽는 쪽에서 일관성 없는 데이터 볼 수 있음
    // 문제 3: 이전 버전 복구 불가능
    // 문제 4: 동시성 제한 (락 필요)
    
    pthread_mutex_unlock(&buffer->lock);
}

// 결과:
// - 데이터 레이스 위험
// - 감사 추적 불가능
// - 롤백 불가능
// - 성능 병목 (락 경합)
```

### 1.2 동시성과 일관성 문제
```c
// [SEQUENCE: 360] 읽기-쓰기 충돌
void reader_thread(mutable_log_buffer_t* buffer) {
    // 읽는 동안 다른 스레드가 수정할 수 있음
    log_entry_t* entry = &buffer->entries[10];
    
    // 이 시점에 다른 스레드가 entry를 수정하면?
    printf("Message: %s\n", entry->message);  // 크래시 가능!
}

void writer_thread(mutable_log_buffer_t* buffer) {
    // 동시에 같은 위치 수정 시도
    update_log_entry(buffer, 10, "New message");
}
```

## 2. Append-Only 구조 설계

### 2.1 불변 로그 엔트리
```c
// [SEQUENCE: 361] 불변 로그 엔트리 구조
typedef struct immutable_log_entry {
    const uint64_t id;
    const uint64_t timestamp;
    const char* const message;
    const log_level_t level;
    
    // 메타데이터
    const uint32_t checksum;
    const struct immutable_log_entry* const prev;  // 이전 버전 링크
} immutable_log_entry_t;

// [SEQUENCE: 362] Append-only 로그 구조
typedef struct append_only_log {
    // 현재 tail (최신 엔트리)
    atomic_ptr_t tail;
    
    // 세그먼트 관리
    struct segment {
        immutable_log_entry_t* entries;
        size_t count;
        size_t capacity;
        uint64_t min_timestamp;
        uint64_t max_timestamp;
        struct segment* next;
    } *segments;
    
    // 인덱스 (읽기 최적화)
    struct {
        skiplist_t* by_timestamp;
        hash_table_t* by_id;
    } indexes;
    
    // 통계
    atomic_uint64_t total_entries;
    atomic_uint64_t total_bytes;
} append_only_log_t;

// [SEQUENCE: 363] 로그 추가 (락 없음!)
immutable_log_entry_t* append_log(append_only_log_t* log,
                                  const char* message,
                                  log_level_t level) {
    // 새 엔트리 생성 (불변)
    immutable_log_entry_t* entry = malloc(sizeof(immutable_log_entry_t));
    
    // const 필드 초기화 (한 번만 설정)
    *(uint64_t*)&entry->id = generate_unique_id();
    *(uint64_t*)&entry->timestamp = get_timestamp_ns();
    *(char**)&entry->message = strdup(message);
    *(log_level_t*)&entry->level = level;
    *(uint32_t*)&entry->checksum = calculate_checksum(entry);
    
    // 원자적으로 tail에 추가
    immutable_log_entry_t* old_tail;
    do {
        old_tail = atomic_load(&log->tail);
        *(immutable_log_entry_t**)&entry->prev = old_tail;
    } while (!atomic_compare_exchange_weak(&log->tail, &old_tail, entry));
    
    // 인덱스 업데이트 (비동기 가능)
    update_indexes_async(log, entry);
    
    // 통계 업데이트
    atomic_fetch_add(&log->total_entries, 1);
    atomic_fetch_add(&log->total_bytes, strlen(message));
    
    return entry;
}
```

### 2.2 Persistent Append-Only 구조
```c
// [SEQUENCE: 364] 디스크 기반 append-only 저장소
typedef struct {
    int fd;
    off_t write_offset;
    
    // 메모리 매핑 영역
    void* mmap_base;
    size_t mmap_size;
    
    // Write-ahead 버퍼
    struct {
        char* buffer;
        size_t size;
        size_t capacity;
    } write_buffer;
    
    // 체크포인트
    struct checkpoint {
        off_t offset;
        uint64_t entry_count;
        uint64_t timestamp;
        uint32_t checksum;
    } *checkpoints;
    size_t num_checkpoints;
} persistent_log_t;

// [SEQUENCE: 365] 영속적 추가
void persist_append(persistent_log_t* plog, 
                   immutable_log_entry_t* entry) {
    // 직렬화
    size_t serialized_size = serialize_entry_size(entry);
    char* serialized = alloca(serialized_size);
    serialize_entry(entry, serialized);
    
    // Write buffer에 추가
    if (plog->write_buffer.size + serialized_size > 
        plog->write_buffer.capacity) {
        // 버퍼 플러시
        flush_write_buffer(plog);
    }
    
    memcpy(plog->write_buffer.buffer + plog->write_buffer.size,
           serialized, serialized_size);
    plog->write_buffer.size += serialized_size;
    
    // 주기적 체크포인트
    if (should_checkpoint(plog)) {
        create_checkpoint(plog);
    }
}

// [SEQUENCE: 366] Append-only 파일 회전
void rotate_append_only_file(persistent_log_t* plog) {
    // 새 파일 생성
    char new_filename[PATH_MAX];
    snprintf(new_filename, sizeof(new_filename),
             "log_%lu.aof", get_timestamp_ms());
    
    int new_fd = open(new_filename, 
                     O_CREAT | O_WRONLY | O_APPEND | O_DIRECT,
                     0644);
    
    // 원자적 전환
    int old_fd = plog->fd;
    plog->fd = new_fd;
    plog->write_offset = 0;
    
    // 이전 파일 닫기 (읽기는 계속 가능)
    close(old_fd);
    
    // 새 메모리 매핑
    update_memory_mapping(plog);
}
```

### 2.3 시간 여행 쿼리
```c
// [SEQUENCE: 367] 과거 시점 조회
typedef struct {
    uint64_t query_timestamp;
    
    // 스냅샷 뷰
    struct snapshot_view {
        immutable_log_entry_t** entries;
        size_t count;
        uint64_t start_time;
        uint64_t end_time;
    } view;
} time_travel_query_t;

// [SEQUENCE: 368] 특정 시점의 로그 상태 재구성
snapshot_view_t* query_at_timestamp(append_only_log_t* log,
                                   uint64_t timestamp) {
    snapshot_view_t* view = calloc(1, sizeof(snapshot_view_t));
    view->start_time = 0;
    view->end_time = timestamp;
    
    // 타임스탬프 인덱스 사용
    skiplist_iterator_t* iter = skiplist_lower_bound(
        log->indexes.by_timestamp, &timestamp);
    
    size_t capacity = 1000;
    view->entries = malloc(capacity * sizeof(immutable_log_entry_t*));
    
    // 해당 시점까지의 모든 엔트리 수집
    while (skiplist_iterator_valid(iter)) {
        immutable_log_entry_t* entry = skiplist_iterator_value(iter);
        
        if (entry->timestamp > timestamp) break;
        
        if (view->count >= capacity) {
            capacity *= 2;
            view->entries = realloc(view->entries,
                                   capacity * sizeof(immutable_log_entry_t*));
        }
        
        view->entries[view->count++] = entry;
        skiplist_iterator_next(iter);
    }
    
    skiplist_iterator_destroy(iter);
    return view;
}

// [SEQUENCE: 369] 변경 이력 추적
typedef struct {
    uint64_t entry_id;
    
    struct change_history {
        immutable_log_entry_t* version;
        uint64_t timestamp;
        const char* change_type;  // "CREATE", "UPDATE", "DELETE"
    } *changes;
    size_t change_count;
} entry_history_t;

entry_history_t* get_entry_history(append_only_log_t* log, uint64_t id) {
    entry_history_t* history = calloc(1, sizeof(entry_history_t));
    history->entry_id = id;
    
    // ID로 최신 버전 찾기
    immutable_log_entry_t* current = hash_table_get(log->indexes.by_id, &id);
    
    // 이전 버전 체인 따라가기
    while (current) {
        add_to_history(history, current);
        current = current->prev;
    }
    
    // 시간순 정렬
    qsort(history->changes, history->change_count,
          sizeof(struct change_history), compare_by_timestamp);
    
    return history;
}
```

## 3. 고급 Append-Only 기법

### 3.1 Merkle Tree 기반 검증
```c
// [SEQUENCE: 370] Merkle Tree로 무결성 보장
typedef struct merkle_node {
    uint8_t hash[32];  // SHA-256
    struct merkle_node* left;
    struct merkle_node* right;
    
    // 리프 노드인 경우
    immutable_log_entry_t* entry;
} merkle_node_t;

typedef struct {
    merkle_node_t* root;
    size_t leaf_count;
    
    // 증분 업데이트를 위한 경로 캐시
    merkle_node_t** rightmost_path;
    size_t path_length;
} merkle_tree_t;

// [SEQUENCE: 371] 증분 Merkle Tree 업데이트
void merkle_append(merkle_tree_t* tree, immutable_log_entry_t* entry) {
    // 엔트리 해시 계산
    uint8_t entry_hash[32];
    sha256(entry, sizeof(*entry), entry_hash);
    
    // 새 리프 노드
    merkle_node_t* leaf = create_leaf_node(entry_hash, entry);
    
    // 증분 업데이트
    merkle_node_t* current = leaf;
    size_t height = 0;
    
    while (height < tree->path_length && 
           tree->rightmost_path[height] != NULL) {
        // 형제 노드와 결합
        merkle_node_t* sibling = tree->rightmost_path[height];
        merkle_node_t* parent = create_internal_node(sibling, current);
        
        tree->rightmost_path[height] = NULL;
        current = parent;
        height++;
    }
    
    // 경로 업데이트
    if (height >= tree->path_length) {
        expand_path(tree);
    }
    tree->rightmost_path[height] = current;
    
    // 루트 재계산
    update_root(tree);
    tree->leaf_count++;
}

// [SEQUENCE: 372] 감사 증명 생성
typedef struct {
    immutable_log_entry_t* entry;
    uint8_t entry_hash[32];
    
    struct proof_node {
        uint8_t hash[32];
        bool is_left;  // true if this node is on the left
    } *proof_path;
    size_t path_length;
    
    uint8_t root_hash[32];
} audit_proof_t;

audit_proof_t* generate_audit_proof(merkle_tree_t* tree,
                                   immutable_log_entry_t* entry) {
    audit_proof_t* proof = calloc(1, sizeof(audit_proof_t));
    proof->entry = entry;
    sha256(entry, sizeof(*entry), proof->entry_hash);
    
    // 엔트리 위치 찾기
    size_t leaf_index = find_leaf_index(tree, entry);
    
    // 증명 경로 구성
    build_proof_path(tree, leaf_index, proof);
    
    // 루트 해시 저장
    memcpy(proof->root_hash, tree->root->hash, 32);
    
    return proof;
}
```

### 3.2 Copy-on-Write B-Tree
```c
// [SEQUENCE: 373] COW B-Tree 노드
typedef struct cow_btree_node {
    bool is_leaf;
    size_t num_keys;
    
    // 불변 데이터
    struct {
        uint64_t key;
        union {
            immutable_log_entry_t* entry;  // 리프 노드
            struct cow_btree_node* child;  // 내부 노드
        } value;
    } items[BTREE_ORDER];
    
    // 버전 정보
    uint64_t version;
    uint32_t checksum;
} cow_btree_node_t;

typedef struct {
    atomic_ptr_t root;
    atomic_uint64_t version;
    
    // 이전 버전 관리
    struct version_info {
        uint64_t version;
        cow_btree_node_t* root;
        uint64_t timestamp;
    } *versions;
    size_t version_count;
} cow_btree_t;

// [SEQUENCE: 374] Copy-on-Write 삽입
cow_btree_node_t* cow_insert(cow_btree_t* tree,
                             uint64_t key,
                             immutable_log_entry_t* entry) {
    // 새 버전 번호
    uint64_t new_version = atomic_fetch_add(&tree->version, 1) + 1;
    
    // 루트부터 경로 복사
    cow_btree_node_t* old_root = atomic_load(&tree->root);
    cow_btree_node_t* new_root = copy_node(old_root);
    new_root->version = new_version;
    
    // 삽입 경로만 복사
    cow_btree_node_t* current = new_root;
    cow_btree_node_t* parent = NULL;
    
    while (!current->is_leaf) {
        size_t child_idx = find_child_index(current, key);
        
        // 자식 노드 복사
        cow_btree_node_t* old_child = current->items[child_idx].value.child;
        cow_btree_node_t* new_child = copy_node(old_child);
        new_child->version = new_version;
        
        current->items[child_idx].value.child = new_child;
        parent = current;
        current = new_child;
    }
    
    // 리프에 삽입
    insert_into_leaf(current, key, entry);
    
    // 분할 필요시 처리
    if (current->num_keys >= BTREE_ORDER) {
        split_and_propagate(new_root, parent, current);
    }
    
    // 원자적 루트 교체
    cow_btree_node_t* expected = old_root;
    if (atomic_compare_exchange_strong(&tree->root, &expected, new_root)) {
        // 버전 기록
        record_version(tree, new_version, new_root);
        return new_root;
    } else {
        // 충돌 - 재시도
        free_tree(new_root);
        return cow_insert(tree, key, entry);
    }
}
```

### 3.3 Event Sourcing 패턴
```c
// [SEQUENCE: 375] 이벤트 소싱 로그
typedef enum {
    EVENT_LOG_CREATED,
    EVENT_LOG_APPENDED,
    EVENT_LOG_ROTATED,
    EVENT_LOG_COMPRESSED,
    EVENT_LOG_INDEXED,
    EVENT_QUERY_EXECUTED
} event_type_t;

typedef struct {
    uint64_t event_id;
    uint64_t timestamp;
    event_type_t type;
    
    // 이벤트별 데이터
    union {
        struct {
            uint64_t entry_id;
            const char* message;
        } append_data;
        
        struct {
            const char* old_file;
            const char* new_file;
        } rotate_data;
        
        struct {
            const char* query;
            uint64_t result_count;
            uint64_t duration_ns;
        } query_data;
    } data;
} log_event_t;

// [SEQUENCE: 376] 이벤트 스트림
typedef struct {
    // 이벤트 저장소
    append_only_log_t* event_log;
    
    // 프로젝션 (구체화된 뷰)
    struct {
        hash_table_t* entry_count_by_level;
        skiplist_t* entries_by_timestamp;
        uint64_t total_size;
    } projections;
    
    // 이벤트 핸들러
    void (*handlers[EVENT_TYPE_COUNT])(log_event_t*);
} event_sourced_log_t;

// [SEQUENCE: 377] 이벤트 재생으로 상태 복구
void replay_events(event_sourced_log_t* es_log, 
                  uint64_t from_event_id) {
    // 프로젝션 초기화
    reset_projections(&es_log->projections);
    
    // 이벤트 스트림 읽기
    immutable_log_entry_t* event_entry = find_event(es_log->event_log,
                                                    from_event_id);
    
    while (event_entry) {
        log_event_t* event = deserialize_event(event_entry);
        
        // 핸들러 실행
        if (es_log->handlers[event->type]) {
            es_log->handlers[event->type](event);
        }
        
        // 프로젝션 업데이트
        update_projections(&es_log->projections, event);
        
        event_entry = event_entry->prev;
    }
}
```

## 4. 가비지 컬렉션과 압축

### 4.1 세그먼트 기반 GC
```c
// [SEQUENCE: 378] 세그먼트 관리
typedef struct segment_manager {
    struct segment {
        char filename[PATH_MAX];
        int fd;
        size_t size;
        
        uint64_t min_timestamp;
        uint64_t max_timestamp;
        uint64_t entry_count;
        
        bool is_compacted;
        atomic_int ref_count;  // 읽기 참조 카운트
    } *segments;
    size_t num_segments;
    
    // 압축 정책
    struct {
        size_t min_segment_size;
        float min_dead_ratio;
        size_t max_segments_per_compaction;
    } compaction_policy;
    
    pthread_t gc_thread;
} segment_manager_t;

// [SEQUENCE: 379] 세그먼트 압축
void compact_segments(segment_manager_t* mgr) {
    // 압축 대상 선택
    struct segment** candidates = select_compaction_candidates(mgr);
    
    if (!candidates) return;
    
    // 새 세그먼트 생성
    char new_filename[PATH_MAX];
    generate_segment_filename(new_filename);
    
    int new_fd = open(new_filename, O_CREAT | O_WRONLY | O_DIRECT, 0644);
    
    // 병합 및 중복 제거
    merge_iterator_t* iter = create_segment_merge_iterator(candidates);
    
    uint64_t last_id = 0;
    size_t written_entries = 0;
    
    while (merge_iterator_valid(iter)) {
        immutable_log_entry_t* entry = merge_iterator_value(iter);
        
        // 중복 제거 (최신 버전만 유지)
        if (entry->id != last_id) {
            write_entry_to_segment(new_fd, entry);
            written_entries++;
            last_id = entry->id;
        }
        
        merge_iterator_next(iter);
    }
    
    fsync(new_fd);
    close(new_fd);
    
    // 원자적 교체
    replace_segments_atomic(mgr, candidates, new_filename);
    
    merge_iterator_destroy(iter);
}

// [SEQUENCE: 380] 참조 카운팅 GC
void segment_release(struct segment* seg) {
    if (atomic_fetch_sub(&seg->ref_count, 1) == 1) {
        // 마지막 참조 해제
        if (seg->is_compacted) {
            // 압축된 세그먼트는 삭제 가능
            unlink(seg->filename);
            close(seg->fd);
            free(seg);
        }
    }
}
```

### 4.2 증분 체크포인트
```c
// [SEQUENCE: 381] 증분 체크포인트 시스템
typedef struct {
    // 전체 스냅샷
    struct full_checkpoint {
        char filename[PATH_MAX];
        uint64_t timestamp;
        uint64_t entry_count;
        uint64_t size;
    } *full_checkpoints;
    
    // 증분 체크포인트
    struct incremental_checkpoint {
        char filename[PATH_MAX];
        uint64_t base_checkpoint_id;
        uint64_t from_entry_id;
        uint64_t to_entry_id;
        uint64_t size;
    } *incremental_checkpoints;
    
    // 체크포인트 정책
    struct {
        size_t full_checkpoint_interval;  // 엔트리 수
        size_t incremental_interval;
        time_t max_checkpoint_age;
    } policy;
} checkpoint_manager_t;

// [SEQUENCE: 382] 증분 체크포인트 생성
void create_incremental_checkpoint(checkpoint_manager_t* mgr,
                                  append_only_log_t* log) {
    // 마지막 체크포인트 찾기
    uint64_t last_entry_id = get_last_checkpoint_entry_id(mgr);
    uint64_t current_entry_id = get_current_entry_id(log);
    
    if (current_entry_id - last_entry_id < mgr->policy.incremental_interval) {
        return;  // 아직 체크포인트 불필요
    }
    
    // 증분 데이터 수집
    char checkpoint_file[PATH_MAX];
    generate_checkpoint_filename(checkpoint_file, "incremental");
    
    int fd = open(checkpoint_file, O_CREAT | O_WRONLY, 0644);
    
    // 변경된 엔트리만 저장
    write_checkpoint_header(fd, last_entry_id, current_entry_id);
    
    immutable_log_entry_t* entry = find_entry_by_id(log, last_entry_id + 1);
    while (entry && entry->id <= current_entry_id) {
        serialize_entry_to_checkpoint(fd, entry);
        entry = get_next_entry(entry);
    }
    
    fsync(fd);
    close(fd);
    
    // 체크포인트 등록
    register_incremental_checkpoint(mgr, checkpoint_file,
                                   last_entry_id, current_entry_id);
}
```

## 5. 성능 최적화와 모니터링

### 5.1 읽기 최적화
```c
// [SEQUENCE: 383] 블룸 필터 기반 존재 확인
typedef struct {
    bloom_filter_t** segment_filters;
    size_t num_filters;
    
    // 글로벌 필터 (최근 데이터)
    bloom_filter_t* recent_filter;
    size_t recent_capacity;
} existence_index_t;

bool entry_exists_fast(existence_index_t* index, uint64_t entry_id) {
    // 최근 필터 확인
    if (bloom_contains(index->recent_filter, &entry_id, sizeof(entry_id))) {
        return true;  // 아마도 존재 (false positive 가능)
    }
    
    // 세그먼트 필터 확인
    for (size_t i = 0; i < index->num_filters; i++) {
        if (bloom_contains(index->segment_filters[i], 
                          &entry_id, sizeof(entry_id))) {
            return true;
        }
    }
    
    return false;  // 확실히 존재하지 않음
}

// [SEQUENCE: 384] 캐시 친화적 순회
void cache_friendly_scan(append_only_log_t* log,
                        void (*callback)(immutable_log_entry_t*)) {
    // 프리페칭으로 캐시 미스 최소화
    const size_t PREFETCH_DISTANCE = 8;
    
    immutable_log_entry_t* current = atomic_load(&log->tail);
    immutable_log_entry_t* prefetch_ptrs[PREFETCH_DISTANCE];
    
    // 초기 프리페치
    for (size_t i = 0; i < PREFETCH_DISTANCE && current; i++) {
        prefetch_ptrs[i] = current;
        __builtin_prefetch(current, 0, 3);
        current = current->prev;
    }
    
    // 순회
    size_t prefetch_idx = 0;
    current = atomic_load(&log->tail);
    
    while (current) {
        // 다음 프리페치
        if (prefetch_ptrs[prefetch_idx]) {
            immutable_log_entry_t* to_prefetch = prefetch_ptrs[prefetch_idx];
            for (int i = 0; i < PREFETCH_DISTANCE && to_prefetch; i++) {
                to_prefetch = to_prefetch->prev;
            }
            if (to_prefetch) {
                __builtin_prefetch(to_prefetch, 0, 3);
            }
            prefetch_ptrs[prefetch_idx] = to_prefetch;
        }
        
        callback(current);
        
        current = current->prev;
        prefetch_idx = (prefetch_idx + 1) % PREFETCH_DISTANCE;
    }
}
```

### 5.2 메트릭과 모니터링
```c
// [SEQUENCE: 385] Append-only 메트릭
typedef struct {
    // 쓰기 성능
    atomic_uint64_t appends_per_sec;
    atomic_uint64_t bytes_per_sec;
    
    // 읽기 성능
    atomic_uint64_t queries_per_sec;
    atomic_uint64_t time_travel_queries;
    
    // 저장소 효율성
    atomic_uint64_t total_entries;
    atomic_uint64_t unique_entries;
    atomic_uint64_t compression_ratio;
    
    // GC 통계
    atomic_uint64_t gc_runs;
    atomic_uint64_t gc_reclaimed_bytes;
    atomic_uint64_t gc_duration_ms;
} append_only_metrics_t;

void report_append_only_metrics(append_only_log_t* log) {
    append_only_metrics_t* m = &log->metrics;
    
    printf("=== Append-Only Log Metrics ===\n");
    
    // 처리량
    printf("Write Performance:\n");
    printf("  Appends: %lu/sec\n", atomic_load(&m->appends_per_sec));
    printf("  Throughput: %s/sec\n", 
           format_bytes(atomic_load(&m->bytes_per_sec)));
    
    // 쿼리 성능
    printf("\nQuery Performance:\n");
    printf("  Queries: %lu/sec\n", atomic_load(&m->queries_per_sec));
    printf("  Time Travel: %lu queries\n", 
           atomic_load(&m->time_travel_queries));
    
    // 저장소 효율성
    uint64_t total = atomic_load(&m->total_entries);
    uint64_t unique = atomic_load(&m->unique_entries);
    printf("\nStorage Efficiency:\n");
    printf("  Total Entries: %lu\n", total);
    printf("  Unique Entries: %lu (%.1f%% dedup)\n",
           unique, 100.0 * (total - unique) / total);
    
    // GC 효율성
    printf("\nGarbage Collection:\n");
    printf("  Runs: %lu\n", atomic_load(&m->gc_runs));
    printf("  Reclaimed: %s\n", 
           format_bytes(atomic_load(&m->gc_reclaimed_bytes)));
}
```

## 마무리

Append-only 구조로 달성한 개선사항:
1. **동시성**: 락 없는 추가 연산
2. **일관성**: 불변성으로 데이터 레이스 제거
3. **감사성**: 완전한 변경 이력 보존
4. **복구성**: 시간 여행 쿼리 지원

이는 현대적인 로그 시스템의 핵심 설계 패턴입니다.