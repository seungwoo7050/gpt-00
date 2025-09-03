// [HISTORICAL NOTE for C++ MVP3]
// This C++ Query Parser was introduced in the third C++ MVP. It provides
// the same advanced query features as the C version's MVP3, but is implemented
// using modern C++ features like std::regex, std::optional, and std::chrono
// for improved type safety and expressiveness.
//
// To see the original specification for this module, please refer to:
// - Document: /devhistory/DevHistory/DevHistory06.md
// - Sequences: [SEQUENCE: MVP3-3] through [SEQUENCE: MVP3-10]

#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <string>
#include <vector>
#include <regex>
#include <chrono>
#include <memory>
#include <optional>
#include <unordered_map>
#include <functional>

// [SEQUENCE: 270] Query parameter types for C++ version
enum class ParamType {
    KEYWORD,
    KEYWORDS,
    REGEX,
    TIME_FROM,
    TIME_TO,
    OPERATOR
};

// [SEQUENCE: 271] Operator types for multi-keyword search
enum class OperatorType {
    AND,
    OR
};

// [SEQUENCE: 272] Parsed query structure with modern C++ features
class ParsedQuery {
public:
    std::vector<std::string> keywords;
    std::optional<std::regex> compiled_regex;
    std::optional<std::string> regex_pattern;
    std::optional<std::chrono::system_clock::time_point> time_from;
    std::optional<std::chrono::system_clock::time_point> time_to;
    OperatorType op = OperatorType::AND;
    
    // [SEQUENCE: 273] Check if query matches log entry
    bool matches(const std::string& log_message, 
                 const std::chrono::system_clock::time_point& timestamp) const;
};

// [SEQUENCE: 274] Modern C++ query parser with builder pattern
class QueryParser {
private:
    static const std::unordered_map<std::string, ParamType> param_types;
    
    // [SEQUENCE: 275] Parse individual parameter
    static ParamType getParamType(const std::string& param);
    static std::vector<std::string> splitString(const std::string& str, char delimiter);
    static OperatorType parseOperator(const std::string& value);
    
public:
    // [SEQUENCE: 276] Parse query string into structured query
    static std::unique_ptr<ParsedQuery> parse(const std::string& query_string);
};

#endif // QUERY_PARSER_H