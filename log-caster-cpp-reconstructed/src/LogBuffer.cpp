// [SEQUENCE: CPP-MVP2-17]
#include "LogBuffer.h"
#include "QueryParser.h"
#include <sstream>
#include <iomanip>

LogBuffer::LogBuffer(size_t capacity) : capacity_(capacity) {}

// [SEQUENCE: CPP-MVP6-6]
void LogBuffer::push(std::string message, const std::string& level, const std::string& source) {
    LogEntry entry(std::move(message), level, source);
    std::lock_guard<std::mutex> lock(mutex_);
    if (buffer_.size() >= capacity_) {
        dropOldest_();
    }
    buffer_.push_back(entry);
    totalLogs_++;

    // Notify callbacks
    for (auto const& [channel, callbacks] : callbacks_) {
        // Simple matching for now
        if (channel == "#logs-all" || (channel == "#logs-error" && level == "ERROR")) {
            for (const auto& callback : callbacks) {
                callback(entry);
            }
        }
    }
}

void LogBuffer::dropOldest_() {
    if (!buffer_.empty()) {
        buffer_.pop_front();
        droppedLogs_++;
    }
}

std::vector<std::string> LogBuffer::search(const std::string& keyword) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> results;
    for (const auto& entry : buffer_) {
        if (entry.message.find(keyword) != std::string::npos) {
            auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
            std::stringstream ss;
            ss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
            ss << entry.message;
            results.push_back(ss.str());
        }
    }
    return results;
}

std::vector<std::string> LogBuffer::searchEnhanced(const ParsedQuery& query) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> results;
    for (const auto& entry : buffer_) {
        if (query.matches(entry.message, entry.timestamp)) {
            auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
            std::stringstream ss;
            ss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
            ss << entry.message;
            results.push_back(ss.str());
        }
    }
    return results;
}

// [SEQUENCE: CPP-MVP6-8]
void LogBuffer::registerCallback(const std::string& channel, LogCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_[channel].push_back(callback);
}

size_t LogBuffer::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_.size();
}

LogBuffer::StatsSnapshot LogBuffer::getStats() const {
    return { totalLogs_.load(), droppedLogs_.load() };
}