# DevHistory 11: LogCaster-CPP MVP6 - IRC Server Integration

## 1. 개요 (Overview)

C++ 구현의 마지막 단계(MVP6)는 `DEVELOPMENT_JOURNAL.md`에는 기록되지 않았지만, 코드베이스에 존재하는 매우 중요한 기능인 **IRC(Internet Relay Chat) 서버 통합**입니다. 이 기능은 LogCaster를 단순한 로그 수집 및 조회 도구에서, 실시간으로 로그를 구독하고 알림을 받을 수 있는 강력한 대화형 모니터링 플랫폼으로 격상시킵니다.

**주요 기능 추가:**
- **내장 IRC 서버:** LogCaster는 이제 자체적으로 IRC 프로토콜을 지원하는 서버를 내장합니다. 사용자는 표준 IRC 클라이언트(예: HexChat, mIRC)를 사용하여 LogCaster에 접속할 수 있습니다.
- **채널 기반 로그 구독:** 사용자는 특정 패턴을 가진 채널에 참여(`JOIN`)함으로써, 해당 패턴과 일치하는 로그만 실시간으로 받아볼 수 있습니다. 예를 들어, `#errors` 채널에 참여하면 `level`이 `ERROR`인 로그만 수신하고, `#services-auth` 채널에 참여하면 `category`가 `auth`인 `services` 로그만 수신하는 식입니다.
- **실시간 알림:** `LogBuffer`에 새로운 로그가 추가될 때, 등록된 채널 패턴과 일치하는지 확인하고, 일치할 경우 해당 채널에 참여한 모든 IRC 클라이언트에게 즉시 로그 메시지를 전송합니다.
- **구조화된 로그 활용:** 이 기능을 위해 기존의 단순 문자열 로그는 `level`, `source`, `category` 등 다양한 메타데이터를 포함하는 구조화된 `LogEntry`로 확장됩니다.

**아키텍처 변경:**
- **IRC 모듈 추가:** `IRCServer`, `IRCClient`, `IRCChannel`, `IRCCommandHandler`, `IRCCommandParser` 등 IRC 프로토콜을 처리하기 위한 다수의 클래스가 새로 추가됩니다.
- **`LogBuffer`와 IRC 연동:** `LogBuffer`에 콜백(callback) 메커니즘이 도입됩니다. `IRCCommandHandler`는 `JOIN` 명령을 처리할 때, 특정 로그 패턴과 해당 알림을 처리할 함수(콜백)를 `LogBuffer`에 등록합니다. `LogBuffer`는 새 로그가 들어올 때마다 등록된 콜백들을 실행하여 IRC 서버에 알림을 보냅니다.
- **`main` 함수 수정:** IRC 서버를 활성화하고 지정된 포트에서 실행하기 위한 커맨드 라인 인자(`-i`, `-I`)와 로직이 추가됩니다.

이 MVP는 LogCaster의 활용성을 극적으로 확장하여, 개발자와 시스템 관리자가 터미널에 직접 접속하지 않고도 익숙한 IRC 클라이언트를 통해 여러 시스템의 로그를 중앙에서 편리하게 모니터링할 수 있게 해줍니다.

## 2. 빌드 시스템 (Build System)

다수의 IRC 관련 소스 파일들이 `CMakeLists.txt`에 추가됩니다.

```cmake
# [SEQUENCE: MVP11-1]
# ... (project, CMAKE_CXX_STANDARD 등은 이전과 동일) ...

# [SEQUENCE: MVP11-2]
# MVP6에 필요한 전체 소스 파일 목록 (IRC 모듈 추가)
set(SOURCES
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/LogBuffer.cpp
    src/QueryHandler.cpp
    src/QueryParser.cpp
    src/Persistence.cpp
    # IRC integration
    src/IRCServer.cpp
    src/IRCClient.cpp
    src/IRCChannel.cpp
    src/IRCChannelManager.cpp
    src/IRCClientManager.cpp
    src/IRCCommandParser.cpp
    src/IRCCommandHandler.cpp
)

# ... (나머지는 MVP8과 동일) 
```

## 3. 전체 소스 코드 (Full Source Code)

### 3.1. 신규 IRC 모듈 파일

*(이 단계에서는 많은 파일이 추가되므로, 주요 클래스의 정의(헤더 파일)와 핵심 로직 위주로 요약하여 설명합니다. 전체 코드는 코드베이스를 참조하십시오.)*

- **`include/IRCServer.h`:** IRC 클라이언트 연결을 수락하고 각 클라이언트를 스레드 풀에 위임하는 메인 서버 클래스.
- **`include/IRCClient.h`:** 접속한 클라이언트의 정보(소켓, 닉네임, 현재 채널 등)를 관리하는 클래스.
- **`include/IRCChannel.h`:** IRC 채널 정보를 관리하고, 채널에 속한 클라이언트들에게 메시지를 방송(broadcast)하는 클래스.
- **`include/IRCClientManager.h`, `include/IRCChannelManager.h`:** 여러 클라이언트와 채널 객체를 스레드 안전하게 관리하는 매니저 클래스.
- **`include/IRCCommandParser.h`:** 클라이언트가 보낸 원시 IRC 메시지(예: `JOIN #channel\r\n`)를 파싱하여 `IRCCommand` 객체로 변환하는 클래스.
- **`include/IRCCommandHandler.h`:** 파싱된 `IRCCommand`를 받아 실제 동작(채널 참여, 로그 구독 콜백 등록 등)을 수행하는 클래스.

**핵심 로직 예시 (`src/IRCCommandHandler.cpp`의 JOIN 명령 처리):**
```cpp
// [SEQUENCE: MVP11-3]
void IRCCommandHandler::handleJoin(const IRCCommand& command) {
    // ... (클라이언트 정보 가져오기)
    std::string channel_name = command.get_params()[0];

    // 채널에 클라이언트 추가
    auto channel = channel_manager_.get_or_create_channel(channel_name);
    channel->add_client(client);

    // ... (JOIN 성공 응답 클라이언트에 전송)

    // [SEQUENCE: MVP11-4]
    // LogBuffer에 콜백 등록으로 로그 구독
    std::string pattern = channel_name.substr(1); // # 제거
    log_buffer_->registerChannelCallback(pattern, [channel](const LogEntry& entry) {
        std::stringstream ss;
        ss << "[" << entry.level << "] " << entry.message;
        channel->broadcast("SYSTEM", ss.str());
    });
}
```

### 3.2. `include/LogBuffer.h` (수정)

구조화된 로그를 위한 `LogEntry` 확장 및 IRC 연동을 위한 콜백 인터페이스를 추가합니다.

```cpp
// [SEQUENCE: MVP11-5]
#ifndef LOGBUFFER_H
#define LOGBUFFER_H

#include <functional> // for std::function
#include <map> // for std::map
// ... (기존 include)

// [SEQUENCE: MVP11-6]
// 구조화된 로그 항목 (IRC 통합 버전)
struct LogEntry {
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    
    // MVP6 추가 필드
    std::string level;      // 예: ERROR, WARNING, INFO
    std::string source;     // 예: service-auth, 192.168.1.5
    std::string category;   // 예: login, database
    
    LogEntry(std::string msg, std::string lvl = "INFO", std::string src = "") 
        : message(std::move(msg)), timestamp(std::chrono::system_clock::now()), 
          level(std::move(lvl)), source(std::move(src)) {}
};

class LogBuffer {
public:
    // ... (기존 public 메소드)

    // [SEQUENCE: MVP11-7]
    // MVP6 추가: IRC 연동을 위한 메소드
    void addLogWithNotification(const LogEntry& entry);
    void registerChannelCallback(const std::string& pattern, std::function<void(const LogEntry&)> callback);
    void unregisterChannelCallback(const std::string& pattern);

private:
    // ... (기존 private 멤버)

    // [SEQUENCE: MVP11-8]
    // MVP6 추가: 콜백 저장을 위한 맵
    std::map<std::string, std::vector<std::function<void(const LogEntry&)>>> channelCallbacks_;
};

#endif // LOGBUFFER_H
```

### 3.3. `src/LogBuffer.cpp` (수정)

`addLogWithNotification` 메소드에서 로그를 저장한 후, 등록된 콜백들을 실행하여 IRC 채널에 알림을 보내는 로직을 구현합니다.

```cpp
// [SEQUENCE: MVP11-9]
// ... (기존 함수들)

// [SEQUENCE: MVP11-10]
// 로그 추가 및 알림 처리
void LogBuffer::addLogWithNotification(const LogEntry& entry) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (buffer_.size() >= capacity_) {
            dropOldest_();
        }
        buffer_.push_back(entry);
        totalLogs_++;
    }
    notEmpty_.notify_one();

    // [SEQUENCE: MVP11-11]
    // 등록된 콜백 실행하여 알림 전송
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [pattern, callbacks] : channelCallbacks_) {
        // (단순화를 위해 여기서는 간단한 키워드 매칭만 예시)
        bool match = (entry.level == pattern || entry.source == pattern || 
                      entry.message.find(pattern) != std::string::npos);
        if (match) {
            for (const auto& callback : callbacks) {
                callback(entry);
            }
        }
    }
}

// [SEQUENCE: MVP11-12]
// 콜백 등록 및 해제 함수 구현
void LogBuffer::registerChannelCallback(const std::string& pattern, std::function<void(const LogEntry&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    channelCallbacks_[pattern].push_back(callback);
}

void LogBuffer::unregisterChannelCallback(const std::string& pattern) {
    std::lock_guard<std::mutex> lock(mutex_);
    channelCallbacks_.erase(pattern);
}
```

### 3.4. `include/LogServer.h` (수정)

`main` 함수에서 `IRCServer`가 `LogBuffer`에 접근할 수 있도록 `getLogBuffer` 메소드를 추가합니다.

```cpp
// [SEQUENCE: MVP11-13]
class LogServer {
public:
    // ... (기존 public 메소드)
    void setPersistenceManager(std::unique_ptr<PersistenceManager> persistence);

    // [SEQUENCE: MVP11-14]
    // MVP6 추가: IRC 서버와의 연동을 위해 LogBuffer 공유
    std::shared_ptr<LogBuffer> getLogBuffer() const { return logBuffer_; }

private:
    // ... (기존 private 멤버)
};
```

### 3.5. `src/main.cpp` (수정)

IRC 서버를 활성화하는 커맨드 라인 인자(`-i`, `-I`)를 처리하고, `IRCServer`를 생성 및 실행하는 로직을 추가합니다.

```cpp
// [SEQUENCE: MVP11-15]
#include "LogServer.h"
#include "IRCServer.h"
#include <iostream>
#include <getopt.h>
#include <signal.h>

static std::unique_ptr<LogServer> g_logServer;
static std::unique_ptr<IRCServer> g_ircServer;

void signalHandler(int signal) {
    std::cout << "\nShutting down..." << std::endl;
    if (g_ircServer) g_ircServer->stop();
    if (g_logServer) g_logServer->stop();
    exit(0);
}

int main(int argc, char* argv[]) {
    int port = 9999;
    bool irc_enabled = false;
    int irc_port = 6667;
    // ... (영속성 설정 변수)

    // [SEQUENCE: MVP11-16]
    // getopt 파싱 루프에 'i'와 'I:' 옵션 추가
    while ((opt = getopt(argc, argv, "p:d:s:PiI:h")) != -1) {
        switch (opt) {
            // ... (기존 case 문)
            case 'i': irc_enabled = true; break;
            case 'I': 
                irc_enabled = true;
                irc_port = std::stoi(optarg);
                break;
        }
    }

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        g_logServer = std::make_unique<LogServer>(port);
        // ... (영속성 관리자 설정)

        // [SEQUENCE: MVP11-17]
        // IRC 서버 생성 및 별도 스레드에서 실행
        if (irc_enabled) {
            g_ircServer = std::make_unique<IRCServer>(irc_port, g_logServer->getLogBuffer());
            std::thread irc_thread([] { g_ircServer->start(); });
            irc_thread.detach();
            std::cout << "IRC Server enabled on port " << irc_port << std::endl;
        }

        g_logServer->start(); // LogServer는 메인 스레드에서 실행

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```