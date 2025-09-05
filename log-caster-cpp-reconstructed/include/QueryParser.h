// [SEQUENCE: MVP3-3]
#ifndef QUERYPARSER_H
#define QUERYPARSER_H

#include <string>
#include <vector>
#include <optional>
#include <regex>
#include <chrono>
#include <memory>
#include "LogBuffer.h" // For LogEntry

// [SEQUENCE: MVP3-4]
// 쿼리 연산자 종류
enum class OperatorType {
    AND,
    OR
};

// [SEQUENCE: MVP3-5]
// 파싱된 쿼리 정보를 담는 클래스
class ParsedQuery {
public:
    bool matches(const std::string& message, const std::chrono::system_clock::time_point& timestamp) const;

private:
    friend class QueryParser; // QueryParser가 private 멤버에 접근할 수 있도록 허용

    std::vector<std::string> keywords_;
    std::optional<std::regex> compiled_regex_;
    std::optional<std::chrono::system_clock::time_point> time_from_;
    std::optional<std::chrono::system_clock::time_point> time_to_;
    OperatorType op_ = OperatorType::AND;
};

// [SEQUENCE: MVP3-6]
// 쿼리 문자열을 파싱하는 정적 클래스
class QueryParser {
public:
    static std::unique_ptr<ParsedQuery> parse(const std::string& query_string);
};

#endif // QUERYPARSER_H
