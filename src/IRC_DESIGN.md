# LogCaster-IRC 하이브리드 설계서

## 1. 개요

LogCaster에 IRC(Internet Relay Chat) 프로토콜을 통합하여 실시간 로그 모니터링과 채팅 기능을 결합한 하이브리드 시스템을 구현합니다.

## 2. 아키텍처 구조

```
LogCaster-IRC
├── 기존 로그 수집 (9999 포트)
├── 기존 쿼리 인터페이스 (9998 포트)
└── IRC 서버 (6667 포트) - 새로운 기능
    ├── 채널 기반 로그 분류
    ├── 실시간 로그 스트리밍
    └── 대화형 쿼리 인터페이스
```

## 3. 핵심 클래스 설계

### 3.1 IRCServer 클래스
```cpp
class IRCServer {
public:
    IRCServer(int port = 6667, std::shared_ptr<LogBuffer> logBuffer = nullptr);
    ~IRCServer();
    
    void start();
    void stop();
    void setLogBuffer(std::shared_ptr<LogBuffer> buffer);
    
private:
    int port_;
    int listenFd_;
    std::atomic<bool> running_;
    std::shared_ptr<LogBuffer> logBuffer_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::unique_ptr<IRCChannelManager> channelManager_;
    std::unique_ptr<IRCClientManager> clientManager_;
    std::unique_ptr<IRCCommandParser> commandParser_;
};
```

### 3.2 IRCClient 클래스
```cpp
class IRCClient {
public:
    IRCClient(int fd, const std::string& address);
    
    void setNickname(const std::string& nick);
    void setUsername(const std::string& user);
    void joinChannel(const std::string& channel);
    void partChannel(const std::string& channel);
    void sendMessage(const std::string& message);
    
    const std::string& getNickname() const;
    const std::set<std::string>& getChannels() const;
    bool isAuthenticated() const;
    
private:
    int fd_;
    std::string address_;
    std::string nickname_;
    std::string username_;
    std::string realname_;
    std::set<std::string> channels_;
    bool authenticated_;
    std::mutex mutex_;
};
```

### 3.3 IRCChannel 클래스
```cpp
class IRCChannel {
public:
    IRCChannel(const std::string& name);
    
    void addClient(std::shared_ptr<IRCClient> client);
    void removeClient(const std::string& nickname);
    void broadcast(const std::string& message, const std::string& sender = "");
    void setTopic(const std::string& topic);
    
    // 로그 필터 설정
    void setLogFilter(const std::function<bool(const LogEntry&)>& filter);
    void enableLogStreaming(bool enable);
    
private:
    std::string name_;
    std::string topic_;
    std::map<std::string, std::shared_ptr<IRCClient>> clients_;
    std::shared_mutex mutex_;
    
    // 로그 스트리밍 설정
    bool streamingEnabled_;
    std::function<bool(const LogEntry&)> logFilter_;
};
```

### 3.4 IRCCommandParser 클래스
```cpp
class IRCCommandParser {
public:
    struct IRCCommand {
        std::string prefix;
        std::string command;
        std::vector<std::string> params;
    };
    
    static IRCCommand parse(const std::string& line);
    static std::string formatReply(int code, const std::string& params);
    
    // IRC 숫자 응답 코드
    enum ReplyCode {
        RPL_WELCOME = 001,
        RPL_YOURHOST = 002,
        RPL_CREATED = 003,
        RPL_MYINFO = 004,
        RPL_NAMREPLY = 353,
        RPL_ENDOFNAMES = 366,
        ERR_NOSUCHNICK = 401,
        ERR_NOSUCHCHANNEL = 403,
        ERR_CANNOTSENDTOCHAN = 404,
        ERR_NICKNAMEINUSE = 433
    };
};
```

### 3.5 IRCCommandHandler 클래스
```cpp
class IRCCommandHandler {
public:
    IRCCommandHandler(IRCServer* server);
    
    void handleCommand(std::shared_ptr<IRCClient> client, 
                      const IRCCommandParser::IRCCommand& cmd);
    
private:
    // 기본 IRC 명령어 핸들러
    void handleNICK(std::shared_ptr<IRCClient> client, const std::vector<std::string>& params);
    void handleUSER(std::shared_ptr<IRCClient> client, const std::vector<std::string>& params);
    void handleJOIN(std::shared_ptr<IRCClient> client, const std::vector<std::string>& params);
    void handlePART(std::shared_ptr<IRCClient> client, const std::vector<std::string>& params);
    void handlePRIVMSG(std::shared_ptr<IRCClient> client, const std::vector<std::string>& params);
    void handleQUIT(std::shared_ptr<IRCClient> client, const std::vector<std::string>& params);
    
    // LogCaster 전용 명령어
    void handleLOGQUERY(std::shared_ptr<IRCClient> client, const std::vector<std::string>& params);
    void handleLOGFILTER(std::shared_ptr<IRCClient> client, const std::vector<std::string>& params);
    void handleLOGSTREAM(std::shared_ptr<IRCClient> client, const std::vector<std::string>& params);
    
    IRCServer* server_;
    std::map<std::string, std::function<void(std::shared_ptr<IRCClient>, const std::vector<std::string>&)>> handlers_;
};
```

## 4. 로그-IRC 통합 기능

### 4.1 자동 채널 생성
```cpp
// 로그 레벨별 기본 채널
#logs-all     // 모든 로그
#logs-error   // ERROR 레벨 로그
#logs-warning // WARNING 레벨 로그
#logs-info    // INFO 레벨 로그
#logs-debug   // DEBUG 레벨 로그

// 키워드 기반 채널
#logs-database  // "database" 키워드 포함 로그
#logs-network   // "network" 키워드 포함 로그
#logs-security  // "security" 키워드 포함 로그
```

### 4.2 LogBuffer 확장
```cpp
class LogBuffer {
public:
    // 기존 메서드...
    
    // IRC 통합을 위한 새 메서드
    void addLogWithNotification(const LogEntry& entry);
    void registerChannelCallback(const std::string& pattern, 
                                std::function<void(const LogEntry&)> callback);
    void unregisterChannelCallback(const std::string& pattern);
    
private:
    // 채널별 콜백 관리
    std::map<std::string, std::vector<std::function<void(const LogEntry&)>>> channelCallbacks_;
};
```

### 4.3 로그 엔트리 확장
```cpp
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    std::string message;
    
    // IRC 통합을 위한 추가 필드
    std::string level;      // ERROR, WARNING, INFO, DEBUG
    std::string source;     // 로그 소스 (IP, 서비스명 등)
    std::string category;   // 로그 카테고리
    std::map<std::string, std::string> metadata; // 추가 메타데이터
};
```

## 5. IRC 프로토콜 구현

### 5.1 연결 핸드셰이크
```
Client -> Server: NICK logviewer
Client -> Server: USER logviewer 0 * :Log Viewer Client
Server -> Client: :server 001 logviewer :Welcome to LogCaster-IRC
Server -> Client: :server 002 logviewer :Your host is logcaster-irc
Server -> Client: :server 003 logviewer :This server was created...
Server -> Client: :server 004 logviewer logcaster-irc 1.0 ...
```

### 5.2 채널 참여 및 로그 스트리밍
```
Client -> Server: JOIN #logs-error
Server -> Client: :logviewer!user@host JOIN :#logs-error
Server -> Client: :server 353 logviewer = #logs-error :@logviewer
Server -> Client: :server 366 logviewer #logs-error :End of /NAMES list

// 로그 스트리밍 시작
Server -> Client: :LogBot!log@system PRIVMSG #logs-error :[2024-01-01 12:00:00] ERROR: Database connection failed
Server -> Client: :LogBot!log@system PRIVMSG #logs-error :[2024-01-01 12:00:01] ERROR: Retry attempt 1 of 3
```

### 5.3 대화형 쿼리
```
Client -> Server: PRIVMSG #logs-all :!query COUNT level:ERROR
Server -> Client: :LogBot!log@system PRIVMSG #logs-all :Query result: 42 ERROR logs in buffer

Client -> Server: PRIVMSG #logs-all :!filter add database
Server -> Client: :LogBot!log@system PRIVMSG #logs-all :Filter added: showing only logs containing "database"
```

## 6. 스레드 안전성 고려사항

### 6.1 동기화 전략
- IRCClient: 개별 뮤텍스로 클라이언트 상태 보호
- IRCChannel: shared_mutex로 읽기 성능 최적화
- LogBuffer: 기존 동기화 메커니즘 재사용
- 채널 브로드캐스트: 비동기 큐 사용

### 6.2 데드락 방지
- 락 순서: LogBuffer -> Channel -> Client
- 콜백 실행 시 락 해제 후 호출
- 타임아웃 있는 락 사용

## 7. 성능 최적화

### 7.1 메시지 배칭
```cpp
class MessageBatcher {
    void addMessage(int clientFd, const std::string& message);
    void flush(); // 주기적으로 또는 버퍼 가득 시 호출
};
```

### 7.2 채널별 스레드풀
```cpp
// 채널별 전용 워커 스레드로 브로드캐스트 처리
channelThreadPools_["#logs-error"] = std::make_unique<ThreadPool>(2);
channelThreadPools_["#logs-all"] = std::make_unique<ThreadPool>(4);
```

## 8. 구현 순서

1. **MVP1**: 기본 IRC 서버 (NICK, USER, JOIN, PART, PRIVMSG)
2. **MVP2**: 로그 스트리밍 (#logs-* 채널 자동 생성)
3. **MVP3**: 대화형 쿼리 (!query, !filter 명령어)
4. **MVP4**: 고급 기능 (정규식 필터, 실시간 알림, 통계)

## 9. 테스트 계획

### 9.1 단위 테스트
- IRCCommandParser 파싱 테스트
- 채널 관리 테스트
- 메시지 라우팅 테스트

### 9.2 통합 테스트
- 다중 클라이언트 동시 접속
- 로그 스트리밍 성능 테스트
- IRC 클라이언트 호환성 (irssi, weechat, HexChat)

### 9.3 부하 테스트
- 1000개 동시 연결
- 초당 10,000 로그 메시지 처리
- 100개 채널 동시 운영

## 10. 보안 고려사항

- 닉네임/채널명 검증 (특수문자 제한)
- 메시지 크기 제한 (512바이트)
- Rate limiting (초당 메시지 수 제한)
- 인증 옵션 (SASL, 패스워드)

## 11. 확장 가능성

- SSL/TLS 지원
- 웹소켓 브리지 (웹 기반 IRC 클라이언트)
- 로그 필터 DSL (Domain Specific Language)
- 플러그인 시스템 (커스텀 명령어)
- 분산 IRC 네트워크 지원