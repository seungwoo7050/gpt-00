// [SEQUENCE: C-MVP2-18]
#ifndef LOG_BUFFER_H
#define LOG_BUFFER_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

// [SEQUENCE: C-MVP3-15]
#include "query_parser.h"


#define DEFAULT_BUFFER_SIZE 10000

// [SEQUENCE: C-MVP2-19]
// 로그 항목을 표현하는 구조체
typedef struct {
    char* message;
    time_t timestamp;
} log_entry_t;

// [SEQUENCE: C-MVP2-20]
// 스레드 안전 원형 로그 버퍼 구조체
typedef struct {
    log_entry_t** entries;    // 로그 항목 포인터의 원형 배열
    size_t capacity;          // 최대 용량
    size_t size;              // 현재 크기
    size_t head;              // 다음 쓰기 위치
    size_t tail;              // 다음 읽기 위치
    
    pthread_mutex_t mutex;    // 버퍼 접근을 위한 뮤텍스
    
    // 통계 정보
    unsigned long total_logs;
    unsigned long dropped_logs;
} log_buffer_t;

// [SEQUENCE: C-MVP2-21]
// 버퍼 생명주기 함수
log_buffer_t* log_buffer_create(size_t capacity);
void log_buffer_destroy(log_buffer_t* buffer);

// [SEQUENCE: C-MVP2-22]
// 버퍼 조작 함수
int log_buffer_push(log_buffer_t* buffer, const char* message);

// [SEQUENCE: C-MVP3-16]
// MVP3에 추가된 고급 검색 함수
int log_buffer_search_enhanced(log_buffer_t* buffer, const parsed_query_t* query, char*** results, int* count);


// [SEQUENCE: C-MVP2-23]
// 버퍼 상태 조회 함수
size_t log_buffer_size(log_buffer_t* buffer);
void log_buffer_get_stats(log_buffer_t* buffer, unsigned long* total, unsigned long* dropped);

#endif // LOG_BUFFER_H
