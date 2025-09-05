// [SEQUENCE: C-MVP2-49]
#include "server.h"
// [SEQUENCE: C-MVP4-23]
#include "persistence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
// [SEQUENCE: C-MVP5-12]
#include <errno.h>
#include <limits.h>
#include <stdint.h>


// [SEQUENCE: C-MVP2-49]
int main(int argc, char* argv[]) {
    // [SEQUENCE: C-MVP4-23]
    int port = DEFAULT_PORT;
    persistence_config_t persist_config = { .enabled = false };
    strncpy(persist_config.log_directory, "./logs", sizeof(persist_config.log_directory) - 1);
    persist_config.max_file_size = 10 * 1024 * 1024; // 10MB

    // [SEQUENCE: C-MVP4-24]
    // getopt를 사용한 커맨드 라인 인자 파싱
    int opt;
    while ((opt = getopt(argc, argv, "p:d:s:Ph")) != -1) {
        switch (opt) {
            case 'p': {
                // [SEQUENCE: C-MVP5-13]
                // 포트 번호에 대한 안전한 파싱
                char* endptr;
                errno = 0;
                long val = strtol(optarg, &endptr, 10);
                if (errno != 0 || *endptr != '\0' || val <= 0 || val > 65535) {
                    fprintf(stderr, "Invalid port number: %s. Must be between 1 and 65535.\n", optarg);
                    return 1;
                }
                port = (int)val;
                break;
            }
            case 'P': persist_config.enabled = true; break;
            case 'd': strncpy(persist_config.log_directory, optarg, sizeof(persist_config.log_directory) - 1); break;
            case 's': {
                // [SEQUENCE: C-MVP5-14]
                // 파일 크기에 대한 안전한 파싱 및 오버플로우 방지
                char* endptr;
                errno = 0;
                long mb_val = strtol(optarg, &endptr, 10);
                if (errno != 0 || *endptr != '\0' || mb_val <= 0) {
                    fprintf(stderr, "Invalid file size: %s\n", optarg);
                    return 1;
                }
                if (mb_val > (long)(SIZE_MAX / (1024 * 1024))) {
                    fprintf(stderr, "File size too large: %s MB\n", optarg);
                    return 1;
                }
                persist_config.max_file_size = (size_t)mb_val * 1024 * 1024;
                break;
            }
            case 'h':
                printf("Usage: %s [-p port] [-P] [-d dir] [-s size_mb] [-h]\n", argv[0]);
                return 0;
        }
    }

    // [SEQUENCE: C-MVP2-50]
    // 서버 생성
    log_server_t* server = server_create(port);
    if (!server) {
        fprintf(stderr, "Failed to create server.\n");
        return 1;
    }

    // [SEQUENCE: C-MVP4-25]
    // 영속성 기능이 활성화되었으면, 관리자를 생성하여 서버에 연결
    if (persist_config.enabled) {
        printf("Persistence enabled. Dir: %s, Max Size: %zu MB\n", 
               persist_config.log_directory, persist_config.max_file_size / (1024*1024));
        server->persistence = persistence_create(&persist_config);
        if (!server->persistence) {
            fprintf(stderr, "Failed to initialize persistence.\n");
            server_destroy(server);
            return 1;
        }
    }


    // [SEQUENCE: C-MVP2-51]
    // 서버 초기화
    if (server_init(server) < 0) {
        fprintf(stderr, "Failed to initialize server.\n");
        server_destroy(server);
        return 1;
    }

    // [SEQUENCE: C-MVP2-52]
    // 서버 실행 및 소멸
    server_run(server);
    server_destroy(server);

    return 0;
}