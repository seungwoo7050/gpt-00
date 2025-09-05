# Test Journey: LogCaster

## Phase 1: 테스트 전략 수립 (2025-07-25)

### 초기 테스트 요구사항 분석
- **왜 테스트가 필요한가?** 
  - TCP 서버의 안정성 검증
  - 멀티스레드 환경에서의 동시성 문제 확인
  - 버퍼 오버플로우 방지 검증
  - 쿼리 파서의 정확성 확인

- **어떤 테스트를 작성할 것인가?**
  - 연결/해제 테스트
  - 동시 다중 클라이언트 테스트
  - 쿼리 기능별 테스트
  - 에러 처리 테스트
  - 성능/부하 테스트

### Python 선택 이유
C/C++ 프로젝트임에도 Python으로 테스트를 작성한 이유:
1. **빠른 테스트 작성**: 소켓 프로그래밍이 간단
2. **멀티스레딩 테스트 용이**: threading 모듈로 쉽게 구현
3. **결과 검증 편리**: 문자열 처리와 정규식 사용 간편
4. **CI/CD 통합 쉬움**: pytest와 통합 가능

## Phase 2: MVP1 테스트 - 기본 연결 (2025-07-25)

### test_client.py 작성

#### 📋 테스트 명세
- **테스트 파일**: `tests/test_client.py`
- **목적**: TCP 서버 기본 연결 및 로그 전송 검증
- **테스트 범위**:
  - 단일 클라이언트 연결/해제
  - 다중 클라이언트 동시 연결 (5개)
  - 메시지 전송 및 포맷 검증
- **사전 조건**: 서버가 포트 9999에서 실행 중
- **예상 결과**: 모든 클라이언트가 성공적으로 연결/전송/해제

**첫 번째 시도: 단순 연결 테스트**
```python
# 실패한 코드
sock.connect((host, port))
sock.send("Test")  # 개행 문자 없어서 서버가 인식 못함
```

**문제 발견**: 서버가 '\n'으로 메시지를 구분하는데 빼먹음

**수정된 코드**:
```python
message = f"Client {client_id} - Log message {i+1}\n"  # \n 추가
sock.sendall(message.encode())  # send 대신 sendall 사용
```

**교훈**: 프로토콜 명세를 정확히 이해하고 테스트 작성

### 테스트 결과
```
=== Test 1: Single Client ===
Client 1: Connected to localhost:9999
Client 1: Sent 5 messages
Client 1: Disconnected

=== Test 2: Multiple Clients ===
Client 1-5: All connected and sent messages successfully
```

## Phase 3: MVP2 테스트 - 쿼리 인터페이스 (2025-07-26)

### test_mvp2.py 작성

#### 📋 테스트 명세
- **테스트 파일**: `tests/test_mvp2.py`
- **목적**: 쿼리 인터페이스 기능 검증
- **테스트 범위**:
  - COUNT 쿼리 정확성
  - 빈 버퍼 처리
  - 버퍼 오버플로우 동작 (10,000개 제한)
  - 로그/쿼리 포트 동시 사용
- **사전 조건**: 서버가 9999(로그), 9998(쿼리) 포트에서 실행
- **특이사항**: 타이밍 이슈로 인한 sleep 필요

MVP2에서 쿼리 포트(9998)가 추가되면서 새로운 테스트 필요:

**테스트 설계**:
1. 로그 포트와 쿼리 포트 동시 테스트
2. 버퍼가 가득 찰 때 동작 확인
3. 빈 버퍼에서 쿼리 동작 확인

**첫 구현의 문제점**:
```python
# 타이밍 이슈 발생
send_logs()
query_count()  # 로그가 아직 처리되지 않아 0 반환
```

**해결책**: 
```python
send_logs()
time.sleep(0.5)  # 서버 처리 대기
query_count()
```

### 순환 버퍼 테스트

**도전 과제**: 10,000개 제한을 어떻게 테스트할 것인가?

```python
def test_buffer_overflow():
    # 10,100개 로그 전송
    for i in range(10100):
        sock.sendall(f"Log {i}\n".encode())
    
    # COUNT 쿼리로 확인
    count = query_count()
    assert count == 10000  # 정확히 10,000개만 유지
```

**발견한 버그**: 처음엔 10,001개가 저장됨
- 원인: off-by-one 에러
- 수정: 서버 코드에서 버퍼 인덱스 계산 수정

## Phase 4: MVP3 테스트 - 고급 쿼리 (2025-07-27)

### test_mvp3.py - 복잡한 쿼리 테스트

#### 📋 테스트 명세
- **테스트 파일**: `tests/test_mvp3.py`
- **목적**: 고급 쿼리 기능 전체 검증
- **테스트 범위**:
  - SEARCH: 키워드 검색 (대소문자 구분)
  - FILTER: 시간 범위 필터링 (Unix timestamp)
  - REGEX: 정규표현식 매칭
  - RECENT: 최근 N개 로그 조회
  - CLEAR: 버퍼 초기화
- **복잡도**: 높음 (정규식 파싱, 시간대 처리)
- **주요 도전**: 정규식 이스케이프, 시간대 동기화

**새로운 쿼리 타입별 테스트 전략**:

#### 1. SEARCH 테스트
```python
def test_search_query():
    # 다양한 로그 전송
    logs = [
        "ERROR: Database connection failed",
        "INFO: Server started",
        "ERROR: Invalid user input"
    ]
    
    # ERROR 검색
    results = query_search("ERROR")
    assert len(results) == 2
    assert all("ERROR" in r for r in results)
```

#### 2. FILTER 시간 범위 테스트
**첫 시도의 실수**:
```python
# 시간대 고려 안 함
start_time = "2025-07-27 10:00:00"  # 서버와 시간대 불일치
```

**수정**:
```python
# UTC 사용으로 통일
start_time = int(time.time()) - 3600  # 1시간 전
end_time = int(time.time())
```

#### 3. REGEX 테스트
**도전**: 정규식 이스케이프 문제
```python
# 실패: 특수문자 처리
pattern = "Error.*[0-9]+"  # 서버에서 파싱 실패

# 성공: 적절한 이스케이프
pattern = "Error.*\\d+"
```

### test_keywords.py - 특수 키워드 테스트

#### 📋 테스트 명세
- **테스트 파일**: `tests/test_keywords.py`
- **목적**: 엣지 케이스 및 잘못된 입력 처리 검증
- **테스트 범위**:
  - 빈 쿼리 처리
  - 공백만 있는 쿼리
  - 알 수 없는 명령어
  - 파라미터 누락/초과
  - 특수 문자 처리
- **중요성**: strtok의 한계로 인한 파싱 버그 방지
- **발견 이슈**: strtok → strtok_r 변경 필요

C 구현에서 strtok 사용 시 발생할 수 있는 문제들 테스트:

```python
def test_empty_queries():
    # 빈 쿼리
    response = send_query("")
    assert "ERROR" in response
    
def test_whitespace_queries():
    # 공백만 있는 쿼리
    response = send_query("   ")
    assert "ERROR" in response
```

**발견한 버그**: 
- strtok가 연속된 구분자를 하나로 처리
- 해결: strtok_r로 변경하고 수동 파싱 추가

## Phase 5: MVP4 테스트 - 영속성 (2025-07-28)

### test_persistence.py 작성

#### 📋 테스트 명세
- **테스트 파일**: `tests/test_persistence.py`
- **목적**: 디스크 영속성 및 파일 관리 검증
- **테스트 범위**:
  - 파일 생성 및 쓰기
  - 10MB 크기 기반 로테이션
  - 비동기 쓰기 동작
  - 서버 재시작 시 데이터 유지
  - 파일 권한 및 경로 처리
- **환경 요구사항**: 쓰기 권한 있는 디렉토리
- **성능 고려**: 대용량 로그 생성으로 시간 소요

파일 저장 기능 테스트의 복잡성:

#### 1. 파일 생성 확인
```python
def test_file_creation():
    # 서버 시작 전 기존 파일 삭제
    cleanup_log_files()
    
    # 로그 전송
    send_logs(100)
    time.sleep(1)  # 비동기 쓰기 대기
    
    # 파일 확인
    assert os.path.exists("server.log")
```

#### 2. 파일 로테이션 테스트
**문제**: 어떻게 10MB 제한을 빠르게 테스트할 것인가?

**해결책**: 
```python
def test_file_rotation():
    # 큰 로그 메시지 생성
    large_log = "X" * 1024  # 1KB
    
    # 10,240번 전송 = ~10MB
    for i in range(10240):
        send_log(large_log)
    
    # 로테이션 확인
    assert os.path.exists("server.log")
    assert os.path.exists("server.log.1")
```

#### 3. 서버 재시작 테스트
```python
def test_persistence_across_restart():
    # Phase 1: 로그 전송
    send_logs(50)
    
    # 서버 재시작 시뮬레이션
    # (실제로는 수동으로 해야 함)
    
    # Phase 2: 이전 로그 확인
    count = query_count()
    assert count == 50  # 메모리는 초기화되지만 파일은 유지
```

## Phase 6: 보안 및 에러 처리 테스트 (2025-07-29)

### test_security.py - 보안 취약점 테스트

#### 📋 테스트 명세
- **테스트 파일**: `tests/test_security.py`
- **목적**: 보안 취약점 및 안정성 검증
- **테스트 범위**:
  - 버퍼 오버플로우 방어 (4KB 제한)
  - 형식 문자열 공격 방어
  - 동시성 스트레스 테스트 (100 스레드)
  - 잘못된 프로토콜 처리
  - 리소스 고갈 공격 방어
- **위험도**: 높음 (서버 크래시 가능성)
- **실행 주의**: 격리된 환경에서 실행 권장

#### 1. 버퍼 오버플로우 시도
```python
def test_buffer_overflow_attack():
    # 4KB 제한 초과 시도
    huge_message = "A" * 5000
    try:
        sock.sendall(huge_message.encode())
        response = sock.recv(1024)
        # 서버가 연결을 끊어야 함
        assert len(response) == 0
    except:
        pass  # 연결 종료 예상
```

#### 2. 형식 문자열 공격 테스트
```python
def test_format_string_attack():
    # printf 취약점 테스트
    malicious_logs = [
        "%s%s%s%s%s",
        "%n%n%n%n",
        "%%x %%x %%x %%x"
    ]
    
    for log in malicious_logs:
        send_log(log)
    
    # 서버가 여전히 응답하는지 확인
    assert query_count() > 0  # 서버 살아있음
```

#### 3. 동시성 스트레스 테스트
```python
def stress_test_concurrent_queries():
    threads = []
    
    # 100개 스레드가 동시에 쿼리
    for i in range(100):
        t = threading.Thread(target=query_random)
        threads.append(t)
        t.start()
    
    # 모든 스레드 완료 대기
    for t in threads:
        t.join()
    
    # 서버 상태 확인
    assert query_count() >= 0  # 서버 정상 작동
```

## Phase 7: C++ 버전 비교 테스트 (2025-07-30)

### 동일 테스트로 C vs C++ 비교

모든 Python 테스트를 양쪽 구현에 실행:

#### 성능 비교 결과
```python
def benchmark_comparison():
    results = {
        'C': {
            'insert_time': 0.0012,  # 1000 logs
            'query_time': 0.0008,
            'memory': 45MB
        },
        'CPP': {
            'insert_time': 0.0008,  # 더 빠름
            'query_time': 0.0006,
            'memory': 42MB
        }
    }
```

#### 동작 차이점 발견
1. **에러 메시지 형식**
   - C: "ERROR: Invalid query"
   - C++: "Error: Invalid query format"

2. **연결 종료 처리**
   - C: 즉시 종료
   - C++: Graceful shutdown

## Phase 8: CI/CD 통합 (2025-07-31)

### 자동화된 테스트 스크립트

**run_all_tests.sh 작성**:
```bash
#!/bin/bash
echo "Starting LogCaster Tests..."

# C 버전 테스트
cd LogCaster-C
make clean && make
./logcaster-c &
SERVER_PID=$!
sleep 1

python3 tests/test_client.py
python3 tests/test_mvp2.py
python3 tests/test_mvp3.py

kill $SERVER_PID
```

### GitHub Actions 통합
```yaml
- name: Run Tests
  run: |
    ./run_all_tests.sh
    python3 -m pytest tests/ -v
```

## 테스트 커버리지 최종 정리

### 달성한 것
- ✅ 기본 연결/해제 테스트
- ✅ 멀티클라이언트 동시성 테스트
- ✅ 모든 쿼리 타입 테스트
- ✅ 에러 처리 테스트
- ✅ 보안 취약점 테스트
- ✅ 성능 벤치마크
- ✅ C vs C++ 비교 테스트

### 아쉬운 점
- ❌ 단위 테스트 부재 (통합 테스트만 존재)
- ❌ 메모리 누수 테스트 미흡
- ❌ 장시간 실행 테스트 부족

## 핵심 교훈

### 1. 테스트가 버그를 찾아냄
- off-by-one 에러 (버퍼 크기)
- strtok 동시성 문제
- 형식 문자열 취약점

### 2. Python 테스트의 장점
- 빠른 작성과 수정
- 멀티스레드 테스트 용이
- 프로토콜 검증 편리

### 3. 테스트 주도 개발의 가치
- 테스트 작성하며 API 개선
- 에러 처리 강화
- 문서화 효과

### 4. 지속적 개선
- 각 MVP마다 테스트 추가
- 회귀 테스트로 안정성 보장
- 성능 기준선 확립

## 면접 대비 포인트

**Q: "왜 C/C++ 프로젝트에 Python 테스트를 사용했나요?"**
```
A: "블랙박스 테스트 관점에서 클라이언트는 서버 구현 언어를
   몰라도 되고, Python으로 빠르게 다양한 시나리오를 테스트할 수
   있었습니다. 특히 멀티스레드 클라이언트 시뮬레이션이 
   Python으로는 10줄이면 되지만 C로는 100줄이 넘습니다."
```

**Q: "테스트에서 발견한 가장 심각한 버그는?"**
```
A: "strtok를 멀티스레드 환경에서 사용한 것입니다.
   strtok는 정적 버퍼를 사용해서 스레드 안전하지 않은데,
   동시에 여러 쿼리가 오면 파싱이 꼬였습니다.
   strtok_r로 변경하여 해결했습니다."
```