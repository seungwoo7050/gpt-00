// [SEQUENCE: MVP3-8]
#include "query_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <strings.h> // for strcasecmp

// [SEQUENCE: MVP3-9]
// parsed_query_t 구조체 생성 및 초기화
parsed_query_t* query_parser_create(void) {
    parsed_query_t* query = calloc(1, sizeof(parsed_query_t));
    if (query) {
        query->op = OP_AND; // 기본 연산자는 AND
    }
    return query;
}

// [SEQUENCE: MVP3-10]
// 파서가 할당한 모든 동적 메모리 해제
void query_parser_destroy(parsed_query_t* query) {
    if (!query) return;
    for (int i = 0; i < query->keyword_count; i++) {
        free(query->keywords[i]);
    }
    if (query->regex_pattern) {
        free(query->regex_pattern);
    }
    if (query->compiled_regex) {
        regfree(query->compiled_regex);
        free(query->compiled_regex);
    }
    free(query);
}

// [SEQUENCE: MVP3-11]
// 쿼리 문자열 파싱
int query_parser_parse(parsed_query_t* query, const char* query_string) {
    if (!query || !query_string) return -1;

        const char* params_start = query_string;
    if (strncmp(params_start, "QUERY ", 6) == 0) {
        params_start += 6;
    }
    char* params_copy = strdup(params_start);
    if (!params_copy) return -1;

    // [SEQUENCE: C-MVP3-12]
    // `strtok_r`을 사용하여 스페이스 기준으로 파라미터 분리 (스레드 안전)
    char* saveptr1;
    char* token = strtok_r(params_copy, " ", &saveptr1);
    while (token) {
        char* equals = strchr(token, '=');
        if (equals) {
            *equals = '\0'; // key와 value 분리
            char* key = token;
            char* value = equals + 1;

            char* key_copy = strdup(key);
            char* value_copy = strdup(value);

            // [SEQUENCE: C-MVP3-13]
            // 파라미터 종류에 따라 처리
            if (strcasecmp(key_copy, "keywords") == 0 || strcasecmp(key_copy, "keyword") == 0) {
                char* saveptr2;
                char* keyword_token = strtok_r(value_copy, ",", &saveptr2);
                while (keyword_token && query->keyword_count < 10) {
                    query->keywords[query->keyword_count++] = strdup(keyword_token);
                    keyword_token = strtok_r(NULL, ",", &saveptr2);
                }
            } else if (strcasecmp(key_copy, "regex") == 0) {
                query->has_regex = true;
                query->regex_pattern = strdup(value_copy);
                query->compiled_regex = malloc(sizeof(regex_t));
                if (regcomp(query->compiled_regex, value_copy, REG_EXTENDED | REG_NOSUB) != 0) {
                    // 정규식 컴파일 실패 처리
                    free(query->compiled_regex);
                    query->compiled_regex = NULL;
                    // [SEQUENCE: C-MVP5-11]
                    if (query->regex_pattern) {
                        free(query->regex_pattern);
                        query->regex_pattern = NULL;
                    }
                }
            } else if (strcasecmp(key_copy, "time_from") == 0) {
                query->has_time_filter = true;
                query->time_from = atol(value_copy);
            } else if (strcasecmp(key_copy, "time_to") == 0) {
                query->has_time_filter = true;
                query->time_to = atol(value_copy);
            } else if (strcasecmp(key_copy, "operator") == 0) {
                if (strcasecmp(value_copy, "OR") == 0) {
                    query->op = OP_OR;
                }
            }
            free(key_copy);
            free(value_copy);
        }
        token = strtok_r(NULL, " ", &saveptr1);
    }

    free(params_copy);
    return 0;
}

// [SEQUENCE: MVP3-14]
// 로그가 쿼리 조건에 부합하는지 검사
bool query_matches_log(const parsed_query_t* query, const char* log_message, time_t timestamp) {
    // 시간 필터 검사
    if (query->has_time_filter) {
        if (query->time_from > 0 && timestamp < query->time_from) return false;
        if (query->time_to > 0 && timestamp > query->time_to) return false;
    }

    // 정규식 필터 검사
    if (query->has_regex && query->compiled_regex) {
        if (regexec(query->compiled_regex, log_message, 0, NULL, 0) != 0) {
            return false; // 정규식 불일치
        }
    }

    // 키워드 필터 검사
    if (query->keyword_count > 0) {
        bool match = (query->op == OP_AND);
        for (int i = 0; i < query->keyword_count; i++) {
            bool found = (strstr(log_message, query->keywords[i]) != NULL);
            if (query->op == OP_AND && !found) return false;
            if (query->op == OP_OR && found) {
                match = true;
                break;
            }
        }
        if (!match) return false;
    }

    return true;
}
