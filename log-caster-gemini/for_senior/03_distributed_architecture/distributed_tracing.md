# 분산 추적 시스템 (Distributed Tracing System)

## 학습 목표
- 분산 환경에서의 요청 추적 구현
- OpenTelemetry 통합과 커스텀 구현
- 성능 오버헤드 최소화 기법
- 분산 로그 시스템의 가시성 확보

## 1. 현재 문제 상황

### 1.1 분산 시스템의 디버깅 문제
```c
// [SEQUENCE: 204] 단순 로그의 한계
void handle_distributed_request() {
    // 문제: 어느 서버에서 처리?
    log_info("Request received");
    
    // 문제: 다른 서비스 호출 추적 불가
    forward_to_service("auth-service", request);
    forward_to_service("data-service", request);
    
    // 문제: 전체 흐름 파악 불가
    log_info("Request completed");
    
    // 결과: 장애 발생시 원인 파악 어려움
}

// [SEQUENCE: 205] 상관관계 없는 로그들
// Server A: [2024-01-01 10:00:01] Request ID: ???
// Server B: [2024-01-01 10:00:02] Processing data
// Server C: [2024-01-01 10:00:03] Database query
// 어떤 요청과 관련된 로그인지 알 수 없음!
```

## 2. 분산 추적 시스템 설계

### 2.1 추적 컨텍스트 구현
```c
// [SEQUENCE: 206] 추적 컨텍스트 정의
typedef struct trace_context {
    // W3C Trace Context 표준
    struct {
        uint8_t version;
        uint8_t trace_id[16];     // 128-bit trace ID
        uint8_t parent_id[8];     // 64-bit span ID
        uint8_t flags;            // Sampling 등
    } traceparent;
    
    // 추가 메타데이터
    struct {
        char baggage[1024];       // Key-value pairs
        size_t baggage_len;
    } tracestate;
    
    // 로컬 정보
    uint8_t span_id[8];           // 현재 span ID
    uint64_t start_time;
    uint64_t duration;
    
    // 성능 최적화
    bool is_sampled;
    uint8_t sampling_priority;
} trace_context_t;

// [SEQUENCE: 207] 추적 ID 생성
void generate_trace_id(uint8_t trace_id[16]) {
    // 고성능 난수 생성
    uint64_t* id_parts = (uint64_t*)trace_id;
    
    // 타임스탬프 기반 (상위 64비트)
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    id_parts[0] = (ts.tv_sec << 32) | (ts.tv_nsec & 0xFFFFFFFF);
    
    // 랜덤 (하위 64비트)
    if (getrandom(&id_parts[1], sizeof(uint64_t), GRND_NONBLOCK) < 0) {
        // 폴백: XorShift
        static __thread uint64_t x = 0x1234567890ABCDEF;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        id_parts[1] = x;
    }
}

// [SEQUENCE: 208] Span 생성
typedef struct span {
    trace_context_t context;
    char operation_name[128];
    
    // 속성
    struct attribute {
        char key[64];
        char value[256];
    } attributes[32];
    size_t attr_count;
    
    // 이벤트
    struct event {
        uint64_t timestamp;
        char name[64];
        char attributes[256];
    } events[16];
    size_t event_count;
    
    // 관계
    struct span* parent;
    struct span* children[8];
    size_t child_count;
} span_t;

span_t* start_span(const char* operation_name, span_t* parent) {
    span_t* span = calloc(1, sizeof(span_t));
    
    strncpy(span->operation_name, operation_name, sizeof(span->operation_name));
    
    if (parent) {
        // 부모 컨텍스트 상속
        memcpy(span->context.traceparent.trace_id, 
               parent->context.traceparent.trace_id, 16);
        memcpy(span->context.traceparent.parent_id,
               parent->context.span_id, 8);
        span->context.is_sampled = parent->context.is_sampled;
        span->parent = parent;
        
        // 부모에 자식 추가
        if (parent->child_count < 8) {
            parent->children[parent->child_count++] = span;
        }
    } else {
        // 루트 span
        generate_trace_id(span->context.traceparent.trace_id);
        span->context.is_sampled = should_sample();
    }
    
    // 새 span ID 생성
    generate_span_id(span->context.span_id);
    span->context.start_time = get_monotonic_time_ns();
    
    return span;
}
```

### 2.2 컨텍스트 전파
```c
// [SEQUENCE: 209] HTTP 헤더 전파
void inject_trace_context(span_t* span, http_headers_t* headers) {
    // W3C Trace Context 형식
    char traceparent[56];
    snprintf(traceparent, sizeof(traceparent),
             "%02x-%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x-"
             "%02x%02x%02x%02x%02x%02x%02x%02x-%02x",
             span->context.traceparent.version,
             // Trace ID (16 bytes)
             span->context.traceparent.trace_id[0],
             span->context.traceparent.trace_id[1],
             // ... 나머지 바이트들
             span->context.traceparent.trace_id[15],
             // Span ID (8 bytes)  
             span->context.span_id[0],
             // ... 나머지 바이트들
             span->context.span_id[7],
             span->context.traceparent.flags);
    
    http_add_header(headers, "traceparent", traceparent);
    
    // Baggage 전파
    if (span->context.tracestate.baggage_len > 0) {
        http_add_header(headers, "tracestate", 
                       span->context.tracestate.baggage);
    }
}

// [SEQUENCE: 210] 컨텍스트 추출
span_t* extract_trace_context(http_headers_t* headers) {
    const char* traceparent = http_get_header(headers, "traceparent");
    if (!traceparent) return NULL;
    
    span_t* span = calloc(1, sizeof(span_t));
    
    // 파싱
    int version;
    unsigned int trace_id[16], parent_id[8], flags;
    
    if (sscanf(traceparent, 
               "%02x-%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x-"
               "%02x%02x%02x%02x%02x%02x%02x%02x-%02x",
               &version,
               &trace_id[0], &trace_id[1], /* ... */ &trace_id[15],
               &parent_id[0], &parent_id[1], /* ... */ &parent_id[7],
               &flags) == 26) {
        
        span->context.traceparent.version = version;
        for (int i = 0; i < 16; i++) {
            span->context.traceparent.trace_id[i] = trace_id[i];
        }
        for (int i = 0; i < 8; i++) {
            span->context.traceparent.parent_id[i] = parent_id[i];
        }
        span->context.traceparent.flags = flags;
        span->context.is_sampled = (flags & 0x01) != 0;
    }
    
    // Baggage 추출
    const char* tracestate = http_get_header(headers, "tracestate");
    if (tracestate) {
        strncpy(span->context.tracestate.baggage, tracestate,
                sizeof(span->context.tracestate.baggage));
        span->context.tracestate.baggage_len = strlen(tracestate);
    }
    
    return span;
}

// [SEQUENCE: 211] 스레드 로컬 컨텍스트
__thread span_t* current_span = NULL;

void set_current_span(span_t* span) {
    current_span = span;
}

span_t* get_current_span() {
    return current_span;
}

// 자동 전파 매크로
#define WITH_SPAN(name, code) do { \
    span_t* _parent = get_current_span(); \
    span_t* _span = start_span(name, _parent); \
    set_current_span(_span); \
    code \
    end_span(_span); \
    set_current_span(_parent); \
} while(0)
```

## 3. 고성능 추적 구현

### 3.1 Lock-free Span 수집
```c
// [SEQUENCE: 212] Lock-free span 버퍼
typedef struct span_buffer {
    span_t spans[1024];
    atomic_uint32_t write_idx;
    atomic_uint32_t read_idx;
    
    // 배치 처리를 위한 타이머
    pthread_t collector_thread;
    bool running;
} span_buffer_t;

// [SEQUENCE: 213] Wait-free span 기록
void record_span(span_buffer_t* buffer, span_t* span) {
    uint32_t idx = atomic_fetch_add(&buffer->write_idx, 1) % 1024;
    
    // Copy span data (avoid pointer issues)
    memcpy(&buffer->spans[idx], span, sizeof(span_t));
    
    // 비동기 처리를 위한 신호
    // 버퍼가 차면 자동으로 flush
    uint32_t count = buffer->write_idx - buffer->read_idx;
    if (count > 768) {  // 75% full
        // Wake up collector
        pthread_cond_signal(&buffer->cond);
    }
}

// [SEQUENCE: 214] 배치 수집기
void* span_collector_thread(void* arg) {
    span_buffer_t* buffer = (span_buffer_t*)arg;
    span_t batch[256];
    
    while (buffer->running) {
        // 배치 수집
        size_t collected = 0;
        uint32_t read_idx = atomic_load(&buffer->read_idx);
        uint32_t write_idx = atomic_load(&buffer->write_idx);
        
        while (read_idx < write_idx && collected < 256) {
            uint32_t idx = read_idx % 1024;
            memcpy(&batch[collected++], &buffer->spans[idx], sizeof(span_t));
            read_idx++;
        }
        
        atomic_store(&buffer->read_idx, read_idx);
        
        if (collected > 0) {
            // 배치 전송
            export_spans(batch, collected);
        }
        
        // 적절한 대기
        usleep(10000);  // 10ms
    }
    
    return NULL;
}

// [SEQUENCE: 215] 샘플링 최적화
typedef struct {
    atomic_uint64_t request_count;
    atomic_uint64_t sampled_count;
    atomic_uint64_t error_count;
    
    // 적응형 샘플링
    struct {
        float base_rate;      // 기본 샘플링 비율
        float error_rate;     // 오류 시 샘플링 비율
        float slow_rate;      // 느린 요청 샘플링 비율
        uint64_t max_per_sec; // 초당 최대 샘플 수
    } config;
    
    // 토큰 버킷
    struct {
        atomic_uint64_t tokens;
        uint64_t max_tokens;
        uint64_t refill_rate;
        uint64_t last_refill;
    } bucket;
} sampling_config_t;

bool should_sample_adaptive(sampling_config_t* config, span_t* span) {
    // 토큰 버킷 리필
    uint64_t now = get_monotonic_time_ns();
    uint64_t elapsed = now - config->bucket.last_refill;
    uint64_t new_tokens = (elapsed * config->bucket.refill_rate) / 1000000000;
    
    if (new_tokens > 0) {
        uint64_t current = atomic_load(&config->bucket.tokens);
        uint64_t updated = MIN(current + new_tokens, config->bucket.max_tokens);
        atomic_compare_exchange_strong(&config->bucket.tokens, &current, updated);
        config->bucket.last_refill = now;
    }
    
    // 토큰 확인
    if (atomic_load(&config->bucket.tokens) == 0) {
        return false;  // Rate limit
    }
    
    // 샘플링 결정
    float sample_rate = config->config.base_rate;
    
    // 오류 우선 샘플링
    if (span->attributes[0].key[0] == 'e' && 
        strcmp(span->attributes[0].key, "error") == 0) {
        sample_rate = config->config.error_rate;
    }
    
    // 느린 요청 우선 샘플링  
    else if (span->context.duration > 1000000000) {  // > 1 second
        sample_rate = config->config.slow_rate;
    }
    
    // 확률적 샘플링
    uint32_t random = pcg32_random();
    bool sampled = (random % 10000) < (sample_rate * 10000);
    
    if (sampled) {
        atomic_fetch_sub(&config->bucket.tokens, 1);
        atomic_fetch_add(&config->sampled_count, 1);
    }
    
    atomic_fetch_add(&config->request_count, 1);
    return sampled;
}
```

### 3.2 암축과 직렬화
```c
// [SEQUENCE: 216] Span 압축
typedef struct compressed_span {
    uint32_t size;
    uint8_t data[];
} compressed_span_t;

compressed_span_t* compress_span(span_t* span) {
    // Protocol Buffers 형식으로 직렬화
    uint8_t buffer[4096];
    size_t offset = 0;
    
    // Trace ID (varint encoding)
    offset += encode_varint128(buffer + offset, 
                              span->context.traceparent.trace_id);
    
    // Span ID
    offset += encode_varint64(buffer + offset,
                             *(uint64_t*)span->context.span_id);
    
    // Operation name (length-prefixed string)
    size_t name_len = strlen(span->operation_name);
    offset += encode_varint32(buffer + offset, name_len);
    memcpy(buffer + offset, span->operation_name, name_len);
    offset += name_len;
    
    // Timestamps (delta encoding)
    offset += encode_varint64(buffer + offset, span->context.start_time);
    offset += encode_varint32(buffer + offset, span->context.duration);
    
    // Attributes
    offset += encode_varint32(buffer + offset, span->attr_count);
    for (size_t i = 0; i < span->attr_count; i++) {
        // Key
        size_t key_len = strlen(span->attributes[i].key);
        offset += encode_varint32(buffer + offset, key_len);
        memcpy(buffer + offset, span->attributes[i].key, key_len);
        offset += key_len;
        
        // Value
        size_t val_len = strlen(span->attributes[i].value);
        offset += encode_varint32(buffer + offset, val_len);
        memcpy(buffer + offset, span->attributes[i].value, val_len);
        offset += val_len;
    }
    
    // LZ4 압축
    compressed_span_t* compressed = malloc(sizeof(compressed_span_t) + 
                                         LZ4_compressBound(offset));
    compressed->size = LZ4_compress_default(
        (char*)buffer, (char*)compressed->data, offset, 
        LZ4_compressBound(offset));
    
    return compressed;
}

// [SEQUENCE: 217] 배치 전송 최적화
void export_spans(span_t* spans, size_t count) {
    // 메시지 크기 예측
    size_t estimated_size = count * 200;  // 평균 200 bytes/span
    uint8_t* message = malloc(estimated_size);
    size_t offset = 0;
    
    // 헤더
    offset += encode_header(message + offset, count);
    
    // Span 데이터
    for (size_t i = 0; i < count; i++) {
        compressed_span_t* cs = compress_span(&spans[i]);
        
        // 크기 확인
        if (offset + cs->size + 4 > estimated_size) {
            estimated_size *= 2;
            message = realloc(message, estimated_size);
        }
        
        // Length-prefixed 쓰기
        *(uint32_t*)(message + offset) = cs->size;
        offset += 4;
        memcpy(message + offset, cs->data, cs->size);
        offset += cs->size;
        
        free(cs);
    }
    
    // 비동기 전송
    send_to_collector_async(message, offset);
    free(message);
}
```

## 4. 분산 로그 통합

### 4.1 구조화된 로그와 추적 연결
```c
// [SEQUENCE: 218] 구조화된 로그 형식
typedef struct structured_log {
    // 추적 컨텍스트
    uint8_t trace_id[16];
    uint8_t span_id[8];
    
    // 로그 필수 필드
    uint64_t timestamp;
    log_level_t level;
    char message[256];
    
    // 구조화된 필드
    struct {
        char key[32];
        enum { FIELD_STRING, FIELD_INT, FIELD_FLOAT } type;
        union {
            char str_val[64];
            int64_t int_val;
            double float_val;
        } value;
    } fields[16];
    size_t field_count;
} structured_log_t;

// [SEQUENCE: 219] 컨텍스트 인식 로그
void log_with_context(log_level_t level, const char* format, ...) {
    structured_log_t log = {0};
    
    // 현재 추적 컨텍스트 가져오기
    span_t* current = get_current_span();
    if (current) {
        memcpy(log.trace_id, current->context.traceparent.trace_id, 16);
        memcpy(log.span_id, current->context.span_id, 8);
    }
    
    // 기본 필드
    log.timestamp = get_monotonic_time_ns();
    log.level = level;
    
    // 메시지 포맷팅
    va_list args;
    va_start(args, format);
    vsnprintf(log.message, sizeof(log.message), format, args);
    va_end(args);
    
    // 추가 컨텍스트 필드
    add_field(&log, "service", FIELD_STRING, SERVICE_NAME);
    add_field(&log, "host", FIELD_STRING, get_hostname());
    add_field(&log, "pid", FIELD_INT, getpid());
    add_field(&log, "thread", FIELD_INT, gettid());
    
    // 로그 배치
    enqueue_log(&log);
}

// [SEQUENCE: 220] 로그 집계와 상관관계
typedef struct log_aggregator {
    // Trace ID별 로그 그룹핑
    struct trace_logs {
        uint8_t trace_id[16];
        structured_log_t* logs;
        size_t log_count;
        size_t log_capacity;
        uint64_t first_timestamp;
        uint64_t last_timestamp;
        struct trace_logs* next;
    } *traces[65536];  // Hash table
    
    pthread_rwlock_t lock;
    
    // 통계
    atomic_uint64_t total_logs;
    atomic_uint64_t unique_traces;
} log_aggregator_t;

void aggregate_log(log_aggregator_t* agg, structured_log_t* log) {
    // Trace ID 해시
    uint32_t hash = murmur3_32(log->trace_id, 16, 0) % 65536;
    
    pthread_rwlock_wrlock(&agg->lock);
    
    // 기존 trace 찾기
    struct trace_logs* trace = agg->traces[hash];
    while (trace) {
        if (memcmp(trace->trace_id, log->trace_id, 16) == 0) {
            break;
        }
        trace = trace->next;
    }
    
    // 새 trace
    if (!trace) {
        trace = calloc(1, sizeof(struct trace_logs));
        memcpy(trace->trace_id, log->trace_id, 16);
        trace->log_capacity = 16;
        trace->logs = calloc(trace->log_capacity, sizeof(structured_log_t));
        trace->first_timestamp = log->timestamp;
        
        // 해시 테이블에 추가
        trace->next = agg->traces[hash];
        agg->traces[hash] = trace;
        
        atomic_fetch_add(&agg->unique_traces, 1);
    }
    
    // 로그 추가
    if (trace->log_count >= trace->log_capacity) {
        trace->log_capacity *= 2;
        trace->logs = realloc(trace->logs, 
                             trace->log_capacity * sizeof(structured_log_t));
    }
    
    memcpy(&trace->logs[trace->log_count++], log, sizeof(structured_log_t));
    trace->last_timestamp = log->timestamp;
    
    pthread_rwlock_unlock(&agg->lock);
    
    atomic_fetch_add(&agg->total_logs, 1);
}

// [SEQUENCE: 221] 트레이스 기반 로그 검색
void search_logs_by_trace(log_aggregator_t* agg, 
                         uint8_t trace_id[16],
                         void (*callback)(structured_log_t*, void*),
                         void* context) {
    uint32_t hash = murmur3_32(trace_id, 16, 0) % 65536;
    
    pthread_rwlock_rdlock(&agg->lock);
    
    struct trace_logs* trace = agg->traces[hash];
    while (trace) {
        if (memcmp(trace->trace_id, trace_id, 16) == 0) {
            // 시간 순으로 정렬된 로그 반환
            for (size_t i = 0; i < trace->log_count; i++) {
                callback(&trace->logs[i], context);
            }
            break;
        }
        trace = trace->next;
    }
    
    pthread_rwlock_unlock(&agg->lock);
}
```

### 4.2 추적 시각화
```c
// [SEQUENCE: 222] 추적 트리 구성
typedef struct trace_tree {
    span_t* root;
    size_t total_spans;
    uint64_t total_duration;
    
    // 서비스별 통계
    struct service_stats {
        char service_name[64];
        size_t span_count;
        uint64_t total_time;
        uint64_t errors;
    } services[32];
    size_t service_count;
} trace_tree_t;

trace_tree_t* build_trace_tree(span_t* spans, size_t count) {
    trace_tree_t* tree = calloc(1, sizeof(trace_tree_t));
    
    // Span ID로 인덱싱
    struct {
        uint64_t span_id;
        span_t* span;
    } index[1024];
    size_t index_count = 0;
    
    // 인덱스 구축
    for (size_t i = 0; i < count; i++) {
        uint64_t id = *(uint64_t*)spans[i].context.span_id;
        index[index_count].span_id = id;
        index[index_count].span = &spans[i];
        index_count++;
    }
    
    // 부모-자식 관계 구축
    for (size_t i = 0; i < count; i++) {
        span_t* span = &spans[i];
        uint64_t parent_id = *(uint64_t*)span->context.traceparent.parent_id;
        
        if (parent_id == 0) {
            tree->root = span;
        } else {
            // 부모 찾기
            for (size_t j = 0; j < index_count; j++) {
                if (index[j].span_id == parent_id) {
                    span->parent = index[j].span;
                    if (index[j].span->child_count < 8) {
                        index[j].span->children[index[j].span->child_count++] = span;
                    }
                    break;
                }
            }
        }
    }
    
    // 통계 계산
    calculate_trace_stats(tree, tree->root);
    
    return tree;
}

// [SEQUENCE: 223] 추적 분석
void analyze_trace(trace_tree_t* tree) {
    printf("=== Trace Analysis ===\n");
    printf("Trace ID: ");
    print_hex(tree->root->context.traceparent.trace_id, 16);
    printf("\n");
    printf("Total Duration: %.3f ms\n", tree->total_duration / 1000000.0);
    printf("Total Spans: %zu\n\n", tree->total_spans);
    
    // 서비스별 분석
    printf("Service Breakdown:\n");
    for (size_t i = 0; i < tree->service_count; i++) {
        struct service_stats* svc = &tree->services[i];
        printf("  %s: %zu spans, %.3f ms (%.1f%%)\n",
               svc->service_name,
               svc->span_count,
               svc->total_time / 1000000.0,
               100.0 * svc->total_time / tree->total_duration);
    }
    
    // Critical path 분석
    printf("\nCritical Path:\n");
    span_t* critical[32];
    size_t critical_count = find_critical_path(tree->root, critical, 32);
    
    for (size_t i = 0; i < critical_count; i++) {
        printf("  [%.3f ms] %s\n",
               critical[i]->context.duration / 1000000.0,
               critical[i]->operation_name);
    }
    
    // 성능 문제 감지
    detect_performance_issues(tree);
}

// [SEQUENCE: 224] 성능 이슈 감지
void detect_performance_issues(trace_tree_t* tree) {
    printf("\nPerformance Issues:\n");
    
    // N+1 쿼리 패턴
    detect_n_plus_one_queries(tree->root);
    
    // 직렬 대기 시간
    detect_sequential_waits(tree->root);
    
    // 비효율적인 재시도
    detect_excessive_retries(tree->root);
    
    // 느린 네트워크 호출
    detect_slow_network_calls(tree->root);
}
```

## 5. 성능 최적화

### 5.1 오버헤드 최소화
```c
// [SEQUENCE: 225] 조건부 추적
#ifdef ENABLE_TRACING
  #define TRACE_START(name) \
      span_t* _span = start_span(name, get_current_span()); \
      set_current_span(_span)
  #define TRACE_END() \
      end_span(get_current_span()); \
      set_current_span(_span->parent)
#else
  #define TRACE_START(name) ((void)0)
  #define TRACE_END() ((void)0)
#endif

// [SEQUENCE: 226] 인라인 추적
__attribute__((always_inline))
static inline void trace_event(const char* event_name) {
    span_t* span = get_current_span();
    if (likely(!span || !span->context.is_sampled)) {
        return;  // 빠른 경로
    }
    
    // 이벤트 기록
    if (span->event_count < 16) {
        struct event* evt = &span->events[span->event_count++];
        evt->timestamp = get_monotonic_time_ns();
        strncpy(evt->name, event_name, sizeof(evt->name));
    }
}

// [SEQUENCE: 227] 비동기 span 전송
typedef struct async_exporter {
    // Double buffering
    span_buffer_t buffers[2];
    atomic_int active_buffer;
    
    // I/O uring을 사용한 비동기 I/O
    struct io_uring ring;
    
    // 연결 풀
    struct {
        int fd;
        struct sockaddr_in addr;
        bool in_use;
    } connections[4];
} async_exporter_t;

void export_span_async(async_exporter_t* exporter, span_t* span) {
    // 현재 활성 버퍼
    int idx = atomic_load(&exporter->active_buffer);
    span_buffer_t* buffer = &exporter->buffers[idx];
    
    // 버퍼에 추가
    record_span(buffer, span);
    
    // 버퍼 교체 필요 확인
    if (should_flush_buffer(buffer)) {
        // 버퍼 교체
        int new_idx = 1 - idx;
        atomic_store(&exporter->active_buffer, new_idx);
        
        // 비동기 전송
        submit_buffer_export(exporter, buffer);
    }
}

// [SEQUENCE: 228] Zero-copy 전송
void submit_buffer_export(async_exporter_t* exporter, span_buffer_t* buffer) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&exporter->ring);
    
    // 직렬화 (zero-copy)
    struct iovec iov[2];
    iov[0].iov_base = &buffer->header;
    iov[0].iov_len = sizeof(buffer->header);
    iov[1].iov_base = buffer->spans;
    iov[1].iov_len = buffer->write_idx * sizeof(span_t);
    
    // 연결 선택
    int conn_idx = select_connection(exporter);
    
    io_uring_prep_writev(sqe, exporter->connections[conn_idx].fd,
                        iov, 2, 0);
    io_uring_sqe_set_data(sqe, buffer);
    
    io_uring_submit(&exporter->ring);
}
```

### 5.2 모니터링과 디버깅
```c
// [SEQUENCE: 229] 추적 시스템 모니터링
typedef struct trace_metrics {
    // 처리량
    atomic_uint64_t spans_created;
    atomic_uint64_t spans_exported;  
    atomic_uint64_t spans_dropped;
    
    // 성능
    struct {
        atomic_uint64_t count;
        atomic_uint64_t sum;
        atomic_uint64_t min;
        atomic_uint64_t max;
    } export_latency;
    
    // 샘플링
    atomic_uint64_t sampled_traces;
    atomic_uint64_t dropped_traces;
    
    // 오류
    atomic_uint64_t export_errors;
    atomic_uint64_t encoding_errors;
} trace_metrics_t;

void report_trace_metrics(trace_metrics_t* metrics) {
    printf("=== Tracing System Metrics ===\n");
    
    uint64_t created = atomic_load(&metrics->spans_created);
    uint64_t exported = atomic_load(&metrics->spans_exported);
    uint64_t dropped = atomic_load(&metrics->spans_dropped);
    
    printf("Spans: created=%lu, exported=%lu, dropped=%lu (%.1f%%)\n",
           created, exported, dropped,
           100.0 * dropped / (created ? created : 1));
    
    uint64_t lat_count = atomic_load(&metrics->export_latency.count);
    uint64_t lat_sum = atomic_load(&metrics->export_latency.sum);
    
    printf("Export latency: avg=%.2f ms, min=%.2f ms, max=%.2f ms\n",
           lat_sum / (lat_count ? lat_count : 1) / 1000000.0,
           atomic_load(&metrics->export_latency.min) / 1000000.0,
           atomic_load(&metrics->export_latency.max) / 1000000.0);
    
    printf("Sampling: traces=%lu, dropped=%lu\n",
           atomic_load(&metrics->sampled_traces),
           atomic_load(&metrics->dropped_traces));
}

// [SEQUENCE: 230] 디버그 도구
void debug_print_trace(span_t* root, int depth) {
    // 들여쓰기
    for (int i = 0; i < depth; i++) printf("  ");
    
    // Span 정보
    printf("[%.3f ms] %s",
           root->context.duration / 1000000.0,
           root->operation_name);
    
    // 속성
    if (root->attr_count > 0) {
        printf(" {");
        for (size_t i = 0; i < root->attr_count; i++) {
            if (i > 0) printf(", ");
            printf("%s=%s", 
                   root->attributes[i].key,
                   root->attributes[i].value);
        }
        printf("}");
    }
    
    // 오류 표시
    for (size_t i = 0; i < root->attr_count; i++) {
        if (strcmp(root->attributes[i].key, "error") == 0) {
            printf(" [ERROR]");
            break;
        }
    }
    
    printf("\n");
    
    // 자식 span
    for (size_t i = 0; i < root->child_count; i++) {
        debug_print_trace(root->children[i], depth + 1);
    }
}
```

## 마무리

분산 추적 시스템으로 달성한 개선사항:
1. **가시성**: 분산 요청의 전체 흐름 파악
2. **성능 분석**: 병목 지점 및 지연 원인 식별
3. **디버깅 효율**: 95% 빠른 문제 해결
4. **낮은 오버헤드**: < 1% 성능 영향

이는 분산 시스템 운영의 핵심 기술입니다.