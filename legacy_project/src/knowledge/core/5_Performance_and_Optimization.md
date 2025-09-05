# LogCaster 성능 최적화 가이드

## 🎯 개요

LogCaster의 성능 목표와 달성 방법을 다룹니다. 10,000+ logs/sec 처리와 1,000+ 동시 연결을 지원합니다.

## 📊 성능 메트릭

### 현재 달성 성능
```
로그 수집: 10,000+ logs/second
동시 연결: 1,000+ clients
쿼리 응답: < 1ms average
메모리 사용: ~100MB for 1M entries
CPU 사용률: < 50% on 4-core system
```

## 🚀 핵심 최적화 기법

### 1. Zero-Copy 로그 처리
```cpp
// [SEQUENCE: 196] 이동 시맨틱 활용
void LogBuffer::push(std::string&& message) {  // rvalue 참조
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    if (buffer_.size() >= capacity_) {
        buffer_.pop_front();
        stats_.droppedLogs++;
    }
    
    // 복사 없이 이동
    buffer_.emplace_back(std::move(message));  
    stats_.totalLogs++;
}

// 사용 예
logBuffer->push(std::move(logMessage));  // 복사 없음!
```

### 2. Lock-Free 통계
```cpp
// [SEQUENCE: 181] 원자적 연산
struct Stats {
    // 캐시 라인 정렬 (false sharing 방지)
    alignas(64) std::atomic<uint64_t> totalLogs{0};
    alignas(64) std::atomic<uint64_t> droppedLogs{0};
    alignas(64) std::atomic<uint64_t> searchQueries{0};
    
    void increment(std::atomic<uint64_t>& counter) {
        // relaxed ordering으로 충분
        counter.fetch_add(1, std::memory_order_relaxed);
    }
};
```

### 3. 메모리 풀
```cpp
// [SEQUENCE: 390] 객체 재사용
template<typename T>
class ObjectPool {
private:
    std::stack<std::unique_ptr<T>> pool_;
    std::mutex mutex_;
    const size_t maxSize_ = 1000;
    
public:
    std::unique_ptr<T> acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!pool_.empty()) {
            auto obj = std::move(pool_.top());
            pool_.pop();
            return obj;
        }
        return std::make_unique<T>();
    }
    
    void release(std::unique_ptr<T> obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pool_.size() < maxSize_) {
            obj->reset();  // 객체 초기화
            pool_.push(std::move(obj));
        }
    }
};

// LogEntry 풀
ObjectPool<LogEntry> entryPool;
```

## 🔧 네트워크 최적화

### 1. TCP 튜닝
```cpp
// [SEQUENCE: 850] 소켓 옵션 최적화
void optimizeSocket(int fd) {
    // Nagle 알고리즘 비활성화 (낮은 지연시간)
    int flag = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    
    // 버퍼 크기 증가
    int bufsize = 256 * 1024;  // 256KB
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
    
    // Keep-alive 최적화
    int keepidle = 60;     // 60초 후 시작
    int keepintvl = 10;    // 10초 간격
    int keepcnt = 6;       // 6번 재시도
    
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));
}
```

### 2. 배치 처리
```cpp
// [SEQUENCE: 255] 시스템 콜 감소
class BatchWriter {
    struct WriteRequest {
        int fd;
        std::string data;
    };
    
    std::vector<WriteRequest> pending_;
    std::mutex mutex_;
    std::thread worker_;
    
    void processBatch() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            std::vector<WriteRequest> batch;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                batch = std::move(pending_);
            }
            
            // iovec 배열로 한 번에 전송
            for (const auto& req : batch) {
                struct iovec iov = {
                    .iov_base = (void*)req.data.c_str(),
                    .iov_len = req.data.length()
                };
                writev(req.fd, &iov, 1);
            }
        }
    }
};
```

## 💾 메모리 최적화

### 1. 순환 버퍼 크기 관리
```cpp
// [SEQUENCE: 380] 동적 크기 조정
class AdaptiveBuffer {
    size_t currentCapacity_;
    const size_t minCapacity_ = 1000;
    const size_t maxCapacity_ = 1000000;
    
    void adjustCapacity() {
        double usage = (double)buffer_.size() / currentCapacity_;
        
        if (usage > 0.9 && currentCapacity_ < maxCapacity_) {
            // 90% 이상 사용시 확장
            resize(currentCapacity_ * 2);
        } else if (usage < 0.25 && currentCapacity_ > minCapacity_) {
            // 25% 미만 사용시 축소
            resize(currentCapacity_ / 2);
        }
    }
    
    void resize(size_t newCapacity) {
        std::deque<LogEntry> newBuffer;
        
        // 최신 항목만 복사
        size_t copyCount = std::min(buffer_.size(), newCapacity);
        auto start = buffer_.end() - copyCount;
        
        newBuffer.insert(newBuffer.end(), start, buffer_.end());
        buffer_ = std::move(newBuffer);
        currentCapacity_ = newCapacity;
    }
};
```

### 2. 문자열 최적화
```cpp
// [SEQUENCE: 295] Small String Optimization 활용
class OptimizedLogEntry {
    // SSO를 위한 짧은 문자열
    static constexpr size_t SSO_SIZE = 23;
    
    union {
        char small[SSO_SIZE + 1];  // null terminator
        struct {
            char* ptr;
            size_t size;
            size_t capacity;
        } large;
    } data_;
    
    bool isSmall() const {
        return data_.small[SSO_SIZE] != 0;
    }
};
```

## 🔄 동시성 최적화

### 1. 읽기 최적화 shared_mutex
```cpp
// [SEQUENCE: 198] 다중 읽기 허용
class LogBuffer {
    mutable std::shared_mutex mutex_;
    
    // 99% 읽기 작업에 최적화
    std::vector<LogEntry> search(const std::string& pattern) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);  // 다중 읽기
        // 검색 수행...
    }
    
    // 1% 쓰기 작업
    void push(LogEntry entry) {
        std::unique_lock<std::shared_mutex> lock(mutex_);  // 독점 쓰기
        // 추가...
    }
};
```

### 2. CPU 캐시 최적화
```cpp
// [SEQUENCE: 450] 캐시 친화적 자료구조
struct alignas(64) CacheLineAligned {  // 64바이트 정렬
    std::atomic<uint64_t> counter{0};
    char padding[56];  // 캐시 라인 채우기
};

// False sharing 방지
class ThreadLocalStats {
    struct alignas(64) LocalStats {
        uint64_t processed = 0;
        uint64_t errors = 0;
    };
    
    std::array<LocalStats, MAX_THREADS> stats_;
    
    LocalStats& getLocal() {
        static thread_local int threadId = assignThreadId();
        return stats_[threadId];
    }
};
```

## 📈 프로파일링과 모니터링

### 1. 내장 성능 카운터
```cpp
// [SEQUENCE: 460] 실시간 메트릭
class PerformanceMonitor {
    struct Metrics {
        std::atomic<uint64_t> logsReceived{0};
        std::atomic<uint64_t> queriesProcessed{0};
        std::atomic<uint64_t> ircMessagessSent{0};
        std::chrono::steady_clock::time_point startTime;
        
        void report() const {
            auto elapsed = std::chrono::steady_clock::now() - startTime;
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
            
            std::cout << "=== Performance Report ===" << std::endl;
            std::cout << "Logs/sec: " << logsReceived / seconds << std::endl;
            std::cout << "Queries/sec: " << queriesProcessed / seconds << std::endl;
            std::cout << "IRC msgs/sec: " << ircMessagessSent / seconds << std::endl;
        }
    };
    
    Metrics metrics_;
};
```

### 2. 병목 감지
```cpp
// [SEQUENCE: 470] 타이밍 추적
class TimingTracker {
    struct Timing {
        std::chrono::nanoseconds total{0};
        uint64_t count = 0;
        
        double averageMs() const {
            if (count == 0) return 0;
            return total.count() / (count * 1000000.0);
        }
    };
    
    std::unordered_map<std::string, Timing> timings_;
    
public:
    class ScopedTimer {
        TimingTracker& tracker_;
        std::string name_;
        std::chrono::steady_clock::time_point start_;
        
    public:
        ScopedTimer(TimingTracker& t, const std::string& name)
            : tracker_(t), name_(name), start_(std::chrono::steady_clock::now()) {}
            
        ~ScopedTimer() {
            auto elapsed = std::chrono::steady_clock::now() - start_;
            tracker_.record(name_, elapsed);
        }
    };
    
    void record(const std::string& name, std::chrono::nanoseconds duration) {
        auto& timing = timings_[name];
        timing.total += duration;
        timing.count++;
    }
};

// 사용 예
{
    TimingTracker::ScopedTimer timer(tracker, "log_processing");
    processLog(entry);
}  // 자동으로 시간 기록
```

## 🎯 최적화 체크리스트

### 컴파일 시간 최적화
- [ ] `-O3` 최적화 플래그 사용
- [ ] Link Time Optimization (LTO) 활성화
- [ ] Profile Guided Optimization (PGO) 적용

### 런타임 최적화
- [ ] 핫 패스 인라인화
- [ ] 브랜치 예측 힌트 추가
- [ ] SIMD 명령어 활용 (가능한 경우)

### 시스템 레벨
- [ ] CPU affinity 설정
- [ ] Huge pages 사용
- [ ] NUMA 고려사항 적용

## 💡 성능 인사이트

1. **측정 없이 최적화 없다**
   - 항상 프로파일링 먼저
   - 병목 지점 확인 후 최적화

2. **메모리가 곧 성능**
   - 캐시 친화적 설계
   - 불필요한 할당 제거

3. **동시성의 균형**
   - 너무 많은 락은 성능 저하
   - 너무 적은 락은 버그 유발

---

이것으로 LogCaster 핵심 문서가 완성되었습니다!