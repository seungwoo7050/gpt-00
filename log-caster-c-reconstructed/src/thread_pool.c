// [SEQUENCE: MVP2-7]
#include "thread_pool.h"
#include <stdlib.h>
#include <stdio.h>

// [SEQUENCE: MVP2-8]
// 작업자 스레드가 실행하는 함수
static void* thread_pool_worker(void* arg) {
    thread_pool_t* pool = (thread_pool_t*)arg;
    thread_job_t* job;
    
    while (1) {
        // [SEQUENCE: MVP2-9]
        // 작업 큐에 접근하기 위해 뮤텍스 잠금
        pthread_mutex_lock(&pool->queue_mutex);
        
        // 작업 큐가 비어있고, 풀이 종료되지 않았으면 대기
        while (pool->job_count == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->job_available, &pool->queue_mutex);
        }
        
        // 풀이 종료 신호를 받았으면 루프 탈출
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }
        
        // [SEQUENCE: MVP2-10]
        // 작업 큐에서 작업(job)을 하나 가져옴
        job = pool->job_queue_head;
        pool->job_queue_head = job->next;
        if (!pool->job_queue_head) {
            pool->job_queue_tail = NULL;
        }
        pool->job_count--;
        pool->working_threads++;
        
        pthread_mutex_unlock(&pool->queue_mutex);
        
        // [SEQUENCE: MVP2-11]
        // 잠금을 해제한 상태에서 실제 작업 수행
        if (job) {
            job->function(job->arg);
            free(job);
        }

        pthread_mutex_lock(&pool->queue_mutex);
        pool->working_threads--;
        if (!pool->shutdown && pool->working_threads == 0 && pool->job_count == 0) {
            pthread_cond_signal(&pool->all_jobs_done);
        }
        pthread_mutex_unlock(&pool->queue_mutex);
    }
    
    pthread_exit(NULL);
}

// [SEQUENCE: MVP2-12]
// 스레드 풀 생성
thread_pool_t* thread_pool_create(int thread_count) {
    if (thread_count <= 0 || thread_count > MAX_THREAD_COUNT) {
        thread_count = DEFAULT_THREAD_COUNT;
    }
    
    thread_pool_t* pool = calloc(1, sizeof(thread_pool_t));
    if (!pool) return NULL;

    pool->thread_count = thread_count;
    pool->threads = malloc(sizeof(pthread_t) * thread_count);
    if (!pool->threads) {
        free(pool);
        return NULL;
    }
    
    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->job_available, NULL);
    pthread_cond_init(&pool->all_jobs_done, NULL);
    
    // [SEQUENCE: MVP2-13]
    // 작업자 스레드 생성
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&pool->threads[i], NULL, thread_pool_worker, pool) != 0) {
            thread_pool_destroy(pool); // 실패 시 정리
            return NULL;
        }
    }
    
    return pool;
}

// [SEQUENCE: MVP2-14]
// 작업 큐에 작업 추가
int thread_pool_add_job(thread_pool_t* pool, void (*function)(void*), void* arg) {
    if (!pool || !function || pool->shutdown) return -1;

    thread_job_t* job = malloc(sizeof(thread_job_t));
    if (!job) return -1;

    job->function = function;
    job->arg = arg;
    job->next = NULL;

    pthread_mutex_lock(&pool->queue_mutex);
    
    if (pool->job_queue_tail) {
        pool->job_queue_tail->next = job;
        pool->job_queue_tail = job;
    } else {
        pool->job_queue_head = job;
        pool->job_queue_tail = job;
    }
    pool->job_count++;

    // [SEQUENCE: MVP2-15]
    // 대기 중인 스레드에게 새 작업이 있음을 알림
    pthread_cond_signal(&pool->job_available);
    pthread_mutex_unlock(&pool->queue_mutex);

    return 0;
}

// [SEQUENCE: MVP2-16]
// 모든 작업이 완료될 때까지 대기
void thread_pool_wait(thread_pool_t* pool) {
    if (!pool) return;
    pthread_mutex_lock(&pool->queue_mutex);
    while (pool->job_count > 0 || pool->working_threads > 0) {
        pthread_cond_wait(&pool->all_jobs_done, &pool->queue_mutex);
    }
    pthread_mutex_unlock(&pool->queue_mutex);
}

// [SEQUENCE: MVP2-17]
// 스레드 풀 종료 및 리소스 해제
void thread_pool_destroy(thread_pool_t* pool) {
    if (!pool) return;

    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = true;
    pthread_cond_broadcast(&pool->job_available);
    pthread_mutex_unlock(&pool->queue_mutex);

    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    // 남은 작업 큐 정리
    thread_job_t* job;
    while (pool->job_queue_head) {
        job = pool->job_queue_head;
        pool->job_queue_head = job->next;
        free(job);
    }

    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->job_available);
    pthread_cond_destroy(&pool->all_jobs_done);

    free(pool->threads);
    free(pool);
}
