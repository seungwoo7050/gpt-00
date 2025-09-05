// [SEQUENCE: MVP2-2]
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdbool.h>

#define DEFAULT_THREAD_COUNT 4
#define MAX_THREAD_COUNT 32

// [SEQUENCE: MVP2-3]
// 스레드 풀이 처리할 작업을 정의하는 구조체
typedef struct thread_job {
    void (*function)(void* arg); // 작업 함수 포인터
    void* arg;                   // 작업 함수에 전달될 인자
    struct thread_job* next;     // 다음 작업을 가리키는 포인터 (연결 리스트)
} thread_job_t;

// [SEQUENCE: MVP2-4]
// 스레드 풀을 관리하는 메인 구조체
typedef struct {
    pthread_t* threads;          // 작업자 스레드 배열
    int thread_count;            // 스레드 수
    
    thread_job_t* job_queue_head; // 작업 큐의 시작
    thread_job_t* job_queue_tail; // 작업 큐의 끝
    int job_count;               // 큐에 있는 작업의 수
    
    // 동기화 프리미티브
    pthread_mutex_t queue_mutex; // 작업 큐 접근을 위한 뮤텍스
    pthread_cond_t job_available;  // 새로운 작업이 추가되었음을 알리는 조건 변수
    pthread_cond_t all_jobs_done;  // 모든 작업이 완료되었음을 알리는 조건 변수
    
    // 풀 상태 관리
    volatile bool shutdown;        // 풀 종료 여부 플래그
    int working_threads;         // 현재 작업 중인 스레드 수
} thread_pool_t;

// [SEQUENCE: MVP2-5]
// 스레드 풀 생명주기 함수
thread_pool_t* thread_pool_create(int thread_count);
void thread_pool_destroy(thread_pool_t* pool);

// [SEQUENCE: MVP2-6]
// 작업 제출 및 대기 함수
int thread_pool_add_job(thread_pool_t* pool, void (*function)(void*), void* arg);
void thread_pool_wait(thread_pool_t* pool);

#endif // THREAD_POOL_H
