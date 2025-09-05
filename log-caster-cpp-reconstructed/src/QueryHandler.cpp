// [SEQUENCE: C-MVP2-24]
#include "QueryHandler.h"
// [SEQUENCE: C-MVP3-16]
#include "QueryParser.h"
#include <sstream>

QueryHandler::QueryHandler(std::shared_ptr<LogBuffer> buffer) : buffer_(buffer) {}

// [SEQUENCE: C-MVP2-25]
// 쿼리 문자열을 파싱하고 적절한 핸들러 호출
// [SEQUENCE: C-MVP3-17]
// 쿼리 처리 로직 (MVP3 버전)
std::string QueryHandler::processQuery(const std::string& query) {
    if (query.find("QUERY") == 0) {
        return handleSearch(query);
    } else if (query == "STATS") {
        return handleStats();
    } else if (query == "COUNT") {
        return handleCount();
    } else if (query == "HELP") {
        return handleHelp();
    }
    return "ERROR: Unknown command. Use HELP for usage.\n";
}

// [SEQUENCE: C-MVP3-18]
// 검색 쿼리 처리 로직 수정
std::string QueryHandler::handleSearch(const std::string& query) {
    try {
        // [SEQUENCE: C-MVP3-19]
        // QueryParser를 사용하여 쿼리 문자열을 파싱
        auto parsed_query = QueryParser::parse(query);
        if (!parsed_query) {
            return "ERROR: Failed to parse query.\n";
        }

        // [SEQUENCE: C-MVP3-20]
        // LogBuffer의 고급 검색 메소드 호출
        auto results = buffer_->searchEnhanced(*parsed_query);
        std::stringstream ss;
        ss << "FOUND: " << results.size() << " matches\n";
        for (const auto& res : results) {
            ss << res << "\n";
        }
        return ss.str();
    } catch (const std::exception& e) {
        return std::string("ERROR: ") + e.what() + "\n";
    }
}


// [SEQUENCE: C-MVP2-27]
// 통계 쿼리 처리
std::string QueryHandler::handleStats() {
    auto stats = buffer_->getStats();
    std::stringstream ss;
    ss << "STATS: Total=" << stats.totalLogs << ", Dropped=" << stats.droppedLogs 
       << ", Current=" << buffer_->size() << "\n";
    return ss.str();
}

// [SEQUENCE: C-MVP2-28]
// 카운트 쿼리 처리
std::string QueryHandler::handleCount() {
    std::stringstream ss;
    ss << "COUNT: " << buffer_->size() << "\n";
    return ss.str();
}

// [SEQUENCE: C-MVP3-21]
// HELP 명령 내용 보강
std::string QueryHandler::handleHelp() {
    return "Available commands:\n"
           "  STATS - Show buffer statistics\n"
           "  COUNT - Show number of logs in buffer\n"
           "  HELP  - Show this help message\n"
           "  QUERY <parameters> - Search logs with parameters:\n"
           "\n"
           "Query parameters:\n"
           "  keywords=<w1,w2,..> - Multiple keywords (comma-separated)\n"
           "  operator=<AND|OR>   - Keyword matching logic (default: AND)\n"
           "  regex=<pattern>     - Regular expression pattern (case-insensitive)\n"
           "  time_from=<unix_ts> - Start time (Unix timestamp)\n"
           "  time_to=<unix_ts>   - End time (Unix timestamp)\n"
           "\n"
           "Example: QUERY keywords=error,timeout operator=AND regex=failed\n";
}
