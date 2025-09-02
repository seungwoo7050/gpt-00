# LogCaster TCP 서버 구현 가이드

## 🎯 개요

LogCaster의 핵심인 TCP 서버 구현을 상세히 다룹니다. 비동기 I/O, 멀티스레딩, 에러 처리를 포함합니다.

## 🔌 소켓 프로그래밍 핵심

### 1. 서버 소켓 설정
```cpp
// [SEQUENCE: 47] 서버 초기화
void LogServer::initialize() {
    // 1. 소켓 생성
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        throw std::runtime_error("Socket creation failed");
    }
    
    // 2. 재사용 옵션 (TIME_WAIT 방지)
    int opt = 1;
    setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 3. Non-blocking 모드
    int flags = fcntl(listenFd_, F_GETFL, 0);
    fcntl(listenFd_, F_SETFL, flags | O_NONBLOCK);
    
    // 4. 주소 바인딩
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);
    
    if (bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Bind failed");
    }
    
    // 5. 리스닝 시작
    listen(listenFd_, BACKLOG);
}
```

### 2. 비동기 Accept Loop
```cpp
// [SEQUENCE: 852] 클라이언트 연결 수락
void IRCServer::acceptClients() {
    while (running_.load()) {
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        
        int clientFd = accept(listenFd_, 
                             (struct sockaddr*)&clientAddr, 
                             &addrLen);
        
        if (clientFd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Non-blocking 모드에서 정상
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            // 실제 에러
            std::cerr << "Accept error: " << strerror(errno) << std::endl;
            continue;
        }
        
        // 클라이언트도 non-blocking으로
        setNonBlocking(clientFd);
        
        // 스레드 풀에 위임
        threadPool_->enqueue([this, clientFd, clientAddr]() {
            handleClient(clientFd, clientAddr);
        });
    }
}
```

## 🔄 이벤트 기반 vs 스레드 기반

### LogCaster의 하이브리드 접근법

```cpp
// [SEQUENCE: 48] 메인 이벤트 루프
void LogServer::runEventLoop() {
    fd_set readfds;
    struct timeval tv;
    
    while (running_) {
        FD_ZERO(&readfds);
        FD_SET(listenFd_, &readfds);
        FD_SET(queryFd_, &readfds);
        
        int maxFd = std::max(listenFd_, queryFd_);
        
        // 100ms 타임아웃
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        
        int activity = select(maxFd + 1, &readfds, NULL, NULL, &tv);
        
        if (activity > 0) {
            if (FD_ISSET(listenFd_, &readfds)) {
                handleNewConnection();  // 로그 클라이언트
            }
            if (FD_ISSET(queryFd_, &readfds)) {
                handleQueryConnection(); // 쿼리 클라이언트
            }
        }
    }
}
```

**왜 하이브리드?**
- Accept: select()로 효율적 대기
- Processing: 스레드 풀로 병렬 처리
- Best of both worlds!

## 📨 클라이언트 처리

### 1. 연결별 처리 로직
```cpp
// [SEQUENCE: 860] 클라이언트 핸들러
void IRCServer::handleClient(int clientFd, const std::string& clientAddr) {
    char buffer[4096];
    std::string incomplete;  // 불완전한 라인 저장
    
    while (running_.load()) {
        ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            
            // 이전 불완전한 데이터와 결합
            std::string data = incomplete + std::string(buffer, bytesRead);
            incomplete.clear();
            
            // 라인 단위로 처리
            size_t pos = 0;
            while ((pos = data.find('\n')) != std::string::npos) {
                std::string line = data.substr(0, pos);
                data.erase(0, pos + 1);
                
                // CRLF 처리
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                
                processLine(clientFd, line);
            }
            
            // 남은 불완전한 데이터 저장
            incomplete = data;
            
        } else if (bytesRead == 0) {
            // 클라이언트 연결 종료
            break;
        } else {
            // 에러 처리
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                break;
            }
            // Non-blocking 모드에서 데이터 없음
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    close(clientFd);
}
```

### 2. 프로토콜 파싱
```cpp
// 로그 서버: 단순 라인 기반
void processLogLine(const std::string& line) {
    if (line.empty()) return;
    
    // [SEQUENCE: 196] LogEntry 생성
    LogEntry entry(line);
    
    // 레벨 추출 (선택적)
    if (line.find("ERROR:") == 0) {
        entry.level = "ERROR";
        entry.message = line.substr(6);
    } else if (line.find("WARNING:") == 0) {
        entry.level = "WARNING";
        entry.message = line.substr(8);
    }
    // ... 기타 레벨
    
    logBuffer_->push(entry);
}

// IRC 서버: 복잡한 프로토콜
void processIRCLine(const std::string& line) {
    // [SEQUENCE: 602] IRC 명령 파싱
    IRCCommand cmd = IRCCommandParser::parse(line);
    commandHandler_->handleCommand(client, cmd);
}
```

## 🛡️ 에러 처리와 복원력

### 1. 연결 관리
```cpp
class ConnectionGuard {
    int fd_;
public:
    explicit ConnectionGuard(int fd) : fd_(fd) {}
    ~ConnectionGuard() {
        if (fd_ >= 0) close(fd_);
    }
    // RAII로 자동 정리
};
```

### 2. 에러 전파
```cpp
enum class SocketError {
    NONE,
    WOULD_BLOCK,    // EAGAIN/EWOULDBLOCK
    CONNECTION_RESET, // 클라이언트 강제 종료
    BROKEN_PIPE,     // 파이프 깨짐
    UNKNOWN
};

SocketError getLastSocketError() {
    switch (errno) {
        case EAGAIN:
        case EWOULDBLOCK:
            return SocketError::WOULD_BLOCK;
        case ECONNRESET:
            return SocketError::CONNECTION_RESET;
        case EPIPE:
            return SocketError::BROKEN_PIPE;
        default:
            return SocketError::UNKNOWN;
    }
}
```

## 📊 성능 최적화

### 1. 버퍼 관리
```cpp
// 작은 버퍼 재사용
class BufferPool {
    std::stack<std::unique_ptr<char[]>> pool_;
    std::mutex mutex_;
    
public:
    std::unique_ptr<char[]> acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!pool_.empty()) {
            auto buffer = std::move(pool_.top());
            pool_.pop();
            return buffer;
        }
        return std::make_unique<char[]>(4096);
    }
    
    void release(std::unique_ptr<char[]> buffer) {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push(std::move(buffer));
    }
};
```

### 2. TCP 튜닝
```cpp
// Nagle 알고리즘 비활성화 (실시간성)
int flag = 1;
setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

// 송수신 버퍼 크기 조정
int bufsize = 64 * 1024; // 64KB
setsockopt(clientFd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
setsockopt(clientFd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));

// Keep-alive 설정
int keepalive = 1;
setsockopt(clientFd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
```

## 🔍 디버깅 팁

### 1. 연결 상태 추적
```cpp
void LogServer::printConnectionStats() {
    std::cout << "Active connections: " << activeConnections_.load() << std::endl;
    std::cout << "Total processed: " << totalProcessed_.load() << std::endl;
    std::cout << "Errors: " << errorCount_.load() << std::endl;
}
```

### 2. 네트워크 도구 활용
```bash
# 포트 확인
netstat -an | grep 9999

# 연결 추적
tcpdump -i lo -n port 9999

# 부하 테스트
ab -n 10000 -c 100 http://localhost:9999/
```

## 💡 베스트 프랙티스

1. **항상 Non-blocking I/O 사용**
   - 하나의 느린 클라이언트가 전체를 막지 않도록

2. **적절한 타임아웃 설정**
   - 좀비 연결 방지
   - 리소스 효율적 사용

3. **에러를 우아하게 처리**
   - 클라이언트 에러가 서버를 죽이지 않도록
   - 로깅하되 계속 실행

4. **백프레셔 구현**
   - 클라이언트가 너무 빨리 보낼 때 대응
   - 메모리 폭발 방지

---

다음: [3_Concurrent_Log_Processing.md](3_Concurrent_Log_Processing.md) - 동시성과 스레드 안전성