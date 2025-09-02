# 07. 보안 체크리스트

## 🎯 목표
LogCaster를 프로덕션 환경에 안전하게 배포하기 위한 포괄적인 보안 체크리스트와 구현 가이드입니다.

## 📋 Prerequisites
- 보안 스캔 도구 (Trivy, Snyk, OWASP ZAP)
- SSL/TLS 인증서
- 방화벽 설정 권한
- 보안 정책 이해

---

## 1. 네트워크 보안

### 1.1 방화벽 규칙

```bash
# iptables 규칙 설정
#!/bin/bash

# 기본 정책: 모든 트래픽 차단
iptables -P INPUT DROP
iptables -P FORWARD DROP
iptables -P OUTPUT ACCEPT

# 로컬 루프백 허용
iptables -A INPUT -i lo -j ACCEPT

# 기존 연결 허용
iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT

# SSH (제한된 IP에서만)
iptables -A INPUT -p tcp --dport 22 -s YOUR_ADMIN_IP/32 -j ACCEPT

# LogCaster 포트 (특정 소스에서만)
iptables -A INPUT -p tcp --dport 9999 -s ALLOWED_NETWORK/24 -j ACCEPT
iptables -A INPUT -p tcp --dport 9998 -s ALLOWED_NETWORK/24 -j ACCEPT

# Rate limiting
iptables -A INPUT -p tcp --dport 9999 -m state --state NEW -m recent --set
iptables -A INPUT -p tcp --dport 9999 -m state --state NEW -m recent --update --seconds 60 --hitcount 100 -j DROP

# DDoS 방어
iptables -A INPUT -p tcp --tcp-flags ALL NONE -j DROP
iptables -A INPUT -p tcp --tcp-flags SYN,FIN SYN,FIN -j DROP
iptables -A INPUT -p tcp --tcp-flags SYN,RST SYN,RST -j DROP

# 로그 및 차단
iptables -A INPUT -j LOG --log-prefix "iptables-dropped: "
iptables -A INPUT -j DROP

# 규칙 저장
iptables-save > /etc/iptables/rules.v4
```

### 1.2 네트워크 세그멘테이션

```yaml
# docker-compose-secure.yml
version: '3.8'

networks:
  frontend:
    driver: bridge
    ipam:
      config:
        - subnet: 172.20.0.0/24
  backend:
    driver: bridge
    internal: true
    ipam:
      config:
        - subnet: 172.21.0.0/24
  monitoring:
    driver: bridge
    internal: true
    ipam:
      config:
        - subnet: 172.22.0.0/24

services:
  logcaster:
    networks:
      - frontend
      - backend
    cap_drop:
      - ALL
    cap_add:
      - NET_BIND_SERVICE
    security_opt:
      - no-new-privileges:true
```

---

## 2. 애플리케이션 보안

### 2.1 입력 검증 강화

```c
// secure_input_handler.c
#include <regex.h>
#include <ctype.h>

#define MAX_INPUT_LENGTH 4096
#define MAX_QUERY_LENGTH 256

typedef struct {
    const char* pattern;
    regex_t regex;
    const char* error_msg;
} input_validator_t;

static input_validator_t validators[] = {
    {"^[A-Z]+$", {0}, "Invalid command format"},
    {"^[0-9]{4}-[0-9]{2}-[0-9]{2}$", {0}, "Invalid date format"},
    {"^[a-zA-Z0-9_.-]+$", {0}, "Invalid identifier"},
    {NULL, {0}, NULL}
};

int validate_input(const char* input, int validator_type) {
    // 길이 체크
    if (strlen(input) > MAX_INPUT_LENGTH) {
        return -1;
    }
    
    // NULL 바이트 체크
    for (size_t i = 0; i < strlen(input); i++) {
        if (input[i] == '\0' && i < strlen(input) - 1) {
            return -1;
        }
    }
    
    // 정규식 검증
    if (validator_type >= 0 && validators[validator_type].pattern) {
        return regexec(&validators[validator_type].regex, input, 0, NULL, 0) == 0 ? 0 : -1;
    }
    
    return 0;
}

// SQL Injection 방지
char* sanitize_query(const char* input) {
    char* sanitized = malloc(MAX_QUERY_LENGTH * 2);
    int j = 0;
    
    for (int i = 0; input[i] && i < MAX_QUERY_LENGTH; i++) {
        switch(input[i]) {
            case '\'':
            case '"':
            case '\\':
            case '\n':
            case '\r':
            case '\0':
                sanitized[j++] = '\\';
                sanitized[j++] = input[i];
                break;
            default:
                if (isprint(input[i])) {
                    sanitized[j++] = input[i];
                }
        }
    }
    sanitized[j] = '\0';
    return sanitized;
}
```

### 2.2 메모리 보안

```c
// secure_memory.c
#include <string.h>
#include <sodium.h>

// 안전한 메모리 할당
void* secure_malloc(size_t size) {
    void* ptr = sodium_malloc(size);
    if (ptr) {
        sodium_mlock(ptr, size);
    }
    return ptr;
}

// 안전한 메모리 해제
void secure_free(void* ptr, size_t size) {
    if (ptr) {
        sodium_memzero(ptr, size);
        sodium_munlock(ptr, size);
        sodium_free(ptr);
    }
}

// 버퍼 오버플로우 방지
int safe_copy(char* dest, size_t dest_size, const char* src) {
    if (!dest || !src || dest_size == 0) {
        return -1;
    }
    
    size_t src_len = strnlen(src, dest_size);
    if (src_len >= dest_size) {
        return -1;
    }
    
    memcpy(dest, src, src_len);
    dest[src_len] = '\0';
    return 0;
}
```

---

## 3. 인증 및 인가

### 3.1 JWT 기반 인증

```cpp
// JWTAuth.cpp
#include <jwt-cpp/jwt.h>
#include <chrono>

class JWTAuthenticator {
private:
    std::string secret;
    std::string issuer;
    
public:
    JWTAuthenticator(const std::string& secret, const std::string& issuer)
        : secret(secret), issuer(issuer) {}
    
    std::string generateToken(const std::string& username, const std::vector<std::string>& roles) {
        auto token = jwt::create()
            .set_issuer(issuer)
            .set_type("JWT")
            .set_subject(username)
            .set_issued_at(std::chrono::system_clock::now())
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours{24})
            .set_payload_claim("roles", jwt::claim(roles))
            .sign(jwt::algorithm::hs256{secret});
        
        return token;
    }
    
    bool verifyToken(const std::string& token) {
        try {
            auto decoded = jwt::decode(token);
            auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{secret})
                .with_issuer(issuer)
                .with_claim("roles", jwt::claim(std::vector<std::string>{}));
            
            verifier.verify(decoded);
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }
};
```

### 3.2 Rate Limiting

```cpp
// RateLimiter.cpp
#include <unordered_map>
#include <chrono>
#include <mutex>

class RateLimiter {
private:
    struct ClientInfo {
        int count;
        std::chrono::steady_clock::time_point reset_time;
    };
    
    std::unordered_map<std::string, ClientInfo> clients;
    std::mutex mutex;
    int max_requests;
    std::chrono::seconds window;
    
public:
    RateLimiter(int max_requests = 100, int window_seconds = 60)
        : max_requests(max_requests), window(window_seconds) {}
    
    bool allowRequest(const std::string& client_ip) {
        std::lock_guard<std::mutex> lock(mutex);
        auto now = std::chrono::steady_clock::now();
        
        auto it = clients.find(client_ip);
        if (it == clients.end()) {
            clients[client_ip] = {1, now + window};
            return true;
        }
        
        if (now > it->second.reset_time) {
            it->second.count = 1;
            it->second.reset_time = now + window;
            return true;
        }
        
        if (it->second.count >= max_requests) {
            return false;
        }
        
        it->second.count++;
        return true;
    }
};
```

---

## 4. 컨테이너 보안

### 4.1 안전한 Dockerfile

```dockerfile
# Secure Dockerfile
FROM ubuntu:22.04 AS builder

# 보안 업데이트 적용
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY --chown=nobody:nogroup . .

# 빌드
RUN cmake -DCMAKE_BUILD_TYPE=Release . && make

# 런타임 이미지
FROM ubuntu:22.04

# 보안 업데이트
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends \
    libpthread-stubs0-dev \
    && rm -rf /var/lib/apt/lists/*

# 비root 사용자 생성
RUN groupadd -r logcaster && \
    useradd -r -g logcaster -d /home/logcaster -s /sbin/nologin logcaster

# 실행 파일 복사
COPY --from=builder --chown=logcaster:logcaster /build/logcaster /usr/local/bin/

# 읽기 전용 파일시스템
RUN chmod 755 /usr/local/bin/logcaster && \
    mkdir -p /var/log/logcaster && \
    chown logcaster:logcaster /var/log/logcaster

# 보안 설정
USER logcaster
WORKDIR /home/logcaster

# 헬스체크
HEALTHCHECK --interval=30s --timeout=3s \
    CMD nc -z localhost 9999 || exit 1

EXPOSE 9999 9998

ENTRYPOINT ["/usr/local/bin/logcaster"]
```

### 4.2 컨테이너 스캔

```bash
#!/bin/bash
# container-scan.sh

IMAGE=$1

echo "Scanning container image: $IMAGE"

# Trivy 스캔
echo "Running Trivy scan..."
trivy image --severity HIGH,CRITICAL --exit-code 1 $IMAGE

# Snyk 스캔
echo "Running Snyk scan..."
snyk container test $IMAGE --severity-threshold=high

# Docker Bench Security
echo "Running Docker Bench..."
docker run --rm --net host --pid host --userns host --cap-add audit_control \
    -e DOCKER_CONTENT_TRUST=$DOCKER_CONTENT_TRUST \
    -v /var/lib:/var/lib \
    -v /var/run/docker.sock:/var/run/docker.sock \
    -v /etc:/etc \
    docker/docker-bench-security
```

---

## 5. 비밀 정보 관리

### 5.1 HashiCorp Vault 통합

```bash
# Vault 설정
vault secrets enable -path=logcaster kv-v2

# 비밀 정보 저장
vault kv put logcaster/database \
    username="dbuser" \
    password="$(openssl rand -base64 32)"

vault kv put logcaster/jwt \
    secret="$(openssl rand -base64 64)"

# 정책 생성
cat <<EOF | vault policy write logcaster-policy -
path "logcaster/*" {
  capabilities = ["read", "list"]
}
EOF

# AppRole 인증 설정
vault auth enable approle
vault write auth/approle/role/logcaster \
    token_policies="logcaster-policy" \
    token_ttl=1h \
    token_max_ttl=4h
```

### 5.2 환경 변수 암호화

```bash
# secrets.enc 파일 생성
cat <<EOF > secrets.env
DB_PASSWORD=supersecret
JWT_SECRET=anothersecret
API_KEY=apikeysecret
EOF

# 암호화
openssl enc -aes-256-cbc -salt -in secrets.env -out secrets.enc -k $ENCRYPTION_KEY

# 복호화 및 로드
openssl enc -d -aes-256-cbc -in secrets.enc -k $ENCRYPTION_KEY | source /dev/stdin
```

---

## 6. 로깅 및 감사

### 6.1 보안 로깅

```c
// security_logger.c
#include <syslog.h>
#include <time.h>

typedef enum {
    SEC_EVENT_AUTH_SUCCESS,
    SEC_EVENT_AUTH_FAILURE,
    SEC_EVENT_ACCESS_DENIED,
    SEC_EVENT_INVALID_INPUT,
    SEC_EVENT_RATE_LIMIT,
    SEC_EVENT_SUSPICIOUS_ACTIVITY
} security_event_t;

void log_security_event(security_event_t event, const char* client_ip, 
                        const char* username, const char* details) {
    char timestamp[64];
    time_t now = time(NULL);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    char log_message[1024];
    snprintf(log_message, sizeof(log_message),
        "[SECURITY] %s | Event: %d | IP: %s | User: %s | Details: %s",
        timestamp, event, client_ip, username ?: "anonymous", details);
    
    // syslog에 기록
    syslog(LOG_WARNING | LOG_AUTH, "%s", log_message);
    
    // 별도 보안 로그 파일에도 기록
    FILE* sec_log = fopen("/var/log/logcaster/security.log", "a");
    if (sec_log) {
        fprintf(sec_log, "%s\n", log_message);
        fclose(sec_log);
    }
}
```

---

## 7. 보안 체크리스트

### 7.1 배포 전 체크리스트

```markdown
## 네트워크 보안
- [ ] 방화벽 규칙 설정 완료
- [ ] 불필요한 포트 차단
- [ ] 네트워크 세그멘테이션 구성
- [ ] DDoS 방어 메커니즘 활성화
- [ ] TLS/SSL 인증서 설치

## 애플리케이션 보안
- [ ] 입력 검증 구현
- [ ] SQL/Command Injection 방지
- [ ] XSS 방지 (해당 시)
- [ ] CSRF 보호 (해당 시)
- [ ] 안전한 세션 관리

## 인증/인가
- [ ] 강력한 인증 메커니즘
- [ ] 적절한 권한 분리
- [ ] Rate limiting 구현
- [ ] 계정 잠금 정책
- [ ] 패스워드 정책 적용

## 데이터 보호
- [ ] 전송 중 암호화 (TLS)
- [ ] 저장 시 암호화
- [ ] 민감 데이터 마스킹
- [ ] 안전한 백업 절차
- [ ] 데이터 보존 정책

## 컨테이너 보안
- [ ] 최소 권한 원칙 적용
- [ ] 비root 사용자 실행
- [ ] 읽기 전용 파일시스템
- [ ] 보안 스캔 통과
- [ ] 이미지 서명 검증

## 비밀 정보 관리
- [ ] 하드코딩된 비밀 정보 제거
- [ ] 비밀 정보 암호화 저장
- [ ] 정기적인 키 로테이션
- [ ] 접근 로그 기록
- [ ] 비밀 정보 스캔 도구 실행

## 모니터링/감사
- [ ] 보안 이벤트 로깅
- [ ] 실시간 알림 설정
- [ ] 정기적인 로그 검토
- [ ] 침입 탐지 시스템
- [ ] 보안 대시보드 구성

## 컴플라이언스
- [ ] GDPR 준수 (해당 시)
- [ ] PCI DSS 준수 (해당 시)
- [ ] SOC2 요구사항 충족
- [ ] 데이터 지역성 요구사항
- [ ] 감사 추적 유지
```

---

## 8. 보안 테스트

### 8.1 자동화된 보안 테스트

```bash
#!/bin/bash
# security-test.sh

echo "Running comprehensive security tests..."

# OWASP ZAP 스캔
docker run -t owasp/zap2docker-stable zap-baseline.py \
    -t http://logcaster:9999 -r security-report.html

# Nikto 웹 스캔
nikto -h http://logcaster:9999 -o nikto-report.txt

# SQLMap (SQL Injection 테스트)
sqlmap -u "http://logcaster:9998/query?search=test" \
    --batch --random-agent -o -report

# 퍼징 테스트
python3 fuzzer.py --target logcaster:9999 --iterations 10000

echo "Security tests completed. Check reports for findings."
```

---

## ✅ 최종 체크리스트

### 필수 확인사항
- [ ] 모든 보안 스캔 통과
- [ ] 침투 테스트 완료
- [ ] 보안 정책 문서화
- [ ] 인시던트 대응 계획
- [ ] 백업/복구 테스트
- [ ] 보안 교육 완료

### 권장사항
- [ ] Bug Bounty 프로그램
- [ ] 정기 보안 감사
- [ ] 취약점 관리 프로세스
- [ ] 보안 챔피언 지정
- [ ] 보안 메트릭 추적

---

## 🎯 다음 단계
→ [08_ssl_certificates.md](08_ssl_certificates.md) - SSL/TLS 인증서 설정