# Raft 합의 알고리즘 (Raft Consensus Algorithm)

## 학습 목표
- Raft 합의 알고리즘의 원리와 구현
- 리더 선출과 로그 복제 메커니즘
- 분산 로그 시스템의 일관성 보장
- 네트워크 파티션과 장애 처리

## 1. 현재 문제 상황

### 1.1 단일 장애점 문제
```c
// [SEQUENCE: 148] 현재 로그 서버의 문제
typedef struct single_log_server {
    int listen_fd;
    log_buffer_t* buffer;
    // 문제: 서버 장애시 모든 로그 손실
    // 문제: 확장성 제한
    // 문제: 고가용성 불가능
} single_log_server_t;

// [SEQUENCE: 149] 나이브한 복제 시도
void naive_replication(log_entry_t* entry) {
    // 문제 1: 순서 보장 안됨
    send_to_replica1(entry);
    send_to_replica2(entry);
    send_to_replica3(entry);
    
    // 문제 2: 일부만 성공시?
    // 문제 3: 네트워크 파티션시?
    // 문제 4: 리더는 누구?
}
```

## 2. Raft 알고리즘 구현

### 2.1 핵심 데이터 구조
```c
// [SEQUENCE: 150] Raft 노드 상태
typedef enum {
    FOLLOWER,
    CANDIDATE,
    LEADER
} node_state_t;

typedef struct {
    uint64_t term;
    char node_id[32];
} vote_t;

typedef struct {
    uint64_t term;
    uint64_t index;
    uint64_t timestamp;
    char data[LOG_ENTRY_SIZE];
    bool committed;
} raft_log_entry_t;

typedef struct raft_node {
    // 영속 상태 (디스크 저장)
    uint64_t current_term;
    vote_t voted_for;
    raft_log_entry_t* log;
    size_t log_count;
    size_t log_capacity;
    
    // 휘발성 상태
    node_state_t state;
    uint64_t commit_index;
    uint64_t last_applied;
    
    // 리더 전용
    uint64_t* next_index;    // 각 팔로워의 다음 인덱스
    uint64_t* match_index;   // 각 팔로워의 일치 인덱스
    
    // 선거 관련
    struct timespec election_timeout;
    int votes_received;
    
    // 클러스터 정보
    char node_id[32];
    peer_t* peers;
    size_t num_peers;
    
    // 동기화
    pthread_mutex_t lock;
    pthread_cond_t apply_cond;
} raft_node_t;

// [SEQUENCE: 151] RPC 메시지 정의
typedef struct {
    uint64_t term;
    char candidate_id[32];
    uint64_t last_log_index;
    uint64_t last_log_term;
} request_vote_t;

typedef struct {
    uint64_t term;
    bool vote_granted;
} request_vote_response_t;

typedef struct {
    uint64_t term;
    char leader_id[32];
    uint64_t prev_log_index;
    uint64_t prev_log_term;
    raft_log_entry_t* entries;
    size_t entry_count;
    uint64_t leader_commit;
} append_entries_t;

typedef struct {
    uint64_t term;
    bool success;
    uint64_t match_index;
} append_entries_response_t;
```

### 2.2 리더 선출
```c
// [SEQUENCE: 152] 선거 타임아웃과 상태 전환
void* raft_timer_thread(void* arg) {
    raft_node_t* node = (raft_node_t*)arg;
    
    while (running) {
        pthread_mutex_lock(&node->lock);
        
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        
        if (node->state != LEADER && 
            timespec_compare(&now, &node->election_timeout) > 0) {
            // 타임아웃 - 선거 시작
            start_election(node);
        }
        
        pthread_mutex_unlock(&node->lock);
        
        usleep(10000);  // 10ms
    }
    
    return NULL;
}

// [SEQUENCE: 153] 선거 시작
void start_election(raft_node_t* node) {
    // Candidate로 전환
    node->state = CANDIDATE;
    node->current_term++;
    node->voted_for.term = node->current_term;
    strcpy(node->voted_for.node_id, node->node_id);
    node->votes_received = 1;  // 자신에게 투표
    
    // 새 선거 타임아웃 설정 (150-300ms 랜덤)
    reset_election_timeout(node);
    
    // 영속 상태 저장
    persist_state(node);
    
    log_info("Node %s starting election for term %lu", 
             node->node_id, node->current_term);
    
    // 모든 노드에게 투표 요청
    request_vote_t request = {
        .term = node->current_term,
        .last_log_index = get_last_log_index(node),
        .last_log_term = get_last_log_term(node)
    };
    strcpy(request.candidate_id, node->node_id);
    
    for (size_t i = 0; i < node->num_peers; i++) {
        send_request_vote_async(node, &node->peers[i], &request);
    }
}

// [SEQUENCE: 154] 투표 요청 처리
void handle_request_vote(raft_node_t* node, 
                        const request_vote_t* request,
                        request_vote_response_t* response) {
    pthread_mutex_lock(&node->lock);
    
    response->term = node->current_term;
    response->vote_granted = false;
    
    // 요청자의 term이 더 높으면 follower로 전환
    if (request->term > node->current_term) {
        become_follower(node, request->term);
    }
    
    // 투표 조건 확인
    if (request->term < node->current_term) {
        // 구식 term
        pthread_mutex_unlock(&node->lock);
        return;
    }
    
    // 이미 투표했는지 확인
    bool can_vote = (node->voted_for.term < request->term) ||
                   (node->voted_for.term == request->term &&
                    strcmp(node->voted_for.node_id, request->candidate_id) == 0);
    
    // 로그가 최소한 자신만큼 최신인지 확인
    uint64_t last_log_index = get_last_log_index(node);
    uint64_t last_log_term = get_last_log_term(node);
    
    bool log_ok = (request->last_log_term > last_log_term) ||
                 (request->last_log_term == last_log_term &&
                  request->last_log_index >= last_log_index);
    
    if (can_vote && log_ok) {
        node->voted_for.term = request->term;
        strcpy(node->voted_for.node_id, request->candidate_id);
        response->vote_granted = true;
        
        reset_election_timeout(node);
        persist_state(node);
        
        log_info("Node %s voted for %s in term %lu",
                node->node_id, request->candidate_id, request->term);
    }
    
    pthread_mutex_unlock(&node->lock);
}

// [SEQUENCE: 155] 투표 응답 처리
void handle_vote_response(raft_node_t* node,
                         const char* peer_id,
                         const request_vote_response_t* response) {
    pthread_mutex_lock(&node->lock);
    
    if (response->term > node->current_term) {
        become_follower(node, response->term);
        pthread_mutex_unlock(&node->lock);
        return;
    }
    
    if (node->state != CANDIDATE || response->term != node->current_term) {
        pthread_mutex_unlock(&node->lock);
        return;
    }
    
    if (response->vote_granted) {
        node->votes_received++;
        
        // 과반수 확인
        if (node->votes_received > (node->num_peers + 1) / 2) {
            become_leader(node);
        }
    }
    
    pthread_mutex_unlock(&node->lock);
}
```

### 2.3 로그 복제
```c
// [SEQUENCE: 156] 리더의 로그 복제
void become_leader(raft_node_t* node) {
    node->state = LEADER;
    
    log_info("Node %s became leader for term %lu",
            node->node_id, node->current_term);
    
    // next_index와 match_index 초기화
    uint64_t last_log_index = get_last_log_index(node);
    for (size_t i = 0; i < node->num_peers; i++) {
        node->next_index[i] = last_log_index + 1;
        node->match_index[i] = 0;
    }
    
    // 즉시 하트비트 전송
    send_heartbeats(node);
    
    // no-op 엔트리 추가 (리더십 확립)
    raft_log_entry_t noop = {
        .term = node->current_term,
        .index = last_log_index + 1,
        .timestamp = get_timestamp(),
        .data = "NOOP",
        .committed = false
    };
    append_log_entry(node, &noop);
}

// [SEQUENCE: 157] 클라이언트 요청 처리
bool raft_append(raft_node_t* node, const char* data, size_t len) {
    pthread_mutex_lock(&node->lock);
    
    if (node->state != LEADER) {
        pthread_mutex_unlock(&node->lock);
        return false;  // 리더가 아님
    }
    
    // 새 로그 엔트리 생성
    raft_log_entry_t entry = {
        .term = node->current_term,
        .index = get_last_log_index(node) + 1,
        .timestamp = get_timestamp(),
        .committed = false
    };
    memcpy(entry.data, data, MIN(len, LOG_ENTRY_SIZE));
    
    // 로컬 로그에 추가
    append_log_entry(node, &entry);
    
    pthread_mutex_unlock(&node->lock);
    
    // 팔로워들에게 복제
    replicate_log_entries(node);
    
    // 커밋될 때까지 대기
    return wait_for_commit(node, entry.index);
}

// [SEQUENCE: 158] AppendEntries RPC
void send_append_entries(raft_node_t* node, peer_t* peer) {
    pthread_mutex_lock(&node->lock);
    
    uint64_t next_idx = node->next_index[peer->index];
    uint64_t prev_idx = next_idx - 1;
    
    append_entries_t request = {
        .term = node->current_term,
        .prev_log_index = prev_idx,
        .prev_log_term = get_log_term(node, prev_idx),
        .leader_commit = node->commit_index
    };
    strcpy(request.leader_id, node->node_id);
    
    // 전송할 엔트리 수집
    size_t count = 0;
    raft_log_entry_t entries[MAX_ENTRIES_PER_RPC];
    
    for (uint64_t i = next_idx; i <= get_last_log_index(node) && 
         count < MAX_ENTRIES_PER_RPC; i++) {
        entries[count++] = node->log[i - 1];  // 0-indexed
    }
    
    request.entries = entries;
    request.entry_count = count;
    
    pthread_mutex_unlock(&node->lock);
    
    // RPC 전송
    send_append_entries_rpc(peer, &request);
}

// [SEQUENCE: 159] AppendEntries 응답 처리
void handle_append_entries_response(raft_node_t* node,
                                   peer_t* peer,
                                   const append_entries_response_t* response) {
    pthread_mutex_lock(&node->lock);
    
    if (response->term > node->current_term) {
        become_follower(node, response->term);
        pthread_mutex_unlock(&node->lock);
        return;
    }
    
    if (node->state != LEADER) {
        pthread_mutex_unlock(&node->lock);
        return;
    }
    
    if (response->success) {
        // 성공 - next_index와 match_index 업데이트
        node->next_index[peer->index] = response->match_index + 1;
        node->match_index[peer->index] = response->match_index;
        
        // 커밋 인덱스 업데이트 가능한지 확인
        update_commit_index(node);
    } else {
        // 실패 - next_index 감소
        if (node->next_index[peer->index] > 1) {
            node->next_index[peer->index]--;
        }
    }
    
    pthread_mutex_unlock(&node->lock);
}

// [SEQUENCE: 160] 커밋 인덱스 업데이트
void update_commit_index(raft_node_t* node) {
    // 과반수가 복제한 가장 높은 인덱스 찾기
    for (uint64_t n = get_last_log_index(node); n > node->commit_index; n--) {
        if (node->log[n - 1].term != node->current_term) {
            continue;  // 현재 term의 엔트리만 커밋
        }
        
        int replicas = 1;  // 리더 자신
        for (size_t i = 0; i < node->num_peers; i++) {
            if (node->match_index[i] >= n) {
                replicas++;
            }
        }
        
        if (replicas > (node->num_peers + 1) / 2) {
            node->commit_index = n;
            pthread_cond_broadcast(&node->apply_cond);
            
            log_info("Committed log entry %lu", n);
            break;
        }
    }
}
```

## 3. 분산 로그 시스템 구현

### 3.1 로그 적용과 상태 머신
```c
// [SEQUENCE: 161] 로그 적용 스레드
void* apply_committed_entries(void* arg) {
    raft_node_t* node = (raft_node_t*)arg;
    
    while (running) {
        pthread_mutex_lock(&node->lock);
        
        // 커밋된 엔트리 적용 대기
        while (node->last_applied >= node->commit_index && running) {
            pthread_cond_wait(&node->apply_cond, &node->lock);
        }
        
        // 커밋된 엔트리 적용
        while (node->last_applied < node->commit_index) {
            node->last_applied++;
            raft_log_entry_t* entry = &node->log[node->last_applied - 1];
            
            pthread_mutex_unlock(&node->lock);
            
            // 상태 머신에 적용
            apply_to_state_machine(entry);
            
            pthread_mutex_lock(&node->lock);
        }
        
        pthread_mutex_unlock(&node->lock);
    }
    
    return NULL;
}

// [SEQUENCE: 162] 분산 로그 버퍼
typedef struct distributed_log_buffer {
    raft_node_t* raft;
    
    // 로컬 버퍼 (빠른 읽기)
    circular_buffer_t* local_buffer;
    
    // 쿼리 인터페이스
    int query_port;
    pthread_t query_thread;
    
    // 통계
    atomic_uint64_t total_logs;
    atomic_uint64_t committed_logs;
    atomic_uint64_t applied_logs;
} distributed_log_buffer_t;

void apply_to_state_machine(raft_log_entry_t* entry) {
    // NOOP 엔트리 무시
    if (strcmp(entry->data, "NOOP") == 0) {
        return;
    }
    
    // 로컬 버퍼에 추가
    add_to_local_buffer(entry->data, entry->timestamp);
    
    // 디스크 영속화 (옵션)
    if (persistence_enabled) {
        persist_log_entry(entry);
    }
    
    atomic_fetch_add(&applied_logs, 1);
}
```

### 3.2 네트워크 통신
```c
// [SEQUENCE: 163] 비동기 RPC 구현
typedef struct {
    rpc_type_t type;
    void* request;
    void* response;
    peer_t* peer;
    void (*callback)(raft_node_t*, peer_t*, void*);
} rpc_call_t;

typedef struct {
    thread_pool_t* pool;
    int epoll_fd;
    
    // 진행중인 RPC
    struct {
        rpc_call_t* call;
        int fd;
        time_t start_time;
    } pending_rpcs[MAX_PENDING_RPCS];
    
    size_t num_pending;
    pthread_mutex_t lock;
} rpc_client_t;

// [SEQUENCE: 164] 효율적인 RPC 전송
void send_rpc_async(rpc_client_t* client, rpc_call_t* call) {
    // 연결 풀에서 연결 가져오기
    int fd = get_connection(call->peer);
    
    // 논블로킹 전송
    set_nonblocking(fd);
    
    // 직렬화
    size_t size;
    void* buffer = serialize_rpc(call, &size);
    
    // 전송 시작
    ssize_t sent = send(fd, buffer, size, MSG_NOSIGNAL);
    
    if (sent < 0 && errno != EAGAIN) {
        // 오류 처리
        handle_rpc_error(call);
        return;
    }
    
    // epoll에 등록
    struct epoll_event ev = {
        .events = EPOLLIN | EPOLLOUT | EPOLLET,
        .data.ptr = call
    };
    epoll_ctl(client->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
    
    // 펜딩 목록에 추가
    add_pending_rpc(client, call, fd);
}

// [SEQUENCE: 165] 배치 처리
void process_rpc_responses(rpc_client_t* client) {
    struct epoll_event events[256];
    
    int n = epoll_wait(client->epoll_fd, events, 256, 10);
    
    for (int i = 0; i < n; i++) {
        rpc_call_t* call = (rpc_call_t*)events[i].data.ptr;
        
        if (events[i].events & EPOLLIN) {
            // 응답 수신
            void* response = receive_response(call);
            if (response) {
                call->callback(global_raft_node, call->peer, response);
                remove_pending_rpc(client, call);
            }
        }
        
        if (events[i].events & EPOLLERR) {
            // 오류 처리
            handle_rpc_error(call);
        }
    }
}
```

## 4. 장애 처리와 복구

### 4.1 네트워크 파티션
```c
// [SEQUENCE: 166] 파티션 감지
typedef struct {
    bool is_partitioned;
    size_t partition_size;
    time_t partition_start;
    peer_t* reachable_peers;
    size_t num_reachable;
} partition_info_t;

partition_info_t detect_partition(raft_node_t* node) {
    partition_info_t info = {0};
    
    // 연결 가능한 피어 확인
    for (size_t i = 0; i < node->num_peers; i++) {
        if (ping_peer(&node->peers[i], 1000)) {  // 1초 타임아웃
            info.reachable_peers[info.num_reachable++] = node->peers[i];
        }
    }
    
    info.partition_size = info.num_reachable + 1;  // 자신 포함
    
    // 과반수 확인
    if (info.partition_size <= (node->num_peers + 1) / 2) {
        info.is_partitioned = true;
        info.partition_start = time(NULL);
        
        log_warn("Network partition detected! Only %zu/%zu nodes reachable",
                info.partition_size, node->num_peers + 1);
    }
    
    return info;
}

// [SEQUENCE: 167] 파티션 중 동작
void handle_partition(raft_node_t* node, partition_info_t* partition) {
    if (partition->is_partitioned) {
        if (node->state == LEADER) {
            // 과반수 없으면 리더 포기
            if (partition->partition_size <= (node->num_peers + 1) / 2) {
                log_warn("Stepping down due to partition");
                become_follower(node, node->current_term);
            }
        }
        
        // 읽기 전용 모드 고려
        if (partition->partition_size < (node->num_peers + 1) / 2) {
            enter_read_only_mode(node);
        }
    }
}
```

### 4.2 로그 압축
```c
// [SEQUENCE: 168] 스냅샷 구현
typedef struct {
    uint64_t last_included_index;
    uint64_t last_included_term;
    size_t data_size;
    char data[];  // 압축된 상태
} snapshot_t;

void create_snapshot(raft_node_t* node) {
    pthread_mutex_lock(&node->lock);
    
    if (node->last_applied <= node->snapshot_index) {
        pthread_mutex_unlock(&node->lock);
        return;  // 이미 스냅샷 존재
    }
    
    // 상태 머신 스냅샷
    size_t state_size;
    void* state_data = serialize_state_machine(&state_size);
    
    // 스냅샷 생성
    snapshot_t* snapshot = malloc(sizeof(snapshot_t) + state_size);
    snapshot->last_included_index = node->last_applied;
    snapshot->last_included_term = node->log[node->last_applied - 1].term;
    snapshot->data_size = state_size;
    memcpy(snapshot->data, state_data, state_size);
    
    // 디스크에 저장
    save_snapshot(node, snapshot);
    
    // 로그 잘라내기
    compact_log(node, snapshot->last_included_index);
    
    pthread_mutex_unlock(&node->lock);
    
    free(state_data);
    free(snapshot);
}

// [SEQUENCE: 169] InstallSnapshot RPC
void handle_install_snapshot(raft_node_t* node,
                           const install_snapshot_t* request) {
    pthread_mutex_lock(&node->lock);
    
    if (request->term < node->current_term) {
        // 거부
        send_install_snapshot_response(node, false);
        pthread_mutex_unlock(&node->lock);
        return;
    }
    
    if (request->term > node->current_term) {
        become_follower(node, request->term);
    }
    
    // 스냅샷 저장
    save_received_snapshot(node, request);
    
    // 상태 머신 리셋
    reset_state_machine();
    apply_snapshot(request->snapshot_data, request->snapshot_size);
    
    // 로그 교체
    node->log_count = 0;
    node->last_applied = request->last_included_index;
    node->commit_index = request->last_included_index;
    
    pthread_mutex_unlock(&node->lock);
    
    send_install_snapshot_response(node, true);
}
```

## 5. 성능 최적화

### 5.1 배치와 파이프라이닝
```c
// [SEQUENCE: 170] 로그 엔트리 배치
typedef struct {
    raft_log_entry_t entries[MAX_BATCH_SIZE];
    size_t count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} log_batch_t;

void batch_append(raft_node_t* node, log_batch_t* batch) {
    pthread_mutex_lock(&batch->lock);
    
    // 배치가 차거나 타임아웃까지 대기
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_nsec += 10000000;  // 10ms
    
    while (batch->count < MAX_BATCH_SIZE && running) {
        if (pthread_cond_timedwait(&batch->not_empty, &batch->lock, 
                                   &timeout) == ETIMEDOUT) {
            break;
        }
    }
    
    if (batch->count > 0) {
        // 리더에게 배치 전송
        send_batch_to_leader(node, batch->entries, batch->count);
        batch->count = 0;
        pthread_cond_signal(&batch->not_full);
    }
    
    pthread_mutex_unlock(&batch->lock);
}

// [SEQUENCE: 171] 파이프라이닝
void pipelined_replication(raft_node_t* node) {
    if (node->state != LEADER) return;
    
    for (size_t i = 0; i < node->num_peers; i++) {
        peer_t* peer = &node->peers[i];
        
        // 여러 RPC 동시 전송
        size_t in_flight = 0;
        while (node->next_index[i] <= get_last_log_index(node) &&
               in_flight < MAX_IN_FLIGHT_RPCS) {
            send_append_entries_async(node, peer);
            in_flight++;
        }
    }
}
```

### 5.2 최적화된 영속화
```c
// [SEQUENCE: 172] Group Commit
typedef struct {
    int fd;
    pthread_mutex_t lock;
    
    struct pending_write {
        void* data;
        size_t size;
        void (*callback)(void*);
        void* arg;
    } writes[MAX_PENDING_WRITES];
    
    size_t num_pending;
    pthread_cond_t flush_cond;
} group_commit_t;

void* group_commit_thread(void* arg) {
    group_commit_t* gc = (group_commit_t*)arg;
    
    while (running) {
        pthread_mutex_lock(&gc->lock);
        
        // 쓰기 대기
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_nsec += 1000000;  // 1ms
        
        pthread_cond_timedwait(&gc->flush_cond, &gc->lock, &timeout);
        
        if (gc->num_pending > 0) {
            // 모든 펜딩 쓰기를 하나의 벡터 I/O로
            struct iovec iov[MAX_PENDING_WRITES];
            for (size_t i = 0; i < gc->num_pending; i++) {
                iov[i].iov_base = gc->writes[i].data;
                iov[i].iov_len = gc->writes[i].size;
            }
            
            pthread_mutex_unlock(&gc->lock);
            
            // 원자적 쓰기
            writev(gc->fd, iov, gc->num_pending);
            fdatasync(gc->fd);
            
            // 콜백 호출
            for (size_t i = 0; i < gc->num_pending; i++) {
                if (gc->writes[i].callback) {
                    gc->writes[i].callback(gc->writes[i].arg);
                }
            }
            
            pthread_mutex_lock(&gc->lock);
            gc->num_pending = 0;
        }
        
        pthread_mutex_unlock(&gc->lock);
    }
    
    return NULL;
}
```

## 6. 모니터링과 디버깅

### 6.1 Raft 상태 모니터링
```c
// [SEQUENCE: 173] 상태 정보 수집
typedef struct {
    node_state_t state;
    uint64_t term;
    char leader_id[32];
    uint64_t commit_index;
    uint64_t last_applied;
    uint64_t log_length;
    
    // 성능 메트릭
    uint64_t elections_started;
    uint64_t elections_won;
    uint64_t rpcs_sent;
    uint64_t rpcs_received;
    double average_rtt;
    
    // 건강 상태
    bool is_healthy;
    size_t connected_peers;
    time_t last_heartbeat;
} raft_status_t;

raft_status_t get_raft_status(raft_node_t* node) {
    pthread_mutex_lock(&node->lock);
    
    raft_status_t status = {
        .state = node->state,
        .term = node->current_term,
        .commit_index = node->commit_index,
        .last_applied = node->last_applied,
        .log_length = node->log_count,
        // ... 기타 필드
    };
    
    if (node->state == LEADER) {
        strcpy(status.leader_id, node->node_id);
    }
    
    pthread_mutex_unlock(&node->lock);
    
    // 건강 상태 확인
    status.is_healthy = check_health(node);
    status.connected_peers = count_connected_peers(node);
    
    return status;
}

// [SEQUENCE: 174] 디버그 정보
void dump_raft_state(raft_node_t* node) {
    printf("=== Raft Node State Dump ===\n");
    printf("Node ID: %s\n", node->node_id);
    printf("State: %s\n", state_to_string(node->state));
    printf("Term: %lu\n", node->current_term);
    printf("Voted For: %s\n", node->voted_for.node_id);
    printf("Commit Index: %lu\n", node->commit_index);
    printf("Last Applied: %lu\n", node->last_applied);
    printf("\nLog Entries:\n");
    
    for (size_t i = 0; i < MIN(node->log_count, 10); i++) {
        printf("  [%lu] Term=%lu, Data=%s\n",
               node->log[i].index,
               node->log[i].term,
               node->log[i].data);
    }
    
    if (node->state == LEADER) {
        printf("\nFollower Progress:\n");
        for (size_t i = 0; i < node->num_peers; i++) {
            printf("  %s: next=%lu, match=%lu\n",
                   node->peers[i].id,
                   node->next_index[i],
                   node->match_index[i]);
        }
    }
}
```

## 마무리

Raft 합의 알고리즘으로 달성한 개선사항:
1. **고가용성**: N개 노드 중 (N/2)+1개만 있으면 동작
2. **일관성**: 모든 노드가 같은 순서로 로그 적용
3. **내결함성**: 자동 리더 선출과 복구
4. **성능**: 파이프라이닝으로 높은 처리량

이는 분산 시스템의 핵심 기술입니다.