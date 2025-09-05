// [SEQUENCE: MVP4-3]
#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include <stdio.h>

// [SEQUENCE: MVP4-4]
// 영속성 설정 구조체
typedef struct {
    bool enabled;
    char log_directory[256];
    size_t max_file_size;
} persistence_config_t;

// [SEQUENCE: MVP4-5]
// 파일에 쓸 로그 항목 (큐에 저장될 데이터)
typedef struct write_entry {
    char* message;
    struct write_entry* next;
} write_entry_t;

// [SEQUENCE: MVP4-6]
// 영속성 관리자 메인 구조체
typedef struct {
    persistence_config_t config;
    FILE* current_file;
    char current_filepath[512];
    size_t current_file_size;
    pthread_t writer_thread;
    write_entry_t* write_queue;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
    bool stop_thread;
} persistence_manager_t;

// [SEQUENCE: MVP4-7]
// 영속성 관리자 생명주기 및 API 함수
persistence_manager_t* persistence_create(const persistence_config_t* config);
void persistence_destroy(persistence_manager_t* manager);
int persistence_write(persistence_manager_t* manager, const char* message);

#endif // PERSISTENCE_H
