# 로그 구조화 스토리지 (Log-Structured Storage)

## 학습 목표
- Log-Structured Merge Tree (LSM-Tree) 구현
- Write-Ahead Logging (WAL) 최적화
- 압축과 병합 전략
- 고성능 로그 저장소 설계

## 1. 현재 문제 상황

### 1.1 전통적인 저장 방식의 한계
```c
// [SEQUENCE: 312] B-Tree 기반 저장소의 문제
typedef struct {
    int fd;
    btree_t* index;
    pthread_mutex_t lock;
} traditional_storage_t;

void traditional_write(traditional_storage_t* storage, 
                      log_entry_t* entry) {
    pthread_mutex_lock(&storage->lock);
    
    // 문제 1: 랜덤 I/O
    off_t offset = allocate_space(storage, entry->size);
    pwrite(storage->fd, entry, entry->size, offset);
    
    // 문제 2: 인덱스 업데이트 (추가 랜덤 I/O)
    btree_insert(storage->index, entry->id, offset);
    
    // 문제 3: 동기 fsync (느림)
    fsync(storage->fd);
    
    pthread_mutex_unlock(&storage->lock);
}

// 성능 측정:
// - 쓰기: 1,000 ops/sec (SSD)
// - 읽기: 10,000 ops/sec
// - 공간 효율: 60% (단편화)
```

### 1.2 로그 데이터의 특성
```c
// [SEQUENCE: 313] 로그 워크로드 분석
typedef struct {
    // 쓰기 패턴
    uint64_t writes_per_sec;      // 100,000+
    size_t avg_entry_size;        // 200-500 bytes
    bool append_only;             // true (수정 없음)
    
    // 읽기 패턴
    float recent_read_ratio;      // 90% (최근 데이터)
    float range_scan_ratio;       // 70% (시간 범위)
    float point_lookup_ratio;     // 30% (특정 로그)
    
    // 보존 정책
    uint32_t retention_days;      // 30-90일
    bool compression_enabled;     // true
} log_workload_t;
```

## 2. Log-Structured Storage 설계

### 2.1 기본 구조
```c
// [SEQUENCE: 314] LSM-Tree 구성요소
typedef struct {
    // In-memory 구성요소
    struct {
        skiplist_t* active;       // 쓰기 버퍼
        skiplist_t* immutable;    // 플러시 대기
        size_t size_limit;        // 64MB
        pthread_rwlock_t lock;
    } memtable;
    
    // On-disk 구성요소
    struct {
        sstable_t** tables;       // Sorted String Tables
        size_t* level_counts;     // 각 레벨의 테이블 수
        size_t num_levels;        // 일반적으로 7
        
        // 레벨별 크기 제한
        size_t level_max_bytes[MAX_LEVELS];
    } disk;
    
    // Write-Ahead Log
    struct {
        int fd;
        off_t offset;
        char* buffer;
        size_t buffer_size;
        pthread_mutex_t lock;
    } wal;
    
    // 백그라운드 작업
    pthread_t compaction_thread;
    pthread_t flush_thread;
    
    // 통계
    atomic_uint64_t bytes_written;
    atomic_uint64_t bytes_read;
    atomic_uint64_t compactions;
} lsm_tree_t;

// [SEQUENCE: 315] Memtable (Skip List)
typedef struct skiplist_node {
    char* key;
    size_t key_len;
    char* value;
    size_t value_len;
    uint64_t seq_num;
    
    struct skiplist_node* forward[1];  // 가변 크기
} skiplist_node_t;

typedef struct {
    skiplist_node_t* header;
    int max_level;
    int level;
    size_t size;
    
    // 메모리 할당자
    arena_allocator_t* arena;
} skiplist_t;

// [SEQUENCE: 316] LSM 쓰기 경로
void lsm_put(lsm_tree_t* lsm, const char* key, size_t key_len,
             const char* value, size_t value_len) {
    // 1. WAL에 먼저 기록
    wal_append(&lsm->wal, key, key_len, value, value_len);
    
    // 2. Memtable에 삽입
    pthread_rwlock_wrlock(&lsm->memtable.lock);
    
    skiplist_insert(lsm->memtable.active, key, key_len, 
                   value, value_len, get_sequence_number());
    
    // 3. Memtable 크기 확인
    if (lsm->memtable.active->size >= lsm->memtable.size_limit) {
        // Immutable로 전환
        lsm->memtable.immutable = lsm->memtable.active;
        lsm->memtable.active = create_skiplist();
        
        // 백그라운드 플러시 트리거
        trigger_flush(lsm);
    }
    
    pthread_rwlock_unlock(&lsm->memtable.lock);
    
    atomic_fetch_add(&lsm->bytes_written, key_len + value_len);
}
```

### 2.2 SSTable 구현
```c
// [SEQUENCE: 317] SSTable 구조
typedef struct {
    // 파일 정보
    int fd;
    size_t file_size;
    
    // 메타데이터
    struct {
        char min_key[MAX_KEY_SIZE];
        char max_key[MAX_KEY_SIZE];
        uint64_t num_entries;
        uint64_t data_size;
        uint64_t index_offset;
        uint64_t bloom_offset;
    } meta;
    
    // 인덱스 (메모리)
    struct index_entry {
        char* key;
        uint64_t offset;
        uint32_t size;
    } *index;
    size_t index_size;
    
    // Bloom filter
    bloom_filter_t* bloom;
    
    // 압축 정보
    compression_type_t compression;
    
    // 캐시
    lru_cache_t* block_cache;
} sstable_t;

// [SEQUENCE: 318] SSTable 생성
sstable_t* create_sstable(const char* filename, 
                         skiplist_t* memtable) {
    sstable_t* sst = calloc(1, sizeof(sstable_t));
    
    // 파일 생성
    sst->fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    
    // Bloom filter 초기화
    sst->bloom = bloom_create(memtable->size * 10, 0.01);
    
    // 데이터 블록 쓰기
    write_data_blocks(sst, memtable);
    
    // 인덱스 블록 쓰기
    write_index_block(sst);
    
    // Bloom filter 쓰기
    write_bloom_filter(sst);
    
    // 메타데이터 쓰기
    write_metadata(sst);
    
    // 파일 동기화
    fsync(sst->fd);
    close(sst->fd);
    
    // 읽기 모드로 다시 열기
    sst->fd = open(filename, O_RDONLY);
    
    // 인덱스와 bloom filter 로드
    load_index(sst);
    load_bloom_filter(sst);
    
    return sst;
}

// [SEQUENCE: 319] 블록 기반 저장
#define BLOCK_SIZE 4096

typedef struct {
    uint32_t num_entries;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint8_t compression_type;
    char data[];
} data_block_t;

void write_data_blocks(sstable_t* sst, skiplist_t* memtable) {
    data_block_t* block = malloc(BLOCK_SIZE * 2);
    size_t block_offset = 0;
    size_t file_offset = 0;
    
    skiplist_node_t* node = memtable->header->forward[0];
    
    while (node) {
        // 엔트리 직렬화
        size_t entry_size = serialize_entry(block->data + block_offset,
                                           node);
        
        // 블록이 가득 찼으면
        if (block_offset + entry_size > BLOCK_SIZE) {
            // 압축
            size_t compressed_size = compress_block(block, block_offset);
            
            // 파일에 쓰기
            pwrite(sst->fd, block, compressed_size, file_offset);
            
            // 인덱스 업데이트
            add_index_entry(sst, node->key, file_offset, compressed_size);
            
            file_offset += compressed_size;
            block_offset = 0;
        }
        
        block_offset += entry_size;
        
        // Bloom filter 업데이트
        bloom_add(sst->bloom, node->key, node->key_len);
        
        node = node->forward[0];
    }
    
    // 마지막 블록 처리
    if (block_offset > 0) {
        size_t compressed_size = compress_block(block, block_offset);
        pwrite(sst->fd, block, compressed_size, file_offset);
    }
    
    free(block);
}
```

### 2.3 읽기 경로
```c
// [SEQUENCE: 320] LSM 읽기
char* lsm_get(lsm_tree_t* lsm, const char* key, size_t key_len,
              size_t* value_len) {
    // 1. Memtable 확인 (최신 데이터)
    pthread_rwlock_rdlock(&lsm->memtable.lock);
    
    char* value = skiplist_find(lsm->memtable.active, key, key_len);
    if (!value && lsm->memtable.immutable) {
        value = skiplist_find(lsm->memtable.immutable, key, key_len);
    }
    
    pthread_rwlock_unlock(&lsm->memtable.lock);
    
    if (value) {
        atomic_fetch_add(&lsm->bytes_read, *value_len);
        return value;
    }
    
    // 2. SSTable 확인 (레벨 순서대로)
    for (int level = 0; level < lsm->disk.num_levels; level++) {
        value = search_level(lsm, level, key, key_len, value_len);
        if (value) {
            return value;
        }
    }
    
    return NULL;  // 키 없음
}

// [SEQUENCE: 321] SSTable 검색
char* search_sstable(sstable_t* sst, const char* key, size_t key_len,
                    size_t* value_len) {
    // 1. 키 범위 확인
    if (strcmp(key, sst->meta.min_key) < 0 ||
        strcmp(key, sst->meta.max_key) > 0) {
        return NULL;
    }
    
    // 2. Bloom filter 확인
    if (!bloom_contains(sst->bloom, key, key_len)) {
        return NULL;  // 확실히 없음
    }
    
    // 3. 이진 탐색으로 인덱스 검색
    int left = 0, right = sst->index_size - 1;
    int target_idx = -1;
    
    while (left <= right) {
        int mid = (left + right) / 2;
        int cmp = strcmp(key, sst->index[mid].key);
        
        if (cmp == 0) {
            target_idx = mid;
            break;
        } else if (cmp < 0) {
            right = mid - 1;
        } else {
            left = mid + 1;
            target_idx = mid;  // 가장 가까운 작은 키
        }
    }
    
    if (target_idx < 0) return NULL;
    
    // 4. 데이터 블록 읽기
    data_block_t* block = read_block(sst, sst->index[target_idx].offset,
                                    sst->index[target_idx].size);
    
    // 5. 블록 내에서 검색
    char* value = search_in_block(block, key, key_len, value_len);
    
    free(block);
    return value;
}

// [SEQUENCE: 322] 블록 캐시
char* read_block_cached(sstable_t* sst, uint64_t offset, size_t size) {
    // 캐시 키 생성
    uint64_t cache_key = (uint64_t)sst->fd << 32 | offset;
    
    // 캐시 확인
    data_block_t* block = lru_get(sst->block_cache, cache_key);
    if (block) {
        return block;  // 캐시 히트
    }
    
    // 디스크에서 읽기
    block = malloc(size);
    pread(sst->fd, block, size, offset);
    
    // 압축 해제
    if (block->compression_type != COMP_NONE) {
        data_block_t* uncompressed = decompress_block(block);
        free(block);
        block = uncompressed;
    }
    
    // 캐시에 추가
    lru_put(sst->block_cache, cache_key, block, block->uncompressed_size);
    
    return block;
}
```

## 3. Compaction 전략

### 3.1 Leveled Compaction
```c
// [SEQUENCE: 323] Leveled Compaction 정책
typedef struct {
    size_t level0_file_num_trigger;    // 4
    size_t level0_slowdown_writes;     // 20
    size_t level0_stop_writes;         // 36
    
    size_t max_bytes_for_level_base;   // 256MB
    int max_bytes_for_level_multiplier; // 10
    
    size_t target_file_size_base;      // 64MB
    int target_file_size_multiplier;   // 1
} compaction_options_t;

// [SEQUENCE: 324] Compaction 선택
compaction_t* pick_compaction(lsm_tree_t* lsm) {
    compaction_t* c = calloc(1, sizeof(compaction_t));
    
    // Level 0 compaction (파일 수 기반)
    if (lsm->disk.level_counts[0] >= 
        lsm->options.level0_file_num_trigger) {
        c->level = 0;
        c->output_level = 1;
        
        // 모든 L0 파일 선택 (겹칠 수 있음)
        for (int i = 0; i < lsm->disk.level_counts[0]; i++) {
            add_input_file(c, 0, lsm->disk.tables[0][i]);
        }
        
        // L1에서 겹치는 파일 찾기
        find_overlapping_files(c, 1);
        
        return c;
    }
    
    // Level N compaction (크기 기반)
    for (int level = 1; level < lsm->disk.num_levels - 1; level++) {
        size_t level_bytes = calculate_level_bytes(lsm, level);
        size_t max_bytes = get_max_bytes_for_level(lsm, level);
        
        if (level_bytes > max_bytes) {
            c->level = level;
            c->output_level = level + 1;
            
            // 가장 오래된 파일 선택
            sstable_t* oldest = find_oldest_file(lsm, level);
            add_input_file(c, level, oldest);
            
            // 다음 레벨에서 겹치는 파일
            find_overlapping_files(c, level + 1);
            
            return c;
        }
    }
    
    free(c);
    return NULL;  // Compaction 불필요
}

// [SEQUENCE: 325] Compaction 실행
void run_compaction(lsm_tree_t* lsm, compaction_t* c) {
    // 병합 반복자 생성
    merge_iterator_t* iter = create_merge_iterator(c->inputs);
    
    // 새 SSTable 생성
    char filename[PATH_MAX];
    snprintf(filename, sizeof(filename), "level%d_%lu.sst",
             c->output_level, get_next_file_number());
    
    sstable_builder_t* builder = sstable_builder_create(filename);
    
    // 병합하면서 쓰기
    char *prev_key = NULL;
    while (merge_iterator_valid(iter)) {
        const char* key = merge_iterator_key(iter);
        const char* value = merge_iterator_value(iter);
        
        // 중복 제거 (최신 버전만 유지)
        if (!prev_key || strcmp(key, prev_key) != 0) {
            sstable_builder_add(builder, key, value);
            prev_key = strdup(key);
        }
        
        merge_iterator_next(iter);
    }
    
    // SSTable 완성
    sstable_t* new_sst = sstable_builder_finish(builder);
    
    // 원자적 교체
    install_compaction_results(lsm, c, new_sst);
    
    // 이전 파일 삭제
    delete_obsolete_files(c);
    
    free(prev_key);
    merge_iterator_destroy(iter);
    
    atomic_fetch_add(&lsm->compactions, 1);
}
```

### 3.2 Write-Ahead Log 최적화
```c
// [SEQUENCE: 326] 고성능 WAL
typedef struct {
    int fd;
    
    // Double buffering
    struct wal_buffer {
        char* data;
        size_t size;
        size_t capacity;
        atomic_bool ready;
    } buffers[2];
    
    atomic_int active_buffer;
    
    // 비동기 I/O
    struct io_uring ring;
    
    // 그룹 커밋
    pthread_mutex_t lock;
    pthread_cond_t sync_cond;
    size_t pending_syncs;
    
    // 체크섬
    bool checksum_enabled;
} wal_t;

// [SEQUENCE: 327] WAL 쓰기
void wal_append(wal_t* wal, const void* data, size_t size) {
    // 레코드 헤더
    wal_record_t record = {
        .length = size,
        .type = WAL_FULL_RECORD,
        .crc32 = 0
    };
    
    if (wal->checksum_enabled) {
        record.crc32 = crc32c(data, size);
    }
    
    // 활성 버퍼 선택
    int buf_idx = atomic_load(&wal->active_buffer);
    struct wal_buffer* buffer = &wal->buffers[buf_idx];
    
    pthread_mutex_lock(&wal->lock);
    
    // 공간 확인
    if (buffer->size + sizeof(record) + size > buffer->capacity) {
        // 버퍼 전환
        atomic_store(&buffer->ready, true);
        
        // io_uring으로 비동기 쓰기
        submit_wal_write(wal, buffer);
        
        // 다음 버퍼로 전환
        buf_idx = 1 - buf_idx;
        atomic_store(&wal->active_buffer, buf_idx);
        buffer = &wal->buffers[buf_idx];
        
        // 이전 쓰기 완료 대기
        wait_for_buffer(buffer);
        buffer->size = 0;
    }
    
    // 레코드 추가
    memcpy(buffer->data + buffer->size, &record, sizeof(record));
    buffer->size += sizeof(record);
    
    memcpy(buffer->data + buffer->size, data, size);
    buffer->size += size;
    
    pthread_mutex_unlock(&wal->lock);
}

// [SEQUENCE: 328] 그룹 커밋
void wal_sync(wal_t* wal) {
    pthread_mutex_lock(&wal->lock);
    
    wal->pending_syncs++;
    
    // 일정 수 이상 모이면 sync
    if (wal->pending_syncs >= 100 || 
        time_since_last_sync() > 10) {  // 10ms
        
        // 현재 버퍼 플러시
        int buf_idx = atomic_load(&wal->active_buffer);
        struct wal_buffer* buffer = &wal->buffers[buf_idx];
        
        if (buffer->size > 0) {
            submit_wal_write(wal, buffer);
            wait_for_completion(wal);
        }
        
        // 모든 대기중인 sync 깨우기
        wal->pending_syncs = 0;
        pthread_cond_broadcast(&wal->sync_cond);
    } else {
        // 다른 스레드의 sync 대기
        pthread_cond_wait(&wal->sync_cond, &wal->lock);
    }
    
    pthread_mutex_unlock(&wal->lock);
}
```

## 4. 압축과 인코딩

### 4.1 블록 압축
```c
// [SEQUENCE: 329] 압축 알고리즘 선택
typedef enum {
    COMP_NONE = 0,
    COMP_SNAPPY = 1,
    COMP_LZ4 = 2,
    COMP_ZSTD = 3,
    COMP_ADAPTIVE = 4
} compression_type_t;

typedef struct {
    compression_type_t type;
    int level;  // 압축 레벨 (zstd)
    
    // 통계
    atomic_uint64_t bytes_in;
    atomic_uint64_t bytes_out;
    atomic_uint64_t compress_time;
} compression_context_t;

// [SEQUENCE: 330] 적응형 압축
size_t compress_block_adaptive(compression_context_t* ctx,
                              const void* src, size_t src_size,
                              void* dst, size_t dst_capacity) {
    // 샘플링으로 압축률 예측
    size_t sample_size = MIN(src_size / 10, 1024);
    char sample_buf[2048];
    
    size_t lz4_size = LZ4_compress_default(src, sample_buf, 
                                           sample_size, sizeof(sample_buf));
    
    float lz4_ratio = (float)lz4_size / sample_size;
    
    // 압축률에 따라 알고리즘 선택
    compression_type_t selected;
    if (lz4_ratio > 0.9) {
        // 압축이 잘 안됨 - 압축 안함
        selected = COMP_NONE;
    } else if (lz4_ratio > 0.7) {
        // 적당한 압축 - LZ4 (빠름)
        selected = COMP_LZ4;
    } else {
        // 높은 압축 가능 - ZSTD (압축률 높음)
        selected = COMP_ZSTD;
    }
    
    uint64_t start = rdtsc();
    size_t compressed_size;
    
    switch (selected) {
        case COMP_NONE:
            memcpy(dst, src, src_size);
            compressed_size = src_size;
            break;
            
        case COMP_LZ4:
            compressed_size = LZ4_compress_default(src, dst + 1, 
                                                  src_size, dst_capacity - 1);
            *(uint8_t*)dst = COMP_LZ4;
            compressed_size++;
            break;
            
        case COMP_ZSTD:
            compressed_size = ZSTD_compress(dst + 1, dst_capacity - 1,
                                           src, src_size, ctx->level);
            *(uint8_t*)dst = COMP_ZSTD;
            compressed_size++;
            break;
    }
    
    uint64_t elapsed = rdtsc() - start;
    
    // 통계 업데이트
    atomic_fetch_add(&ctx->bytes_in, src_size);
    atomic_fetch_add(&ctx->bytes_out, compressed_size);
    atomic_fetch_add(&ctx->compress_time, elapsed);
    
    return compressed_size;
}

// [SEQUENCE: 331] 델타 인코딩
void encode_timestamps(uint64_t* timestamps, size_t count,
                      uint8_t* output, size_t* output_size) {
    if (count == 0) return;
    
    // 첫 타임스탬프는 그대로
    memcpy(output, &timestamps[0], sizeof(uint64_t));
    size_t offset = sizeof(uint64_t);
    
    // 나머지는 델타 인코딩
    uint64_t prev = timestamps[0];
    
    for (size_t i = 1; i < count; i++) {
        uint64_t delta = timestamps[i] - prev;
        
        // Varint 인코딩
        offset += encode_varint(output + offset, delta);
        
        prev = timestamps[i];
    }
    
    *output_size = offset;
}
```

## 5. 로그 특화 최적화

### 5.1 시계열 인덱싱
```c
// [SEQUENCE: 332] 시간 기반 파티셔닝
typedef struct {
    time_t start_time;
    time_t end_time;
    sstable_t* sst;
    
    // 시간 인덱스
    struct time_index {
        time_t timestamp;
        uint64_t offset;
    } *index;
    size_t index_size;
} time_partition_t;

typedef struct {
    time_partition_t** partitions;
    size_t num_partitions;
    
    // 핫 파티션 캐시
    lru_cache_t* hot_partitions;
    
    // 보존 정책
    retention_policy_t* policy;
} time_series_storage_t;

// [SEQUENCE: 333] 시간 범위 쿼리
void query_time_range(time_series_storage_t* storage,
                     time_t start, time_t end,
                     void (*callback)(log_entry_t*)) {
    // 관련 파티션 찾기
    int start_idx = find_partition(storage, start);
    int end_idx = find_partition(storage, end);
    
    for (int i = start_idx; i <= end_idx; i++) {
        time_partition_t* part = storage->partitions[i];
        
        // 시간 인덱스로 시작 위치 찾기
        size_t offset = binary_search_time(part->index, 
                                          part->index_size, start);
        
        // 순차 스캔
        sstable_iterator_t* iter = sstable_seek(part->sst, offset);
        
        while (sstable_iterator_valid(iter)) {
            log_entry_t* entry = sstable_iterator_value(iter);
            
            if (entry->timestamp > end) break;
            
            if (entry->timestamp >= start) {
                callback(entry);
            }
            
            sstable_iterator_next(iter);
        }
        
        sstable_iterator_destroy(iter);
    }
}

// [SEQUENCE: 334] 자동 보존 관리
void* retention_manager_thread(void* arg) {
    time_series_storage_t* storage = (time_series_storage_t*)arg;
    
    while (running) {
        time_t now = time(NULL);
        time_t cutoff = now - storage->policy->retention_seconds;
        
        // 만료된 파티션 찾기
        for (int i = 0; i < storage->num_partitions; i++) {
            time_partition_t* part = storage->partitions[i];
            
            if (part->end_time < cutoff) {
                // 파티션 삭제
                delete_partition(storage, i);
                
                log_info("Deleted expired partition: %s", 
                        format_time(part->end_time));
            }
        }
        
        // 하루에 한 번
        sleep(86400);
    }
    
    return NULL;
}
```

### 5.2 성능 모니터링
```c
// [SEQUENCE: 335] 스토리지 메트릭
typedef struct {
    // 쓰기 성능
    atomic_uint64_t write_ops;
    atomic_uint64_t write_bytes;
    atomic_uint64_t write_latency_us;
    
    // 읽기 성능
    atomic_uint64_t read_ops;
    atomic_uint64_t read_bytes;
    atomic_uint64_t read_latency_us;
    
    // Compaction
    atomic_uint64_t compaction_count;
    atomic_uint64_t compaction_bytes;
    atomic_uint64_t compaction_time_ms;
    
    // 공간 사용
    atomic_uint64_t total_size;
    atomic_uint64_t compressed_size;
    atomic_uint64_t num_files;
    
    // 캐시 효율
    atomic_uint64_t cache_hits;
    atomic_uint64_t cache_misses;
} storage_metrics_t;

void report_storage_metrics(lsm_tree_t* lsm) {
    storage_metrics_t* m = &lsm->metrics;
    
    printf("=== Log Storage Metrics ===\n");
    
    uint64_t write_ops = atomic_load(&m->write_ops);
    uint64_t write_bytes = atomic_load(&m->write_bytes);
    uint64_t write_latency = atomic_load(&m->write_latency_us);
    
    printf("Write Performance:\n");
    printf("  Throughput: %lu ops/sec, %s/sec\n",
           write_ops, format_bytes(write_bytes));
    printf("  Avg Latency: %.2f μs\n",
           (double)write_latency / write_ops);
    
    // 압축률
    uint64_t total = atomic_load(&m->total_size);
    uint64_t compressed = atomic_load(&m->compressed_size);
    printf("\nCompression:\n");
    printf("  Ratio: %.2f:1 (%lu%% saved)\n",
           (double)total / compressed,
           100 - (compressed * 100 / total));
    
    // 캐시 히트율
    uint64_t hits = atomic_load(&m->cache_hits);
    uint64_t misses = atomic_load(&m->cache_misses);
    printf("\nCache Performance:\n");
    printf("  Hit Rate: %.2f%%\n",
           100.0 * hits / (hits + misses));
    
    // Compaction 통계
    uint64_t comp_count = atomic_load(&m->compaction_count);
    uint64_t comp_bytes = atomic_load(&m->compaction_bytes);
    uint64_t comp_time = atomic_load(&m->compaction_time_ms);
    
    printf("\nCompaction:\n");
    printf("  Count: %lu\n", comp_count);
    printf("  Throughput: %s/sec\n",
           format_bytes(comp_bytes * 1000 / comp_time));
}
```

## 마무리

로그 구조화 스토리지로 달성한 개선사항:
1. **쓰기 성능**: 100,000+ ops/sec (100x 향상)
2. **공간 효율**: 3:1 압축률
3. **읽기 최적화**: 시간 기반 인덱싱
4. **운영 편의**: 자동 보존 관리

이는 대용량 로그 시스템의 핵심 기술입니다.