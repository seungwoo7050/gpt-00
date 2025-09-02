# 19. 실시간 방송 시스템 📺
## 고성능 실시간 데이터 전송 완벽 구현

---

## 📋 문서 정보

- **난이도**: 고급 (Advanced)
- **예상 학습 시간**: 20-25시간
- **필요 선수 지식**: 
  - 네트워크 프로그래밍 고급
  - 멀티스레딩과 동시성
  - I/O 멀티플렉싱
  - 성능 최적화
- **실습 환경**: C++17, epoll/kqueue, 고성능 네트워크 환경

---

## Overview

**"TV 방송처럼 수천 명에게 동시에 메시지를 보내려면?"** 📺

정말 흥미로운 질문이에요! 실시간 브로드캐스팅은 마치 **TV 방송국**과 같아요. 하나의 스튜디오에서 전국의 수백만 TV로 동시에 방송을 보내는 것처럼, 우리도 하나의 서버에서 수천 개의 클라이언트에게 동시에 로그를 전달해야 해요!

하지만 이건 단순히 복사-붙여넣기가 아니에요. 마치 **효율적인 우체국 시스템**처럼, 똑똑한 라우팅과 최적화가 필요하죠! 🚀

실시간 메시지 브로드캐스팅은 LogCaster-IRC의 핵심 기능입니다. 수천 개의 클라이언트에게 로그를 효율적으로 전달하면서도 시스템 리소스를 최적화하는 것이 중요합니다. 이 가이드는 Pub/Sub 패턴, 메시지 라우팅, 백프레셔 처리 등 실시간 브로드캐스팅의 핵심 기술을 다룹니다.

## 1. Pub/Sub 아키텍처

**"신문사는 어떻게 독자들에게 뉴스를 전달할까요?"** 📰

바로 **구독 시스템**이에요! 독자들이 관심 있는 분야(정치, 스포츠, 경제)를 구독하면, 신문사는 해당 뉴스만 골라서 보내주죠. Pub/Sub도 똑같아요!

### 1.1 기본 Pub/Sub 구현 - 디지털 신문 구독 시스템 📬
```cpp
template<typename Message>
class PubSubBroker {
private:
    using Callback = std::function<void(const Message&)>;
    using SubscriberId = uint64_t;
    
    struct Subscription {
        SubscriberId id;
        std::string topic;
        Callback callback;
        std::weak_ptr<void> lifetime;  // 수명 추적
    };
    
    std::unordered_map<std::string, std::vector<Subscription>> topics_;
    std::shared_mutex mutex_;
    std::atomic<SubscriberId> nextId_{1};
    
public:
    // 구독
    SubscriberId subscribe(const std::string& topic, 
                          Callback callback,
                          std::shared_ptr<void> lifetime = nullptr) {
        std::unique_lock lock(mutex_);
        
        SubscriberId id = nextId_++;
        topics_[topic].push_back({id, topic, callback, lifetime});
        
        return id;
    }
    
    // 구독 해제
    void unsubscribe(SubscriberId id) {
        std::unique_lock lock(mutex_);
        
        for (auto& [topic, subs] : topics_) {
            subs.erase(
                std::remove_if(subs.begin(), subs.end(),
                    [id](const auto& sub) { return sub.id == id; }),
                subs.end()
            );
        }
    }
    
    // 발행
    void publish(const std::string& topic, const Message& message) {
        std::vector<Callback> callbacks;
        
        {
            std::shared_lock lock(mutex_);
            
            auto it = topics_.find(topic);
            if (it == topics_.end()) return;
            
            // 유효한 구독자만 수집
            for (const auto& sub : it->second) {
                if (!sub.lifetime.expired()) {
                    callbacks.push_back(sub.callback);
                }
            }
        }
        
        // 락 해제 후 콜백 실행 (데드락 방지)
        for (const auto& callback : callbacks) {
            try {
                callback(message);
            } catch (const std::exception& e) {
                logError("Subscriber callback error: {}", e.what());
            }
        }
    }
};
```

### 1.2 LogCaster 통합 Pub/Sub
```cpp
class LogBroadcaster {
private:
    PubSubBroker<LogEntry> broker_;
    std::unordered_map<std::string, std::function<bool(const LogEntry&)>> filters_;
    
public:
    // 채널별 필터 설정
    void setupChannelFilters() {
        filters_["#logs-error"] = [](const LogEntry& log) {
            return log.level == "ERROR";
        };
        
        filters_["#logs-warning"] = [](const LogEntry& log) {
            return log.level == "WARNING" || log.level == "ERROR";
        };
        
        filters_["#logs-database"] = [](const LogEntry& log) {
            return log.message.find("database") != std::string::npos ||
                   log.message.find("sql") != std::string::npos;
        };
        
        filters_["#logs-security"] = [](const LogEntry& log) {
            return log.category == "SECURITY" ||
                   log.message.find("auth") != std::string::npos ||
                   log.message.find("permission") != std::string::npos;
        };
    }
    
    // IRC 클라이언트를 구독자로 등록
    void subscribeClient(std::shared_ptr<IRCClient> client, 
                        const std::string& channel) {
        auto filter = filters_[channel];
        auto clientLifetime = client->getLifetime();
        
        broker_.subscribe(channel, 
            [client, channel, filter](const LogEntry& log) {
                if (!filter || filter(log)) {
                    // IRC 메시지 형식으로 변환
                    std::string message = formatLogForIRC(log);
                    client->sendToChannel(channel, message);
                }
            },
            clientLifetime
        );
    }
    
    // 새 로그 브로드캐스트
    void broadcastLog(const LogEntry& log) {
        // 모든 채널로 발행 (필터는 구독자 측에서 처리)
        for (const auto& [channel, _] : filters_) {
            broker_.publish(channel, log);
        }
        
        // 항상 #logs-all로도 발행
        broker_.publish("#logs-all", log);
    }
};
```

## 2. 효율적인 메시지 라우팅

### 2.1 라우팅 테이블
```cpp
class MessageRouter {
private:
    // 토픽 기반 라우팅
    struct Route {
        std::regex pattern;
        std::vector<std::string> destinations;
        std::function<bool(const LogEntry&)> condition;
    };
    
    std::vector<Route> routes_;
    std::shared_mutex routeMutex_;
    
    // 목적지별 큐
    std::unordered_map<std::string, 
                      std::unique_ptr<ConcurrentQueue<LogEntry>>> queues_;
    
public:
    // 라우팅 규칙 추가
    void addRoute(const std::string& pattern, 
                 const std::vector<std::string>& destinations,
                 std::function<bool(const LogEntry&)> condition = nullptr) {
        std::unique_lock lock(routeMutex_);
        routes_.push_back({std::regex(pattern), destinations, condition});
    }
    
    // 메시지 라우팅
    void route(const LogEntry& log) {
        std::shared_lock lock(routeMutex_);
        
        for (const auto& route : routes_) {
            if (std::regex_match(log.category, route.pattern)) {
                if (!route.condition || route.condition(log)) {
                    for (const auto& dest : route.destinations) {
                        if (auto it = queues_.find(dest); it != queues_.end()) {
                            it->second->push(log);
                        }
                    }
                }
            }
        }
    }
    
    // 스마트 라우팅 (부하 분산)
    void routeWithLoadBalancing(const LogEntry& log, 
                               const std::vector<std::string>& destinations) {
        if (destinations.empty()) return;
        
        // 가장 여유있는 큐 선택
        std::string bestDest;
        size_t minSize = SIZE_MAX;
        
        for (const auto& dest : destinations) {
            if (auto it = queues_.find(dest); it != queues_.end()) {
                size_t size = it->second->size();
                if (size < minSize) {
                    minSize = size;
                    bestDest = dest;
                }
            }
        }
        
        if (!bestDest.empty()) {
            queues_[bestDest]->push(log);
        }
    }
};
```

### 2.2 계층적 토픽 라우팅
```cpp
class HierarchicalRouter {
private:
    // 토픽 트리 노드
    struct TopicNode {
        std::string name;
        std::vector<std::weak_ptr<IRCClient>> subscribers;
        std::unordered_map<std::string, std::unique_ptr<TopicNode>> children;
        bool isWildcard = false;
    };
    
    std::unique_ptr<TopicNode> root_;
    std::shared_mutex mutex_;
    
    // 토픽 경로 파싱 (예: "logs/error/database")
    std::vector<std::string> parseTopic(const std::string& topic) {
        std::vector<std::string> parts;
        std::stringstream ss(topic);
        std::string part;
        
        while (std::getline(ss, part, '/')) {
            parts.push_back(part);
        }
        
        return parts;
    }
    
public:
    // 구독 (와일드카드 지원)
    void subscribe(const std::string& topicPattern, 
                  std::shared_ptr<IRCClient> client) {
        auto parts = parseTopic(topicPattern);
        std::unique_lock lock(mutex_);
        
        TopicNode* node = root_.get();
        
        for (const auto& part : parts) {
            if (part == "*") {
                // 단일 레벨 와일드카드
                node->isWildcard = true;
                node->subscribers.push_back(client);
                return;
            } else if (part == "#") {
                // 다중 레벨 와일드카드
                node->isWildcard = true;
                node->subscribers.push_back(client);
                return;
            }
            
            if (!node->children[part]) {
                node->children[part] = std::make_unique<TopicNode>();
                node->children[part]->name = part;
            }
            
            node = node->children[part].get();
        }
        
        node->subscribers.push_back(client);
    }
    
    // 발행 (매칭되는 모든 구독자에게)
    void publish(const std::string& topic, const LogEntry& log) {
        auto parts = parseTopic(topic);
        std::vector<std::shared_ptr<IRCClient>> recipients;
        
        {
            std::shared_lock lock(mutex_);
            collectSubscribers(root_.get(), parts, 0, recipients);
        }
        
        // 락 해제 후 전송
        for (const auto& client : recipients) {
            if (client) {
                client->sendLogEntry(log);
            }
        }
    }
    
private:
    void collectSubscribers(TopicNode* node, 
                           const std::vector<std::string>& parts,
                           size_t index,
                           std::vector<std::shared_ptr<IRCClient>>& recipients) {
        if (!node) return;
        
        // 현재 노드의 구독자 수집
        for (const auto& weakClient : node->subscribers) {
            if (auto client = weakClient.lock()) {
                recipients.push_back(client);
            }
        }
        
        if (index >= parts.size()) return;
        
        // 정확한 매치
        if (auto it = node->children.find(parts[index]); 
            it != node->children.end()) {
            collectSubscribers(it->second.get(), parts, index + 1, recipients);
        }
        
        // 와일드카드 매치
        if (auto it = node->children.find("*"); it != node->children.end()) {
            collectSubscribers(it->second.get(), parts, index + 1, recipients);
        }
        
        // 다중 레벨 와일드카드
        if (auto it = node->children.find("#"); it != node->children.end()) {
            for (const auto& weakClient : it->second->subscribers) {
                if (auto client = weakClient.lock()) {
                    recipients.push_back(client);
                }
            }
        }
    }
};
```

## 3. 백프레셔 처리

### 3.1 적응형 백프레셔
```cpp
class BackpressureManager {
private:
    struct ClientMetrics {
        std::atomic<size_t> pendingMessages{0};
        std::atomic<size_t> droppedMessages{0};
        std::chrono::steady_clock::time_point lastActivity;
        std::atomic<bool> isPaused{false};
    };
    
    std::unordered_map<int, ClientMetrics> clientMetrics_;
    std::mutex metricsMutex_;
    
    static constexpr size_t HIGH_WATER_MARK = 1000;
    static constexpr size_t LOW_WATER_MARK = 100;
    
public:
    // 메시지 전송 시도
    bool tryEnqueue(int clientFd, const std::function<void()>& sendFunc) {
        auto& metrics = getMetrics(clientFd);
        
        // 고수위 마크 초과 시
        if (metrics.pendingMessages >= HIGH_WATER_MARK) {
            if (!metrics.isPaused) {
                pauseClient(clientFd);
                metrics.isPaused = true;
            }
            
            metrics.droppedMessages++;
            return false;  // 메시지 드롭
        }
        
        metrics.pendingMessages++;
        
        // 비동기 전송
        std::thread([this, clientFd, sendFunc, &metrics]() {
            try {
                sendFunc();
                
                metrics.pendingMessages--;
                metrics.lastActivity = std::chrono::steady_clock::now();
                
                // 저수위 마크 도달 시 재개
                if (metrics.isPaused && 
                    metrics.pendingMessages <= LOW_WATER_MARK) {
                    resumeClient(clientFd);
                    metrics.isPaused = false;
                }
            } catch (...) {
                metrics.pendingMessages--;
                handleClientError(clientFd);
            }
        }).detach();
        
        return true;
    }
    
    // 통계 보고
    void reportStats() {
        std::lock_guard lock(metricsMutex_);
        
        for (const auto& [fd, metrics] : clientMetrics_) {
            if (metrics.droppedMessages > 0) {
                logWarning("Client {} dropped {} messages due to backpressure",
                          fd, metrics.droppedMessages.load());
            }
        }
    }
    
private:
    void pauseClient(int clientFd) {
        // TCP 수신 버퍼 일시 정지
        int val = 0;
        setsockopt(clientFd, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val));
        
        logInfo("Pausing client {} due to backpressure", clientFd);
    }
    
    void resumeClient(int clientFd) {
        // TCP 수신 버퍼 재개
        int val = 65536;  // 64KB
        setsockopt(clientFd, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val));
        
        logInfo("Resuming client {}", clientFd);
    }
};
```

### 3.2 동적 레이트 제한
```cpp
class DynamicRateLimiter {
private:
    struct TokenBucket {
        std::atomic<double> tokens;
        std::chrono::steady_clock::time_point lastRefill;
        double capacity;
        double refillRate;  // 초당 토큰
        std::mutex mutex;
    };
    
    std::unordered_map<std::string, TokenBucket> buckets_;
    std::shared_mutex bucketsMutex_;
    
    // 시스템 부하 모니터
    std::atomic<double> systemLoad_{0.0};
    
public:
    // 적응형 레이트 제한
    bool allowMessage(const std::string& clientId, double cost = 1.0) {
        auto& bucket = getBucket(clientId);
        
        std::lock_guard lock(bucket.mutex);
        
        // 토큰 리필
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - bucket.lastRefill);
        
        // 시스템 부하에 따라 리필 속도 조정
        double adjustedRate = bucket.refillRate * (2.0 - systemLoad_);
        double refill = elapsed.count() * adjustedRate;
        
        bucket.tokens = std::min(bucket.capacity, 
                                bucket.tokens.load() + refill);
        bucket.lastRefill = now;
        
        // 토큰 소비
        if (bucket.tokens >= cost) {
            bucket.tokens -= cost;
            return true;
        }
        
        return false;
    }
    
    // 시스템 부하 업데이트
    void updateSystemLoad(double cpuUsage, double memoryUsage, 
                         double networkUsage) {
        // 가중 평균
        systemLoad_ = (cpuUsage * 0.4 + memoryUsage * 0.3 + 
                      networkUsage * 0.3);
        
        // 부하가 높으면 전체적으로 레이트 감소
        if (systemLoad_ > 0.8) {
            logWarning("High system load: {:.2f}, reducing rates", 
                      systemLoad_.load());
        }
    }
    
private:
    TokenBucket& getBucket(const std::string& clientId) {
        std::shared_lock readLock(bucketsMutex_);
        
        if (auto it = buckets_.find(clientId); it != buckets_.end()) {
            return it->second;
        }
        
        readLock.unlock();
        std::unique_lock writeLock(bucketsMutex_);
        
        // 새 버킷 생성
        auto& bucket = buckets_[clientId];
        bucket.tokens = 10.0;      // 초기 토큰
        bucket.capacity = 100.0;   // 최대 용량
        bucket.refillRate = 50.0;  // 초당 50개
        bucket.lastRefill = std::chrono::steady_clock::now();
        
        return bucket;
    }
};
```

## 4. 메시지 배칭

### 4.1 시간 기반 배칭
```cpp
class MessageBatcher {
private:
    struct Batch {
        std::vector<std::string> messages;
        std::chrono::steady_clock::time_point createTime;
        size_t totalSize = 0;
    };
    
    std::unordered_map<int, Batch> clientBatches_;
    std::mutex batchMutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_{true};
    std::thread flushThread_;
    
    static constexpr size_t MAX_BATCH_SIZE = 4096;  // 4KB
    static constexpr auto MAX_BATCH_DELAY = std::chrono::milliseconds(100);
    
public:
    MessageBatcher() : flushThread_(&MessageBatcher::flushWorker, this) {}
    
    ~MessageBatcher() {
        running_ = false;
        cv_.notify_all();
        if (flushThread_.joinable()) {
            flushThread_.join();
        }
    }
    
    // 메시지 추가
    void addMessage(int clientFd, const std::string& message) {
        std::lock_guard lock(batchMutex_);
        
        auto& batch = clientBatches_[clientFd];
        
        if (batch.messages.empty()) {
            batch.createTime = std::chrono::steady_clock::now();
        }
        
        batch.messages.push_back(message);
        batch.totalSize += message.size() + 2;  // +2 for \r\n
        
        // 크기 제한 도달 시 즉시 전송
        if (batch.totalSize >= MAX_BATCH_SIZE) {
            flushBatch(clientFd, batch);
            batch = Batch{};  // 리셋
        }
        
        cv_.notify_one();
    }
    
private:
    // 주기적 플러시
    void flushWorker() {
        while (running_) {
            std::unique_lock lock(batchMutex_);
            
            cv_.wait_for(lock, std::chrono::milliseconds(50), [this] {
                return !running_;
            });
            
            auto now = std::chrono::steady_clock::now();
            
            for (auto it = clientBatches_.begin(); it != clientBatches_.end();) {
                if (!it->second.messages.empty() &&
                    now - it->second.createTime >= MAX_BATCH_DELAY) {
                    
                    flushBatch(it->first, it->second);
                    it = clientBatches_.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    
    // 배치 전송
    void flushBatch(int clientFd, const Batch& batch) {
        if (batch.messages.empty()) return;
        
        // 모든 메시지를 하나로 결합
        std::string combined;
        combined.reserve(batch.totalSize);
        
        for (const auto& msg : batch.messages) {
            combined += msg + "\r\n";
        }
        
        // 한 번의 시스템 콜로 전송
        send(clientFd, combined.c_str(), combined.size(), MSG_NOSIGNAL);
    }
};
```

### 4.2 컨텐츠 기반 배칭
```cpp
class ContentAwareBatcher {
private:
    enum class Priority {
        HIGH = 0,    // 에러, 중요 알림
        MEDIUM = 1,  // 일반 로그
        LOW = 2      // 디버그, 정보성
    };
    
    struct PriorityBatch {
        std::queue<std::string> messages;
        size_t totalSize = 0;
    };
    
    // 클라이언트별, 우선순위별 큐
    using ClientBatches = std::array<PriorityBatch, 3>;
    std::unordered_map<int, ClientBatches> clientQueues_;
    
public:
    void addMessage(int clientFd, const LogEntry& log) {
        Priority priority = categorizePriority(log);
        std::string formatted = formatMessage(log);
        
        auto& batches = clientQueues_[clientFd];
        auto& batch = batches[static_cast<int>(priority)];
        
        batch.messages.push(formatted);
        batch.totalSize += formatted.size();
        
        // 높은 우선순위는 즉시 전송
        if (priority == Priority::HIGH || 
            batch.totalSize >= MAX_BATCH_SIZE) {
            flushPriority(clientFd, priority);
        }
    }
    
    // 우선순위별 플러시
    void flushAll(int clientFd) {
        auto& batches = clientQueues_[clientFd];
        
        // 높은 우선순위부터 전송
        for (int p = 0; p < 3; ++p) {
            flushPriority(clientFd, static_cast<Priority>(p));
        }
    }
    
private:
    Priority categorizePriority(const LogEntry& log) {
        if (log.level == "ERROR" || log.level == "CRITICAL") {
            return Priority::HIGH;
        } else if (log.level == "WARNING" || log.level == "INFO") {
            return Priority::MEDIUM;
        } else {
            return Priority::LOW;
        }
    }
    
    void flushPriority(int clientFd, Priority priority) {
        auto& batch = clientQueues_[clientFd][static_cast<int>(priority)];
        
        if (batch.messages.empty()) return;
        
        std::vector<std::string> messages;
        while (!batch.messages.empty()) {
            messages.push_back(std::move(batch.messages.front()));
            batch.messages.pop();
        }
        
        sendBatch(clientFd, messages);
        batch.totalSize = 0;
    }
};
```

## 5. 이벤트 루프 최적화

### 5.1 고성능 이벤트 루프
```cpp
class OptimizedEventLoop {
private:
    int epollFd_;
    std::atomic<bool> running_{true};
    
    // 이벤트 핸들러
    using EventHandler = std::function<void(uint32_t events)>;
    std::unordered_map<int, EventHandler> handlers_;
    
    // 작업 큐
    moodycamel::ConcurrentQueue<std::function<void()>> taskQueue_;
    
    static constexpr int MAX_EVENTS = 1024;
    static constexpr int BATCH_SIZE = 64;
    
public:
    OptimizedEventLoop() {
        epollFd_ = epoll_create1(EPOLL_CLOEXEC);
        if (epollFd_ < 0) {
            throw std::runtime_error("Failed to create epoll");
        }
    }
    
    // 파일 디스크립터 등록
    void registerFd(int fd, uint32_t events, EventHandler handler) {
        struct epoll_event ev;
        ev.events = events | EPOLLET;  // Edge-triggered
        ev.data.fd = fd;
        
        if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
            throw std::runtime_error("Failed to add fd to epoll");
        }
        
        handlers_[fd] = std::move(handler);
    }
    
    // 메인 루프
    void run() {
        struct epoll_event events[MAX_EVENTS];
        std::vector<std::function<void()>> tasks;
        tasks.reserve(BATCH_SIZE);
        
        while (running_) {
            // 이벤트 대기
            int nfds = epoll_wait(epollFd_, events, MAX_EVENTS, 10);
            
            if (nfds < 0) {
                if (errno == EINTR) continue;
                break;
            }
            
            // I/O 이벤트 처리
            for (int i = 0; i < nfds; ++i) {
                int fd = events[i].data.fd;
                
                if (auto it = handlers_.find(fd); it != handlers_.end()) {
                    it->second(events[i].events);
                }
            }
            
            // 작업 큐 처리 (배치)
            tasks.clear();
            size_t dequeued = taskQueue_.try_dequeue_bulk(
                std::back_inserter(tasks), BATCH_SIZE);
            
            for (auto& task : tasks) {
                task();
            }
        }
    }
    
    // 비동기 작업 제출
    void post(std::function<void()> task) {
        taskQueue_.enqueue(std::move(task));
        
        // 이벤트 루프 깨우기
        uint64_t one = 1;
        write(eventFd_, &one, sizeof(one));
    }
};
```

### 5.2 코루틴 기반 브로드캐스팅
```cpp
#ifdef __cpp_lib_coroutine
class CoroutineBroadcaster {
private:
    struct BroadcastTask {
        struct promise_type {
            BroadcastTask get_return_object() {
                return {std::coroutine_handle<promise_type>::from_promise(*this)};
            }
            std::suspend_never initial_suspend() { return {}; }
            std::suspend_never final_suspend() noexcept { return {}; }
            void return_void() {}
            void unhandled_exception() {}
        };
        
        std::coroutine_handle<promise_type> h_;
    };
    
public:
    // 비동기 브로드캐스트
    BroadcastTask broadcastAsync(const std::string& channel, 
                                const LogEntry& log) {
        auto clients = getChannelClients(channel);
        
        // 청크 단위로 처리
        const size_t CHUNK_SIZE = 100;
        
        for (size_t i = 0; i < clients.size(); i += CHUNK_SIZE) {
            auto end = std::min(i + CHUNK_SIZE, clients.size());
            
            // 청크 병렬 처리
            co_await processChunk(clients, i, end, log);
            
            // CPU 양보
            co_await std::suspend_always{};
        }
    }
    
private:
    struct ChunkAwaiter {
        std::vector<std::shared_ptr<IRCClient>>& clients;
        size_t start;
        size_t end;
        const LogEntry& log;
        
        bool await_ready() { return false; }
        
        void await_suspend(std::coroutine_handle<> h) {
            // 스레드 풀에서 처리
            threadPool.enqueue([this, h]() {
                for (size_t i = start; i < end; ++i) {
                    if (clients[i]) {
                        clients[i]->sendLogEntry(log);
                    }
                }
                h.resume();
            });
        }
        
        void await_resume() {}
    };
    
    ChunkAwaiter processChunk(std::vector<std::shared_ptr<IRCClient>>& clients,
                             size_t start, size_t end, const LogEntry& log) {
        return {clients, start, end, log};
    }
};
#endif
```

## 6. 모니터링과 메트릭

### 6.1 브로드캐스트 성능 메트릭
```cpp
class BroadcastMetrics {
private:
    struct ChannelStats {
        std::atomic<uint64_t> messagesSent{0};
        std::atomic<uint64_t> messagesDropped{0};
        std::atomic<uint64_t> bytesTransferred{0};
        std::atomic<uint64_t> subscriberCount{0};
        
        // 레이턴시 히스토그램
        folly::Histogram<int64_t> latencyHistogram{1, 0, 10000};  // 0-10초
        std::mutex histogramMutex;
    };
    
    std::unordered_map<std::string, ChannelStats> channelStats_;
    std::shared_mutex statsMutex_;
    
public:
    // 메시지 전송 기록
    void recordSend(const std::string& channel, size_t bytes, 
                   std::chrono::microseconds latency) {
        auto& stats = getStats(channel);
        
        stats.messagesSent++;
        stats.bytesTransferred += bytes;
        
        {
            std::lock_guard lock(stats.histogramMutex);
            stats.latencyHistogram.addValue(latency.count());
        }
    }
    
    // 드롭 기록
    void recordDrop(const std::string& channel) {
        auto& stats = getStats(channel);
        stats.messagesDropped++;
    }
    
    // 통계 보고
    void reportStats() {
        std::shared_lock lock(statsMutex_);
        
        for (const auto& [channel, stats] : channelStats_) {
            auto sent = stats.messagesSent.load();
            auto dropped = stats.messagesDropped.load();
            auto bytes = stats.bytesTransferred.load();
            
            double dropRate = sent > 0 ? 
                (double)dropped / (sent + dropped) * 100 : 0;
            
            logInfo("Channel {}: {} sent, {} dropped ({:.2f}%), {} bytes",
                   channel, sent, dropped, dropRate, bytes);
            
            // 레이턴시 통계
            {
                std::lock_guard histLock(stats.histogramMutex);
                logInfo("  Latency - P50: {}us, P95: {}us, P99: {}us",
                       stats.latencyHistogram.getPercentile(0.50),
                       stats.latencyHistogram.getPercentile(0.95),
                       stats.latencyHistogram.getPercentile(0.99));
            }
        }
    }
    
private:
    ChannelStats& getStats(const std::string& channel) {
        std::shared_lock readLock(statsMutex_);
        
        if (auto it = channelStats_.find(channel); it != channelStats_.end()) {
            return it->second;
        }
        
        readLock.unlock();
        std::unique_lock writeLock(statsMutex_);
        return channelStats_[channel];
    }
};
```

## 🔥 흔한 실수와 해결방법

### 1. 메모리 누수

#### [SEQUENCE: 123] weak_ptr 미사용으로 인한 순환 참조
```cpp
// ❌ 잘못된 예: shared_ptr로 구독자 저장
class BadPubSub {
    std::vector<std::shared_ptr<Subscriber>> subscribers_;
    
    void subscribe(std::shared_ptr<Subscriber> sub) {
        subscribers_.push_back(sub);  // 구독자가 영구히 유지됨
    }
};

// ✅ 올바른 예: weak_ptr 사용
class GoodPubSub {
    std::vector<std::weak_ptr<Subscriber>> subscribers_;
    
    void publish() {
        // 만료된 구독자 자동 정리
        auto it = std::remove_if(subscribers_.begin(), subscribers_.end(),
            [](const auto& weak) { return weak.expired(); });
        subscribers_.erase(it, subscribers_.end());
    }
};
```

### 2. 데드락 문제

#### [SEQUENCE: 124] 콜백 내에서 락 재획득
```cpp
// ❌ 잘못된 예: 콜백 실행 중 락 유지
void publish(const Message& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& callback : callbacks_) {
        callback(msg);  // 콜백이 publish() 호출하면 데드락
    }
}

// ✅ 올바른 예: 콜백 복사 후 락 해제
void publish(const Message& msg) {
    std::vector<Callback> localCallbacks;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        localCallbacks = callbacks_;
    }
    
    for (auto& callback : localCallbacks) {
        callback(msg);
    }
}
```

### 3. 백프레셔 미처리

#### [SEQUENCE: 125] 무제한 큐 사용
```cpp
// ❌ 잘못된 예: 큐 크기 제한 없음
void broadcast(const Message& msg) {
    for (auto& client : clients_) {
        client->queue.push(msg);  // 메모리 폭발 가능
    }
}

// ✅ 올바른 예: 백프레셔 적용
void broadcast(const Message& msg) {
    for (auto& client : clients_) {
        if (client->queue.size() >= MAX_QUEUE_SIZE) {
            client->markSlow();
            dropMessage(client, msg);
        } else {
            client->queue.push(msg);
        }
    }
}
```

## 마무리

실시간 브로드캐스팅은 LogCaster-IRC의 핵심 성능을 결정합니다. 이 가이드에서 다룬 기술들:

1. **Pub/Sub 패턴**으로 느슨한 결합과 확장성 확보
2. **효율적인 라우팅**으로 메시지 전달 최적화
3. **백프레셔 처리**로 시스템 안정성 보장
4. **메시지 배칭**으로 네트워크 효율성 향상
5. **이벤트 루프 최적화**로 고성능 달성

다음 Chat_Server_Security.md에서는 이러한 브로드캐스팅 시스템을 안전하게 보호하는 방법을 다룹니다.

## 실습 프로젝트

### 프로젝트 1: 간단한 Pub/Sub 시스템 (기초)
**목표**: 기본적인 발행-구독 패턴 구현

**구현 사항**:
- 토픽 기반 구독
- 메시지 발행
- 구독자 관리
- 메모리 안전성

### 프로젝트 2: 스케일러블 브로드캐스터 (중급)
**목표**: 수천 클라이언트 지원 시스템

**구현 사항**:
- 계층적 토픽 라우팅
- 백프레셔 처리
- 메시지 배칭
- 성능 모니터링

### 프로젝트 3: 분산 브로드캐스트 시스템 (고급)
**목표**: 멀티 노드 브로드캐스트 클러스터

**구현 사항**:
- 노드 간 동기화
- 파티셔닝 전략
- 장애 복구
- 일관성 보장

## 학습 체크리스트

### 기초 레벨 ✅
- [ ] Pub/Sub 패턴 이해
- [ ] 기본 브로드캐스트 구현
- [ ] 메모리 안전성 확보
- [ ] 간단한 백프레셔 처리

### 중급 레벨 📚
- [ ] 효율적인 메시지 라우팅
- [ ] 동적 레이트 제한
- [ ] 메시지 배칭 최적화
- [ ] 이벤트 루프 통합

### 고급 레벨 🚀
- [ ] 제로카피 브로드캐스팅
- [ ] 적응형 백프레셔
- [ ] 코루틴 기반 처리
- [ ] 분산 시스템 설계

### 전문가 레벨 🏆
- [ ] 커스텀 프로토콜 최적화
- [ ] 하드웨어 가속 활용
- [ ] 실시간 성능 튜닝
- [ ] 대규모 시스템 아키텍처

## 추가 학습 자료

### 추천 도서
- "Designing Data-Intensive Applications" - Martin Kleppmann
- "High Performance Messaging Systems" - Mark Richards
- "The Art of Multiprocessor Programming" - Herlihy & Shavit
- "Reactive Messaging Patterns" - Vaughn Vernon

### 온라인 리소스
- [NATS Documentation](https://docs.nats.io/) - 고성능 메시징 시스템
- [Apache Kafka Design](https://kafka.apache.org/documentation/) - 분산 스트리밍
- [ZeroMQ Guide](https://zguide.zeromq.org/) - 메시징 패턴
- [LMAX Disruptor](https://lmax-exchange.github.io/disruptor/) - 고성능 큐

### 실습 도구
- perf - 성능 프로파일링
- tcpdump - 네트워크 분석
- iperf3 - 대역폭 테스트
- Apache JMeter - 부하 테스트

---

*💡 팁: 브로드캐스팅 성능은 가장 느린 구독자에 의해 결정됩니다. 항상 느린 클라이언트를 고려한 설계를 하세요.*