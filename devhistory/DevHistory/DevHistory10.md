# DevHistory 10: LogCaster-CPP MVP5 - Security Hardening

## 1. 개요 (Overview)

C++ 구현의 다섯 번째 단계(MVP5)는 C 버전의 MVP5와 마찬가지로, 새로운 기능 추가가 아닌 기존 코드베이스의 안정성과 보안을 강화하는 데 집중합니다. C++의 현대적인 기능들 덕분에 C 버전에서 발견된 대부분의 메모리 및 동시성 문제는 사전에 방지되었지만, 외부 입력에 대한 검증은 여전히 중요한 과제로 남아있습니다. 이 단계는 LogCaster C++ 버전을 프로덕션 환경에 한 걸음 더 가깝게 만드는 마무리 작업입니다.

**주요 해결 과제:**
- **리소스 고갈(Resource Exhaustion) 방어:** C 버전과 동일하게, 악의적인 클라이언트가 의도적으로 매우 큰 로그 메시지를 전송하여 서버의 메모리를 고갈시키려는 공격을 방어해야 합니다. `LogServer`가 클라이언트로부터 데이터를 수신할 때, 메시지의 크기를 검증하고 설정된 최대 길이를 초과할 경우 이를 안전하게 절단하는 로직을 추가합니다.

**아키텍처 변경:**
- 이번 MVP에서는 아키텍처의 큰 변경 없이, `LogServer` 클래스 내부의 클라이언트 데이터 처리 로직만 소폭 수정됩니다.

이 단계를 통해 C++ 버전 또한 외부의 비정상적인 입력에 대해 더 견고하게 대응할 수 있는 능력을 갖추게 됩니다.

## 2. 주요 수정 사항 및 코드

### 2.1. `include/LogServer.h` (수정)

`LogServer` 클래스 내부에 로그 메시지의 최대 허용 길이를 나타내는 상수를 추가합니다.

```cpp
// [SEQUENCE: MVP5-1]
#ifndef LOGSERVER_H
#define LOGSERVER_H

// ... (기존 include는 MVP4와 동일)
#include "Persistence.h"

class LogServer {
public:
    // ... (기존 public 메소드는 MVP4와 동일)

    // [SEQUENCE: MVP5-2]
    // MVP5 추가: 안전한 로그 길이를 위한 상수
    static constexpr size_t SAFE_LOG_LENGTH = 1024;

private:
    // ... (나머지는 MVP4와 동일)
};

#endif // LOGSERVER_H
```

### 2.2. `src/LogServer.cpp` (수정)

`handleClientTask` 메소드에서 `recv`로 데이터를 수신한 후, 해당 데이터의 길이를 `SAFE_LOG_LENGTH`와 비교하여 초과 시 절단하는 로직을 추가합니다.

```cpp
// [SEQUENCE: MVP5-3]
#include "LogServer.h"
// ... (다른 include는 MVP4와 동일)

// ... (생성자, 소멸자, start, stop, initialize, runEventLoop, handleNewConnection 등은 MVP4와 동일)

// [SEQUENCE: MVP5-4]
// 로그 클라이언트 작업 (MVP5 버전)
void LogServer::handleClientTask(int client_fd) {
    char buffer[4096]; // 수신을 위한 물리적 버퍼
    while (true) {
        ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (nbytes <= 0) break;
        
        // [SEQUENCE: MVP5-5]
        // 수신된 데이터로 std::string 생성 및 크기 검증
        std::string log_message(buffer, nbytes);
        if (log_message.length() > SAFE_LOG_LENGTH) {
            log_message.resize(SAFE_LOG_LENGTH - 3);
            log_message += "...";
        }

        // Remove trailing newline
        size_t last_char_pos = log_message.find_last_not_of("\r\n");
        if (std::string::npos != last_char_pos) {
            log_message.erase(last_char_pos + 1);
        }

        // 1. 인메모리 버퍼에 저장
        logBuffer_->push(log_message);

        // 2. 영속성 관리자에게 쓰기 요청 (활성화된 경우)
        if (persistence_) {
            persistence_->write(log_message);
        }
    }
    close(client_fd);
}

// ... (나머지 함수는 MVP4와 동일)
```

## 3. 테스트 코드 (Test Code)

이 변경 사항을 검증하기 위해, C 버전의 보안 테스트(`test_security.py`)와 유사한 테스트를 수행할 수 있습니다. 특히 1KB가 넘는 긴 로그 메시지를 전송하고, 서버가 비정상 종료되지 않고 안정적으로 동작하는지를 확인하는 것이 중요합니다.

*(테스트 코드는 C 버전 MVP5의 `test_log_truncation` 부분과 유사하게 구성할 수 있어 별도로 첨부하지 않음)*

```
