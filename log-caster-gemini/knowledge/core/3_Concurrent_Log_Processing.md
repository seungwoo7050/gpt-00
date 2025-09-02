# LogCaster 동시성 처리 가이드

## 🎯 개요

LogCaster는 수천 개의 동시 연결을 처리하며, 스레드 안전성을 보장합니다. 이 문서는 동시성 설계와 구현을 다룹니다.

## 🧵 스레드 아키텍처

### 전체 스레드 구조
```
Main Thread
├── Accept Thread (로그 포트 9999)
├── Accept Thread (쿼리 포트 9998)
├── Accept Thread (IRC 포트 6667)
└── Thread Pool (8-16 workers)
    ├── Worker 1: Client Handler
    ├── Worker 2: Client Handler
    └── ...
```

### ThreadPool 구현
```cpp
// [SEQUENCE: 120] 현대적 C++ 스레드 풀
class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};

public:
    ThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    // 태스크 대기
                    {
                        std::unique_lock<std::mutex> lock(queueMutex_);
                        condition_.wait(lock, [this] { 
                            return stop_ || !tasks_.empty(); 
                        });
                        
                        if (stop_ && tasks_.empty()) return;
                        
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    
                    // 태스크 실행
                    task();
                }
            });
        }
    }
    
    // [SEQUENCE: 127] 태스크 추가
    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            tasks_.emplace(std::forward<F>(f));
        }
        condition_.notify_one();
    }
};
```

## 🔒 LogBuffer 동시성

### 1. 다중 읽기/단일 쓰기 패턴
```cpp
// [SEQUENCE: 173] 스레드 안전 순환 버퍼
class LogBuffer {
private:
    mutable std::shared_mutex mutex_;  // C++17 읽기/쓰기 락
    std::deque<LogEntry> buffer_;
    
public:
    // 쓰기 작업 (단일 쓰기)
    void push(const std::string& message) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        if (buffer_.size() >= capacity_) {
            buffer_.pop_front();  // 오래된 것 제거
        }
        
        buffer_.emplace_back(message);
    }
    
    // 읽기 작업 (다중 읽기)
    std::vector<std::string> search(const std::string& keyword) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);  // 읽기 락
        
        std::vector<std::string> results;
        for (const auto& entry : buffer_) {
            if (entry.message.find(keyword) != std::string::npos) {
                results.push_back(entry.message);
            }
        }
        return results;
    }
};
```

### 2. Lock-Free 통계
```cpp
// [SEQUENCE: 181] 원자적 통계
struct Stats {
    std::atomic<uint64_t> totalLogs{0};
    std::atomic<uint64_t> droppedLogs{0};
    
    void incrementTotal() {
        totalLogs.fetch_add(1, std::memory_order_relaxed);
    }
    
    void incrementDropped() {
        droppedLogs.fetch_add(1, std::memory_order_relaxed);
    }
    
    // 스냅샷 읽기
    uint64_t getTotalLogs() const {
        return totalLogs.load(std::memory_order_relaxed);
    }
};
```

## 🔄 IRC 채널 동기화

### 1. 채널별 브로드캐스트
```cpp
// [SEQUENCE: 664] 효율적인 브로드캐스트
class IRCChannel {
private:
    mutable std::shared_mutex mutex_;
    std::map<std::string, std::shared_ptr<IRCClient>> clients_;
    
public:
    void broadcast(const std::string& message) {
        // 클라이언트 목록 스냅샷
        std::vector<std::shared_ptr<IRCClient>> snapshot;
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            for (const auto& [nick, client] : clients_) {
                snapshot.push_back(client);
            }
        }
        
        // 락 없이 전송 (데드락 방지)
        for (const auto& client : snapshot) {
            client->sendMessage(message);
        }
    }
};
```

### 2. 클라이언트 관리자 동기화
```cpp
// [SEQUENCE: 739] 이중 인덱싱 관리
class IRCClientManager {
private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<int, std::shared_ptr<IRCClient>> clientsByFd_;
    std::unordered_map<std::string, std::shared_ptr<IRCClient>> clientsByNick_;
    
public:
    // 원자적 닉네임 변경
    bool changeNickname(const std::string& oldNick, const std::string& newNick) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        // 새 닉네임이 이미 사용중인지 확인
        if (clientsByNick_.find(newNick) != clientsByNick_.end()) {
            return false;
        }
        
        // 원자적으로 변경
        auto it = clientsByNick_.find(oldNick);
        if (it != clientsByNick_.end()) {
            auto client = it->second;
            clientsByNick_.erase(it);
            clientsByNick_[newNick] = client;
            return true;
        }
        
        return false;
    }
};
```

## 🎯 데드락 방지

### 1. 락 순서 규칙
```cpp
// 항상 같은 순서로 락 획득
// 순서: LogBuffer -> Channel -> Client

void IRCChannel::processLogEntry(const LogEntry& entry) {
    // 1. LogBuffer는 이미 처리됨 (콜백으로 호출됨)
    
    // 2. Channel 락
    std::shared_lock<std::shared_mutex> channelLock(mutex_);
    
    // 3. 각 Client 락 (필요시)
    for (const auto& [nick, client] : clients_) {
        // Client는 자체 락 보유
        client->sendMessage(formatLogEntry(entry));
    }
}
```

### 2. 타임아웃 있는 락
```cpp
template<typename Mutex>
class TimedLock {
    Mutex& mutex_;
    bool locked_;
    
public:
    TimedLock(Mutex& m, std::chrono::milliseconds timeout) 
        : mutex_(m), locked_(false) {
        locked_ = mutex_.try_lock_for(timeout);
        if (!locked_) {
            throw std::runtime_error("Lock timeout");
        }
    }
    
    ~TimedLock() {
        if (locked_) mutex_.unlock();
    }
};
```

## 📊 성능 최적화

### 1. 메시지 배칭
```cpp
// [SEQUENCE: 255] 배치 처리로 락 경합 감소
class MessageBatcher {
private:
    std::vector<std::pair<int, std::string>> pending_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread worker_;
    
public:
    void addMessage(int clientFd, const std::string& msg) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            pending_.emplace_back(clientFd, msg);
        }
        cv_.notify_one();
    }
    
    void processBatch() {
        while (running_) {
            std::vector<std::pair<int, std::string>> batch;
            
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait_for(lock, std::chrono::milliseconds(10),
                    [this] { return !pending_.empty() || !running_; });
                
                batch = std::move(pending_);
                pending_.clear();
            }
            
            // 배치 전송
            for (const auto& [fd, msg] : batch) {
                send(fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
            }
        }
    }
};
```

### 2. 채널별 전용 스레드
```cpp
// [SEQUENCE: 262] 핫 채널용 전용 처리
class ChannelProcessor {
    std::unordered_map<std::string, std::unique_ptr<std::thread>> dedicated_;
    
public:
    void dedicateThread(const std::string& channel) {
        dedicated_[channel] = std::make_unique<std::thread>([channel] {
            // CPU 친화성 설정
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(2, &cpuset);  // CPU 2에 고정
            pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
            
            // 채널 전용 처리
            while (running_) {
                processChannelMessages(channel);
            }
        });
    }
};
```

## 🔍 동시성 디버깅

### 1. ThreadSanitizer 사용
```bash
# 컴파일 시
g++ -fsanitize=thread -g -O2 *.cpp -o logcaster

# 실행하면 데이터 레이스 자동 감지
```

### 2. 락 통계
```cpp
class LockStats {
    std::atomic<uint64_t> lockAttempts{0};
    std::atomic<uint64_t> lockSuccesses{0};
    std::atomic<uint64_t> lockTimeouts{0};
    
    void report() {
        std::cout << "Lock attempts: " << lockAttempts << std::endl;
        std::cout << "Success rate: " 
                  << (100.0 * lockSuccesses / lockAttempts) << "%" << std::endl;
    }
};
```

## 💡 베스트 프랙티스

1. **최소한의 락 범위**
   ```cpp
   // 나쁜 예
   {
       std::lock_guard<std::mutex> lock(mutex_);
       processData();      // 긴 작업
       updateStats();      // 락 필요
   }
   
   // 좋은 예
   processData();          // 락 없이
   {
       std::lock_guard<std::mutex> lock(mutex_);
       updateStats();      // 짧은 락
   }
   ```

2. **RAII 사용**
   ```cpp
   // 항상 lock_guard 또는 unique_lock 사용
   std::lock_guard<std::mutex> lock(mutex_);
   // 자동으로 언락됨
   ```

3. **원자적 연산 선호**
   ```cpp
   // 락 대신 atomic 사용
   std::atomic<int> counter{0};
   counter.fetch_add(1);  // 락 불필요
   ```

---

다음: [4_IRC_Protocol_Integration.md](4_IRC_Protocol_Integration.md) - IRC 프로토콜 통합