// [SEQUENCE: C-MVP2-14]
#ifndef LOGBUFFER_H
#define LOGBUFFER_H

#include <deque>
#include <string>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>
#include <functional>
#include <map>

// [SEQUENCE: C-MVP3-11]
// Forward declaration
class ParsedQuery;

// [SEQUENCE: CPP-MVP6-2]
// LogEntry 확장 및 콜백 타입 정의
struct LogEntry {
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    std::string level;
    std::string source;

    LogEntry(std::string msg, std::string lvl, std::string src)
        : message(std::move(msg)), timestamp(std::chrono::system_clock::now()), level(std::move(lvl)), source(std::move(src)) {}
};

// [SEQUENCE: CPP-MVP6-3]
typedef std::function<void(const LogEntry&)> LogCallback;

// [SEQUENCE: C-MVP2-16]
// 로그 버퍼 클래스
class LogBuffer {
public:
    explicit LogBuffer(size_t capacity = 10000);
    ~LogBuffer() = default;

    void push(std::string message, const std::string& level, const std::string& source);
    std::vector<std::string> search(const std::string& keyword) const;

    // [SEQUENCE: C-MVP3-12]
    // MVP3에 추가된 고급 검색 메소드
    std::vector<std::string> searchEnhanced(const ParsedQuery& query) const;

    // [SEQUENCE: CPP-MVP6-4]
    void registerCallback(const std::string& channel, LogCallback callback);

    struct StatsSnapshot {
        uint64_t totalLogs;
        uint64_t droppedLogs;
    };
    StatsSnapshot getStats() const;
    size_t size() const;

private:
    void dropOldest_();

    mutable std::mutex mutex_;
    std::deque<LogEntry> buffer_;
    size_t capacity_;

    std::atomic<uint64_t> totalLogs_{0};
    std::atomic<uint64_t> droppedLogs_{0};

    // [SEQUENCE: CPP-MVP6-5]
    std::map<std::string, std::vector<LogCallback>> callbacks_;
};

#endif // LOGBUFFER_H