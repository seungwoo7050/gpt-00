// [SEQUENCE: MVP3-7]
#include "QueryParser.h"
#include <sstream>
#include <algorithm>
#include <iostream>

// [SEQUENCE: MVP3-8]
// 쿼리 문자열을 파싱하여 ParsedQuery 객체를 생성
std::unique_ptr<ParsedQuery> QueryParser::parse(const std::string& query_string) {
    auto parsed_query = std::make_unique<ParsedQuery>();
    std::stringstream ss(query_string);
    std::string segment;
    std::vector<std::string> segments;

    // "QUERY" 단어 스킵
    ss >> segment;

    while (ss >> segment) {
        segments.push_back(segment);
    }

    for (const auto& seg : segments) {
        size_t pos = seg.find('=');
        if (pos == std::string::npos) continue;

        std::string key = seg.substr(0, pos);
        std::string value = seg.substr(pos + 1);

        // [SEQUENCE: MVP3-9]
        // 각 파라미터에 맞게 값 설정
        if (key == "keywords" || key == "keyword") {
            std::stringstream v_ss(value);
            std::string keyword;
            while (std::getline(v_ss, keyword, ',')) {
                parsed_query->keywords_.push_back(keyword);
            }
        } else if (key == "regex") {
            try {
                parsed_query->compiled_regex_.emplace(value, std::regex_constants::icase);
            } catch (const std::regex_error& e) {
                throw std::runtime_error("Invalid regex pattern: " + std::string(e.what()));
            }
        } else if (key == "time_from") {
            parsed_query->time_from_ = std::chrono::system_clock::from_time_t(std::stol(value));
        } else if (key == "time_to") {
            parsed_query->time_to_ = std::chrono::system_clock::from_time_t(std::stol(value));
        } else if (key == "operator") {
            std::transform(value.begin(), value.end(), value.begin(), ::toupper);
            if (value == "OR") {
                parsed_query->op_ = OperatorType::OR;
            }
        }
    }
    return parsed_query;
}

// [SEQUENCE: MVP3-10]
// 로그가 쿼리 조건에 부합하는지 검사
bool ParsedQuery::matches(const std::string& message, const std::chrono::system_clock::time_point& timestamp) const {
    // 시간 필터
    if (time_from_ && timestamp < *time_from_) return false;
    if (time_to_ && timestamp > *time_to_) return false;

    // 정규식 필터
    if (compiled_regex_ && !std::regex_search(message, *compiled_regex_)) {
        return false;
    }

    // 키워드 필터
    if (!keywords_.empty()) {
        if (op_ == OperatorType::AND) {
            for (const auto& kw : keywords_) {
                if (message.find(kw) == std::string::npos) return false;
            }
        } else { // OR
            bool found = false;
            for (const auto& kw : keywords_) {
                if (message.find(kw) != std::string::npos) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
    }
    return true;
}
