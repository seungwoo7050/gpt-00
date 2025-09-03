# LogCaster 전체 아키텍처 가이드

## 🎯 개요

LogCaster는 고성능 로그 수집 시스템으로, TCP 서버, 쿼리 인터페이스, IRC 통합을 제공합니다.

```
┌─────────────────┐     ┌──────────────┐     ┌─────────────┐
│   Log Clients   │────▶│  LogServer   │────▶│  LogBuffer  │
│   (Port 9999)   │     │              │     │ (In-Memory) │
└─────────────────┘     └──────────────┘     └─────────────┘
                                                     │
┌─────────────────┐     ┌──────────────┐            ▼
│ Query Clients   │────▶│QueryHandler  │────────────┘
│   (Port 9998)   │     │              │
└─────────────────┘     └──────────────┘

┌─────────────────┐     ┌──────────────┐     ┌─────────────┐
│  IRC Clients    │────▶│  IRCServer   │────▶│IRC Channels │
│   (Port 6667)   │     │              │     │ (#logs-*)   │
└─────────────────┘     └──────────────┘     └─────────────┘
```

## 🏗️ 핵심 컴포넌트

### 1. LogServer (메인 서버)
```cpp
// [SEQUENCE: 35] RAII 기반 서버 클래스
class LogServer {
    int listenFd_;           // 로그 수집 포트
    int queryFd_;            // 쿼리 포트
    ThreadPool* threadPool_; // 연결 처리용
    LogBuffer* logBuffer_;   // 중앙 저장소
};
```

**역할:**
- 포트 9999에서 로그 수신
- 포트 9998에서 쿼리 처리
- 멀티스레드 연결 관리

### 2. LogBuffer (중앙 저장소)
```cpp
// [SEQUENCE: 172] 스레드 안전 순환 버퍼
class LogBuffer {
    std::deque<LogEntry> buffer_;
    std::shared_mutex mutex_;      // 읽기/쓰기 락
    size_t capacity_;              // 최대 크기
    
    // [SEQUENCE: 692] IRC 통합
    std::map<std::string, callbacks> channelCallbacks_;
};
```

**특징:**
- Lock-free 통계
- Observer 패턴으로 IRC 알림
- 효율적인 메모리 관리

### 3. ThreadPool (동시성 처리)
```cpp
// [SEQUENCE: 120] 현대적 C++ 스레드 풀
class ThreadPool {
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::condition_variable cv_;
};
```

**최적화:**
- 고정 크기 워커 스레드
- Lock-free 큐 (가능한 경우)
- 효율적인 태스크 분배

### 4. IRCServer (실시간 모니터링)
```cpp
// [SEQUENCE: 500] IRC 프로토콜 서버
class IRCServer {
    IRCChannelManager* channelManager_;
    IRCClientManager* clientManager_;
    IRCCommandHandler* commandHandler_;
};
```

**혁신적 기능:**
- 로그 레벨별 자동 채널
- 실시간 필터링
- 대화형 쿼리

## 🔄 데이터 흐름

### 1. 로그 수집 흐름
```
Client → TCP Socket → LogServer → ThreadPool → LogBuffer
                                                    ↓
                                              IRC Notification
```

### 2. 쿼리 처리 흐름
```
Query Client → QueryHandler → QueryParser → LogBuffer
                                              ↓
                                          Response
```

### 3. IRC 스트리밍 흐름
```
LogBuffer → Callback → IRCChannel → Broadcast → IRC Clients
```

## 🔧 설계 결정

### 1. 왜 3개의 포트?
- **9999**: 로그 수집 전용 (높은 처리량)
- **9998**: 쿼리 전용 (복잡한 처리)
- **6667**: IRC 표준 포트 (호환성)

### 2. 왜 shared_mutex?
```cpp
// 다중 읽기, 단일 쓰기
std::shared_lock<std::shared_mutex> lock(mutex_); // 읽기
std::unique_lock<std::shared_mutex> lock(mutex_); // 쓰기
```

### 3. 왜 deque?
- O(1) push/pop
- 연속 메모리 블록
- 순환 버퍼 구현 용이

## 📊 성능 특성

### 메모리 사용량
```
LogEntry 크기: ~100 bytes
10,000 엔트리: ~1 MB
1,000,000 엔트리: ~100 MB
```

### 처리 성능
- 로그 수신: 10,000+ logs/sec
- 쿼리 응답: < 1ms
- IRC 브로드캐스트: < 10ms

### 동시성
- 최대 연결: 1,000+
- 스레드 풀: 8-16 스레드
- Lock contention: < 5%

## 🚀 확장 포인트

### 1. 플러그인 시스템
```cpp
class LogProcessor {
    virtual void process(LogEntry& entry) = 0;
};
```

### 2. 저장소 추상화
```cpp
class StorageBackend {
    virtual void store(const LogEntry& entry) = 0;
    virtual std::vector<LogEntry> query(...) = 0;
};
```

### 3. 프로토콜 확장
- WebSocket 지원
- gRPC 인터페이스
- REST API

## 💡 핵심 통찰

1. **단순함이 성능이다**: 복잡한 파싱 없이 라인 기반 프로토콜
2. **분리가 확장성이다**: 각 포트가 독립적 역할
3. **관찰이 통합이다**: Observer 패턴으로 느슨한 결합

---

다음: [2_TCP_Server_Implementation.md](2_TCP_Server_Implementation.md) - TCP 서버 구현 상세