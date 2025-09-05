// [SEQUENCE: MVP4-4]
#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>
#include <filesystem>

// [SEQUENCE: MVP4-5]
// 영속성 설정을 위한 구조체
struct PersistenceConfig {
    bool enabled = false;
    std::filesystem::path log_directory = "./logs";
    size_t max_file_size = 10 * 1024 * 1024; // 10MB
    std::chrono::milliseconds flush_interval = std::chrono::milliseconds(1000);
};

// [SEQUENCE: MVP4-6]
// 영속성 관리자 클래스
class PersistenceManager {
public:
    explicit PersistenceManager(const PersistenceConfig& config);
    ~PersistenceManager();

    PersistenceManager(const PersistenceManager&) = delete;
    PersistenceManager& operator=(const PersistenceManager&) = delete;

    void write(const std::string& message);

private:
    void writerThread();
    void rotateFile();

    PersistenceConfig config_;
    std::ofstream log_file_;
    std::filesystem::path current_filepath_;
    size_t current_file_size_ = 0;

    std::queue<std::string> write_queue_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::thread writer_thread_;
    std::atomic<bool> running_;
};

#endif // PERSISTENCE_H
