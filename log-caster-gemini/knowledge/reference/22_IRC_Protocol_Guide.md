# 18. IRC 프로토콜 가이드 💬
## 채팅 서버 구현을 위한 IRC 완전 분석

---

## 📋 문서 정보

- **난이도**: 중급-고급 (Intermediate-Advanced)
- **예상 학습 시간**: 15-20시간
- **필요 선수 지식**: 
  - TCP/IP 네트워킹
  - 소켓 프로그래밍
  - 멀티스레딩
  - 문자열 파싱
- **실습 환경**: IRC 클라이언트 (HexChat, irssi), telnet

---

## Overview

**"채팅 프로그램은 어떻게 만들어질까요?"** 🤔

좋은 질문이에요! IRC (Internet Relay Chat)는 실시간 채팅의 **할아버지** 같은 존재입니다. 1988년부터 사용되어 온 이 프로토콜은 마치 **라디오 방송국**처럼 작동해요 - 여러 채널이 있고, 사람들이 자유롭게 들어와서 대화하죠!

LogCaster와 IRC를 결합하면? 로그 데이터가 실시간으로 채팅처럼 흘러나오는 멋진 시스템을 만들 수 있어요! 마치 **로그들이 대화하는 것** 같은 경험을 제공할 수 있습니다. 🚀

이 가이드는 RFC 2812 기반의 IRC 프로토콜 구현에 필요한 핵심 지식을 다룹니다.

## 1. IRC 프로토콜 기초

### 1.1 메시지 형식

**"IRC 메시지는 어떻게 생겼을까요?"** 🧐

IRC 메시지는 마치 **편지** 같아요! 편지에 발신자, 제목, 내용이 있듯이 IRC도 비슷한 구조를 가져요.
```
:<prefix> <command> <params> :<trailing>
```

```cpp
struct IRCMessage {
    std::string prefix;     // 선택적: 발신자 정보
    std::string command;    // 필수: 명령어 (PRIVMSG, JOIN 등)
    std::vector<std::string> params;  // 매개변수들
    std::string trailing;   // 마지막 매개변수 (공백 포함 가능)
};

// 파싱 예제
IRCMessage parseMessage(const std::string& raw) {
    IRCMessage msg;
    std::istringstream stream(raw);
    std::string token;
    
    // Prefix 파싱
    if (raw[0] == ':') {
        std::getline(stream, msg.prefix, ' ');
        msg.prefix = msg.prefix.substr(1);  // ':' 제거
    }
    
    // Command 파싱
    stream >> msg.command;
    
    // Parameters 파싱
    while (stream >> token) {
        if (token[0] == ':') {
            // Trailing 파싱
            std::getline(stream, msg.trailing);
            msg.trailing = token.substr(1) + " " + msg.trailing;
            break;
        }
        msg.params.push_back(token);
    }
    
    return msg;
}
```

### 1.2 핵심 명령어

**"IRC에서는 어떤 명령어들을 사용할까요?"** 💬

IRC 명령어는 마치 **마법 주문** 같아요! 각각 특별한 기능이 있어서 채팅방을 조종할 수 있습니다. 가장 기본적인 것들부터 차근차근 알아보죠!

#### 연결 설정 - 첫 만남의 인사 👋
```cpp
// NICK - 닉네임 설정
void handleNICK(IRCClient& client, const std::vector<std::string>& params) {
    if (params.empty()) {
        client.sendReply(ERR_NONICKNAMEGIVEN, ":No nickname given");
        return;
    }
    
    const std::string& nickname = params[0];
    
    // 닉네임 유효성 검사
    if (!isValidNickname(nickname)) {
        client.sendReply(ERR_ERRONEUSNICKNAME, nickname + " :Erroneous nickname");
        return;
    }
    
    // 중복 검사
    if (isNicknameTaken(nickname)) {
        client.sendReply(ERR_NICKNAMEINUSE, nickname + " :Nickname is already in use");
        return;
    }
    
    client.setNickname(nickname);
}

// USER - 사용자 정보 설정
void handleUSER(IRCClient& client, const std::vector<std::string>& params) {
    if (params.size() < 4) {
        client.sendReply(ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
        return;
    }
    
    client.setUsername(params[0]);
    client.setRealname(params[3]);
    
    // 환영 메시지 전송
    if (client.isRegistered()) {
        sendWelcome(client);
    }
}
```

#### 채널 관리 - 채팅방 출입 🚪
```cpp
// JOIN - 채널 참여
void handleJOIN(IRCClient& client, const std::vector<std::string>& params) {
    if (params.empty()) {
        client.sendReply(ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
        return;
    }
    
    std::vector<std::string> channels = split(params[0], ',');
    
    for (const auto& channelName : channels) {
        if (!isValidChannelName(channelName)) {
            client.sendReply(ERR_NOSUCHCHANNEL, channelName + " :No such channel");
            continue;
        }
        
        auto channel = getOrCreateChannel(channelName);
        channel->addClient(client);
        
        // JOIN 알림 브로드캐스트
        channel->broadcast(":" + client.getPrefix() + " JOIN " + channelName);
        
        // 토픽 전송
        if (!channel->getTopic().empty()) {
            client.sendReply(RPL_TOPIC, channelName + " :" + channel->getTopic());
        }
        
        // 사용자 목록 전송
        sendNames(client, channel);
    }
}

// PART - 채널 떠나기
void handlePART(IRCClient& client, const std::vector<std::string>& params) {
    if (params.empty()) {
        client.sendReply(ERR_NEEDMOREPARAMS, "PART :Not enough parameters");
        return;
    }
    
    const std::string& channelName = params[0];
    const std::string& message = params.size() > 1 ? params[1] : client.getNickname();
    
    auto channel = findChannel(channelName);
    if (!channel || !channel->hasClient(client)) {
        client.sendReply(ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
        return;
    }
    
    // PART 알림 브로드캐스트
    channel->broadcast(":" + client.getPrefix() + " PART " + channelName + " :" + message);
    channel->removeClient(client);
}
```

#### 메시지 전송 - 진짜 대화의 시작! 💬
```cpp
// PRIVMSG - 메시지 전송
void handlePRIVMSG(IRCClient& client, const std::vector<std::string>& params) {
    if (params.size() < 2) {
        client.sendReply(ERR_NORECIPIENT, ":No recipient given (PRIVMSG)");
        return;
    }
    
    const std::string& target = params[0];
    const std::string& message = params[1];
    
    // LogCaster 명령어 처리
    if (message[0] == '!') {
        handleLogCasterCommand(client, target, message);
        return;
    }
    
    if (target[0] == '#' || target[0] == '&') {
        // 채널 메시지
        auto channel = findChannel(target);
        if (!channel) {
            client.sendReply(ERR_NOSUCHCHANNEL, target + " :No such channel");
            return;
        }
        
        if (!channel->hasClient(client)) {
            client.sendReply(ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
            return;
        }
        
        channel->broadcast(":" + client.getPrefix() + " PRIVMSG " + target + " :" + message,
                          client.getNickname());
    } else {
        // 개인 메시지
        auto targetClient = findClient(target);
        if (!targetClient) {
            client.sendReply(ERR_NOSUCHNICK, target + " :No such nick/channel");
            return;
        }
        
        targetClient->sendMessage(":" + client.getPrefix() + " PRIVMSG " + target + " :" + message);
    }
}
```

## 2. LogCaster 통합

**"로그 데이터를 채팅처럼 볼 수 있다고요?"** 😲

맞아요! 상상해보세요 - 에러 로그들이 `#error-room` 채널로 실시간으로 들어오고, 경고 메시지들은 `#warning-room`으로 흘러들어와요. 마치 로그들이 각자의 **전용 채팅방**을 가진 것처럼!

### 2.1 로그 스트리밍 채널 - 로그들의 전용 채팅방 🏠
```cpp
class LogStreamChannel : public IRCChannel {
private:
    std::function<bool(const LogEntry&)> filter_;
    std::atomic<bool> streamingEnabled_;
    
public:
    LogStreamChannel(const std::string& name) 
        : IRCChannel(name), streamingEnabled_(true) {
        
        // 채널별 필터 설정
        if (name == "#logs-error") {
            filter_ = [](const LogEntry& entry) {
                return entry.level == "ERROR";
            };
        } else if (name == "#logs-warning") {
            filter_ = [](const LogEntry& entry) {
                return entry.level == "WARNING" || entry.level == "ERROR";
            };
        }
    }
    
    void streamLog(const LogEntry& entry) {
        if (!streamingEnabled_ || !filter_(entry)) {
            return;
        }
        
        // 로그를 IRC 메시지로 변환
        std::string timestamp = formatTimestamp(entry.timestamp);
        std::string message = fmt::format("[{}] {}: {}", 
            timestamp, entry.level, entry.message);
        
        // 모든 채널 멤버에게 브로드캐스트
        broadcast(":LogBot!log@system PRIVMSG " + name_ + " :" + message);
    }
};
```

### 2.2 LogCaster 명령어 - 로그와의 대화 🗣️

**"로그에게 질문할 수 있다고요?"** 

네! 마치 **로그 전용 비서**를 두는 것 같아요. `!query COUNT` 하면 몇 개의 로그가 있는지 알려주고, `!search ERROR` 하면 에러 로그만 찾아줘요!
```cpp
void handleLogCasterCommand(IRCClient& client, const std::string& channel, 
                            const std::string& command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;  // "!query", "!filter" 등
    
    if (cmd == "!query") {
        std::string queryType;
        iss >> queryType;
        
        if (queryType == "COUNT") {
            handleCountQuery(client, channel, iss);
        } else if (queryType == "SEARCH") {
            handleSearchQuery(client, channel, iss);
        } else if (queryType == "FILTER") {
            handleFilterQuery(client, channel, iss);
        } else {
            sendToChannel(channel, ":LogBot PRIVMSG " + channel + 
                         " :Unknown query type: " + queryType);
        }
    } else if (cmd == "!filter") {
        handleFilterCommand(client, channel, iss);
    } else if (cmd == "!stream") {
        handleStreamCommand(client, channel, iss);
    } else if (cmd == "!help") {
        sendHelpMessage(client, channel);
    }
}

// 쿼리 결과 전송
void sendQueryResult(const std::string& channel, const QueryResult& result) {
    if (result.lines.empty()) {
        sendToChannel(channel, ":LogBot PRIVMSG " + channel + " :No results found");
        return;
    }
    
    // 결과를 여러 메시지로 나누어 전송 (IRC 메시지 크기 제한)
    for (const auto& line : result.lines) {
        if (line.length() > 400) {
            // 긴 메시지는 잘라서 전송
            sendToChannel(channel, ":LogBot PRIVMSG " + channel + " :" + 
                         line.substr(0, 400) + "...");
        } else {
            sendToChannel(channel, ":LogBot PRIVMSG " + channel + " :" + line);
        }
    }
}
```

## 3. 상태 관리

### 3.1 클라이언트 상태
```cpp
enum class ClientState {
    UNREGISTERED,   // 연결됨, 인증 전
    REGISTERED,     // NICK과 USER 완료
    ACTIVE,         // 정상 활동 중
    DISCONNECTING   // 연결 종료 중
};

class IRCClientStateMachine {
private:
    ClientState state_;
    std::string pendingNick_;
    std::string pendingUser_;
    
public:
    void handleNickCommand(const std::string& nick) {
        switch (state_) {
        case ClientState::UNREGISTERED:
            pendingNick_ = nick;
            checkRegistration();
            break;
            
        case ClientState::REGISTERED:
        case ClientState::ACTIVE:
            // 닉네임 변경
            changeNickname(nick);
            break;
            
        case ClientState::DISCONNECTING:
            // 무시
            break;
        }
    }
    
    void checkRegistration() {
        if (!pendingNick_.empty() && !pendingUser_.empty()) {
            state_ = ClientState::REGISTERED;
            sendWelcomeMessages();
        }
    }
};
```

### 3.2 채널 상태
```cpp
class ChannelStateMachine {
public:
    enum Mode {
        MODERATED = 1 << 0,    // +m: 음소거된 사용자는 말할 수 없음
        INVITE_ONLY = 1 << 1,  // +i: 초대만 가능
        TOPIC_LOCK = 1 << 2,   // +t: 오퍼레이터만 토픽 변경
        NO_EXTERNAL = 1 << 3   // +n: 채널 멤버만 메시지 가능
    };
    
private:
    uint32_t modes_;
    std::set<std::string> operators_;
    std::set<std::string> voiced_;
    std::set<std::string> banned_;
    
public:
    bool canSpeak(const IRCClient& client) const {
        if (isOperator(client.getNickname())) {
            return true;
        }
        
        if (modes_ & MODERATED) {
            return isVoiced(client.getNickname());
        }
        
        return !isBanned(client.getNickname());
    }
    
    bool canJoin(const IRCClient& client) const {
        if (isBanned(client.getNickname())) {
            return false;
        }
        
        if (modes_ & INVITE_ONLY) {
            return isInvited(client.getNickname());
        }
        
        return true;
    }
};
```

## 4. 성능 최적화

### 4.1 메시지 배칭
```cpp
class MessageBatcher {
private:
    struct BatchedMessage {
        std::vector<std::string> messages;
        std::chrono::steady_clock::time_point lastUpdate;
    };
    
    std::unordered_map<int, BatchedMessage> clientBuffers_;
    std::mutex mutex_;
    static constexpr size_t BATCH_SIZE = 10;
    static constexpr auto BATCH_TIMEOUT = std::chrono::milliseconds(100);
    
public:
    void addMessage(int clientFd, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto& batch = clientBuffers_[clientFd];
        batch.messages.push_back(message);
        batch.lastUpdate = std::chrono::steady_clock::now();
        
        if (batch.messages.size() >= BATCH_SIZE) {
            flushClient(clientFd);
        }
    }
    
    void flushClient(int clientFd) {
        auto it = clientBuffers_.find(clientFd);
        if (it == clientBuffers_.end() || it->second.messages.empty()) {
            return;
        }
        
        // 모든 메시지를 하나로 결합
        std::string combined;
        for (const auto& msg : it->second.messages) {
            combined += msg + "\r\n";
        }
        
        // 한 번에 전송
        send(clientFd, combined.c_str(), combined.length(), MSG_NOSIGNAL);
        it->second.messages.clear();
    }
    
    void periodicFlush() {
        auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (auto& [fd, batch] : clientBuffers_) {
            if (now - batch.lastUpdate > BATCH_TIMEOUT && !batch.messages.empty()) {
                flushClient(fd);
            }
        }
    }
};
```

### 4.2 채널 브로드캐스트 최적화
```cpp
class OptimizedChannel {
private:
    // 채널별 전용 스레드 풀
    std::unique_ptr<ThreadPool> broadcastPool_;
    
    // 클라이언트를 그룹으로 나누어 병렬 전송
    void broadcastParallel(const std::string& message, const std::string& exclude) {
        std::vector<std::shared_ptr<IRCClient>> clients;
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            for (const auto& [nick, client] : members_) {
                if (nick != exclude) {
                    clients.push_back(client);
                }
            }
        }
        
        // 클라이언트를 청크로 나누기
        const size_t chunkSize = std::max(size_t(1), clients.size() / 4);
        
        for (size_t i = 0; i < clients.size(); i += chunkSize) {
            auto end = std::min(i + chunkSize, clients.size());
            
            broadcastPool_->enqueue([message, clients, i, end]() {
                for (size_t j = i; j < end; ++j) {
                    clients[j]->sendMessage(message);
                }
            });
        }
    }
};
```

## 5. 보안 고려사항

### 5.1 입력 검증
```cpp
bool isValidNickname(const std::string& nick) {
    if (nick.empty() || nick.length() > 30) {
        return false;
    }
    
    // 첫 글자는 문자여야 함
    if (!std::isalpha(nick[0])) {
        return false;
    }
    
    // 허용된 문자만 포함
    return std::all_of(nick.begin(), nick.end(), [](char c) {
        return std::isalnum(c) || c == '-' || c == '_' || c == '[' || 
               c == ']' || c == '{' || c == '}' || c == '\\' || c == '|';
    });
}

bool isValidChannelName(const std::string& channel) {
    if (channel.length() < 2 || channel.length() > 50) {
        return false;
    }
    
    // 채널은 # 또는 &로 시작
    if (channel[0] != '#' && channel[0] != '&') {
        return false;
    }
    
    // 공백, 컨트롤 문자, 쉼표 금지
    return std::none_of(channel.begin() + 1, channel.end(), [](char c) {
        return std::iscntrl(c) || c == ' ' || c == ',' || c == '\x07';
    });
}
```

### 5.2 Rate Limiting
```cpp
class RateLimiter {
private:
    struct ClientRate {
        std::deque<std::chrono::steady_clock::time_point> timestamps;
        std::chrono::steady_clock::time_point lastWarning;
    };
    
    std::unordered_map<std::string, ClientRate> clientRates_;
    std::mutex mutex_;
    
    static constexpr size_t MAX_MESSAGES_PER_SECOND = 5;
    static constexpr size_t MAX_MESSAGES_PER_MINUTE = 100;
    
public:
    bool allowMessage(const std::string& clientId) {
        auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto& rate = clientRates_[clientId];
        
        // 오래된 타임스탬프 제거
        while (!rate.timestamps.empty() && 
               now - rate.timestamps.front() > std::chrono::minutes(1)) {
            rate.timestamps.pop_front();
        }
        
        // 분당 제한 확인
        if (rate.timestamps.size() >= MAX_MESSAGES_PER_MINUTE) {
            sendWarning(clientId, "Rate limit exceeded: too many messages per minute");
            return false;
        }
        
        // 초당 제한 확인
        auto oneSecondAgo = now - std::chrono::seconds(1);
        auto recentCount = std::count_if(rate.timestamps.begin(), rate.timestamps.end(),
            [oneSecondAgo](const auto& ts) { return ts > oneSecondAgo; });
        
        if (recentCount >= MAX_MESSAGES_PER_SECOND) {
            if (now - rate.lastWarning > std::chrono::seconds(10)) {
                sendWarning(clientId, "Slow down! Too many messages");
                rate.lastWarning = now;
            }
            return false;
        }
        
        rate.timestamps.push_back(now);
        return true;
    }
};
```

## 6. 에러 처리

### 6.1 연결 에러
```cpp
void handleConnectionError(IRCClient& client, const std::error_code& ec) {
    switch (ec.value()) {
    case ECONNRESET:
        logInfo("Client {} connection reset", client.getAddress());
        break;
        
    case ETIMEDOUT:
        logWarning("Client {} connection timeout", client.getAddress());
        client.sendError("Connection timeout");
        break;
        
    case EPIPE:
        logDebug("Broken pipe for client {}", client.getAddress());
        break;
        
    default:
        logError("Unknown error for client {}: {}", client.getAddress(), ec.message());
    }
    
    // 클라이언트 정리
    removeClient(client);
}
```

### 6.2 프로토콜 에러
```cpp
class ProtocolErrorHandler {
public:
    void handleMalformedMessage(IRCClient& client, const std::string& raw) {
        logWarning("Malformed message from {}: {}", client.getNickname(), raw);
        
        if (++client.errorCount_ > MAX_ERRORS) {
            client.sendError("Too many protocol errors");
            client.disconnect();
        }
    }
    
    void handleUnknownCommand(IRCClient& client, const std::string& command) {
        client.sendReply(ERR_UNKNOWNCOMMAND, command + " :Unknown command");
    }
    
    void handleFlood(IRCClient& client) {
        client.sendError("Excess flood");
        client.disconnect();
    }
};
```

## 🔥 흔한 실수와 해결방법

### 1. 메시지 파싱 오류

#### [SEQUENCE: 120] 잘못된 메시지 형식 처리
```cpp
// ❌ 잘못된 예: 파라미터 검증 없이 파싱
void parseMessage(const std::string& raw) {
    size_t pos = raw.find(' ');
    std::string command = raw.substr(0, pos);  // pos가 npos일 때 크래시
}

// ✅ 올바른 예: 안전한 파싱
void parseMessage(const std::string& raw) {
    if (raw.empty()) return;
    
    size_t pos = raw.find(' ');
    std::string command = (pos != std::string::npos) 
        ? raw.substr(0, pos) 
        : raw;
}
```

### 2. 인코딩 문제

#### [SEQUENCE: 121] UTF-8 처리 누락
```cpp
// ❌ 잘못된 예: 인코딩 검증 없이 전송
void sendMessage(const std::string& msg) {
    send(socket_, msg.c_str(), msg.length(), 0);
}

// ✅ 올바른 예: UTF-8 검증 후 전송
void sendMessage(const std::string& msg) {
    if (!isValidUTF8(msg)) {
        // ASCII로 변환하거나 에러 처리
        std::string safe = sanitizeToASCII(msg);
        send(socket_, safe.c_str(), safe.length(), 0);
    } else {
        send(socket_, msg.c_str(), msg.length(), 0);
    }
}
```

### 3. 채널 동기화 문제

#### [SEQUENCE: 122] 레이스 컨디션
```cpp
// ❌ 잘못된 예: 동기화 없이 채널 멤버 수정
void addToChannel(const std::string& channel, IRCClient* client) {
    channels_[channel].push_back(client);  // 동시 접근 시 문제
}

// ✅ 올바른 예: 뮤텍스로 보호
void addToChannel(const std::string& channel, IRCClient* client) {
    std::lock_guard<std::mutex> lock(channelMutex_);
    channels_[channel].push_back(client);
    
    // 채널 가입 알림
    notifyChannelJoin(channel, client);
}
```

## 마무리

이 가이드는 LogCaster에 IRC 기능을 통합하는 데 필요한 핵심 지식을 제공합니다. IRC 프로토콜의 텍스트 기반 특성을 활용하여 로그 데이터를 실시간으로 스트리밍하고, 채널 기반으로 분류하며, 대화형 쿼리를 지원할 수 있습니다.

다음 단계에서는 Protocol_Design_Patterns.md에서 프로토콜 설계의 일반적인 패턴을 학습하고, Realtime_Broadcasting.md에서 효율적인 메시지 브로드캐스팅 기법을 익히게 됩니다.

## 실습 프로젝트

### 프로젝트 1: IRC 클라이언트 라이브러리 (기초)
**목표**: 기본적인 IRC 클라이언트 기능 구현

**구현 사항**:
- 서버 연결 및 인증
- 채널 가입/탈퇴
- 메시지 송수신
- PING/PONG 처리

### 프로젝트 2: LogCaster IRC 봇 (중급)
**목표**: 로그 쿼리 기능을 갖춘 IRC 봇

**구현 사항**:
- 로그 스트리밍 채널 관리
- 쿼리 명령어 처리
- 결과 포맷팅 및 전송
- Rate limiting

### 프로젝트 3: 분산 IRC 서버 (고급)
**목표**: 멀티 서버 IRC 네트워크 구현

**구현 사항**:
- 서버 간 링크 프로토콜
- 상태 동기화
- 스플릿 복구
- 로드 밸런싱

## 학습 체크리스트

### 기초 레벨 ✅
- [ ] IRC 메시지 형식 이해
- [ ] 기본 명령어 구현 (NICK, USER, JOIN)
- [ ] 채널과 사용자 관리
- [ ] 간단한 메시지 라우팅

### 중급 레벨 📚
- [ ] 완전한 RFC 2812 구현
- [ ] 채널 모드와 권한 관리
- [ ] 효율적인 브로드캐스팅
- [ ] 에러 처리와 복구

### 고급 레벨 🚀
- [ ] 서버 간 통신 프로토콜
- [ ] 네트워크 분할 처리
- [ ] 성능 최적화
- [ ] 보안 기능 구현

### 전문가 레벨 🏆
- [ ] IRC 프로토콜 확장 설계
- [ ] 대규모 네트워크 아키텍처
- [ ] 실시간 모니터링 시스템
- [ ] 커스텀 IRC 서비스 개발

## 추가 학습 자료

### 추천 도서
- "Internet Relay Chat: Server Protocol" - RFC 2813
- "Internet Relay Chat: Client Protocol" - RFC 2812
- "IRC Hacks" - Paul Mutton
- "Network Programming with Rust" - Abhishek Chanda

### 온라인 리소스
- [Modern IRC Documentation](https://modern.ircdocs.horse/)
- [IRCv3 Specifications](https://ircv3.net/)
- [UnrealIRCd Documentation](https://www.unrealircd.org/docs/)
- [InspIRCd Wiki](https://wiki.inspircd.org/)

### 실습 도구
- IRC 테스트 서버 (irc.freenode.net)
- IRC 클라이언트 (HexChat, irssi, WeeChat)
- 프로토콜 분석기 (Wireshark with IRC dissector)
- IRC 봇 프레임워크 (libircclient)

---

*💡 팁: IRC는 간단해 보이지만 edge case가 많습니다. 실제 IRC 서버에 연결해서 다양한 상황을 테스트해보세요.*