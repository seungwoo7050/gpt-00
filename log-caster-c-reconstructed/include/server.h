// [SEQUENCE: C-MVP1-2]
#ifndef SERVER_H
#define SERVER_H

#include <sys/select.h>
#include <signal.h>
// [SEQUENCE: C-MVP2-36]
#include "thread_pool.h"
#include "log_buffer.h"
// [SEQUENCE: C-MVP4-16]
#include "persistence.h"


#define DEFAULT_PORT 9999
// [SEQUENCE: C-MVP2-36]
#define QUERY_PORT 9998
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 4096

// [SEQUENCE: C-MVP1-3]
// 서버 상태를 관리하는 구조체 (MVP1 버전)
// [SEQUENCE: C-MVP2-37]
// 서버 구조체 (MVP2 버전)
typedef struct log_server {
    int port;
    // [SEQUENCE: C-MVP2-37]
    int query_port;
    int listen_fd;
    // [SEQUENCE: C-MVP2-37]
    int query_fd;
    fd_set master_set;
    fd_set read_set;
    int max_fd;
    int client_count;
    volatile sig_atomic_t running;
    
    // [SEQUENCE: C-MVP2-37]
    // MVP2 추가 사항
    thread_pool_t* thread_pool;
    log_buffer_t* log_buffer;

    // [SEQUENCE: C-MVP4-17]
    // MVP4 추가 사항
    persistence_manager_t* persistence;

    // [SEQUENCE: C-MVP5-2]
    // MVP5 추가: client_count 보호를 위한 뮤텍스
    pthread_mutex_t client_count_mutex;
} log_server_t;

// [SEQUENCE: C-MVP2-38]
// 클라이언트 작업을 위한 컨텍스트 구조체
typedef struct {
    int client_fd;
    log_server_t* server;
} client_job_t;


// [SEQUENCE: C-MVP1-4]
// 함수 프로토타입 (MVP1 버전)
// [SEQUENCE: C-MVP2-39]
// 서버 생명주기 함수
log_server_t* server_create(int port);
void server_destroy(log_server_t* server);
// [SEQUENCE: C-MVP2-39]
int server_init(log_server_t* server);
void server_run(log_server_t* server);


#endif // SERVER_H
