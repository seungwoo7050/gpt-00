# 15. 프로덕션 준비 🚀
## 실제 서비스 배포를 위한 완벽 가이드

---

## 📋 문서 정보

- **난이도**: 고급 (Advanced)
- **예상 학습 시간**: 15-20시간
- **필요 선수 지식**: 
  - C/C++ 중급 이상
  - 멀티스레딩 개념
  - 네트워크 프로그래밍
  - 파일 시스템 이해
- **실습 환경**: Linux/Unix 프로덕션 환경

---

## 🎯 이 문서에서 배울 내용

**"테스트는 잘되는데, 실제 서비스에서는 왜 문제가 생길까요?"** 🤔

정말 좋은 질문이에요! 개발 환경은 마치 **조용한 연습실**이고, 프로덕션은 **실제 콘서트 무대**예요. 관객(사용자)들이 몰려오면 예상치 못한 문제들이 터져나오죠!

그래서 **무대 준비**를 철저히 해야 해요 - 백업 장비, 보안 시스템, 모니터링까지! 🎭

이 문서는 LogCaster를 실제 프로덕션 환경에서 안정적으로 운영하기 위한 핵심 기술들을 다룹니다. 로그 로테이션, 고가용성 설계, 성능 모니터링, 장애 복구 등 실무에서 반드시 필요한 내용들을 심도 있게 설명합니다.

---

## 1. 로그 로테이션과 아카이빙

**"로그 파일이 계속 커져서 하드디스크가 꽉 찰까봐 걱정돼요!"** 💾

완전히 이해해요! 로그는 마치 **일기장**처럼 계속 쌓이다 보면 책장이 꽉 찰 수 있어요. 그래서 **똑똑한 정리 시스템**이 필요하죠!

프로덕션 환경에서는 로그 파일이 무제한 증가하는 것을 방지하고, 저장 공간을 효율적으로 관리해야 합니다.

### 시간 기반 로그 로테이션 - 자동 파일 정리 시스템 📅

```cpp
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <thread>
#include <mutex>
#include <iomanip>
#include <sstream>

class TimeBasedLogRotator {
private:
    std::string base_filename_;
    std::string log_directory_;
    std::chrono::hours rotation_interval_;
    std::ofstream current_file_;
    std::chrono::system_clock::time_point next_rotation_;
    std::mutex file_mutex_;
    
    // 압축 관련
    bool enable_compression_;
    int max_archive_files_;
    
public:
    TimeBasedLogRotator(const std::string& base_filename, 
                       const std::string& directory = "./logs",
                       std::chrono::hours interval = std::chrono::hours(24),
                       bool compress = true,
                       int max_archives = 30)
        : base_filename_(base_filename)
        , log_directory_(directory)
        , rotation_interval_(interval)
        , enable_compression_(compress)
        , max_archive_files_(max_archives) {
        
        // 로그 디렉토리 생성
        std::filesystem::create_directories(log_directory_);
        
        // 첫 번째 로그 파일 열기
        rotateLogFile();
    }
    
    ~TimeBasedLogRotator() {
        std::lock_guard<std::mutex> lock(file_mutex_);
        if (current_file_.is_open()) {
            current_file_.close();
        }
    }
    
    void writeLog(const std::string& message) {
        std::lock_guard<std::mutex> lock(file_mutex_);
        
        // 로테이션 시간 확인
        if (std::chrono::system_clock::now() >= next_rotation_) {
            rotateLogFile();
        }
        
        if (current_file_.is_open()) {
            current_file_ << getCurrentTimestamp() << " " << message << std::endl;
            current_file_.flush();
        }
    }
    
private:
    void rotateLogFile() {
        // 현재 파일 닫기
        if (current_file_.is_open()) {
            current_file_.close();
            
            // 이전 파일 압축 (별도 스레드에서)
            if (enable_compression_) {
                std::thread compression_thread(&TimeBasedLogRotator::compressOldFile, 
                                             this, getCurrentLogPath());
                compression_thread.detach();
            }
        }
        
        // 새 파일 열기
        std::string new_filename = generateLogFilename();
        current_file_.open(new_filename, std::ios::app);
        
        if (!current_file_.is_open()) {
            throw std::runtime_error("Cannot open log file: " + new_filename);
        }
        
        // 다음 로테이션 시간 설정
        next_rotation_ = std::chrono::system_clock::now() + rotation_interval_;
        
        std::cout << "Log rotated to: " << new_filename << std::endl;
        
        // 오래된 아카이브 파일 정리
        cleanupOldArchives();
    }
    
    std::string generateLogFilename() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << log_directory_ << "/" << base_filename_ << "_"
           << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S")
           << ".log";
        
        return ss.str();
    }
    
    std::string getCurrentLogPath() {
        // 현재 열린 파일의 경로를 반환하는 로직
        // 실제 구현에서는 파일 경로를 멤버 변수로 저장해야 함
        return ""; // 간소화된 예제
    }
    
    void compressOldFile(const std::string& filepath) {
        if (filepath.empty()) return;
        
        // gzip 압축 (시스템 명령어 사용)
        std::string command = "gzip " + filepath;
        int result = std::system(command.c_str());
        
        if (result == 0) {
            std::cout << "Compressed: " << filepath << std::endl;
        } else {
            std::cerr << "Failed to compress: " << filepath << std::endl;
        }
    }
    
    void cleanupOldArchives() {
        try {
            std::vector<std::filesystem::path> archive_files;
            
            // 아카이브 파일들 수집
            for (const auto& entry : std::filesystem::directory_iterator(log_directory_)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (filename.find(base_filename_) == 0 && 
                        (filename.ends_with(".log.gz") || filename.ends_with(".log"))) {
                        archive_files.push_back(entry.path());
                    }
                }
            }
            
            // 파일을 수정 시간 기준으로 정렬 (오래된 것부터)
            std::sort(archive_files.begin(), archive_files.end(),
                     [](const std::filesystem::path& a, const std::filesystem::path& b) {
                         return std::filesystem::last_write_time(a) < 
                                std::filesystem::last_write_time(b);
                     });
            
            // 최대 개수를 초과하는 파일들 삭제
            while (archive_files.size() > static_cast<size_t>(max_archive_files_)) {
                std::filesystem::remove(archive_files.front());
                std::cout << "Deleted old archive: " << archive_files.front() << std::endl;
                archive_files.erase(archive_files.begin());
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error during archive cleanup: " << e.what() << std::endl;
        }
    }
    
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << "." << std::setfill('0') << std::setw(3) << ms.count();
        
        return ss.str();
    }
};
```

### 크기 기반 로그 로테이션

```cpp
class SizeBasedLogRotator {
private:
    std::string base_filename_;
    std::string log_directory_;
    size_t max_file_size_;
    int max_backup_files_;
    
    std::ofstream current_file_;
    std::string current_filepath_;
    size_t current_file_size_;
    std::mutex file_mutex_;

public:
    SizeBasedLogRotator(const std::string& base_filename,
                       const std::string& directory = "./logs",
                       size_t max_size_mb = 100,
                       int max_backups = 10)
        : base_filename_(base_filename)
        , log_directory_(directory)
        , max_file_size_(max_size_mb * 1024 * 1024)  // MB to bytes
        , max_backup_files_(max_backups)
        , current_file_size_(0) {
        
        std::filesystem::create_directories(log_directory_);
        openNewLogFile();
    }
    
    ~SizeBasedLogRotator() {
        std::lock_guard<std::mutex> lock(file_mutex_);
        if (current_file_.is_open()) {
            current_file_.close();
        }
    }
    
    void writeLog(const std::string& message) {
        std::lock_guard<std::mutex> lock(file_mutex_);
        
        std::string log_entry = getCurrentTimestamp() + " " + message + "\n";
        
        // 파일 크기 확인 후 로테이션
        if (current_file_size_ + log_entry.length() > max_file_size_) {
            rotateLogFiles();
        }
        
        if (current_file_.is_open()) {
            current_file_ << log_entry;
            current_file_.flush();
            current_file_size_ += log_entry.length();
        }
    }

private:
    void openNewLogFile() {
        current_filepath_ = log_directory_ + "/" + base_filename_ + ".log";
        current_file_.open(current_filepath_, std::ios::app);
        
        if (!current_file_.is_open()) {
            throw std::runtime_error("Cannot open log file: " + current_filepath_);
        }
        
        // 현재 파일 크기 확인
        if (std::filesystem::exists(current_filepath_)) {
            current_file_size_ = std::filesystem::file_size(current_filepath_);
        } else {
            current_file_size_ = 0;
        }
    }
    
    void rotateLogFiles() {
        current_file_.close();
        
        // 백업 파일들을 순서대로 이동
        // logfile.log.3 -> logfile.log.4
        // logfile.log.2 -> logfile.log.3
        // logfile.log.1 -> logfile.log.2
        // logfile.log -> logfile.log.1
        
        for (int i = max_backup_files_ - 1; i >= 1; i--) {
            std::string old_name = current_filepath_ + "." + std::to_string(i);
            std::string new_name = current_filepath_ + "." + std::to_string(i + 1);
            
            if (std::filesystem::exists(old_name)) {
                if (i == max_backup_files_ - 1) {
                    // 가장 오래된 파일 삭제
                    std::filesystem::remove(new_name);
                }
                std::filesystem::rename(old_name, new_name);
            }
        }
        
        // 현재 파일을 .1로 이동
        std::string backup_name = current_filepath_ + ".1";
        std::filesystem::rename(current_filepath_, backup_name);
        
        // 새 로그 파일 생성
        current_file_size_ = 0;
        openNewLogFile();
        
        std::cout << "Log rotated. New file: " << current_filepath_ << std::endl;
    }
    
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        
        return ss.str();
    }
};
```

---

## 2. 구조화된 로그와 JSON 포맷

현대적인 로그 시스템은 구조화된 데이터 형태로 로그를 저장하여 검색과 분석을 용이하게 합니다.

### JSON 로그 포맷터

```cpp
#include <nlohmann/json.hpp>  // JSON 라이브러리
#include <iostream>
#include <string>
#include <chrono>
#include <unordered_map>

using json = nlohmann::json;

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

class StructuredLogger {
private:
    std::string service_name_;
    std::string version_;
    std::string environment_;
    std::unordered_map<LogLevel, std::string> level_names_;

public:
    StructuredLogger(const std::string& service_name,
                    const std::string& version = "1.0.0",
                    const std::string& environment = "production")
        : service_name_(service_name)
        , version_(version)
        , environment_(environment) {
        
        level_names_[LogLevel::TRACE] = "TRACE";
        level_names_[LogLevel::DEBUG] = "DEBUG";
        level_names_[LogLevel::INFO] = "INFO";
        level_names_[LogLevel::WARN] = "WARN";
        level_names_[LogLevel::ERROR] = "ERROR";
        level_names_[LogLevel::FATAL] = "FATAL";
    }
    
    // 기본 로그 메시지
    void log(LogLevel level, const std::string& message, 
             const json& context = json::object()) {
        
        json log_entry = createBaseLogEntry(level, message);
        
        // 컨텍스트 정보 추가
        if (!context.empty()) {
            log_entry["context"] = context;
        }
        
        outputLog(log_entry);
    }
    
    // HTTP 요청 로그
    void logHttpRequest(const std::string& method, const std::string& url,
                       int status_code, long response_time_ms,
                       const std::string& user_id = "",
                       const std::string& request_id = "") {
        
        json log_entry = createBaseLogEntry(LogLevel::INFO, "HTTP Request");
        
        log_entry["http"] = {
            {"method", method},
            {"url", url},
            {"status_code", status_code},
            {"response_time_ms", response_time_ms}
        };
        
        if (!user_id.empty()) {
            log_entry["user_id"] = user_id;
        }
        
        if (!request_id.empty()) {
            log_entry["request_id"] = request_id;
        }
        
        outputLog(log_entry);
    }
    
    // 데이터베이스 쿼리 로그
    void logDatabaseQuery(const std::string& query_type, 
                         const std::string& table,
                         long execution_time_ms,
                         bool success = true,
                         const std::string& error = "") {
        
        LogLevel level = success ? LogLevel::DEBUG : LogLevel::ERROR;
        std::string message = success ? "Database Query" : "Database Query Failed";
        
        json log_entry = createBaseLogEntry(level, message);
        
        log_entry["database"] = {
            {"query_type", query_type},
            {"table", table},
            {"execution_time_ms", execution_time_ms},
            {"success", success}
        };
        
        if (!error.empty()) {
            log_entry["error"] = error;
        }
        
        outputLog(log_entry);
    }
    
    // 에러 로그 (스택 트레이스 포함)
    void logError(const std::string& error_message,
                  const std::string& error_code = "",
                  const json& error_details = json::object()) {
        
        json log_entry = createBaseLogEntry(LogLevel::ERROR, error_message);
        
        json error_info = {
            {"message", error_message}
        };
        
        if (!error_code.empty()) {
            error_info["code"] = error_code;
        }
        
        if (!error_details.empty()) {
            error_info["details"] = error_details;
        }
        
        // 스택 트레이스 추가 (실제 구현에서는 더 정교한 방법 필요)
        error_info["stack_trace"] = getCurrentStackTrace();
        
        log_entry["error"] = error_info;
        
        outputLog(log_entry);
    }
    
    // 성능 메트릭 로그
    void logMetric(const std::string& metric_name, double value,
                   const std::string& unit = "",
                   const json& tags = json::object()) {
        
        json log_entry = createBaseLogEntry(LogLevel::INFO, "Metric");
        
        log_entry["metric"] = {
            {"name", metric_name},
            {"value", value}
        };
        
        if (!unit.empty()) {
            log_entry["metric"]["unit"] = unit;
        }
        
        if (!tags.empty()) {
            log_entry["metric"]["tags"] = tags;
        }
        
        outputLog(log_entry);
    }

private:
    json createBaseLogEntry(LogLevel level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        // ISO 8601 포맷 타임스탬프
        std::stringstream timestamp_ss;
        timestamp_ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
        timestamp_ss << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z";
        
        json log_entry = {
            {"timestamp", timestamp_ss.str()},
            {"level", level_names_[level]},
            {"message", message},
            {"service", {
                {"name", service_name_},
                {"version", version_},
                {"environment", environment_}
            }},
            {"host", {
                {"hostname", getHostname()},
                {"pid", getCurrentPid()}
            }}
        };
        
        return log_entry;
    }
    
    void outputLog(const json& log_entry) {
        // JSON을 한 줄로 출력 (개행 없이)
        std::cout << log_entry.dump() << std::endl;
    }
    
    std::string getHostname() {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            return std::string(hostname);
        }
        return "unknown";
    }
    
    int getCurrentPid() {
        #ifdef _WIN32
            return GetCurrentProcessId();
        #else
            return getpid();
        #endif
    }
    
    std::string getCurrentStackTrace() {
        // 실제 구현에서는 backtrace() 함수 등을 사용
        // 여기서는 간소화된 예제
        return "Stack trace not implemented in this example";
    }
};

// 사용 예제
int main() {
    StructuredLogger logger("logcaster-service", "2.1.0", "production");
    
    // 기본 로그
    logger.log(LogLevel::INFO, "Service started successfully");
    
    // HTTP 요청 로그
    logger.logHttpRequest("POST", "/api/logs", 201, 45, "user123", "req-456");
    
    // 데이터베이스 쿼리 로그
    logger.logDatabaseQuery("INSERT", "logs", 12, true);
    
    // 에러 로그
    json error_details = {
        {"connection_string", "postgresql://..."},
        {"retry_count", 3}
    };
    logger.logError("Database connection failed", "DB_CONN_001", error_details);
    
    // 메트릭 로그
    json metric_tags = {
        {"endpoint", "/api/logs"},
        {"method", "POST"}
    };
    logger.logMetric("response_time", 0.045, "seconds", metric_tags);
    
    return 0;
}
```

---

## 3. 고가용성 (High Availability) 설계

### 로그 복제와 백업

```cpp
#include <vector>
#include <memory>
#include <future>
#include <atomic>

// 로그 싱크 인터페이스
class LogSink {
public:
    virtual ~LogSink() = default;
    virtual bool writeLog(const std::string& log_entry) = 0;
    virtual bool isHealthy() = 0;
    virtual std::string getName() const = 0;
};

// 파일 싱크
class FileSink : public LogSink {
private:
    std::string filepath_;
    std::ofstream file_;
    std::mutex file_mutex_;

public:
    FileSink(const std::string& filepath) : filepath_(filepath) {
        file_.open(filepath_, std::ios::app);
    }
    
    bool writeLog(const std::string& log_entry) override {
        std::lock_guard<std::mutex> lock(file_mutex_);
        if (file_.is_open()) {
            file_ << log_entry << std::endl;
            file_.flush();
            return true;
        }
        return false;
    }
    
    bool isHealthy() override {
        std::lock_guard<std::mutex> lock(file_mutex_);
        return file_.is_open() && file_.good();
    }
    
    std::string getName() const override {
        return "FileSink:" + filepath_;
    }
};

// 네트워크 싱크 (간소화된 예제)
class NetworkSink : public LogSink {
private:
    std::string endpoint_;
    std::atomic<bool> connected_{false};

public:
    NetworkSink(const std::string& endpoint) : endpoint_(endpoint) {
        // 실제 구현에서는 네트워크 연결 초기화
        connected_ = true; // 예제에서는 항상 연결된 것으로 가정
    }
    
    bool writeLog(const std::string& log_entry) override {
        if (!connected_) return false;
        
        // 실제 구현에서는 HTTP POST, TCP 소켓 등을 사용하여 전송
        // 여기서는 시뮬레이션
        std::cout << "Sending to " << endpoint_ << ": " << log_entry << std::endl;
        return true;
    }
    
    bool isHealthy() override {
        return connected_;
    }
    
    std::string getName() const override {
        return "NetworkSink:" + endpoint_;
    }
};

// 고가용성 로거
class HighAvailabilityLogger {
private:
    std::vector<std::unique_ptr<LogSink>> primary_sinks_;
    std::vector<std::unique_ptr<LogSink>> backup_sinks_;
    
    // 설정
    int min_successful_writes_;
    std::chrono::milliseconds write_timeout_;
    
    // 통계
    std::atomic<int> total_writes_{0};
    std::atomic<int> successful_writes_{0};
    std::atomic<int> failed_writes_{0};

public:
    HighAvailabilityLogger(int min_success = 1, 
                          std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
        : min_successful_writes_(min_success)
        , write_timeout_(timeout) {}
    
    void addPrimarySink(std::unique_ptr<LogSink> sink) {
        primary_sinks_.push_back(std::move(sink));
    }
    
    void addBackupSink(std::unique_ptr<LogSink> sink) {
        backup_sinks_.push_back(std::move(sink));
    }
    
    bool writeLog(const std::string& log_entry) {
        total_writes_++;
        
        // 먼저 주 싱크들에 시도
        int successful_count = writeToSinks(primary_sinks_, log_entry);
        
        // 주 싱크에서 충분한 성공을 얻지 못했다면 백업 싱크 사용
        if (successful_count < min_successful_writes_) {
            successful_count += writeToSinks(backup_sinks_, log_entry);
        }
        
        if (successful_count >= min_successful_writes_) {
            successful_writes_++;
            return true;
        } else {
            failed_writes_++;
            return false;
        }
    }
    
    // 헬스 체크
    void performHealthCheck() {
        std::cout << "\n=== Health Check ===" << std::endl;
        
        std::cout << "Primary Sinks:" << std::endl;
        checkSinkHealth(primary_sinks_);
        
        std::cout << "Backup Sinks:" << std::endl;
        checkSinkHealth(backup_sinks_);
        
        std::cout << "Statistics:" << std::endl;
        std::cout << "  Total writes: " << total_writes_ << std::endl;
        std::cout << "  Successful: " << successful_writes_ << std::endl;
        std::cout << "  Failed: " << failed_writes_ << std::endl;
        
        if (total_writes_ > 0) {
            double success_rate = (double)successful_writes_ / total_writes_ * 100;
            std::cout << "  Success rate: " << std::fixed << std::setprecision(2) 
                      << success_rate << "%" << std::endl;
        }
    }

private:
    int writeToSinks(const std::vector<std::unique_ptr<LogSink>>& sinks, 
                     const std::string& log_entry) {
        
        std::vector<std::future<bool>> futures;
        
        // 모든 싱크에 병렬로 쓰기 시도
        for (const auto& sink : sinks) {
            if (sink->isHealthy()) {
                futures.push_back(
                    std::async(std::launch::async, [&sink, &log_entry]() {
                        return sink->writeLog(log_entry);
                    })
                );
            }
        }
        
        // 결과 수집 (타임아웃 적용)
        int successful_count = 0;
        for (auto& future : futures) {
            try {
                if (future.wait_for(write_timeout_) == std::future_status::ready) {
                    if (future.get()) {
                        successful_count++;
                    }
                } else {
                    // 타임아웃 발생
                    std::cerr << "Write timeout occurred" << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Write exception: " << e.what() << std::endl;
            }
        }
        
        return successful_count;
    }
    
    void checkSinkHealth(const std::vector<std::unique_ptr<LogSink>>& sinks) {
        for (const auto& sink : sinks) {
            bool healthy = sink->isHealthy();
            std::cout << "  " << sink->getName() << ": " 
                      << (healthy ? "HEALTHY" : "UNHEALTHY") << std::endl;
        }
    }
};

// 사용 예제
int main() {
    HighAvailabilityLogger ha_logger(1, std::chrono::milliseconds(500));
    
    // 주 싱크들 추가
    ha_logger.addPrimarySink(std::make_unique<FileSink>("primary_log.txt"));
    ha_logger.addPrimarySink(std::make_unique<NetworkSink>("http://log-server-1:8080"));
    
    // 백업 싱크들 추가
    ha_logger.addBackupSink(std::make_unique<FileSink>("backup_log.txt"));
    ha_logger.addBackupSink(std::make_unique<NetworkSink>("http://log-server-backup:8080"));
    
    // 로그 작성 테스트
    for (int i = 0; i < 10; i++) {
        std::string log_message = "Test log message " + std::to_string(i);
        bool success = ha_logger.writeLog(log_message);
        
        if (!success) {
            std::cerr << "Failed to write log: " << log_message << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // 헬스 체크 수행
    ha_logger.performHealthCheck();
    
    return 0;
}
```

---

## 4. 백프레셔와 플로우 제어

고부하 상황에서 로그 시스템이 압도되지 않도록 백프레셔를 구현해야 합니다.

### 적응형 백프레셔 시스템

```cpp
#include <queue>
#include <atomic>
#include <chrono>
#include <thread>
#include <condition_variable>

enum class BackpressureStrategy {
    DROP_OLDEST,    // 가장 오래된 로그 드롭
    DROP_NEWEST,    // 새로운 로그 드롭
    BLOCK_PRODUCER, // 생산자 블록
    SAMPLE_LOGS     // 로그 샘플링
};

class BackpressureController {
private:
    // 큐 관련
    std::queue<std::string> log_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
    // 백프레셔 설정
    size_t max_queue_size_;
    size_t warning_threshold_;
    BackpressureStrategy strategy_;
    
    // 샘플링 관련
    std::atomic<int> sampling_rate_{1}; // 1 = 모든 로그, 2 = 50%, 10 = 10%
    std::atomic<int> log_counter_{0};
    
    // 통계
    std::atomic<int> total_received_{0};
    std::atomic<int> total_dropped_{0};
    std::atomic<int> total_processed_{0};
    
    // 동적 조정
    std::chrono::steady_clock::time_point last_adjustment_;
    double current_load_{0.0};

public:
    BackpressureController(size_t max_queue_size = 10000,
                          BackpressureStrategy strategy = BackpressureStrategy::DROP_OLDEST)
        : max_queue_size_(max_queue_size)
        , warning_threshold_(max_queue_size * 0.8)
        , strategy_(strategy)
        , last_adjustment_(std::chrono::steady_clock::now()) {}
    
    bool enqueue(const std::string& log_entry) {
        total_received_++;
        
        // 샘플링 체크
        if (strategy_ == BackpressureStrategy::SAMPLE_LOGS) {
            int current_count = log_counter_++;
            if (current_count % sampling_rate_ != 0) {
                total_dropped_++;
                return false; // 샘플링으로 드롭
            }
        }
        
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        // 큐 크기 체크
        if (log_queue_.size() >= max_queue_size_) {
            switch (strategy_) {
                case BackpressureStrategy::DROP_OLDEST:
                    if (!log_queue_.empty()) {
                        log_queue_.pop();
                        total_dropped_++;
                    }
                    log_queue_.push(log_entry);
                    break;
                    
                case BackpressureStrategy::DROP_NEWEST:
                    total_dropped_++;
                    return false;
                    
                case BackpressureStrategy::BLOCK_PRODUCER:
                    // 큐에 공간이 생길 때까지 대기
                    queue_cv_.wait(lock, [this] { 
                        return log_queue_.size() < max_queue_size_; 
                    });
                    log_queue_.push(log_entry);
                    break;
                    
                case BackpressureStrategy::SAMPLE_LOGS:
                    // 이미 위에서 처리됨
                    break;
            }
        } else {
            log_queue_.push(log_entry);
        }
        
        // 로드 모니터링 및 동적 조정
        updateLoadMetrics();
        
        queue_cv_.notify_one();
        return true;
    }
    
    bool dequeue(std::string& log_entry, 
                std::chrono::milliseconds timeout = std::chrono::milliseconds(1000)) {
        
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        if (queue_cv_.wait_for(lock, timeout, [this] { return !log_queue_.empty(); })) {
            log_entry = log_queue_.front();
            log_queue_.pop();
            total_processed_++;
            
            // 큐에 공간이 생겼음을 알림
            queue_cv_.notify_one();
            return true;
        }
        
        return false;
    }
    
    // 실시간 통계
    void printStatistics() {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        
        std::cout << "\n=== Backpressure Statistics ===" << std::endl;
        std::cout << "Queue size: " << log_queue_.size() << "/" << max_queue_size_ << std::endl;
        std::cout << "Current load: " << std::fixed << std::setprecision(2) << current_load_ * 100 << "%" << std::endl;
        std::cout << "Sampling rate: 1/" << sampling_rate_ << std::endl;
        std::cout << "Total received: " << total_received_ << std::endl;
        std::cout << "Total dropped: " << total_dropped_ << std::endl;
        std::cout << "Total processed: " << total_processed_ << std::endl;
        
        if (total_received_ > 0) {
            double drop_rate = (double)total_dropped_ / total_received_ * 100;
            std::cout << "Drop rate: " << std::fixed << std::setprecision(2) << drop_rate << "%" << std::endl;
        }
    }
    
    size_t getQueueSize() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return log_queue_.size();
    }

private:
    void updateLoadMetrics() {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_adjustment_);
        
        // 1초마다 로드 메트릭 업데이트
        if (duration.count() >= 1) {
            current_load_ = (double)log_queue_.size() / max_queue_size_;
            
            // 동적 샘플링 레이트 조정
            if (strategy_ == BackpressureStrategy::SAMPLE_LOGS) {
                adjustSamplingRate();
            }
            
            last_adjustment_ = now;
        }
    }
    
    void adjustSamplingRate() {
        if (current_load_ > 0.9) {
            // 높은 로드: 샘플링 레이트 증가 (더 적은 로그 수용)
            sampling_rate_ = std::min(sampling_rate_ * 2, 100);
        } else if (current_load_ < 0.5) {
            // 낮은 로드: 샘플링 레이트 감소 (더 많은 로그 수용)
            sampling_rate_ = std::max(sampling_rate_ / 2, 1);
        }
    }
};

// 사용 예제
class AdaptiveLogger {
private:
    BackpressureController controller_;
    std::thread processing_thread_;
    std::atomic<bool> running_{true};

public:
    AdaptiveLogger(BackpressureStrategy strategy = BackpressureStrategy::DROP_OLDEST) 
        : controller_(10000, strategy) {
        
        // 로그 처리 스레드 시작
        processing_thread_ = std::thread(&AdaptiveLogger::processLogs, this);
    }
    
    ~AdaptiveLogger() {
        running_ = false;
        if (processing_thread_.joinable()) {
            processing_thread_.join();
        }
    }
    
    void log(const std::string& message) {
        std::string timestamped_message = getCurrentTimestamp() + " " + message;
        
        if (!controller_.enqueue(timestamped_message)) {
            // 로그가 드롭된 경우의 처리
            // 실제 구현에서는 메트릭 수집, 알림 등을 수행
        }
    }
    
    void printStatistics() {
        controller_.printStatistics();
    }

private:
    void processLogs() {
        std::string log_entry;
        
        while (running_) {
            if (controller_.dequeue(log_entry, std::chrono::milliseconds(100))) {
                // 실제 로그 처리 (파일 쓰기, 네트워크 전송 등)
                std::cout << log_entry << std::endl;
                
                // 처리 지연 시뮬레이션
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
    }
    
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

// 백프레셔 테스트
int main() {
    AdaptiveLogger logger(BackpressureStrategy::SAMPLE_LOGS);
    
    // 고부하 시뮬레이션
    std::vector<std::thread> producer_threads;
    
    for (int i = 0; i < 5; i++) {
        producer_threads.emplace_back([&logger, i]() {
            for (int j = 0; j < 1000; j++) {
                logger.log("High-load log from producer " + std::to_string(i) + 
                          " message " + std::to_string(j));
                
                // 랜덤한 간격으로 로그 생성
                std::this_thread::sleep_for(std::chrono::microseconds(rand() % 1000));
            }
        });
    }
    
    // 통계 출력 스레드
    std::thread stats_thread([&logger]() {
        for (int i = 0; i < 10; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            logger.printStatistics();
        }
    });
    
    // 모든 생산자 스레드 완료 대기
    for (auto& thread : producer_threads) {
        thread.join();
    }
    
    stats_thread.join();
    
    // 최종 통계
    std::this_thread::sleep_for(std::chrono::seconds(2));
    logger.printStatistics();
    
    return 0;
}
```

---

## 🔥 5. 흔한 실수와 해결방법

### 5.1 로그 로테이션 실수

#### [SEQUENCE: 42] 로테이션 타이밍 오류
```cpp
// ❌ 잘못된 예: 로테이션 중 로그 손실
void rotateLogFile() {
    current_file_.close();  // 버퍼링된 데이터 손실 가능
    rename("app.log", "app.log.old");
    current_file_.open("app.log");
}

// ✅ 올바른 예: 안전한 로테이션
void rotateLogFile() {
    current_file_.flush();  // 버퍼 플러시
    current_file_.close();
    
    // 원자적 이름 변경
    std::string new_name = generateTimestampedName();
    rename("app.log", new_name.c_str());
    
    current_file_.open("app.log");
}
```

### 5.2 고가용성 실수

#### [SEQUENCE: 43] 단일 지점 장애
```cpp
// ❌ 잘못된 예: 단일 로그 저장소
class SimpleLogger {
    void log(const std::string& message) {
        file_ << message << std::endl;  // 파일 시스템 장애 시 전체 로그 손실
    }
};

// ✅ 올바른 예: 다중 저장소
class ResilientLogger {
    void log(const std::string& message) {
        // 주 저장소 시도
        if (!primary_sink_->write(message)) {
            // 백업 저장소로 폴백
            backup_sink_->write(message);
        }
    }
};
```

### 5.3 백프레셔 실수

#### [SEQUENCE: 44] 무제한 큐 증가
```cpp
// ❌ 잘못된 예: 큐 크기 제한 없음
void enqueue(const std::string& log) {
    queue_.push(log);  // OOM 가능성
}

// ✅ 올바른 예: 제한된 큐
void enqueue(const std::string& log) {
    if (queue_.size() >= MAX_QUEUE_SIZE) {
        // 백프레셔 전략 적용
        applyBackpressureStrategy();
    }
    queue_.push(log);
}
```

---

## 6. 실습 프로젝트

### 프로젝트 1: 기본 로그 로테이터 (기초)
**목표**: 시간/크기 기반 로그 로테이션 구현

**구현 사항**:
- 파일 크기 모니터링
- 안전한 파일 이름 변경
- 자동 압축 및 아카이빙
- 오래된 파일 자동 삭제

### 프로젝트 2: 고가용성 로그 시스템 (중급)
**목표**: 다중 저장소와 폴백을 갖춘 로거

**구현 사항**:
- 다중 로그 싱크 (file, network, database)
- 헬스 체크 및 자동 폴오버
- 비동기 로그 전송
- 실패 로그 리트라이 큐

### 프로젝트 3: 프로덕션급 로그 파이프라인 (고급)
**목표**: 대규모 트래픽을 처리하는 완전한 로그 시스템

**구현 사항**:
- 적응형 백프레셔 제어
- 구조화된 JSON 로그 포맷
- 실시간 모니터링 대시보드
- 분산 로그 수집 및 집계

---

## 7. 학습 체크리스트

### 기초 레벨 ✅
- [ ] 로그 로테이션 개념 이해
- [ ] JSON 포맷 로그 작성
- [ ] 기본적인 비동기 로깅
- [ ] 예외 처리와 로깅

### 중급 레벨 📚
- [ ] 다중 로그 싱크 구현
- [ ] 백프레셔 전략 이해
- [ ] 로그 필터링과 샘플링
- [ ] 성능 메트릭 수집

### 고급 레벨 🚀
- [ ] 고가용성 아키텍처 설계
- [ ] 적응형 백프레셔 구현
- [ ] 분산 로그 시스템
- [ ] 로그 분석 파이프라인

### 전문가 레벨 🏆
- [ ] 대규모 로그 처리 최적화
- [ ] 실시간 스트림 처리
- [ ] 커스텀 로그 프로토콜
- [ ] 클라우드 네이티브 로깅

---

## 8. 추가 학습 자료

### 추천 도서
- "Site Reliability Engineering" - Google
- "Distributed Systems Observability" - Cindy Sridharan
- "The Art of Monitoring" - James Turnbull

### 온라인 리소스
- [Elasticsearch Documentation](https://www.elastic.co/guide/)
- [Prometheus Best Practices](https://prometheus.io/docs/practices/)
- [Grafana Tutorials](https://grafana.com/tutorials/)

### 실습 도구
- ELK Stack (Elasticsearch, Logstash, Kibana)
- Prometheus + Grafana
- Fluentd/Fluent Bit
- Apache Kafka

---

## 5. 다음 단계 준비

이 프로덕션 준비 가이드를 이해했다면 LogCaster를 실제 운영 환경에서 안정적으로 배포할 수 있습니다:

1. **로그 관리**: 로테이션, 압축, 아카이빙으로 저장 공간 효율성
2. **구조화된 로그**: JSON 포맷으로 검색과 분석 용이성 
3. **고가용성**: 다중 싱크와 백업으로 무중단 서비스
4. **백프레셔**: 고부하 상황에서의 안정적인 동작

### 확인 문제

1. 로그 로테이션을 시간 기반과 크기 기반 중 언제 어떤 것을 선택해야 할까요?
2. 구조화된 로그의 장점과 단점은 무엇인가요?
3. 백프레셔 전략 중 어떤 상황에서 어떤 전략을 사용해야 할까요?

---

*💡 팁: 프로덕션 환경은 예측 불가능한 상황이 많습니다. 모니터링과 알림 시스템을 반드시 구축하세요!*