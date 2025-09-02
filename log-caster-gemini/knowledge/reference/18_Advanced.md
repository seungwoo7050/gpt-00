# 10. 고급 개념과 최적화 🚀
## LogCaster의 전문적 기능들

*LogCaster 프로젝트를 위한 고급 개념 문서*

---

## 📋 문서 정보

- **난이도**: 고급 (Advanced)
- **예상 학습 시간**: 15-20시간
- **필요 선수 지식**: 
  - C/C++ 중급 이상
  - 멀티스레딩 기초
  - 네트워크 프로그래밍 기초
  - 시스템 프로그래밍 개념
- **실습 환경**: Linux/Unix 환경 권장

---

## 🎯 이 문서에서 배울 내용

이 문서는 LogCaster 프로젝트의 고급 기능 구현을 위한 전문적인 개념들을 다룹니다. 멀티스레딩, 네트워크 프로그래밍, 성능 최적화, 파일 I/O 등 실제 로그 시스템에서 필요한 핵심 기술들을 심도 있게 설명합니다.

---

## 1. 다중 스레드와 동시성 프로그래밍

로그 시스템은 여러 클라이언트로부터 동시에 로그를 받아 처리해야 하므로, 멀티스레딩과 동시성 제어가 핵심적입니다.

### C에서의 POSIX 스레드 (pthread)

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// 공유 자원: 로그 큐
#define MAX_LOG_QUEUE 100
typedef struct {
    char messages[MAX_LOG_QUEUE][256];
    int front, rear, count;
    pthread_mutex_t mutex;      // 뮤텍스로 큐 보호
    pthread_cond_t not_empty;   // 큐가 비어있지 않을 때의 조건 변수
    pthread_cond_t not_full;    // 큐가 가득 차지 않았을 때의 조건 변수
} LogQueue;

LogQueue log_queue = {
    .front = 0, .rear = 0, .count = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .not_empty = PTHREAD_COND_INITIALIZER,
    .not_full = PTHREAD_COND_INITIALIZER
};

// 로그를 큐에 추가하는 함수 (생산자)
void enqueue_log(const char* message) {
    pthread_mutex_lock(&log_queue.mutex);
    
    // 큐가 가득 찬 경우 대기
    while (log_queue.count == MAX_LOG_QUEUE) {
        pthread_cond_wait(&log_queue.not_full, &log_queue.mutex);
    }
    
    // 로그 메시지 추가
    strncpy(log_queue.messages[log_queue.rear], message, 255);
    log_queue.messages[log_queue.rear][255] = '\0';
    log_queue.rear = (log_queue.rear + 1) % MAX_LOG_QUEUE;
    log_queue.count++;
    
    // 큐에 데이터가 추가되었음을 알림
    pthread_cond_signal(&log_queue.not_empty);
    pthread_mutex_unlock(&log_queue.mutex);
}

// 로그를 큐에서 제거하는 함수 (소비자)
int dequeue_log(char* buffer, size_t buffer_size) {
    pthread_mutex_lock(&log_queue.mutex);
    
    // 큐가 빈 경우 대기
    while (log_queue.count == 0) {
        pthread_cond_wait(&log_queue.not_empty, &log_queue.mutex);
    }
    
    // 로그 메시지 제거
    strncpy(buffer, log_queue.messages[log_queue.front], buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    log_queue.front = (log_queue.front + 1) % MAX_LOG_QUEUE;
    log_queue.count--;
    
    // 큐에 공간이 생겼음을 알림
    pthread_cond_signal(&log_queue.not_full);
    pthread_mutex_unlock(&log_queue.mutex);
    
    return 1;
}

// 로그 생성자 스레드 함수
void* log_producer(void* arg) {
    int producer_id = *(int*)arg;
    char message[256];
    
    for (int i = 0; i < 5; i++) {
        snprintf(message, sizeof(message), 
                 "Producer %d: Log message %d", producer_id, i + 1);
        enqueue_log(message);
        printf("Produced: %s\n", message);
        usleep(100000);  // 0.1초 대기
    }
    
    return NULL;
}

// 로그 소비자 스레드 함수 (파일에 기록)
void* log_consumer(void* arg) {
    char message[256];
    FILE* log_file = fopen("threaded_log.txt", "a");
    
    if (log_file == NULL) {
        perror("Failed to open log file");
        return NULL;
    }
    
    for (int i = 0; i < 10; i++) {  // 2개 생산자가 각각 5개씩 생성하므로 총 10개 처리
        dequeue_log(message, sizeof(message));
        
        // 타임스탬프와 함께 파일에 기록
        time_t now = time(NULL);
        fprintf(log_file, "[%ld] %s\n", now, message);
        fflush(log_file);  // 버퍼 강제 플러시
        
        printf("Consumed: %s\n", message);
        usleep(150000);  // 0.15초 대기
    }
    
    fclose(log_file);
    return NULL;
}

int main() {
    pthread_t producers[2], consumer;
    int producer_ids[2] = {1, 2};
    
    // 소비자 스레드 생성
    pthread_create(&consumer, NULL, log_consumer, NULL);
    
    // 생산자 스레드들 생성
    for (int i = 0; i < 2; i++) {
        pthread_create(&producers[i], NULL, log_producer, &producer_ids[i]);
    }
    
    // 모든 스레드 완료 대기
    for (int i = 0; i < 2; i++) {
        pthread_join(producers[i], NULL);
    }
    pthread_join(consumer, NULL);
    
    // 자원 정리
    pthread_mutex_destroy(&log_queue.mutex);
    pthread_cond_destroy(&log_queue.not_empty);
    pthread_cond_destroy(&log_queue.not_full);
    
    printf("All threads completed successfully.\n");
    return 0;
}
```

### C++에서의 스레드와 동시성

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <chrono>
#include <fstream>
#include <atomic>

class ThreadSafeLogQueue {
private:
    std::queue<std::string> queue_;
    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    const size_t max_size_;
    std::atomic<bool> shutdown_{false};

public:
    ThreadSafeLogQueue(size_t max_size = 100) : max_size_(max_size) {}
    
    // 로그 추가 (생산자)
    void push(const std::string& message) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // 큐가 가득 찬 경우 대기
        not_full_.wait(lock, [this] { 
            return queue_.size() < max_size_ || shutdown_; 
        });
        
        if (shutdown_) return;
        
        queue_.push(message);
        not_empty_.notify_one();
    }
    
    // 로그 제거 (소비자)
    bool pop(std::string& message, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000)) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (not_empty_.wait_for(lock, timeout, [this] { 
            return !queue_.empty() || shutdown_; 
        })) {
            if (!queue_.empty()) {
                message = queue_.front();
                queue_.pop();
                not_full_.notify_one();
                return true;
            }
        }
        return false;
    }
    
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        not_empty_.notify_all();
        not_full_.notify_all();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};

class AsyncLogger {
private:
    ThreadSafeLogQueue log_queue_;
    std::thread writer_thread_;
    std::ofstream log_file_;
    std::atomic<bool> running_{true};

public:
    AsyncLogger(const std::string& filename) : log_file_(filename, std::ios::app) {
        if (!log_file_.is_open()) {
            throw std::runtime_error("Cannot open log file: " + filename);
        }
        
        // 백그라운드 로그 기록 스레드 시작
        writer_thread_ = std::thread(&AsyncLogger::logWriter, this);
    }
    
    ~AsyncLogger() {
        shutdown();
    }
    
    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::string timestamped_message = 
            "[" + std::to_string(time_t) + "] " + message;
            
        log_queue_.push(timestamped_message);
    }
    
    void shutdown() {
        running_ = false;
        log_queue_.shutdown();
        
        if (writer_thread_.joinable()) {
            writer_thread_.join();
        }
        
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

private:
    void logWriter() {
        std::string message;
        while (running_) {
            if (log_queue_.pop(message)) {
                log_file_ << message << std::endl;
                log_file_.flush();  // 즉시 디스크에 기록
            }
        }
        
        // 남은 로그들 처리
        while (log_queue_.pop(message, std::chrono::milliseconds(100))) {
            log_file_ << message << std::endl;
        }
    }
};

// 사용 예제
int main() {
    try {
        AsyncLogger logger("async_log.txt");
        
        // 여러 스레드에서 동시에 로그 생성
        std::vector<std::thread> threads;
        
        for (int i = 0; i < 3; i++) {
            threads.emplace_back([&logger, i]() {
                for (int j = 0; j < 5; j++) {
                    logger.log("Thread " + std::to_string(i) + 
                              " - Message " + std::to_string(j));
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            });
        }
        
        // 모든 스레드 완료 대기
        for (auto& t : threads) {
            t.join();
        }
        
        std::cout << "All logging completed." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

---

## 2. 네트워크 프로그래밍

LogCaster가 네트워크를 통해 로그를 수집하려면 소켓 프로그래밍이 필요합니다.

### TCP 서버 구현 (C)

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT 8080

// 클라이언트 연결 정보 구조체
typedef struct {
    int socket_fd;
    struct sockaddr_in address;
    int client_id;
} ClientConnection;

// 클라이언트 핸들러 스레드 함수
void* handle_client(void* arg) {
    ClientConnection* client = (ClientConnection*)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    
    printf("Client %d connected from %s:%d\n", 
           client->client_id,
           inet_ntoa(client->address.sin_addr),
           ntohs(client->address.sin_port));
    
    // 클라이언트로부터 로그 메시지 수신
    while ((bytes_received = recv(client->socket_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        
        // 로그 메시지 처리 (파일에 기록)
        FILE* log_file = fopen("network_logs.txt", "a");
        if (log_file) {
            fprintf(log_file, "[Client %d] %s\n", client->client_id, buffer);
            fclose(log_file);
        }
        
        printf("Received from client %d: %s\n", client->client_id, buffer);
        
        // 클라이언트에게 확인 메시지 전송
        const char* ack = "Log received\n";
        send(client->socket_fd, ack, strlen(ack), 0);
    }
    
    if (bytes_received == 0) {
        printf("Client %d disconnected\n", client->client_id);
    } else {
        perror("recv failed");
    }
    
    close(client->socket_fd);
    free(client);
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_counter = 0;
    
    // 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // 소켓 옵션 설정 (주소 재사용)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    // 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // 소켓 바인딩
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    // 연결 대기
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("Log server listening on port %d...\n", PORT);
    
    // 클라이언트 연결 수락 루프
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }
        
        // 클라이언트 연결 정보 할당
        ClientConnection* client = malloc(sizeof(ClientConnection));
        if (!client) {
            perror("Memory allocation failed");
            close(client_fd);
            continue;
        }
        
        client->socket_fd = client_fd;
        client->address = client_addr;
        client->client_id = ++client_counter;
        
        // 클라이언트 처리를 위한 새 스레드 생성
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, client) != 0) {
            perror("Thread creation failed");
            free(client);
            close(client_fd);
            continue;
        }
        
        // 스레드를 분리하여 자동으로 정리되도록 함
        pthread_detach(client_thread);
    }
    
    close(server_fd);
    return 0;
}
```

### C++에서의 네트워크 로거

```cpp
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <memory>
#include <chrono>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

class NetworkLogServer {
private:
    int server_socket_;
    std::atomic<bool> running_{false};
    std::vector<std::thread> client_threads_;
    std::atomic<int> client_counter_{0};
    std::ofstream log_file_;
    std::mutex log_mutex_;

public:
    NetworkLogServer(int port, const std::string& log_filename) 
        : log_file_(log_filename, std::ios::app) {
        
        if (!log_file_.is_open()) {
            throw std::runtime_error("Cannot open log file: " + log_filename);
        }
        
        // 소켓 생성
        server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket_ == -1) {
            throw std::runtime_error("Socket creation failed");
        }
        
        // 소켓 옵션 설정
        int opt = 1;
        if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            close(server_socket_);
            throw std::runtime_error("setsockopt failed");
        }
        
        // 서버 주소 설정
        struct sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);
        
        // 바인딩
        if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(server_socket_);
            throw std::runtime_error("Bind failed");
        }
        
        // 리스닝
        if (listen(server_socket_, 10) < 0) {
            close(server_socket_);
            throw std::runtime_error("Listen failed");
        }
        
        std::cout << "Network log server listening on port " << port << std::endl;
    }
    
    ~NetworkLogServer() {
        stop();
    }
    
    void start() {
        running_ = true;
        
        while (running_) {
            struct sockaddr_in client_addr{};
            socklen_t client_addr_len = sizeof(client_addr);
            
            int client_socket = accept(server_socket_, 
                                     (struct sockaddr*)&client_addr, 
                                     &client_addr_len);
                                     
            if (client_socket < 0) {
                if (running_) {
                    std::cerr << "Accept failed: " << strerror(errno) << std::endl;
                }
                continue;
            }
            
            // 클라이언트 처리 스레드 생성
            int client_id = ++client_counter_;
            client_threads_.emplace_back(&NetworkLogServer::handleClient, 
                                       this, client_socket, client_id, client_addr);
        }
    }
    
    void stop() {
        running_ = false;
        close(server_socket_);
        
        // 모든 클라이언트 스레드 완료 대기
        for (auto& thread : client_threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

private:
    void handleClient(int client_socket, int client_id, struct sockaddr_in client_addr) {
        std::cout << "Client " << client_id << " connected from " 
                  << inet_ntoa(client_addr.sin_addr) << ":" 
                  << ntohs(client_addr.sin_port) << std::endl;
        
        char buffer[1024];
        ssize_t bytes_received;
        
        while (running_ && (bytes_received = recv(client_socket, buffer, 1023, 0)) > 0) {
            buffer[bytes_received] = '\0';
            
            // 타임스탬프와 함께 로그 기록
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            
            {
                std::lock_guard<std::mutex> lock(log_mutex_);
                log_file_ << "[" << time_t << "][Client " << client_id << "] " 
                         << buffer << std::endl;
                log_file_.flush();
            }
            
            std::cout << "Received from client " << client_id << ": " << buffer << std::endl;
            
            // 확인 메시지 전송
            const char* ack = "Log received\n";
            send(client_socket, ack, strlen(ack), 0);
        }
        
        if (bytes_received == 0) {
            std::cout << "Client " << client_id << " disconnected" << std::endl;
        } else if (bytes_received < 0 && running_) {
            std::cerr << "recv failed for client " << client_id << ": " 
                      << strerror(errno) << std::endl;
        }
        
        close(client_socket);
    }
};

// 사용 예제
int main() {
    try {
        NetworkLogServer server(8080, "network_cpp_logs.txt");
        
        // 서버를 별도 스레드에서 실행
        std::thread server_thread(&NetworkLogServer::start, &server);
        
        // 메인 스레드에서 사용자 입력 대기
        std::cout << "Press Enter to stop the server..." << std::endl;
        std::cin.get();
        
        server.stop();
        server_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

---

## 3. 성능 최적화와 파일 I/O

### 버퍼링 전략

```cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

class BufferedFileLogger {
private:
    std::ofstream file_;
    std::vector<std::string> buffer_;
    std::mutex buffer_mutex_;
    std::condition_variable buffer_cv_;
    std::thread flush_thread_;
    std::atomic<bool> running_{true};
    
    const size_t buffer_size_limit_;
    const std::chrono::milliseconds flush_interval_;

public:
    BufferedFileLogger(const std::string& filename, 
                      size_t buffer_size = 1000,
                      std::chrono::milliseconds flush_interval = std::chrono::milliseconds(1000))
        : file_(filename, std::ios::app), 
          buffer_size_limit_(buffer_size),
          flush_interval_(flush_interval) {
        
        if (!file_.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }
        
        buffer_.reserve(buffer_size_limit_);
        flush_thread_ = std::thread(&BufferedFileLogger::flushWorker, this);
    }
    
    ~BufferedFileLogger() {
        shutdown();
    }
    
    void log(const std::string& message) {
        std::unique_lock<std::mutex> lock(buffer_mutex_);
        buffer_.push_back(message);
        
        if (buffer_.size() >= buffer_size_limit_) {
            buffer_cv_.notify_one();
        }
    }
    
    void forceFlush() {
        std::unique_lock<std::mutex> lock(buffer_mutex_);
        if (!buffer_.empty()) {
            flushBuffer(lock);
        }
    }
    
    void shutdown() {
        running_ = false;
        buffer_cv_.notify_all();
        
        if (flush_thread_.joinable()) {
            flush_thread_.join();
        }
        
        forceFlush();
        
        if (file_.is_open()) {
            file_.close();
        }
    }

private:
    void flushWorker() {
        while (running_) {
            std::unique_lock<std::mutex> lock(buffer_mutex_);
            
            // 버퍼가 가득 찼거나 시간 간격에 따라 플러시
            buffer_cv_.wait_for(lock, flush_interval_, [this] {
                return !running_ || buffer_.size() >= buffer_size_limit_;
            });
            
            if (!buffer_.empty()) {
                flushBuffer(lock);
            }
        }
    }
    
    void flushBuffer(std::unique_lock<std::mutex>& lock) {
        // 버퍼의 모든 로그를 파일에 기록
        for (const auto& message : buffer_) {
            file_ << message << '\n';
        }
        file_.flush();
        
        buffer_.clear();
        std::cout << "Flushed buffer to disk" << std::endl;
    }
};

// 성능 테스트
int main() {
    try {
        BufferedFileLogger logger("buffered_log.txt", 100, std::chrono::milliseconds(2000));
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // 10000개의 로그 메시지 생성
        for (int i = 0; i < 10000; i++) {
            logger.log("Log message " + std::to_string(i) + 
                      " with some additional content for testing");
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Logged 10000 messages in " << duration.count() << " ms" << std::endl;
        
        // 강제 플러시
        logger.forceFlush();
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 로그 레벨과 필터링

```cpp
#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <algorithm>

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARNING = 3,
    ERROR = 4,
    FATAL = 5
};

class LogFilter {
public:
    virtual ~LogFilter() = default;
    virtual bool shouldLog(LogLevel level, const std::string& message, 
                          const std::string& category) = 0;
};

class LevelFilter : public LogFilter {
private:
    LogLevel min_level_;

public:
    LevelFilter(LogLevel min_level) : min_level_(min_level) {}
    
    bool shouldLog(LogLevel level, const std::string&, const std::string&) override {
        return static_cast<int>(level) >= static_cast<int>(min_level_);
    }
};

class CategoryFilter : public LogFilter {
private:
    std::vector<std::string> allowed_categories_;

public:
    CategoryFilter(const std::vector<std::string>& categories) 
        : allowed_categories_(categories) {}
    
    bool shouldLog(LogLevel, const std::string&, const std::string& category) override {
        return std::find(allowed_categories_.begin(), allowed_categories_.end(), category) 
               != allowed_categories_.end();
    }
};

class CompositeFilter : public LogFilter {
private:
    std::vector<std::unique_ptr<LogFilter>> filters_;

public:
    void addFilter(std::unique_ptr<LogFilter> filter) {
        filters_.push_back(std::move(filter));
    }
    
    bool shouldLog(LogLevel level, const std::string& message, 
                  const std::string& category) override {
        for (const auto& filter : filters_) {
            if (!filter->shouldLog(level, message, category)) {
                return false;  // 모든 필터를 통과해야 함
            }
        }
        return true;
    }
};

class AdvancedLogger {
private:
    std::unique_ptr<LogFilter> filter_;
    std::unordered_map<LogLevel, std::string> level_names_;
    
public:
    AdvancedLogger() {
        level_names_[LogLevel::TRACE] = "TRACE";
        level_names_[LogLevel::DEBUG] = "DEBUG";
        level_names_[LogLevel::INFO] = "INFO";
        level_names_[LogLevel::WARNING] = "WARNING";
        level_names_[LogLevel::ERROR] = "ERROR";
        level_names_[LogLevel::FATAL] = "FATAL";
    }
    
    void setFilter(std::unique_ptr<LogFilter> filter) {
        filter_ = std::move(filter);
    }
    
    void log(LogLevel level, const std::string& category, const std::string& message) {
        if (filter_ && !filter_->shouldLog(level, message, category)) {
            return;  // 필터에 의해 차단됨
        }
        
        std::cout << "[" << level_names_[level] << "][" << category << "] " 
                  << message << std::endl;
    }
    
    // 편의 메서드들
    void trace(const std::string& category, const std::string& message) {
        log(LogLevel::TRACE, category, message);
    }
    
    void debug(const std::string& category, const std::string& message) {
        log(LogLevel::DEBUG, category, message);
    }
    
    void info(const std::string& category, const std::string& message) {
        log(LogLevel::INFO, category, message);
    }
    
    void warning(const std::string& category, const std::string& message) {
        log(LogLevel::WARNING, category, message);
    }
    
    void error(const std::string& category, const std::string& message) {
        log(LogLevel::ERROR, category, message);
    }
    
    void fatal(const std::string& category, const std::string& message) {
        log(LogLevel::FATAL, category, message);
    }
};

// 사용 예제
int main() {
    AdvancedLogger logger;
    
    // 복합 필터 설정: WARNING 이상 레벨 + 특정 카테고리만 허용
    auto composite_filter = std::make_unique<CompositeFilter>();
    composite_filter->addFilter(std::make_unique<LevelFilter>(LogLevel::WARNING));
    composite_filter->addFilter(std::make_unique<CategoryFilter>(
        std::vector<std::string>{"SECURITY", "DATABASE", "NETWORK"}));
    
    logger.setFilter(std::move(composite_filter));
    
    // 다양한 로그 테스트
    logger.debug("SECURITY", "Debug message - should be filtered out");
    logger.info("DATABASE", "Info message - should be filtered out");
    logger.warning("SECURITY", "Security warning - should appear");
    logger.error("DATABASE", "Database error - should appear");
    logger.error("APPLICATION", "App error - should be filtered out (wrong category)");
    logger.fatal("NETWORK", "Network failure - should appear");
    
    return 0;
}
```

---

## 4. 흔한 실수와 해결방법

### 4.1 멀티스레딩 관련 실수

#### [SEQUENCE: 100] 잘못된 스레드 생성/종료 처리
```c
// ❌ 잘못된 예: 스레드 리소스 누수
void* thread_func(void* arg) {
    // 작업 수행
    return NULL;
}

int main() {
    pthread_t threads[10];
    for (int i = 0; i < 10; i++) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
        // pthread_join이나 pthread_detach 호출 안함 - 리소스 누수!
    }
    return 0;
}

// ✅ 올바른 예: 적절한 스레드 정리
int main() {
    pthread_t threads[10];
    for (int i = 0; i < 10; i++) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }
    
    // 모든 스레드 종료 대기
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
```

#### [SEQUENCE: 101] 조건 변수 사용 시 spurious wakeup
```cpp
// ❌ 잘못된 예: if 문 사용
void consumer() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (queue_.empty()) {  // spurious wakeup 시 문제 발생
        cv_.wait(lock);
    }
    // 큐가 여전히 비어있을 수 있음
}

// ✅ 올바른 예: while 루프 사용
void consumer() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {  // spurious wakeup 방지
        cv_.wait(lock);
    }
    // 큐에 데이터가 있음을 보장
}
```

### 4.2 네트워크 프로그래밍 실수

#### [SEQUENCE: 102] 부분 송수신 처리 누락
```c
// ❌ 잘못된 예: send/recv 반환값 무시
char buffer[1024];
send(socket, buffer, sizeof(buffer), 0);  // 전체가 전송되지 않을 수 있음

// ✅ 올바른 예: 전체 데이터 전송 보장
ssize_t send_all(int socket, const void* buffer, size_t length) {
    const char* ptr = (const char*)buffer;
    size_t remaining = length;
    
    while (remaining > 0) {
        ssize_t sent = send(socket, ptr, remaining, 0);
        if (sent <= 0) {
            return sent;  // 에러 처리
        }
        ptr += sent;
        remaining -= sent;
    }
    return length;
}
```

#### [SEQUENCE: 103] 소켓 옵션 설정 누락
```cpp
// ❌ 잘못된 예: TIME_WAIT 고려 안함
int server_socket = socket(AF_INET, SOCK_STREAM, 0);
bind(server_socket, ...);  // 재시작 시 "Address already in use" 에러

// ✅ 올바른 예: SO_REUSEADDR 설정
int server_socket = socket(AF_INET, SOCK_STREAM, 0);
int opt = 1;
setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
bind(server_socket, ...);
```

### 4.3 성능 최적화 실수

#### [SEQUENCE: 104] 과도한 동기화
```cpp
// ❌ 잘못된 예: 전체 함수를 락으로 보호
class Logger {
    std::mutex mutex_;
public:
    void log(const std::string& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        // 타임스탬프 생성 - 락 필요 없음
        auto timestamp = std::chrono::system_clock::now();
        // 포맷팅 - 락 필요 없음
        std::string formatted = format_message(timestamp, msg);
        // 파일 쓰기만 락 필요
        file_ << formatted << std::endl;
    }
};

// ✅ 올바른 예: 최소한의 크리티컬 섹션
class Logger {
    std::mutex mutex_;
public:
    void log(const std::string& msg) {
        // 락 밖에서 준비 작업
        auto timestamp = std::chrono::system_clock::now();
        std::string formatted = format_message(timestamp, msg);
        
        // 파일 쓰기만 보호
        {
            std::lock_guard<std::mutex> lock(mutex_);
            file_ << formatted << std::endl;
        }
    }
};
```

---

## 5. 실습 프로젝트

### 프로젝트 1: 멀티스레드 로그 수집기 (기초)
**목표**: 여러 스레드가 동시에 로그를 생성하고 단일 파일에 안전하게 기록

**구현 사항**:
- 생산자-소비자 패턴 구현
- 스레드 세이프 큐 구현
- 조건 변수를 통한 효율적인 대기
- 우아한 종료 처리

### 프로젝트 2: 네트워크 로그 서버 (중급)
**목표**: TCP 기반 로그 수집 서버와 클라이언트 구현

**구현 사항**:
- 멀티스레드 TCP 서버
- 프로토콜 설계 (헤더 + 페이로드)
- 연결 관리 및 타임아웃 처리
- 로그 레벨별 필터링

### 프로젝트 3: 고성능 로거 라이브러리 (고급)
**목표**: 프로덕션 수준의 로깅 라이브러리 구현

**구현 사항**:
- 락프리 큐를 사용한 비동기 로깅
- 메모리 매핑 파일 I/O
- 로그 회전 및 압축
- 플러그인 아키텍처 (다양한 출력 대상 지원)
- 성능 메트릭 수집

---

## 6. 학습 체크리스트

### 기초 레벨 ✅
- [ ] pthread 기본 API 이해
- [ ] 뮤텍스와 조건 변수 사용
- [ ] TCP 소켓 프로그래밍 기초
- [ ] 간단한 생산자-소비자 구현

### 중급 레벨 📚
- [ ] C++ std::thread와 동기화 프리미티브
- [ ] 스레드 풀 구현
- [ ] 비블로킹 I/O와 select/poll/epoll
- [ ] 버퍼링 전략 이해 및 구현

### 고급 레벨 🚀
- [ ] 락프리 프로그래밍 기법
- [ ] 메모리 순서와 원자적 연산
- [ ] 고성능 네트워크 서버 설계
- [ ] 시스템 콜 최적화 기법

### 전문가 레벨 🏆
- [ ] NUMA 아키텍처 고려사항
- [ ] CPU 캐시 최적화
- [ ] io_uring을 사용한 비동기 I/O
- [ ] 분산 로깅 시스템 설계

---

## 7. 추가 학습 자료

### 추천 도서
- "The Art of Multiprocessor Programming" - Maurice Herlihy
- "C++ Concurrency in Action" - Anthony Williams
- "UNIX Network Programming" - W. Richard Stevens

### 온라인 리소스
- [C++ Reference - Thread support library](https://en.cppreference.com/w/cpp/thread)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [High Performance Server Architecture](https://highscalability.com/)

### 실습 환경
- Linux perf tools로 성능 분석
- Valgrind로 메모리 누수 검출
- GDB로 멀티스레드 디버깅

---

## 4. 다음 단계 준비

이 고급 개념들을 이해했다면 LogCaster 프로젝트의 복잡한 기능들을 구현할 준비가 되었습니다:

1. **멀티스레딩**: 동시성 제어와 스레드 안전성
2. **네트워크 프로그래밍**: TCP/UDP를 통한 로그 수집
3. **성능 최적화**: 버퍼링, 비동기 I/O, 메모리 효율성
4. **필터링 시스템**: 로그 레벨과 카테고리 기반 필터링

### 확인 문제

1. 멀티스레드 환경에서 공유 자원에 접근할 때 왜 뮤텍스가 필요한가요?
2. 버퍼링을 사용하는 이유와 장단점은 무엇인가요?
3. 로그 필터링 시스템의 이점은 무엇인가요?

---

*💡 팁: 이 고급 개념들은 실제 프로덕션 로그 시스템의 핵심입니다. 각 예제를 직접 실행해보며 동작 원리를 이해해보세요!*