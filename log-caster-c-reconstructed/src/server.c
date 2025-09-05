// [SEQUENCE: C-MVP1-5]
#include "server.h"
// [SEQUENCE: C-MVP2-40]
#include "query_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>


// [SEQUENCE: C-MVP1-5]
// 시그널 처리를 위한 전역 서버 포인터
static log_server_t* g_server = NULL;

// [SEQUENCE: C-MVP1-6]
// 시그널 핸들러: 서버의 running 상태를 0으로 변경하여 안전한 종료를 유도
// [SEQUENCE: C-MVP2-40]
static void sigint_handler(int sig) {
    (void)sig;
    if (g_server) g_server->running = 0;
}

// [SEQUENCE: C-MVP2-41]
// 스레드 풀에서 실행될 클라이언트 처리 작업
static void handle_client_job(void* arg) {
    client_job_t* job = (client_job_t*)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // 데이터 수신 및 로그 버퍼에 저장
    while ((bytes_read = recv(job->client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        }
        // [SEQUENCE: C-MVP5-5]
        // 로그 메시지 크기 제한 및 절단
        const int SAFE_LOG_LENGTH = 1024;
        if (bytes_read > SAFE_LOG_LENGTH) {
            buffer[SAFE_LOG_LENGTH - 4] = '.';
            buffer[SAFE_LOG_LENGTH - 3] = '.';
            buffer[SAFE_LOG_LENGTH - 2] = '.';
            buffer[SAFE_LOG_LENGTH - 1] = '\0';
        }

        if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        }
        // 1. 인메모리 버퍼에 저장
        log_buffer_push(job->server->log_buffer, buffer);

        // [SEQUENCE: C-MVP4-20]
        // 2. 영속성 관리자에게 쓰기 요청 (활성화된 경우)
        if (job->server->persistence) {
            persistence_write(job->server->persistence, buffer);
        }
    }

    // 연결 종료 처리
    close(job->client_fd);

    // [SEQUENCE: C-MVP5-6]
    // client_count를 스레드 안전하게 감소
    pthread_mutex_lock(&job->server->client_count_mutex);
    job->server->client_count--;
    pthread_mutex_unlock(&job->server->client_count_mutex);

    // 참고: 이 버전에서는 client_count를 감소시키지 않아 버그가 존재함 (MVP5에서 수정)
    free(job);
}


// [SEQUENCE: C-MVP1-7]
// 서버 구조체 생성 및 리스닝 소켓 초기화
// [SEQUENCE: C-MVP2-42]
// 서버 생성 (MVP2)
log_server_t* server_create(int port) {
    log_server_t* server = calloc(1, sizeof(log_server_t));
    if (!server) return NULL;

    server->port = port;
    server->query_port = QUERY_PORT;
    server->running = 1;

    server->thread_pool = thread_pool_create(DEFAULT_THREAD_COUNT);
    if (!server->thread_pool) { free(server); return NULL; }

    server->log_buffer = log_buffer_create(DEFAULT_BUFFER_SIZE);
    if (!server->log_buffer) { thread_pool_destroy(server->thread_pool); free(server); return NULL; }

    // [SEQUENCE: C-MVP4-21]
    // persistence는 main에서 생성 후 주입되므로 NULL로 초기화
    server->persistence = NULL; 

    return server;
}

// [SEQUENCE: C-MVP2-43]
// 리스너 소켓 초기화 헬퍼 함수
static int init_listener(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind failed"); close(fd); return -1; }
    if (listen(fd, MAX_CLIENTS) < 0) { close(fd); return -1; }
    return fd;
}

// [SEQUENCE: C-MVP2-44]
// 서버 초기화 (두 개의 리스너)
int server_init(log_server_t* server) {
    server->listen_fd = init_listener(server->port);
    server->query_fd = init_listener(server->query_port);

    if (server->listen_fd < 0 || server->query_fd < 0) return -1;

    FD_ZERO(&server->master_set);
    FD_SET(server->listen_fd, &server->master_set);
    FD_SET(server->query_fd, &server->master_set);
    server->max_fd = (server->listen_fd > server->query_fd) ? server->listen_fd : server->query_fd;

    g_server = server;
    signal(SIGINT, sigint_handler);
    return 0;
}


// [SEQUENCE: C-MVP1-14]
// 서버 메인 루프
// [SEQUENCE: C-MVP2-45]
// 서버 메인 루프 (MVP2)
void server_run(log_server_t* server) {
    printf("LogCaster-C MVP2 server running...\nLog port: %d, Query port: %d\n", server->port, server->query_port);

    while (server->running) {
        server->read_set = server->master_set;
        if (select(server->max_fd + 1, &server->read_set, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) continue;
            perror("select"); break;
        }

        for (int i = 0; i <= server->max_fd; i++) {
            if (FD_ISSET(i, &server->read_set)) {
                if (i == server->listen_fd) {
                    // [SEQUENCE: C-MVP1-17]
                    // [SEQUENCE: C-MVP2-46]
                    // 새 로그 클라이언트 연결 -> 스레드 풀에 작업 제출
                    int new_fd = accept(server->listen_fd, NULL, NULL);
                    if (new_fd < 0) continue;

                    server->client_count++; // 버그: 감소 로직 없음
                    client_job_t* job = malloc(sizeof(client_job_t));
                    job->client_fd = new_fd;
                    job->server = server;
                    thread_pool_add_job(server->thread_pool, handle_client_job, job);
                } else if (i == server->query_fd) {
                    // [SEQUENCE: C-MVP2-47]
                    // 새 쿼리 클라이언트 연결 -> 직접 처리
                    handle_query_connection(server);
                }
            }
        }
    }
}


// [SEQUENCE: C-MVP1-23]
// 서버 리소스 해제
// [SEQUENCE: C-MVP2-48]
// 서버 소멸 (MVP2)
void server_destroy(log_server_t* server) {
    if (!server) return;
    printf("\nShutting down server...\n");
    thread_pool_wait(server->thread_pool);
    thread_pool_destroy(server->thread_pool);
    log_buffer_destroy(server->log_buffer);

    // [SEQUENCE: C-MVP4-22]
    // 영속성 관리자 소멸 로직 추가
    if (server->persistence) {
        persistence_destroy(server->persistence);
    }
    if (server->listen_fd >= 0) close(server->listen_fd);
    if (server->query_fd >= 0) close(server->query_fd);
    free(server);
    printf("Server shut down gracefully.\n");
}