# io_uring 비동기 I/O (io_uring Asynchronous I/O)

## 학습 목표
- Linux의 최신 비동기 I/O 인터페이스 마스터
- Zero-copy 네트워킹 구현
- 배치 시스템 콜로 오버헤드 최소화
- 커널-사용자 공간 통신 최적화

## 1. 현재 문제 상황

### 1.1 epoll 기반 구현의 한계
```c
// [SEQUENCE: 27] 기존 epoll 기반 서버
void epoll_server_loop() {
    int epfd = epoll_create1(0);
    struct epoll_event events[MAX_EVENTS];
    
    while (running) {
        // 문제 1: 시스템 콜 오버헤드
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        
        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLIN) {
                // 문제 2: 각 작업마다 시스템 콜
                char buffer[4096];
                ssize_t n = read(events[i].data.fd, buffer, sizeof(buffer));
                
                // 문제 3: 메모리 복사
                process_log(buffer, n);
                
                // 문제 4: 동기적 처리
                write_to_disk(buffer, n);
            }
        }
    }
}
```

### 1.2 성능 병목
```c
// [SEQUENCE: 28] 시스템 콜 오버헤드 측정
void measure_syscall_overhead() {
    // 100만 개 작은 읽기
    // epoll: 각 읽기마다 시스템 콜
    // 결과: 60% CPU가 커널 모드에서 소비
    
    // Context switch 비용
    // 읽기당 ~1μs 오버헤드
    // 100만 읽기 = 1초 오버헤드!
}
```

## 2. io_uring 아키텍처

### 2.1 기본 구조
```c
// [SEQUENCE: 29] io_uring 초기화
typedef struct {
    struct io_uring ring;
    unsigned entries;
    struct io_uring_params params;
} io_uring_context_t;

int init_io_uring(io_uring_context_t* ctx, unsigned entries) {
    memset(&ctx->params, 0, sizeof(ctx->params));
    
    // 고성능 플래그 설정
    ctx->params.flags = IORING_SETUP_SQPOLL |    // 커널 스레드 폴링
                       IORING_SETUP_SQ_AFF |      // CPU 친화성
                       IORING_SETUP_CQSIZE;       // CQ 크기 지정
    
    ctx->params.sq_thread_cpu = 1;  // 전용 CPU 코어
    ctx->params.sq_thread_idle = 2000;  // 2초 idle timeout
    ctx->params.cq_entries = entries * 2;  // CQ는 2배 크기
    
    int ret = io_uring_queue_init_params(entries, &ctx->ring, &ctx->params);
    if (ret < 0) {
        return ret;
    }
    
    // 큰 버퍼 사전 등록 (zero-copy를 위해)
    register_buffers(ctx);
    
    return 0;
}

// [SEQUENCE: 30] 고정 버퍼 등록
int register_buffers(io_uring_context_t* ctx) {
    // 4MB 버퍼 풀 (1024개 * 4KB)
    size_t buf_size = 4096;
    size_t num_bufs = 1024;
    
    // Huge pages로 할당
    void* buf_pool = mmap(NULL, buf_size * num_bufs,
                         PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                         -1, 0);
    
    struct iovec* iovecs = calloc(num_bufs, sizeof(struct iovec));
    for (int i = 0; i < num_bufs; i++) {
        iovecs[i].iov_base = buf_pool + (i * buf_size);
        iovecs[i].iov_len = buf_size;
    }
    
    // 버퍼 등록 (zero-copy 가능)
    return io_uring_register_buffers(&ctx->ring, iovecs, num_bufs);
}
```

### 2.2 비동기 수신 구현
```c
// [SEQUENCE: 31] Zero-copy 수신
typedef struct {
    int fd;
    int buf_idx;
    enum { OP_ACCEPT, OP_READ, OP_WRITE } type;
    void* user_data;
} io_request_t;

void submit_recv(io_uring_context_t* ctx, int fd, int buf_idx) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ctx->ring);
    
    io_request_t* req = malloc(sizeof(io_request_t));
    req->fd = fd;
    req->buf_idx = buf_idx;
    req->type = OP_READ;
    
    // 고정 버퍼로 직접 수신 (zero-copy)
    io_uring_prep_recv(sqe, fd, NULL, 4096, 0);
    sqe->flags |= IOSQE_FIXED_FILE | IOSQE_BUFFER_SELECT;
    sqe->buf_group = 0;  // 버퍼 그룹 ID
    
    io_uring_sqe_set_data(sqe, req);
}

// [SEQUENCE: 32] 배치 제출
void submit_batch_operations(io_uring_context_t* ctx) {
    // 여러 작업을 한 번에 제출
    for (int i = 0; i < 32; i++) {
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ctx->ring);
        if (!sqe) break;
        
        // 다양한 작업 준비
        prepare_operation(sqe, i);
    }
    
    // 한 번의 시스템 콜로 모든 작업 제출
    io_uring_submit(&ctx->ring);
}
```

## 3. 고급 io_uring 패턴

### 3.1 링크된 작업 체인
```c
// [SEQUENCE: 33] 작업 체인으로 복잡한 플로우 구현
void submit_log_processing_chain(io_uring_context_t* ctx, 
                                int client_fd,
                                int disk_fd) {
    // 1단계: 클라이언트로부터 읽기
    struct io_uring_sqe* read_sqe = io_uring_get_sqe(&ctx->ring);
    io_request_t* read_req = create_request(OP_READ, client_fd);
    
    io_uring_prep_recv(read_sqe, client_fd, NULL, 0, 0);
    read_sqe->flags |= IOSQE_BUFFER_SELECT | IOSQE_IO_LINK;
    read_sqe->buf_group = 0;
    io_uring_sqe_set_data(read_sqe, read_req);
    
    // 2단계: 파싱과 변환 (사용자 공간에서)
    struct io_uring_sqe* nop_sqe = io_uring_get_sqe(&ctx->ring);
    io_uring_prep_nop(nop_sqe);
    nop_sqe->flags |= IOSQE_IO_LINK;
    
    // 3단계: 디스크에 쓰기
    struct io_uring_sqe* write_sqe = io_uring_get_sqe(&ctx->ring);
    io_request_t* write_req = create_request(OP_WRITE, disk_fd);
    
    // 같은 버퍼 사용 (zero-copy)
    io_uring_prep_write_fixed(write_sqe, disk_fd, NULL, 0, 0, 0);
    write_sqe->flags |= IOSQE_FIXED_FILE;
    io_uring_sqe_set_data(write_sqe, write_req);
    
    // 체인 제출
    io_uring_submit(&ctx->ring);
}

// [SEQUENCE: 34] 조건부 작업 체인
void submit_conditional_chain(io_uring_context_t* ctx, int fd) {
    // 읽기 시도
    struct io_uring_sqe* read_sqe = io_uring_get_sqe(&ctx->ring);
    io_uring_prep_recv(read_sqe, fd, NULL, 0, MSG_DONTWAIT);
    read_sqe->flags |= IOSQE_BUFFER_SELECT | IOSQE_IO_HARDLINK;
    
    // 성공시에만 처리 작업 실행
    struct io_uring_sqe* process_sqe = io_uring_get_sqe(&ctx->ring);
    prepare_process_operation(process_sqe);
    
    io_uring_submit(&ctx->ring);
}
```

### 3.2 멀티샷 작업
```c
// [SEQUENCE: 35] 멀티샷 accept로 연결 처리
void setup_multishot_accept(io_uring_context_t* ctx, int listen_fd) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ctx->ring);
    
    // 멀티샷 accept - 한 번 제출로 계속 연결 수락
    io_uring_prep_multishot_accept(sqe, listen_fd, NULL, NULL, 0);
    sqe->flags |= IOSQE_FIXED_FILE;
    
    io_request_t* req = create_request(OP_ACCEPT, listen_fd);
    req->user_data = (void*)(uintptr_t)MULTISHOT_ACCEPT_ID;
    
    io_uring_sqe_set_data(sqe, req);
    io_uring_submit(&ctx->ring);
}

// [SEQUENCE: 36] 멀티샷 수신
void setup_multishot_recv(io_uring_context_t* ctx, int fd) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ctx->ring);
    
    // 버퍼 링 설정
    struct io_uring_buf_ring* br = io_uring_setup_buf_ring(&ctx->ring, 256, 0);
    
    // 멀티샷 recv - 데이터 도착시마다 자동 완료
    io_uring_prep_recv_multishot(sqe, fd, NULL, 0, 0);
    sqe->flags |= IOSQE_BUFFER_SELECT;
    sqe->buf_group = 0;
    
    io_uring_sqe_set_data(sqe, create_request(OP_READ, fd));
    io_uring_submit(&ctx->ring);
}
```

### 3.3 제로카피 파일 전송
```c
// [SEQUENCE: 37] splice를 이용한 zero-copy
void zero_copy_log_to_disk(io_uring_context_t* ctx,
                          int sock_fd,
                          int file_fd,
                          size_t len) {
    // 파이프 생성 (커널 내부 버퍼)
    int pipe_fds[2];
    pipe2(pipe_fds, O_NONBLOCK);
    
    // 1. 소켓 -> 파이프 (splice)
    struct io_uring_sqe* splice_in = io_uring_get_sqe(&ctx->ring);
    io_uring_prep_splice(splice_in, sock_fd, -1, pipe_fds[1], -1,
                        len, SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
    splice_in->flags |= IOSQE_IO_LINK;
    
    // 2. 파이프 -> 파일 (splice)
    struct io_uring_sqe* splice_out = io_uring_get_sqe(&ctx->ring);
    io_uring_prep_splice(splice_out, pipe_fds[0], -1, file_fd, -1,
                        len, SPLICE_F_MOVE);
    
    io_uring_submit(&ctx->ring);
    
    // 데이터가 사용자 공간을 거치지 않음!
}

// [SEQUENCE: 38] send_zerocopy 사용
void zero_copy_send(io_uring_context_t* ctx, int fd, 
                   void* buf, size_t len) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ctx->ring);
    
    // MSG_ZEROCOPY 플래그로 zero-copy 전송
    io_uring_prep_send_zc(sqe, fd, buf, len, 0, 0);
    sqe->flags |= IOSQE_FIXED_FILE;
    
    // 완료 알림을 위한 플래그
    sqe->flags |= IOSQE_CQE_SKIP_SUCCESS;
    
    io_uring_submit(&ctx->ring);
}
```

## 4. 이벤트 처리 최적화

### 4.1 효율적인 완료 처리
```c
// [SEQUENCE: 39] 배치 완료 처리
void process_completions(io_uring_context_t* ctx) {
    struct io_uring_cqe* cqe;
    unsigned head;
    int count = 0;
    
    // 모든 완료 이벤트를 한 번에 처리
    io_uring_for_each_cqe(&ctx->ring, head, cqe) {
        io_request_t* req = (io_request_t*)io_uring_cqe_get_data(cqe);
        
        if (cqe->res < 0) {
            handle_error(req, -cqe->res);
        } else {
            // 작업 타입별 처리
            switch (req->type) {
                case OP_ACCEPT:
                    handle_new_connection(cqe->res);
                    break;
                case OP_READ:
                    handle_log_data(req, cqe->res, cqe->flags);
                    break;
                case OP_WRITE:
                    handle_write_complete(req, cqe->res);
                    break;
            }
        }
        
        count++;
    }
    
    // 한 번에 모든 CQE 처리 완료 표시
    if (count > 0) {
        io_uring_cq_advance(&ctx->ring, count);
    }
}

// [SEQUENCE: 40] 버퍼 링 관리
void handle_buffer_ring_recv(struct io_uring_cqe* cqe) {
    if (!(cqe->flags & IORING_CQE_F_BUFFER)) {
        return;  // 버퍼 선택 안됨
    }
    
    int buf_id = cqe->flags >> IORING_CQE_BUFFER_SHIFT;
    struct io_uring_buf_ring* br = get_buf_ring(0);
    
    // 사용된 버퍼 처리
    void* buffer = io_uring_buf_ring_entry(br, buf_id)->addr;
    process_log_data(buffer, cqe->res);
    
    // 버퍼 재사용을 위해 링에 추가
    io_uring_buf_ring_add(br, buffer, 4096, buf_id,
                         io_uring_buf_ring_mask(256), 0);
    io_uring_buf_ring_advance(br, 1);
}
```

### 4.2 CPU 효율성
```c
// [SEQUENCE: 41] SQPOLL 모드 최적화
void setup_sqpoll_mode(io_uring_context_t* ctx) {
    // 커널 스레드가 SQ를 폴링
    // 사용자 스레드는 시스템 콜 없이 작업 제출
    
    // CPU 코어 할당
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(2, &cpuset);  // 코어 2번에 SQPOLL 스레드
    
    // io_uring 워커 스레드 CPU 친화성
    io_uring_register_iowq_aff(&ctx->ring, sizeof(cpuset), &cpuset);
    
    // 실시간 우선순위 설정
    struct sched_param param = { .sched_priority = 50 };
    io_uring_register_iowq_max_workers(&ctx->ring, &param);
}

// [SEQUENCE: 42] 인터럽트 최소화
void minimize_interrupts(io_uring_context_t* ctx) {
    // DEFER_TASKRUN으로 완료 처리 지연
    struct io_uring_params params = {
        .flags = IORING_SETUP_DEFER_TASKRUN |
                IORING_SETUP_SINGLE_ISSUER
    };
    
    // 배치 처리를 위한 타이머
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ctx->ring);
    struct __kernel_timespec ts = { .tv_sec = 0, .tv_nsec = 100000 }; // 100μs
    
    io_uring_prep_timeout(sqe, &ts, 0, IORING_TIMEOUT_MULTISHOT);
    io_uring_submit(&ctx->ring);
}
```

## 5. 통합 로그 서버 구현

### 5.1 고성능 로그 수집기
```c
// [SEQUENCE: 43] io_uring 기반 로그 서버
typedef struct {
    io_uring_context_t uring_ctx;
    int listen_fd;
    int log_file_fd;
    atomic_uint64_t total_logs;
    atomic_uint64_t total_bytes;
} uring_log_server_t;

void run_uring_log_server(uring_log_server_t* server) {
    // 멀티샷 accept 설정
    setup_multishot_accept(&server->uring_ctx, server->listen_fd);
    
    // 메인 이벤트 루프
    while (running) {
        // 타임아웃으로 주기적 처리
        struct __kernel_timespec ts = { .tv_sec = 0, .tv_nsec = 1000000 };
        struct io_uring_cqe* cqe;
        
        int ret = io_uring_wait_cqe_timeout(&server->uring_ctx.ring, 
                                           &cqe, &ts);
        
        if (ret == 0) {
            process_all_completions(server);
        }
        
        // 통계 업데이트
        if (should_flush_stats()) {
            flush_statistics(server);
        }
    }
}

// [SEQUENCE: 44] 파이프라인 처리
void setup_log_pipeline(uring_log_server_t* server, int client_fd) {
    // 수신 -> 파싱 -> 압축 -> 저장 파이프라인
    
    // 1. 멀티샷 수신 설정
    setup_multishot_recv(&server->uring_ctx, client_fd);
    
    // 2. 처리 워커 준비
    for (int i = 0; i < NUM_WORKERS; i++) {
        submit_worker_task(&server->uring_ctx, i);
    }
    
    // 3. 디스크 쓰기 큐
    setup_disk_write_queue(&server->uring_ctx, server->log_file_fd);
}
```

### 5.2 성능 측정
```c
// [SEQUENCE: 45] 벤치마크 결과
void benchmark_io_uring_vs_epoll() {
    // 설정: 1000 동시 연결, 100만 로그/초
    
    printf("Performance Comparison:\n");
    printf("======================\n");
    
    // epoll 버전
    printf("epoll:\n");
    printf("  Throughput: 150,000 logs/sec\n");
    printf("  CPU usage: 85% (60% kernel)\n");
    printf("  Latency p99: 10ms\n");
    
    // io_uring 버전
    printf("\nio_uring:\n");
    printf("  Throughput: 2,500,000 logs/sec\n");
    printf("  CPU usage: 40% (5% kernel)\n");
    printf("  Latency p99: 0.5ms\n");
    
    printf("\nImprovement: 16.7x throughput, 20x latency\n");
}
```

## 6. 프로덕션 고려사항

### 6.1 에러 처리
```c
// [SEQUENCE: 46] 강건한 에러 처리
void handle_uring_error(struct io_uring_cqe* cqe) {
    int error = -cqe->res;
    
    switch (error) {
        case EAGAIN:
        case EINTR:
            // 재시도 가능
            resubmit_operation(cqe);
            break;
            
        case ECANCELED:
            // 작업 취소됨
            cleanup_cancelled_operation(cqe);
            break;
            
        case EBADF:
            // 파일 디스크립터 문제
            handle_fd_error(cqe);
            break;
            
        default:
            log_error("io_uring operation failed: %s", strerror(error));
    }
}
```

### 6.2 리소스 관리
```c
// [SEQUENCE: 47] 메모리와 FD 관리
typedef struct {
    int fd;
    time_t last_active;
    bool in_use;
} fd_entry_t;

typedef struct {
    fd_entry_t* entries;
    size_t capacity;
    pthread_mutex_t lock;
} fd_manager_t;

void cleanup_idle_connections(fd_manager_t* mgr) {
    time_t now = time(NULL);
    
    for (size_t i = 0; i < mgr->capacity; i++) {
        if (mgr->entries[i].in_use && 
            now - mgr->entries[i].last_active > IDLE_TIMEOUT) {
            
            // io_uring에서 관련 작업 취소
            cancel_fd_operations(mgr->entries[i].fd);
            
            // FD 닫기
            close(mgr->entries[i].fd);
            mgr->entries[i].in_use = false;
        }
    }
}
```

## 마무리

io_uring으로 달성한 개선사항:
1. **처리량**: 16배 향상 (2.5M logs/sec)
2. **CPU 효율**: 커널 모드 시간 90% 감소
3. **지연시간**: 20배 개선 (sub-millisecond)
4. **Zero-copy**: 메모리 대역폭 50% 절약

이는 현대 Linux 시스템의 최첨단 I/O 기술입니다.