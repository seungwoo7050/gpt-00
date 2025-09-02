# LogCaster IRC 프로토콜 통합 가이드

## 🎯 개요

LogCaster의 혁신적인 IRC 통합으로 실시간 로그 모니터링을 구현합니다. RFC 2812 준수와 커스텀 확장을 다룹니다.

## 🌐 왜 IRC인가?

### 장점
1. **범용 클라이언트**: irssi, HexChat, mIRC 등 기존 도구 활용
2. **실시간 스트리밍**: 푸시 기반 알림
3. **대화형 인터페이스**: 채팅으로 쿼리 실행
4. **검증된 프로토콜**: 30년 이상의 안정성

### LogCaster만의 혁신
```
전통적 모니터링: Pull (주기적 확인)
LogCaster IRC: Push (실시간 알림)
```

## 📋 IRC 프로토콜 핵심

### 1. 연결 핸드셰이크
```cpp
// [SEQUENCE: 789] NICK 명령 처리
void IRCCommandHandler::handleNICK(client, cmd) {
    std::string newNick = cmd.getParam(0);
    
    // 닉네임 유효성 검사
    if (!IRCCommandParser::isValidNickname(newNick)) {
        client->sendErrorReply(432, newNick + " :Erroneous nickname");
        return;
    }
    
    // 중복 확인
    if (!isNicknameAvailable(newNick)) {
        client->sendErrorReply(433, newNick + " :Nickname is already in use");
        return;
    }
    
    client->setNickname(newNick);
    checkAuthentication(client);
}

// [SEQUENCE: 795] USER 명령 처리
void IRCCommandHandler::handleUSER(client, cmd) {
    if (cmd.params.size() < 4) {
        client->sendErrorReply(461, "USER :Not enough parameters");
        return;
    }
    
    client->setUsername(cmd.getParam(0));
    client->setRealname(cmd.trailing);
    checkAuthentication(client);
}

// [SEQUENCE: 817] 환영 메시지
void sendWelcome(client) {
    client->sendReply(001, ":Welcome to LogCaster IRC");
    client->sendReply(002, ":Your host is logcaster-irc");
    client->sendReply(003, ":This server was created " + serverCreated_);
    client->sendReply(004, "logcaster-irc 1.0 o o");
    
    // MOTD with LogCaster info
    sendMOTD(client);
}
```

### 2. 메시지 형식
```
// IRC 메시지 구조
:<prefix> <command> <params> :<trailing>

// 예시
:nick!user@host PRIVMSG #channel :Hello world
:server 353 nick = #channel :@op +voice nick1 nick2
```

## 🔧 로그 채널 시스템

### 1. 자동 채널 생성
```cpp
// [SEQUENCE: 724] 기본 로그 채널 초기화
void IRCChannelManager::initializeLogChannels() {
    const std::vector<LogChannelConfig> defaults = {
        {"#logs-all", "*", "All log messages"},
        {"#logs-error", "ERROR", "Error level logs only"},
        {"#logs-warning", "WARNING", "Warning level logs only"},
        {"#logs-info", "INFO", "Info level logs only"},
        {"#logs-debug", "DEBUG", "Debug level logs only"}
    };
    
    for (const auto& config : defaults) {
        auto channel = createChannel(config.name, IRCChannel::Type::LOG_STREAM);
        channel->setTopic(config.description, "LogCaster");
        channel->enableLogStreaming(true);
        
        // 레벨 필터 설정
        if (config.level != "*") {
            channel->setLogFilter(IRCChannel::createLevelFilter(config.level));
        }
    }
}
```

### 2. 로그 배포 메커니즘
```cpp
// [SEQUENCE: 697] LogBuffer에서 IRC로 알림
void LogBuffer::addLogWithNotification(const LogEntry& entry) {
    // 1. 버퍼에 추가
    {
        std::unique_lock<std::mutex> lock(mutex_);
        buffer_.push_back(entry);
    }
    
    // 2. IRC 채널에 알림
    for (const auto& [pattern, callbacks] : channelCallbacks_) {
        if (matchesPattern(pattern, entry)) {
            for (const auto& callback : callbacks) {
                callback(entry);  // 비동기 실행
            }
        }
    }
}

// [SEQUENCE: 675] 채널에서 로그 처리
void IRCChannel::processLogEntry(const LogEntry& entry) {
    if (!streamingEnabled_ || !logFilter_(entry)) {
        return;
    }
    
    // IRC 형식으로 변환
    std::string message = formatLogEntry(entry);
    std::string ircMsg = ":LogBot!log@system PRIVMSG " + name_ + " :" + message;
    
    // 모든 클라이언트에게 브로드캐스트
    broadcast(ircMsg);
}
```

## 💬 커스텀 명령어

### 1. 쿼리 명령어
```cpp
// [SEQUENCE: 803] PRIVMSG에서 명령어 감지
void handlePRIVMSG(client, cmd) {
    std::string target = cmd.getParam(0);
    std::string message = cmd.trailing;
    
    // LogCaster 명령어 확인
    if (target[0] == '#' && message[0] == '!') {
        processLogQuery(client, target, message);
        return;
    }
    
    // 일반 메시지 처리...
}

// [SEQUENCE: 826] 쿼리 처리
void processLogQuery(client, channel, query) {
    std::istringstream iss(query);
    std::string command;
    iss >> command;
    
    if (command == "!query") {
        std::string queryType;
        iss >> queryType;
        
        if (queryType == "COUNT") {
            std::string filter;
            std::getline(iss, filter);
            
            // LogBuffer에서 카운트 실행
            auto result = executeCount(filter);
            
            // 결과를 채널에 전송
            std::string response = "Query result: " + std::to_string(result) + " logs found";
            sendToChannel(channel, "LogBot", response);
        }
        // 다른 쿼리 타입들...
    }
}
```

### 2. 필터 관리
```cpp
// 채널별 동적 필터
class ChannelFilter {
    std::vector<std::function<bool(const LogEntry&)>> filters_;
    
public:
    void addFilter(const std::string& type, const std::string& value) {
        if (type == "level") {
            filters_.push_back([value](const LogEntry& e) {
                return e.level == value;
            });
        } else if (type == "keyword") {
            filters_.push_back([value](const LogEntry& e) {
                return e.message.find(value) != std::string::npos;
            });
        } else if (type == "regex") {
            std::regex pattern(value);
            filters_.push_back([pattern](const LogEntry& e) {
                return std::regex_search(e.message, pattern);
            });
        }
    }
    
    bool matches(const LogEntry& entry) const {
        return std::all_of(filters_.begin(), filters_.end(),
            [&entry](const auto& filter) { return filter(entry); });
    }
};
```

## 📊 성능 고려사항

### 1. 메시지 레이트 제한
```cpp
// [SEQUENCE: 765] 클라이언트별 제한
class RateLimiter {
    struct ClientRate {
        std::chrono::steady_clock::time_point lastReset;
        int messageCount;
    };
    
    std::unordered_map<std::string, ClientRate> rates_;
    const int maxMessagesPerSecond = 10;
    
public:
    bool allowMessage(const std::string& clientId) {
        auto now = std::chrono::steady_clock::now();
        auto& rate = rates_[clientId];
        
        // 1초 경과시 리셋
        if (now - rate.lastReset > std::chrono::seconds(1)) {
            rate.lastReset = now;
            rate.messageCount = 0;
        }
        
        if (rate.messageCount >= maxMessagesPerSecond) {
            return false;  // 제한 초과
        }
        
        rate.messageCount++;
        return true;
    }
};
```

### 2. 효율적인 브로드캐스트
```cpp
// [SEQUENCE: 255] 배치 브로드캐스트
class BatchBroadcaster {
    struct PendingMessage {
        std::string channel;
        std::string message;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    std::vector<PendingMessage> pending_;
    std::mutex mutex_;
    
public:
    void flush() {
        std::vector<PendingMessage> batch;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            batch = std::move(pending_);
        }
        
        // 채널별로 그룹화
        std::unordered_map<std::string, std::vector<std::string>> grouped;
        for (const auto& msg : batch) {
            grouped[msg.channel].push_back(msg.message);
        }
        
        // 채널별 일괄 전송
        for (const auto& [channel, messages] : grouped) {
            sendBatchToChannel(channel, messages);
        }
    }
};
```

## 🔒 보안 고려사항

### 1. 입력 검증
```cpp
// [SEQUENCE: 615] 닉네임 검증
bool isValidNickname(const std::string& nick) {
    if (nick.empty() || nick.length() > 9) return false;
    
    // 첫 문자는 영문/특수문자
    if (!std::isalpha(nick[0]) && !isSpecialChar(nick[0])) {
        return false;
    }
    
    // 나머지는 영숫자/특수문자/하이픈
    return std::all_of(nick.begin() + 1, nick.end(), [](char c) {
        return std::isalnum(c) || isSpecialChar(c) || c == '-';
    });
}
```

### 2. 플러딩 방지
```cpp
// 연결별 제한
if (!clientManager_->canAcceptConnection(address)) {
    send(fd, "ERROR :Too many connections from your host\r\n");
    close(fd);
    return;
}
```

## 💡 구현 인사이트

1. **프로토콜 준수가 호환성**
   - RFC 2812 정확히 구현
   - 기존 클라이언트와 완벽 호환

2. **채널이 곧 필터**
   - 채널 = 로그 스트림
   - JOIN = 구독 시작

3. **비동기가 핵심**
   - 로그 수집이 IRC를 막지 않음
   - IRC가 로그 수집을 막지 않음

---

다음: [5_Performance_and_Optimization.md](5_Performance_and_Optimization.md) - 성능 최적화