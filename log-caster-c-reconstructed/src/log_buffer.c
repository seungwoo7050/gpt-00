// [SEQUENCE: MVP2-24]
#include "log_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// [SEQUENCE: MVP2-25]
// 로그 버퍼 생성
log_buffer_t* log_buffer_create(size_t capacity) {
    if (capacity == 0) capacity = DEFAULT_BUFFER_SIZE;
    
    log_buffer_t* buffer = calloc(1, sizeof(log_buffer_t));
    if (!buffer) return NULL;
    
    buffer->entries = calloc(capacity, sizeof(log_entry_t*));
    if (!buffer->entries) {
        free(buffer);
        return NULL;
    }
    
    buffer->capacity = capacity;
    pthread_mutex_init(&buffer->mutex, NULL);
    
    return buffer;
}

// [SEQUENCE: MVP2-26]
// 로그 버퍼에 로그 추가
int log_buffer_push(log_buffer_t* buffer, const char* message) {
    if (!buffer || !message) return -1;
    
    log_entry_t* entry = malloc(sizeof(log_entry_t));
    if (!entry) return -1;
    
    entry->message = strdup(message);
    if (!entry->message) {
        free(entry);
        return -1;
    }
    entry->timestamp = time(NULL);
    
    pthread_mutex_lock(&buffer->mutex);
    
    // [SEQUENCE: MVP2-27]
    // 버퍼가 가득 찼으면 가장 오래된 로그를 삭제 (덮어쓰기)
    if (buffer->size == buffer->capacity) {
        log_entry_t* old_entry = buffer->entries[buffer->tail];
        free(old_entry->message);
        free(old_entry);
        buffer->tail = (buffer->tail + 1) % buffer->capacity;
        buffer->size--;
        buffer->dropped_logs++;
    }
    
    // [SEQUENCE: MVP2-28]
    // 새로운 로그를 head 위치에 추가
    buffer->entries[buffer->head] = entry;
    buffer->head = (buffer->head + 1) % buffer->capacity;
    buffer->size++;
    buffer->total_logs++;
    
    pthread_mutex_unlock(&buffer->mutex);
    return 0;
}

// [SEQUENCE: C-MVP2-29]
// 키워드를 포함하는 로그 검색 (MVP2 기본 버전)
int log_buffer_search(log_buffer_t* buffer, const char* keyword, char*** results, int* count) {
    if (!buffer || !keyword || !results || !count) return -1;
    
    pthread_mutex_lock(&buffer->mutex);
    
    // 1. 일치하는 로그 개수 세기
    *count = 0;
    for (size_t i = 0; i < buffer->size; i++) {
        size_t idx = (buffer->tail + i) % buffer->capacity;
        if (strstr(buffer->entries[idx]->message, keyword)) {
            (*count)++;
        }
    }
    
    if (*count == 0) {
        pthread_mutex_unlock(&buffer->mutex);
        *results = NULL;
        return 0;
    }
    
    // 2. 결과 저장을 위한 메모리 할당
    *results = malloc(sizeof(char*) * (*count));
    if (!*results) {
        pthread_mutex_unlock(&buffer->mutex);
        return -1;
    }
    
    // 3. 결과 복사
    int result_idx = 0;
    for (size_t i = 0; i < buffer->size && result_idx < *count; i++) {
        size_t idx = (buffer->tail + i) % buffer->capacity;
        if (strstr(buffer->entries[idx]->message, keyword)) {
            (*results)[result_idx++] = strdup(buffer->entries[idx]->message);
        }
    }
    
    pthread_mutex_unlock(&buffer->mutex);
    return 0;
}

// [SEQUENCE: C-MVP3-17]
#include "query_parser.h" // query_matches_log 사용을 위해 추가

// [SEQUENCE: C-MVP3-18]
// 고급 검색 기능 구현
int log_buffer_search_enhanced(log_buffer_t* buffer, const parsed_query_t* query, char*** results, int* count) {
    if (!buffer || !query || !results || !count) return -1;

    pthread_mutex_lock(&buffer->mutex);

    // 1. 일치하는 로그 개수 세기
    *count = 0;
    for (size_t i = 0; i < buffer->size; i++) {
        size_t idx = (buffer->tail + i) % buffer->capacity;
        if (query_matches_log(query, buffer->entries[idx]->message, buffer->entries[idx]->timestamp)) {
            (*count)++;
        }
    }

    if (*count == 0) {
        pthread_mutex_unlock(&buffer->mutex);
        *results = NULL;
        return 0;
    }

    // 2. 결과 저장을 위한 메모리 할당
    *results = malloc(sizeof(char*) * (*count));
    if (!*results) {
        pthread_mutex_unlock(&buffer->mutex);
        return -1;
    }

    // 3. 결과 복사 (타임스탬프 포함)
    int result_idx = 0;
    for (size_t i = 0; i < buffer->size && result_idx < *count; i++) {
        size_t idx = (buffer->tail + i) % buffer->capacity;
        log_entry_t* entry = buffer->entries[idx];
        if (query_matches_log(query, entry->message, entry->timestamp)) {
            char timestamp_str[32];
            strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%d %H:%M:%S", localtime(&entry->timestamp));
            
            size_t len = strlen(timestamp_str) + strlen(entry->message) + 5; // for "[] " and null
            char* res_str = malloc(len);
            snprintf(res_str, len, "[%s] %s", timestamp_str, entry->message);
            (*results)[result_idx++] = res_str;
        }
    }

    pthread_mutex_unlock(&buffer->mutex);
    return 0;
}

// [SEQUENCE: MVP2-30]
// 버퍼 상태 조회 함수들
size_t log_buffer_size(log_buffer_t* buffer) {
    if (!buffer) return 0;
    pthread_mutex_lock(&buffer->mutex);
    size_t size = buffer->size;
    pthread_mutex_unlock(&buffer->mutex);
    return size;
}

void log_buffer_get_stats(log_buffer_t* buffer, unsigned long* total, unsigned long* dropped) {
    if (!buffer) return;
    pthread_mutex_lock(&buffer->mutex);
    *total = buffer->total_logs;
    *dropped = buffer->dropped_logs;
    pthread_mutex_unlock(&buffer->mutex);
}

// [SEQUENCE: MVP2-31]
// 로그 버퍼 소멸
void log_buffer_destroy(log_buffer_t* buffer) {
    if (!buffer) return;
    for (size_t i = 0; i < buffer->size; i++) {
        size_t idx = (buffer->tail + i) % buffer->capacity;
        free(buffer->entries[idx]->message);
        free(buffer->entries[idx]);
    }
    pthread_mutex_destroy(&buffer->mutex);
    free(buffer->entries);
    free(buffer);
}
