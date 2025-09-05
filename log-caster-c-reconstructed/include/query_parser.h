// [SEQUENCE: MVP3-3]
#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <stdbool.h>
#include <time.h>
#include <regex.h>

// [SEQUENCE: MVP3-4]
// 쿼리 연산자 종류 (AND/OR)
typedef enum {
    OP_AND,
    OP_OR
} operator_type_t;

// [SEQUENCE: MVP3-5]
// 파싱된 쿼리 정보를 담는 구조체
typedef struct {
    char* keywords[10];          // 다중 키워드 배열
    int keyword_count;           // 키워드 개수
    char* regex_pattern;         // 정규식 패턴 문자열
    regex_t* compiled_regex;     // 컴파일된 정규식 객체
    time_t time_from;            // 시작 시간 필터
    time_t time_to;              // 종료 시간 필터
    operator_type_t op;          // 키워드 간 논리 연산자
    bool has_regex;
    bool has_time_filter;
} parsed_query_t;

// [SEQUENCE: MVP3-6]
// 쿼리 파서 생명주기 및 파싱 함수
parsed_query_t* query_parser_create(void);
void query_parser_destroy(parsed_query_t* query);
int query_parser_parse(parsed_query_t* query, const char* query_string);

// [SEQUENCE: MVP3-7]
// 로그 항목이 쿼리 조건과 일치하는지 확인하는 함수
bool query_matches_log(const parsed_query_t* query, const char* log_message, time_t timestamp);

#endif // QUERY_PARSER_H
