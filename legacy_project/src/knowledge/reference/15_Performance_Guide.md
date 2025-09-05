# 11. 성능 최적화 가이드 📊
## LogCaster를 더 빠르게 만들기
*LogCaster 프로젝트를 위한 사전지식 문서*

---

## 📋 문서 정보

- **난이도**: 고급 (Advanced)
- **예상 학습 시간**: 15-20시간
- **필요 선수 지식**: 
  - C/C++ 중급 이상
  - 자료구조와 알고리즘
  - 컴퓨터 구조 기초
  - 운영체제 기초
- **실습 환경**: Linux/Unix 환경, 성능 프로파일링 도구

---

## 🎯 이 문서에서 배울 내용

고성능 시스템 개발에서는 알고리즘의 시간/공간 복잡도부터 하드웨어 최적화까지 다양한 요소를 고려해야 합니다. LogCaster 프로젝트는 대량의 로그 데이터를 실시간으로 처리해야 하므로 성능 최적화가 특히 중요합니다. 이 문서에서는 프로파일링, 메모리 지역성, 캐시 최적화, 병렬 처리 등 실제 성능 향상에 직결되는 기법들을 다룹니다.

---

## 📊 1. 시간/공간 복잡도

### Big O 표기법과 실제 성능

```cpp
#include <chrono>
#include <vector>
#include <unordered_map>
#include <algorithm>

// 성능 측정 유틸리티
class PerformanceTimer {
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    std::string operation_name_;
    
public:
    PerformanceTimer(const std::string& name) : operation_name_(name) {
        start_time_ = std::chrono::high_resolution_clock::now();
    }
    
    ~PerformanceTimer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time_);
        std::cout << operation_name_ << " took: " << duration.count() << " μs" << std::endl;
    }
};

// 매크로로 간편하게 사용
#define MEASURE_TIME(name) PerformanceTimer timer(name)

// 다양한 검색 알고리즘 비교
class LogSearchComparison {
private:
    std::vector<LogEntry> logs_;
    std::unordered_map<std::string, std::vector<size_t>> level_index_;
    std::unordered_map<std::string, std::vector<size_t>> source_index_;
    
public:
    LogSearchComparison(size_t log_count) {
        // 테스트 데이터 생성
        logs_.reserve(log_count);
        std::vector<std::string> levels = {"DEBUG", "INFO", "WARNING", "ERROR"};
        std::vector<std::string> sources = {"192.168.1.100", "192.168.1.101", "192.168.1.102"};
        
        for (size_t i = 0; i < log_count; ++i) {
            std::string level = levels[i % levels.size()];
            std::string source = sources[i % sources.size()];
            std::string message = "Log message " + std::to_string(i);
            
            logs_.emplace_back(message, level, source);
            
            // 인덱스 구축
            level_index_[level].push_back(i);
            source_index_[source].push_back(i);
        }
    }
    
    // O(n) - 선형 검색
    std::vector<LogEntry> linearSearchByLevel(const std::string& level) {
        MEASURE_TIME("Linear Search");
        
        std::vector<LogEntry> results;
        for (const auto& log : logs_) {
            if (log.getLevel() == level) {
                results.push_back(log);
            }
        }
        return results;
    }
    
    // O(1) - 인덱스 기반 검색
    std::vector<LogEntry> indexedSearchByLevel(const std::string& level) {
        MEASURE_TIME("Indexed Search");
        
        std::vector<LogEntry> results;
        auto it = level_index_.find(level);
        if (it != level_index_.end()) {
            results.reserve(it->second.size());
            for (size_t index : it->second) {
                results.push_back(logs_[index]);
            }
        }
        return results;
    }
    
    // O(log n) - 이진 검색 (정렬된 데이터 필요)
    std::vector<LogEntry> binarySearchByTimestamp(
        const std::chrono::system_clock::time_point& target_time) {
        MEASURE_TIME("Binary Search");
        
        // 로그가 시간순으로 정렬되어 있다고 가정
        auto lower = std::lower_bound(logs_.begin(), logs_.end(), target_time,
            [](const LogEntry& log, const std::chrono::system_clock::time_point& time) {
                return log.getTimestamp() < time;
            });
        
        std::vector<LogEntry> results;
        if (lower != logs_.end() && lower->getTimestamp() == target_time) {
            results.push_back(*lower);
        }
        return results;
    }
    
    // 복잡도 비교 테스트
    void runComplexityTest() {
        std::cout << "=== Complexity Comparison Test ===" << std::endl;
        std::cout << "Dataset size: " << logs_.size() << " logs" << std::endl;
        
        // 동일한 결과를 다른 방법으로 검색
        auto result1 = linearSearchByLevel("ERROR");
        auto result2 = indexedSearchByLevel("ERROR");
        
        std::cout << "Linear search found: " << result1.size() << " results" << std::endl;
        std::cout << "Indexed search found: " << result2.size() << " results" << std::endl;
        
        // 메모리 사용량 비교
        size_t linear_memory = logs_.size() * sizeof(LogEntry);
        size_t index_memory = linear_memory;
        for (const auto& pair : level_index_) {
            index_memory += pair.second.size() * sizeof(size_t);
        }
        
        std::cout << "Linear search memory: " << linear_memory / 1024 << " KB" << std::endl;
        std::cout << "Indexed search memory: " << index_memory / 1024 << " KB" << std::endl;
        std::cout << "Memory overhead: " << (index_memory - linear_memory) / 1024 << " KB" << std::endl;
    }
};

void complexity_analysis_example() {
    LogSearchComparison comparison(100000);  // 10만 개 로그
    comparison.runComplexityTest();
}
```

### 실제 LogCaster에서의 복잡도 고려사항

```cpp
// 다양한 데이터 구조의 성능 특성
class DataStructurePerformance {
public:
    // O(1) 삽입, O(n) 검색 - 적합: 최근 로그 위주 처리
    void vectorAnalysis() {
        std::vector<LogEntry> logs;
        
        // 장점: 캐시 친화적, 빠른 순회
        // 단점: 중간 삽입/삭제 비용 높음
        
        // 최적 사용 사례: 시간순 로그 저장, 범위 검색
    }
    
    // O(log n) 삽입/검색/삭제 - 적합: 정렬된 로그 관리
    void setAnalysis() {
        std::set<LogEntry> logs;  // 자동 정렬
        
        // 장점: 항상 정렬 상태, 빠른 범위 검색
        // 단점: 메모리 오버헤드, 포인터 추적
        
        // 최적 사용 사례: 중복 제거, 시간 범위 검색
    }
    
    // O(1) 평균 삽입/검색 - 적합: 빠른 키 기반 검색
    void unorderedMapAnalysis() {
        std::unordered_map<std::string, std::vector<LogEntry>> logs_by_level;
        
        // 장점: 매우 빠른 키 검색
        // 단점: 해시 충돌 시 성능 저하, 메모리 파편화
        
        // 최적 사용 사례: 레벨별/소스별 인덱싱
    }
    
    // 실제 LogCaster 시나리오별 권장사항
    void recommendationsForLogCaster() {
        std::cout << "LogCaster 성능 권장사항:" << std::endl;
        std::cout << "1. 실시간 로그 수집: std::vector + circular buffer" << std::endl;
        std::cout << "2. 레벨별 검색: std::unordered_map<level, indices>" << std::endl;
        std::cout << "3. 시간 범위 검색: 시간 파티션 + std::vector" << std::endl;
        std::cout << "4. 전문 검색: Trie 또는 suffix array" << std::endl;
    }
};
```

---

## 🔧 2. 프로파일링 기초

### CPU 프로파일링

```cpp
#include <chrono>
#include <thread>
#include <fstream>
#include <map>

// 간단한 프로파일러 클래스
class SimpleProfiler {
private:
    struct ProfileData {
        std::chrono::nanoseconds total_time{0};
        size_t call_count{0};
        std::chrono::nanoseconds min_time{std::chrono::nanoseconds::max()};
        std::chrono::nanoseconds max_time{0};
    };
    
    static std::map<std::string, ProfileData> profile_data_;
    static std::mutex profile_mutex_;
    
public:
    class Timer {
    private:
        std::string function_name_;
        std::chrono::high_resolution_clock::time_point start_time_;
        
    public:
        Timer(const std::string& name) : function_name_(name) {
            start_time_ = std::chrono::high_resolution_clock::now();
        }
        
        ~Timer() {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                end_time - start_time_);
            
            std::lock_guard<std::mutex> lock(profile_mutex_);
            auto& data = profile_data_[function_name_];
            data.total_time += duration;
            data.call_count++;
            data.min_time = std::min(data.min_time, duration);
            data.max_time = std::max(data.max_time, duration);
        }
    };
    
    static void printReport() {
        std::lock_guard<std::mutex> lock(profile_mutex_);
        
        std::cout << "=== Performance Profile Report ===" << std::endl;
        std::cout << std::left << std::setw(25) << "Function"
                  << std::setw(12) << "Calls"
                  << std::setw(15) << "Total (ms)"
                  << std::setw(15) << "Avg (μs)"
                  << std::setw(15) << "Min (μs)"
                  << std::setw(15) << "Max (μs)" << std::endl;
        std::cout << std::string(97, '-') << std::endl;
        
        for (const auto& [name, data] : profile_data_) {
            auto total_ms = data.total_time.count() / 1e6;
            auto avg_us = data.total_time.count() / (1e3 * data.call_count);
            auto min_us = data.min_time.count() / 1e3;
            auto max_us = data.max_time.count() / 1e3;
            
            std::cout << std::left << std::setw(25) << name
                      << std::setw(12) << data.call_count
                      << std::setw(15) << std::fixed << std::setprecision(2) << total_ms
                      << std::setw(15) << std::fixed << std::setprecision(1) << avg_us
                      << std::setw(15) << std::fixed << std::setprecision(1) << min_us
                      << std::setw(15) << std::fixed << std::setprecision(1) << max_us
                      << std::endl;
        }
    }
    
    static void reset() {
        std::lock_guard<std::mutex> lock(profile_mutex_);
        profile_data_.clear();
    }
    
    static void saveToFile(const std::string& filename) {
        std::ofstream file(filename);
        std::lock_guard<std::mutex> lock(profile_mutex_);
        
        file << "function,calls,total_ns,avg_ns,min_ns,max_ns\n";
        for (const auto& [name, data] : profile_data_) {
            auto avg_ns = data.total_time.count() / data.call_count;
            file << name << "," << data.call_count << "," 
                 << data.total_time.count() << "," << avg_ns << ","
                 << data.min_time.count() << "," << data.max_time.count() << "\n";
        }
    }
};

// 정적 멤버 초기화
std::map<std::string, SimpleProfiler::ProfileData> SimpleProfiler::profile_data_;
std::mutex SimpleProfiler::profile_mutex_;

// 프로파일링 매크로
#define PROFILE_FUNCTION() SimpleProfiler::Timer timer(__FUNCTION__)
#define PROFILE_SCOPE(name) SimpleProfiler::Timer timer(name)

// LogCaster 함수들에 프로파일링 적용
class ProfiledLogProcessor {
public:
    void processLogBatch(const std::vector<LogEntry>& logs) {
        PROFILE_FUNCTION();
        
        for (const auto& log : logs) {
            processLogEntry(log);
        }
    }
    
    void processLogEntry(const LogEntry& log) {
        PROFILE_FUNCTION();
        
        // 로그 파싱
        parseLogMessage(log.getMessage());
        
        // 로그 검증
        validateLogEntry(log);
        
        // 로그 저장
        storeLogEntry(log);
    }
    
private:
    void parseLogMessage(const std::string& message) {
        PROFILE_FUNCTION();
        
        // 메시지 파싱 로직 (시뮬레이션)
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    
    void validateLogEntry(const LogEntry& log) {
        PROFILE_FUNCTION();
        
        // 검증 로직 (시뮬레이션)
        std::this_thread::sleep_for(std::chrono::microseconds(5));
    }
    
    void storeLogEntry(const LogEntry& log) {
        PROFILE_FUNCTION();
        
        // 저장 로직 (시뮬레이션)
        std::this_thread::sleep_for(std::chrono::microseconds(20));
    }
};

void profiling_example() {
    ProfiledLogProcessor processor;
    
    // 테스트 데이터 생성
    std::vector<LogEntry> test_logs;
    for (int i = 0; i < 1000; ++i) {
        test_logs.emplace_back("Test message " + std::to_string(i), "INFO", "localhost");
    }
    
    // 프로파일링 실행
    {
        PROFILE_SCOPE("Main Processing Loop");
        processor.processLogBatch(test_logs);
    }
    
    // 결과 출력
    SimpleProfiler::printReport();
    SimpleProfiler::saveToFile("profile_results.csv");
}
```

### 메모리 프로파일링

```cpp
#include <cstdlib>
#include <new>

// 메모리 사용량 추적
class MemoryTracker {
private:
    static std::atomic<size_t> total_allocated_;
    static std::atomic<size_t> total_deallocated_;
    static std::atomic<size_t> current_usage_;
    static std::atomic<size_t> peak_usage_;
    static std::atomic<size_t> allocation_count_;
    
public:
    static void record_allocation(size_t size) {
        total_allocated_.fetch_add(size);
        current_usage_.fetch_add(size);
        allocation_count_.fetch_add(1);
        
        // 피크 사용량 업데이트
        size_t current = current_usage_.load();
        size_t peak = peak_usage_.load();
        while (current > peak && !peak_usage_.compare_exchange_weak(peak, current)) {
            peak = peak_usage_.load();
        }
    }
    
    static void record_deallocation(size_t size) {
        total_deallocated_.fetch_add(size);
        current_usage_.fetch_sub(size);
    }
    
    static void print_stats() {
        std::cout << "=== Memory Usage Statistics ===" << std::endl;
        std::cout << "Total allocated: " << total_allocated_.load() / 1024 << " KB" << std::endl;
        std::cout << "Total deallocated: " << total_deallocated_.load() / 1024 << " KB" << std::endl;
        std::cout << "Current usage: " << current_usage_.load() / 1024 << " KB" << std::endl;
        std::cout << "Peak usage: " << peak_usage_.load() / 1024 << " KB" << std::endl;
        std::cout << "Allocation count: " << allocation_count_.load() << std::endl;
        
        size_t leaked = total_allocated_.load() - total_deallocated_.load();
        if (leaked > 0) {
            std::cout << "⚠️  Memory leak detected: " << leaked / 1024 << " KB" << std::endl;
        }
    }
    
    static void reset() {
        total_allocated_ = 0;
        total_deallocated_ = 0;
        current_usage_ = 0;
        peak_usage_ = 0;
        allocation_count_ = 0;
    }
};

// 정적 멤버 초기화
std::atomic<size_t> MemoryTracker::total_allocated_{0};
std::atomic<size_t> MemoryTracker::total_deallocated_{0};
std::atomic<size_t> MemoryTracker::current_usage_{0};
std::atomic<size_t> MemoryTracker::peak_usage_{0};
std::atomic<size_t> MemoryTracker::allocation_count_{0};

// 커스텀 할당자 (메모리 추적용)
template<typename T>
class TrackingAllocator {
public:
    using value_type = T;
    
    T* allocate(size_t n) {
        size_t size = n * sizeof(T);
        T* ptr = static_cast<T*>(std::malloc(size));
        if (ptr) {
            MemoryTracker::record_allocation(size);
        }
        return ptr;
    }
    
    void deallocate(T* ptr, size_t n) {
        if (ptr) {
            size_t size = n * sizeof(T);
            MemoryTracker::record_deallocation(size);
            std::free(ptr);
        }
    }
    
    template<typename U>
    bool operator==(const TrackingAllocator<U>&) const { return true; }
    
    template<typename U>
    bool operator!=(const TrackingAllocator<U>&) const { return false; }
};

// 메모리 효율적인 LogCaster 구현
using TrackedLogVector = std::vector<LogEntry, TrackingAllocator<LogEntry>>;

class MemoryEfficientLogStorage {
private:
    TrackedLogVector logs_;
    size_t max_size_;
    
public:
    explicit MemoryEfficientLogStorage(size_t max_size) : max_size_(max_size) {
        logs_.reserve(max_size);  // 한 번에 메모리 할당
    }
    
    void addLog(LogEntry&& log) {
        if (logs_.size() >= max_size_) {
            // 원형 버퍼처럼 동작 (메모리 재할당 없음)
            logs_[logs_.size() % max_size_] = std::move(log);
        } else {
            logs_.push_back(std::move(log));
        }
    }
    
    void shrink_to_fit() {
        logs_.shrink_to_fit();  // 여분 메모리 해제
    }
    
    size_t memory_usage_estimate() const {
        return logs_.capacity() * sizeof(LogEntry);
    }
};

void memory_profiling_example() {
    MemoryTracker::reset();
    
    {
        MemoryEfficientLogStorage storage(10000);
        
        // 로그 추가
        for (int i = 0; i < 15000; ++i) {
            storage.addLog(LogEntry("Message " + std::to_string(i), "INFO", "localhost"));
        }
        
        std::cout << "Estimated storage memory: " 
                  << storage.memory_usage_estimate() / 1024 << " KB" << std::endl;
        
        MemoryTracker::print_stats();
    }
    
    std::cout << "\nAfter storage destruction:" << std::endl;
    MemoryTracker::print_stats();
}
```

---

## 🗃️ 3. 메모리 지역성

### 캐시 친화적 데이터 구조

```cpp
// 구조체 배열 (SoA) vs 배열 구조체 (AoS) 비교
namespace CacheOptimization {

// Array of Structures (AoS) - 캐시 비효율적
struct LogEntryAoS {
    char message[256];     // 256 bytes
    char level[16];        // 16 bytes  
    char source[64];       // 64 bytes
    uint64_t timestamp;    // 8 bytes
    uint32_t padding;      // 4 bytes (정렬)
    // 총 348 bytes per entry
};

// Structure of Arrays (SoA) - 캐시 효율적
class LogEntriesSoA {
private:
    std::vector<std::string> messages_;
    std::vector<std::string> levels_;
    std::vector<std::string> sources_;
    std::vector<uint64_t> timestamps_;
    
public:
    void addLog(const std::string& message, const std::string& level,
               const std::string& source, uint64_t timestamp) {
        messages_.push_back(message);
        levels_.push_back(level);
        sources_.push_back(source);
        timestamps_.push_back(timestamp);
    }
    
    size_t size() const { return messages_.size(); }
    
    // 레벨만 검색 (캐시 효율적)
    std::vector<size_t> findByLevel(const std::string& target_level) const {
        std::vector<size_t> indices;
        
        // levels_ 배열만 순회 (연속된 메모리)
        for (size_t i = 0; i < levels_.size(); ++i) {
            if (levels_[i] == target_level) {
                indices.push_back(i);
            }
        }
        
        return indices;
    }
    
    // 시간 범위 검색 (캐시 효율적)
    std::vector<size_t> findByTimeRange(uint64_t start, uint64_t end) const {
        std::vector<size_t> indices;
        
        // timestamps_ 배열만 순회 (8바이트씩 연속)
        for (size_t i = 0; i < timestamps_.size(); ++i) {
            if (timestamps_[i] >= start && timestamps_[i] <= end) {
                indices.push_back(i);
            }
        }
        
        return indices;
    }
};

// 성능 비교 테스트
void cache_performance_test() {
    const size_t LOG_COUNT = 1000000;
    
    // AoS 테스트
    std::vector<LogEntryAoS> aos_logs(LOG_COUNT);
    for (size_t i = 0; i < LOG_COUNT; ++i) {
        snprintf(aos_logs[i].message, sizeof(aos_logs[i].message), "Message %zu", i);
        strcpy(aos_logs[i].level, (i % 4 == 0) ? "ERROR" : "INFO");
        strcpy(aos_logs[i].source, "localhost");
        aos_logs[i].timestamp = i;
    }
    
    // SoA 테스트
    LogEntriesSoA soa_logs;
    for (size_t i = 0; i < LOG_COUNT; ++i) {
        std::string message = "Message " + std::to_string(i);
        std::string level = (i % 4 == 0) ? "ERROR" : "INFO";
        soa_logs.addLog(message, level, "localhost", i);
    }
    
    // AoS 레벨 검색 (캐시 미스 많음)
    {
        MEASURE_TIME("AoS Level Search");
        int error_count = 0;
        for (const auto& log : aos_logs) {
            if (strcmp(log.level, "ERROR") == 0) {
                error_count++;
            }
        }
        std::cout << "AoS found " << error_count << " ERROR logs" << std::endl;
    }
    
    // SoA 레벨 검색 (캐시 효율적)
    {
        MEASURE_TIME("SoA Level Search");
        auto indices = soa_logs.findByLevel("ERROR");
        std::cout << "SoA found " << indices.size() << " ERROR logs" << std::endl;
    }
}

} // namespace CacheOptimization
```

### 메모리 정렬과 패딩

> 📝 **참고**: 메모리 관리의 기초 개념은 [2. Memory.md](./2.%20Memory.md) 문서를 참조하세요.

```cpp
// 메모리 정렬 최적화
namespace MemoryAlignment {

// 비효율적인 구조체 (패딩 많음)
struct BadAlignment {
    char a;        // 1 byte
    // 3 bytes padding
    int b;         // 4 bytes
    char c;        // 1 byte  
    // 7 bytes padding
    double d;      // 8 bytes
    // 총 24 bytes (11 bytes data + 13 bytes padding)
};

// 효율적인 구조체 (패딩 최소화)
struct GoodAlignment {
    double d;      // 8 bytes
    int b;         // 4 bytes
    char a;        // 1 byte
    char c;        // 1 byte
    // 2 bytes padding
    // 총 16 bytes (14 bytes data + 2 bytes padding)
};

// LogCaster용 최적화된 로그 엔트리
struct alignas(64) OptimizedLogEntry {  // 캐시 라인 정렬
    uint64_t timestamp;        // 8 bytes (가장 큰 것부터)
    uint32_t message_offset;   // 4 bytes (메시지 풀에서의 오프셋)
    uint32_t message_length;   // 4 bytes
    uint16_t level_id;         // 2 bytes (enum으로 레벨 표현)
    uint16_t source_id;        // 2 bytes (소스 ID)
    uint32_t flags;            // 4 bytes
    // 40 bytes padding (캐시 라인 크기 64에 맞춤)
};

static_assert(sizeof(OptimizedLogEntry) == 64, "LogEntry must be 64 bytes");

// 메모리 풀을 사용한 효율적인 문자열 관리
class StringPool {
private:
    std::vector<char> pool_;
    std::unordered_map<std::string, uint32_t> string_to_offset_;
    
public:
    uint32_t addString(const std::string& str) {
        auto it = string_to_offset_.find(str);
        if (it != string_to_offset_.end()) {
            return it->second;  // 이미 존재하는 문자열
        }
        
        uint32_t offset = pool_.size();
        pool_.insert(pool_.end(), str.begin(), str.end());
        pool_.push_back('\0');  // null terminator
        
        string_to_offset_[str] = offset;
        return offset;
    }
    
    const char* getString(uint32_t offset) const {
        return &pool_[offset];
    }
    
    size_t getPoolSize() const { return pool_.size(); }
};

// 최적화된 로그 저장소
class OptimizedLogStorage {
private:
    std::vector<OptimizedLogEntry> entries_;
    StringPool message_pool_;
    std::unordered_map<std::string, uint16_t> level_to_id_;
    std::unordered_map<std::string, uint16_t> source_to_id_;
    std::vector<std::string> id_to_level_;
    std::vector<std::string> id_to_source_;
    
    uint16_t getLevelId(const std::string& level) {
        auto it = level_to_id_.find(level);
        if (it != level_to_id_.end()) {
            return it->second;
        }
        
        uint16_t id = id_to_level_.size();
        level_to_id_[level] = id;
        id_to_level_.push_back(level);
        return id;
    }
    
    uint16_t getSourceId(const std::string& source) {
        auto it = source_to_id_.find(source);
        if (it != source_to_id_.end()) {
            return it->second;
        }
        
        uint16_t id = id_to_source_.size();
        source_to_id_[source] = id;
        id_to_source_.push_back(source);
        return id;
    }
    
public:
    void addLog(const std::string& message, const std::string& level, 
               const std::string& source, uint64_t timestamp) {
        OptimizedLogEntry entry;
        entry.timestamp = timestamp;
        entry.message_offset = message_pool_.addString(message);
        entry.message_length = message.length();
        entry.level_id = getLevelId(level);
        entry.source_id = getSourceId(source);
        entry.flags = 0;
        
        entries_.push_back(entry);
    }
    
    // 캐시 효율적인 레벨 검색
    std::vector<size_t> findByLevelId(uint16_t level_id) const {
        std::vector<size_t> results;
        
        // 연속된 메모리를 64바이트씩 순회 (캐시 라인 단위)
        for (size_t i = 0; i < entries_.size(); ++i) {
            if (entries_[i].level_id == level_id) {
                results.push_back(i);
            }
        }
        
        return results;
    }
    
    // 메모리 사용량 통계
    void printMemoryStats() const {
        size_t entries_size = entries_.size() * sizeof(OptimizedLogEntry);
        size_t pool_size = message_pool_.getPoolSize();
        size_t maps_size = level_to_id_.size() * 32 + source_to_id_.size() * 32; // 추정
        
        std::cout << "=== Memory Usage Statistics ===" << std::endl;
        std::cout << "Entries: " << entries_size / 1024 << " KB" << std::endl;
        std::cout << "String pool: " << pool_size / 1024 << " KB" << std::endl;
        std::cout << "Index maps: " << maps_size / 1024 << " KB" << std::endl;
        std::cout << "Total: " << (entries_size + pool_size + maps_size) / 1024 << " KB" << std::endl;
        std::cout << "Entries count: " << entries_.size() << std::endl;
        std::cout << "Bytes per entry: " << (entries_size + pool_size + maps_size) / entries_.size() << std::endl;
    }
};

void alignment_test() {
    std::cout << "Struct sizes:" << std::endl;
    std::cout << "BadAlignment: " << sizeof(BadAlignment) << " bytes" << std::endl;
    std::cout << "GoodAlignment: " << sizeof(GoodAlignment) << " bytes" << std::endl;
    std::cout << "OptimizedLogEntry: " << sizeof(OptimizedLogEntry) << " bytes" << std::endl;
    
    OptimizedLogStorage storage;
    
    // 테스트 데이터 추가
    for (int i = 0; i < 100000; ++i) {
        storage.addLog("Test message " + std::to_string(i), 
                      (i % 4 == 0) ? "ERROR" : "INFO",
                      "192.168.1." + std::to_string(100 + i % 10),
                      i);
    }
    
    storage.printMemoryStats();
}

} // namespace MemoryAlignment
```

---

## ⚡ 4. 병렬 처리 최적화

### 작업 분할 전략

```cpp
#include <thread>
#include <future>
#include <numeric>

// 병렬 로그 처리를 위한 작업 분할
class ParallelLogProcessor {
private:
    size_t thread_count_;
    
public:
    ParallelLogProcessor(size_t thread_count = 0) 
        : thread_count_(thread_count ? thread_count : std::thread::hardware_concurrency()) {}
    
    // 단순 분할: 연속된 청크로 나누기
    template<typename ProcessFunc>
    void processLogsSimplePartition(const std::vector<LogEntry>& logs, ProcessFunc process_func) {
        if (logs.empty()) return;
        
        size_t chunk_size = logs.size() / thread_count_;
        if (chunk_size == 0) chunk_size = 1;
        
        std::vector<std::future<void>> futures;
        
        for (size_t i = 0; i < thread_count_; ++i) {
            size_t start = i * chunk_size;
            size_t end = (i == thread_count_ - 1) ? logs.size() : (i + 1) * chunk_size;
            
            if (start >= logs.size()) break;
            
            futures.push_back(std::async(std::launch::async, [&logs, start, end, process_func]() {
                for (size_t j = start; j < end; ++j) {
                    process_func(logs[j]);
                }
            }));
        }
        
        // 모든 작업 완료 대기
        for (auto& future : futures) {
            future.wait();
        }
    }
    
    // 동적 작업 분배: 작업 큐 사용
    template<typename ProcessFunc>
    void processLogsDynamic(const std::vector<LogEntry>& logs, ProcessFunc process_func) {
        if (logs.empty()) return;
        
        std::atomic<size_t> next_index{0};
        std::vector<std::thread> workers;
        
        for (size_t i = 0; i < thread_count_; ++i) {
            workers.emplace_back([&logs, &next_index, process_func]() {
                size_t index;
                while ((index = next_index.fetch_add(1)) < logs.size()) {
                    process_func(logs[index]);
                }
            });
        }
        
        for (auto& worker : workers) {
            worker.join();
        }
    }
    
    // 병렬 집계 작업
    struct LogStatistics {
        std::atomic<size_t> total_logs{0};
        std::atomic<size_t> error_count{0};
        std::atomic<size_t> warning_count{0};
        std::atomic<size_t> info_count{0};
    };
    
    LogStatistics gatherStatisticsParallel(const std::vector<LogEntry>& logs) {
        LogStatistics stats;
        
        // 각 스레드별 로컬 통계 (false sharing 방지)
        struct alignas(64) LocalStats {
            size_t total = 0;
            size_t errors = 0;
            size_t warnings = 0;
            size_t infos = 0;
        };
        
        std::vector<LocalStats> local_stats(thread_count_);
        std::vector<std::thread> workers;
        
        size_t chunk_size = logs.size() / thread_count_;
        if (chunk_size == 0) chunk_size = 1;
        
        for (size_t i = 0; i < thread_count_; ++i) {
            workers.emplace_back([&logs, &local_stats, i, chunk_size, this]() {
                size_t start = i * chunk_size;
                size_t end = (i == thread_count_ - 1) ? logs.size() : (i + 1) * chunk_size;
                
                LocalStats& local = local_stats[i];
                
                for (size_t j = start; j < end; ++j) {
                    local.total++;
                    const std::string& level = logs[j].getLevel();
                    if (level == "ERROR") local.errors++;
                    else if (level == "WARNING") local.warnings++;
                    else if (level == "INFO") local.infos++;
                }
            });
        }
        
        for (auto& worker : workers) {
            worker.join();
        }
        
        // 로컬 통계를 전역 통계로 집계
        for (const auto& local : local_stats) {
            stats.total_logs.fetch_add(local.total);
            stats.error_count.fetch_add(local.errors);
            stats.warning_count.fetch_add(local.warnings);
            stats.info_count.fetch_add(local.infos);
        }
        
        return stats;
    }
    
    // 병렬 정렬
    void parallelSort(std::vector<LogEntry>& logs) {
        if (logs.size() < 10000) {
            // 작은 데이터는 단일 스레드가 더 효율적
            std::sort(logs.begin(), logs.end());
            return;
        }
        
        // 병렬 퀵 정렬 (divide and conquer)
        parallelSortImpl(logs, 0, logs.size(), 0);
    }
    
private:
    void parallelSortImpl(std::vector<LogEntry>& logs, size_t begin, size_t end, int depth) {
        const size_t MIN_PARALLEL_SIZE = 10000;
        const int MAX_DEPTH = 4;
        
        if (end - begin < 2) return;
        
        if (end - begin < MIN_PARALLEL_SIZE || depth >= MAX_DEPTH) {
            // 작은 범위이거나 깊이 제한에 도달하면 단일 스레드 정렬
            std::sort(logs.begin() + begin, logs.begin() + end);
            return;
        }
        
        // 파티션
        auto pivot = logs.begin() + begin + (end - begin) / 2;
        std::nth_element(logs.begin() + begin, pivot, logs.begin() + end);
        
        size_t pivot_pos = pivot - logs.begin();
        
        // 두 반쪽을 병렬로 정렬
        auto left_future = std::async(std::launch::async, [this, &logs, begin, pivot_pos, depth]() {
            parallelSortImpl(logs, begin, pivot_pos, depth + 1);
        });
        
        auto right_future = std::async(std::launch::async, [this, &logs, pivot_pos, end, depth]() {
            parallelSortImpl(logs, pivot_pos + 1, end, depth + 1);
        });
        
        left_future.wait();
        right_future.wait();
    }
};

void parallel_processing_example() {
    // 테스트 데이터 생성
    std::vector<LogEntry> test_logs;
    test_logs.reserve(1000000);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> level_dist(0, 3);
    std::vector<std::string> levels = {"DEBUG", "INFO", "WARNING", "ERROR"};
    
    for (int i = 0; i < 1000000; ++i) {
        std::string level = levels[level_dist(gen)];
        test_logs.emplace_back("Message " + std::to_string(i), level, "localhost");
    }
    
    ParallelLogProcessor processor;
    
    // 병렬 통계 수집
    {
        MEASURE_TIME("Parallel Statistics");
        auto stats = processor.gatherStatisticsParallel(test_logs);
        std::cout << "Total: " << stats.total_logs.load() 
                  << ", Errors: " << stats.error_count.load()
                  << ", Warnings: " << stats.warning_count.load()
                  << ", Info: " << stats.info_count.load() << std::endl;
    }
    
    // 병렬 정렬
    {
        auto logs_copy = test_logs;
        MEASURE_TIME("Parallel Sort");
        processor.parallelSort(logs_copy);
        std::cout << "Sorting completed" << std::endl;
    }
    
    // 단일 스레드 비교
    {
        auto logs_copy = test_logs;
        MEASURE_TIME("Single Thread Sort");
        std::sort(logs_copy.begin(), logs_copy.end());
        std::cout << "Single thread sorting completed" << std::endl;
    }
}
```

### Lock-Free 데이터 구조

```cpp
#include <atomic>

// Lock-Free 큐 (Single Producer, Single Consumer)
template<typename T>
class LockFreeQueue {
private:
    struct Node {
        std::atomic<T*> data{nullptr};
        std::atomic<Node*> next{nullptr};
    };
    
    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
    
public:
    LockFreeQueue() {
        Node* dummy = new Node;
        head_.store(dummy);
        tail_.store(dummy);
    }
    
    ~LockFreeQueue() {
        while (Node* old_head = head_.load()) {
            head_.store(old_head->next.load());
            delete old_head;
        }
    }
    
    void enqueue(T item) {
        Node* new_node = new Node;
        T* data = new T(std::move(item));
        new_node->data.store(data);
        
        Node* prev_tail = tail_.exchange(new_node);
        prev_tail->next.store(new_node);
    }
    
    bool dequeue(T& result) {
        Node* head = head_.load();
        Node* next = head->next.load();
        
        if (next == nullptr) {
            return false;  // 큐가 비어있음
        }
        
        T* data = next->data.load();
        if (data == nullptr) {
            return false;
        }
        
        result = *data;
        delete data;
        
        head_.store(next);
        delete head;
        
        return true;
    }
    
    bool empty() const {
        Node* head = head_.load();
        Node* next = head->next.load();
        return next == nullptr;
    }
};

// Lock-Free 로그 버퍼
class LockFreeLogBuffer {
private:
    LockFreeQueue<LogEntry> queue_;
    std::atomic<size_t> size_{0};
    std::atomic<bool> shutdown_{false};
    
public:
    void addLog(LogEntry log) {
        if (!shutdown_.load()) {
            queue_.enqueue(std::move(log));
            size_.fetch_add(1);
        }
    }
    
    bool getLog(LogEntry& log) {
        if (queue_.dequeue(log)) {
            size_.fetch_sub(1);
            return true;
        }
        return false;
    }
    
    size_t size() const { return size_.load(); }
    bool empty() const { return queue_.empty(); }
    
    void shutdown() { shutdown_.store(true); }
    bool isShutdown() const { return shutdown_.load(); }
};

// 고성능 멀티 프로듀서 로그 시스템
class HighPerformanceLogSystem {
private:
    static constexpr size_t BUFFER_COUNT = 8;  // CPU 코어 수에 맞춰 조정
    
    std::array<LockFreeLogBuffer, BUFFER_COUNT> buffers_;
    std::atomic<size_t> producer_counter_{0};
    
    std::thread consumer_thread_;
    std::atomic<bool> running_{false};
    
    // 실제 로그 출력 (파일, 네트워크 등)
    void writeLog(const LogEntry& log) {
        // 실제 구현에서는 파일 쓰기, 네트워크 전송 등
        std::cout << log.toString() << std::endl;
    }
    
    void consumerLoop() {
        LogEntry log;
        size_t current_buffer = 0;
        
        while (running_.load() || hasRemainingLogs()) {
            bool found_log = false;
            
            // 라운드 로빈으로 모든 버퍼 확인
            for (size_t i = 0; i < BUFFER_COUNT; ++i) {
                size_t buffer_idx = (current_buffer + i) % BUFFER_COUNT;
                
                if (buffers_[buffer_idx].getLog(log)) {
                    writeLog(log);
                    found_log = true;
                    current_buffer = buffer_idx;
                    break;
                }
            }
            
            if (!found_log) {
                // 모든 버퍼가 비어있으면 잠시 대기
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
    }
    
    bool hasRemainingLogs() const {
        for (const auto& buffer : buffers_) {
            if (!buffer.empty()) {
                return true;
            }
        }
        return false;
    }
    
public:
    void start() {
        if (running_.exchange(true)) {
            return;  // 이미 실행 중
        }
        
        consumer_thread_ = std::thread(&HighPerformanceLogSystem::consumerLoop, this);
    }
    
    void stop() {
        running_.store(false);
        
        // 모든 버퍼에 종료 신호
        for (auto& buffer : buffers_) {
            buffer.shutdown();
        }
        
        if (consumer_thread_.joinable()) {
            consumer_thread_.join();
        }
    }
    
    // 프로듀서용 로그 함수 (Lock-Free)
    void log(LogEntry log) {
        // 스레드별로 다른 버퍼 사용 (contention 최소화)
        thread_local size_t my_buffer = producer_counter_.fetch_add(1) % BUFFER_COUNT;
        buffers_[my_buffer].addLog(std::move(log));
    }
    
    // 통계 정보
    size_t getTotalQueuedLogs() const {
        size_t total = 0;
        for (const auto& buffer : buffers_) {
            total += buffer.size();
        }
        return total;
    }
    
    void printStats() const {
        std::cout << "=== Log System Statistics ===" << std::endl;
        std::cout << "Total queued logs: " << getTotalQueuedLogs() << std::endl;
        
        for (size_t i = 0; i < BUFFER_COUNT; ++i) {
            std::cout << "Buffer " << i << ": " << buffers_[i].size() << " logs" << std::endl;
        }
    }
};

void lockfree_example() {
    HighPerformanceLogSystem log_system;
    log_system.start();
    
    // 멀티 프로듀서 테스트
    std::vector<std::thread> producers;
    const int PRODUCER_COUNT = 4;
    const int LOGS_PER_PRODUCER = 10000;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < PRODUCER_COUNT; ++i) {
        producers.emplace_back([&log_system, i, LOGS_PER_PRODUCER]() {
            for (int j = 0; j < LOGS_PER_PRODUCER; ++j) {
                LogEntry log("Producer " + std::to_string(i) + " message " + std::to_string(j),
                           "INFO", "thread_" + std::to_string(i));
                log_system.log(std::move(log));
            }
        });
    }
    
    for (auto& producer : producers) {
        producer.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Produced " << (PRODUCER_COUNT * LOGS_PER_PRODUCER) 
              << " logs in " << duration.count() << " ms" << std::endl;
    
    // 남은 로그 처리 대기
    while (log_system.getTotalQueuedLogs() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    log_system.printStats();
    log_system.stop();
}
```

---

## 🔥 5. 흔한 실수와 해결방법

### 5.1 알고리즘 선택 실수

#### [SEQUENCE: 105] 불필요한 중첩 루프
```cpp
// ❌ 잘못된 예: O(n²) 검색
std::vector<LogEntry> findDuplicates(const std::vector<LogEntry>& logs) {
    std::vector<LogEntry> duplicates;
    for (size_t i = 0; i < logs.size(); ++i) {
        for (size_t j = i + 1; j < logs.size(); ++j) {
            if (logs[i].getMessage() == logs[j].getMessage()) {
                duplicates.push_back(logs[j]);
            }
        }
    }
    return duplicates;
}

// ✅ 올바른 예: O(n) 해시맵 사용
std::vector<LogEntry> findDuplicates(const std::vector<LogEntry>& logs) {
    std::unordered_map<std::string, int> seen;
    std::vector<LogEntry> duplicates;
    
    for (const auto& log : logs) {
        if (++seen[log.getMessage()] == 2) {
            duplicates.push_back(log);
        }
    }
    return duplicates;
}
```

### 5.2 메모리 관련 실수

#### [SEQUENCE: 106] 메모리 단편화
```cpp
// ❌ 잘못된 예: 잦은 재할당
class BadLogBuffer {
    std::vector<LogEntry> logs_;
public:
    void addLog(const LogEntry& log) {
        logs_.push_back(log);  // 용량 초과 시 재할당
    }
};

// ✅ 올바른 예: 사전 할당
class GoodLogBuffer {
    std::vector<LogEntry> logs_;
public:
    GoodLogBuffer(size_t expected_size) {
        logs_.reserve(expected_size * 1.5);  // 여유있게 할당
    }
    
    void addLog(const LogEntry& log) {
        if (logs_.size() == logs_.capacity()) {
            // 용량 부족 시 계획적 확장
            logs_.reserve(logs_.capacity() * 2);
        }
        logs_.push_back(log);
    }
};
```

### 5.3 병렬 처리 실수

#### [SEQUENCE: 107] False Sharing
```cpp
// ❌ 잘못된 예: 캐시 라인 공유로 인한 성능 저하
struct BadCounters {
    std::atomic<int> counter1;  // 같은 캐시 라인
    std::atomic<int> counter2;  // 스레드 간 경쟁
};

// ✅ 올바른 예: 캐시 라인 분리
struct alignas(64) GoodCounter {
    std::atomic<int> value;
    char padding[64 - sizeof(std::atomic<int>)];
};

struct GoodCounters {
    GoodCounter counter1;  // 독립적인 캐시 라인
    GoodCounter counter2;  // 경쟁 없음
};
```

### 5.4 프로파일링 실수

#### [SEQUENCE: 108] 릴리즈 모드 측정 누락
```cpp
// ❌ 잘못된 예: 디버그 모드에서만 테스트
#ifdef DEBUG
    auto start = std::chrono::high_resolution_clock::now();
    processLogs();
    auto end = std::chrono::high_resolution_clock::now();
    // 디버그 모드는 최적화가 없어 의미없는 측정
#endif

// ✅ 올바른 예: 조건부 컴파일로 프로파일링
#ifdef ENABLE_PROFILING
class ScopedTimer {
    // 릴리즈 모드에서도 사용 가능한 타이머
};
#else
class ScopedTimer {
    // 빈 구현 (오버헤드 없음)
};
#endif
```

---

## 6. 실습 프로젝트

### 프로젝트 1: 로그 인덱싱 시스템 (기초)
**목표**: 효율적인 검색을 위한 다중 인덱스 구현

**구현 사항**:
- 시간 기반 인덱스 (B+ tree)
- 레벨 기반 해시 인덱스
- 소스 IP 기반 Trie 인덱스
- 메모리 사용량 모니터링

### 프로젝트 2: 병렬 로그 분석기 (중급)
**목표**: 멀티코어를 활용한 실시간 로그 분석

**구현 사항**:
- 작업 스틸링 스케줄러
- Lock-free 통계 수집
- NUMA-aware 메모리 할당
- 실시간 성능 메트릭

### 프로젝트 3: 고성능 로그 압축 시스템 (고급)
**목표**: CPU와 I/O 최적화된 로그 압축

**구현 사항**:
- 스트리밍 압축 알고리즘
- 비동기 I/O 파이프라인
- 적응형 버퍼 관리
- 하드웨어 가속 활용 (SSE/AVX)

---

## 7. 학습 체크리스트

### 기초 레벨 ✅
- [ ] Big O 표기법 이해
- [ ] 기본 프로파일링 도구 사용
- [ ] 메모리 할당 패턴 이해
- [ ] 간단한 벤치마크 작성

### 중급 레벨 📚
- [ ] 캐시 지역성 최적화
- [ ] 병렬 알고리즘 구현
- [ ] 메모리 풀 설계
- [ ] 성능 카운터 활용

### 고급 레벨 🚀
- [ ] Lock-free 자료구조 구현
- [ ] SIMD 명령어 활용
- [ ] 커널 수준 최적화
- [ ] 분산 시스템 성능 튜닝

### 전문가 레벨 🏆
- [ ] 하드웨어별 최적화
- [ ] JIT 컴파일 기법
- [ ] 커스텀 메모리 할당자
- [ ] 실시간 시스템 최적화

---

## 8. 추가 학습 자료

### 추천 도서
- "Computer Systems: A Programmer's Perspective" - Bryant & O'Hallaron
- "The Art of Computer Programming" - Donald Knuth
- "High Performance Computing" - Georg Hager

### 온라인 리소스
- [Agner Fog's Optimization Manuals](https://www.agner.org/optimize/)
- [Intel Intrinsics Guide](https://software.intel.com/sites/landingpage/IntrinsicsGuide/)
- [Brendan Gregg's Performance Resources](http://www.brendangregg.com/)

### 도구
- perf (Linux performance counter)
- Valgrind (메모리 프로파일러)
- Intel VTune Profiler
- Google Benchmark 라이브러리

---

## ✅ 5. 다음 단계 준비

이 문서를 완전히 이해했다면:

1. **시간/공간 복잡도**를 분석하고 최적의 알고리즘을 선택할 수 있어야 합니다
2. **프로파일링 도구**를 사용하여 성능 병목점을 찾을 수 있어야 합니다
3. **메모리 지역성**을 고려한 캐시 친화적 코드를 작성할 수 있어야 합니다
4. **병렬 처리**를 효과적으로 활용할 수 있어야 합니다
5. **Lock-Free 프로그래밍**의 기본 원리를 이해해야 합니다

### 🎯 확인 문제

1. LogCaster에서 100만 개의 로그 중 특정 키워드를 검색하는 가장 효율적인 방법은 무엇인가요?

2. 캐시 미스를 줄이기 위해 LogEntry 구조체를 어떻게 설계해야 할까요?

3. 멀티 프로듀서-싱글 컨슈머 환경에서 Lock-Free 큐가 일반 뮤텍스보다 성능상 유리한 이유는 무엇인가요?

다음 문서에서는 **프로젝트 실습 준비**에 대해 자세히 알아보겠습니다!

---

*💡 팁: 성능 최적화는 측정 가능한 병목점부터 해결하세요. 추측보다는 프로파일링 결과를 믿으세요!*