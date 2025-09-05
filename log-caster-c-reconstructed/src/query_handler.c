// [SEQUENCE: C-MVP2-33]
#include "query_handler.h"
#include "server.h"
// [SEQUENCE: C-MVP3-19]
#include "query_parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

// [SEQUENCE: C-MVP3-20]
// HELP 명령에 대한 응답 생성
static void send_help(int client_fd) {
    const char* help_msg = 
        "LogCaster Query Interface - MVP3\n"
        "================================\n"
        "Commands:\n"
        "  STATS                    - Show statistics\n"
        "  COUNT                    - Show log count\n"
        "  HELP                     - Show this help\n"
        "  QUERY param=value ...    - Enhanced search\n"
        "\n"
        "Query Parameters:\n"
        "  keywords=word1,word2     - Multiple keywords\n"
        "  regex=pattern            - POSIX regex pattern\n"
        "  time_from=timestamp      - Start time (Unix timestamp)\n"
        "  time_to=timestamp        - End time (Unix timestamp)\n"
        "  operator=AND|OR          - Keyword logic (default: AND)\n"
        "\n"
        "Example: QUERY keywords=error,timeout operator=AND regex=.*failed.*\n";
    send(client_fd, help_msg, strlen(help_msg), 0);
}

// [SEQUENCE: C-MVP2-34]
// 쿼리 명령 처리
// [SEQUENCE: C-MVP3-21]
// 쿼리 명령 처리 로직 (MVP3 버전)
static void process_query_command(log_server_t* server, int client_fd, const char* command) {
    char response[BUFFER_SIZE];

    if (strncmp(command, "QUERY", 5) == 0) {
        parsed_query_t* query = query_parser_create();
                if (query_parser_parse(query, command) != 0) {
            snprintf(response, sizeof(response), "ERROR: Invalid query syntax\n");
            send(client_fd, response, strlen(response), 0);
        } else {
            char** results = NULL;

            int count = 0;
            log_buffer_search_enhanced(server->log_buffer, query, &results, &count);
            snprintf(response, sizeof(response), "FOUND: %d matches\n", count);
            send(client_fd, response, strlen(response), 0);
            for (int i = 0; i < count; i++) {
                snprintf(response, sizeof(response), "%s\n", results[i]);
                send(client_fd, response, strlen(response), 0);
                free(results[i]);
            }
            if (results) free(results);
        }
        query_parser_destroy(query);
    } else if (strcmp(command, "STATS") == 0) {
        unsigned long total, dropped;
        log_buffer_get_stats(server->log_buffer, &total, &dropped);
        snprintf(response, sizeof(response), "STATS: Total=%lu, Dropped=%lu, Current=%zu, Clients=%d\n",
                 total, dropped, log_buffer_size(server->log_buffer), server->client_count);
        send(client_fd, response, strlen(response), 0);
    } else if (strcmp(command, "COUNT") == 0) {
        snprintf(response, sizeof(response), "COUNT: %zu\n", log_buffer_size(server->log_buffer));
        send(client_fd, response, strlen(response), 0);
    } else if (strcmp(command, "HELP") == 0) {
        send_help(client_fd);
    } else {
        snprintf(response, sizeof(response), "ERROR: Unknown command. Use HELP for usage.\n");
        send(client_fd, response, strlen(response), 0);
    }
}


// [SEQUENCE: C-MVP2-35]
// [SEQUENCE: C-MVP3-22]
// 쿼리 연결 수락 및 처리 (MVP2와 동일)
void handle_query_connection(log_server_t* server) {
    int client_fd = accept(server->query_fd, NULL, NULL);
    if (client_fd < 0) return;

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        }
        process_query_command(server, client_fd, buffer);
    }
    close(client_fd);
}
