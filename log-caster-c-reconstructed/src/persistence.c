// [SEQUENCE: MVP4-8]
#include "persistence.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

static void* persistence_writer_thread(void* arg);
static void rotate_log_file(persistence_manager_t* manager);

// [SEQUENCE: MVP4-9]
// 영속성 관리자 생성
persistence_manager_t* persistence_create(const persistence_config_t* config) {
    persistence_manager_t* manager = calloc(1, sizeof(persistence_manager_t));
    if (!manager) return NULL;

    manager->config = *config;
    pthread_mutex_init(&manager->queue_mutex, NULL);
    pthread_cond_init(&manager->queue_cond, NULL);

    // 로그 디렉토리 생성
    mkdir(manager->config.log_directory, 0755);
    snprintf(manager->current_filepath, sizeof(manager->current_filepath), "%s/current.log", manager->config.log_directory);

    // 현재 로그 파일 열기
    manager->current_file = fopen(manager->current_filepath, "a");
    if (!manager->current_file) {
        perror("Failed to open log file");
        free(manager);
        return NULL;
    }
    manager->current_file_size = ftell(manager->current_file);

    // [SEQUENCE: MVP4-10]
    // Writer 스레드 생성
    if (pthread_create(&manager->writer_thread, NULL, persistence_writer_thread, manager) != 0) {
        fclose(manager->current_file);
        free(manager);
        return NULL;
    }

    return manager;
}

// [SEQUENCE: MVP4-11]
// 로그 쓰기 요청 (큐에 추가)
int persistence_write(persistence_manager_t* manager, const char* message) {
    if (!manager || !manager->config.enabled) return -1;

    write_entry_t* entry = malloc(sizeof(write_entry_t));
    if (!entry) return -1;
    entry->message = strdup(message);
    entry->next = NULL;

    pthread_mutex_lock(&manager->queue_mutex);
    if (manager->write_queue == NULL) {
        manager->write_queue = entry;
    } else {
        write_entry_t* tail = manager->write_queue;
        while (tail->next) tail = tail->next;
        tail->next = entry;
    }
    pthread_cond_signal(&manager->queue_cond);
    pthread_mutex_unlock(&manager->queue_mutex);

    return 0;
}

// [SEQUENCE: MVP4-12]
// Writer 스레드 메인 루프
static void* persistence_writer_thread(void* arg) {
    persistence_manager_t* manager = (persistence_manager_t*)arg;
    while (!manager->stop_thread) {
        pthread_mutex_lock(&manager->queue_mutex);
        while (manager->write_queue == NULL && !manager->stop_thread) {
            pthread_cond_wait(&manager->queue_cond, &manager->queue_mutex);
        }

        write_entry_t* head = manager->write_queue;
        manager->write_queue = NULL; // 큐를 통째로 가져옴
        pthread_mutex_unlock(&manager->queue_mutex);

        // 큐에 있던 모든 로그를 파일에 씀
        while (head) {
            if (manager->current_file) {
                size_t len = fprintf(manager->current_file, "%s\n", head->message);
                fflush(manager->current_file);
                manager->current_file_size += len;

                // [SEQUENCE: MVP4-13]
                // 파일 크기 확인 및 로테이션
                if (manager->current_file_size >= manager->config.max_file_size) {
                    rotate_log_file(manager);
                }
            }
            write_entry_t* temp = head;
            head = head->next;
            free(temp->message);
            free(temp);
        }
    }
    return NULL;
}

// [SEQUENCE: MVP4-14]
// 로그 파일 로테이션
static void rotate_log_file(persistence_manager_t* manager) {
    fclose(manager->current_file);

    char new_filepath[512];
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    snprintf(new_filepath, sizeof(new_filepath), "%s/log-%04d%02d%02d-%02d%02d%02d.log", 
             manager->config.log_directory, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, 
             tm->tm_hour, tm->tm_min, tm->tm_sec);

    rename(manager->current_filepath, new_filepath);

    manager->current_file = fopen(manager->current_filepath, "a");
    manager->current_file_size = 0;
}

// [SEQUENCE: MVP4-15]
// 영속성 관리자 소멸
void persistence_destroy(persistence_manager_t* manager) {
    if (!manager) return;
    pthread_mutex_lock(&manager->queue_mutex);
    manager->stop_thread = true;
    pthread_cond_signal(&manager->queue_cond);
    pthread_mutex_unlock(&manager->queue_mutex);

    pthread_join(manager->writer_thread, NULL);

    if (manager->current_file) {
        fclose(manager->current_file);
    }
    // 남은 큐 정리 로직 (생략)
    pthread_mutex_destroy(&manager->queue_mutex);
    pthread_cond_destroy(&manager->queue_cond);
    free(manager);
}
