// [SEQUENCE: MVP4-7]
#include "Persistence.h"
#include <iostream>
#include <iomanip>

// [SEQUENCE: MVP4-8]
// 생성자: 디렉토리 생성, 파일 열기, Writer 스레드 시작
PersistenceManager::PersistenceManager(const PersistenceConfig& config)
    : config_(config), running_(true) {
    if (!config_.enabled) return;

    try {
        std::filesystem::create_directories(config_.log_directory);
        current_filepath_ = config_.log_directory / "current.log";
        log_file_.open(current_filepath_, std::ios::app);
        if (!log_file_.is_open()) {
            throw std::runtime_error("Failed to open log file: " + current_filepath_.string());
        }
        current_file_size_ = std::filesystem::exists(current_filepath_) ? std::filesystem::file_size(current_filepath_) : 0;

        writer_thread_ = std::thread(&PersistenceManager::writerThread, this);
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error("Filesystem error: " + std::string(e.what()));
    }
}

// [SEQUENCE: MVP4-9]
// 소멸자: Writer 스레드의 안전한 종료 보장 (RAII)
PersistenceManager::~PersistenceManager() {
    if (!config_.enabled || !running_) return;

    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        running_ = false;
    }
    condition_.notify_one();
    if (writer_thread_.joinable()) {
        writer_thread_.join();
    }
}

// [SEQUENCE: MVP4-10]
// 외부에서 로그 쓰기를 요청하는 API
void PersistenceManager::write(const std::string& message) {
    if (!config_.enabled) return;
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        write_queue_.push(message);
    }
    condition_.notify_one();
}

// [SEQUENCE: MVP4-11]
// Writer 스레드의 메인 루프
void PersistenceManager::writerThread() {
    while (running_ || !write_queue_.empty()) {
        std::queue<std::string> local_queue;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait_for(lock, config_.flush_interval, [this] { return !running_ || !write_queue_.empty(); });
            
            if (!write_queue_.empty()) {
                local_queue.swap(write_queue_);
            }
        }

        while (!local_queue.empty()) {
            const std::string& message = local_queue.front();
            if (log_file_.is_open()) {
                log_file_ << message << std::endl;
                current_file_size_ += message.length() + 1;
            }
            local_queue.pop();
        }

        if (log_file_.is_open() && current_file_size_ >= config_.max_file_size) {
            rotateFile();
        }
    }
}

// [SEQUENCE: MVP4-12]
// 로그 파일 로테이션
void PersistenceManager::rotateFile() {
    log_file_.close();
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream new_filename;
    new_filename << "log-" << std::put_time(std::localtime(&time_t_now), "%Y%m%d-%H%M%S") << ".log";
    
    try {
        std::filesystem::rename(current_filepath_, config_.log_directory / new_filename.str());
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to rotate log file: " << e.what() << std::endl;
    }

    log_file_.open(current_filepath_, std::ios::app);
    current_file_size_ = 0;
}
