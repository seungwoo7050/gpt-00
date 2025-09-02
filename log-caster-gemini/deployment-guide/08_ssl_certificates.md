# 08. SSL/TLS 인증서 설정

## 🎯 목표
LogCaster의 모든 통신을 암호화하고 SSL/TLS 인증서를 관리하기 위한 종합 가이드입니다.

## 📋 Prerequisites
- 도메인 소유권 확인
- DNS 관리 권한
- OpenSSL 1.1.1+ 설치
- 인증서 발급 기관 선택 (Let's Encrypt, 상용 CA)

---

## 1. 인증서 유형 및 선택

### 1.1 인증서 유형 비교

| 유형 | 검증 수준 | 발급 시간 | 비용 | 사용 사례 |
|------|----------|----------|------|----------|
| DV (Domain Validation) | 도메인 소유권만 | 몇 분 | 무료-$100 | 일반 웹사이트 |
| OV (Organization Validation) | 조직 정보 확인 | 1-3일 | $100-500 | 기업 서비스 |
| EV (Extended Validation) | 엄격한 신원 확인 | 3-7일 | $500-2000 | 금융, 전자상거래 |
| Wildcard | 서브도메인 전체 | DV/OV 동일 | $200-800 | 다중 서브도메인 |

### 1.2 추천 설정

```bash
# LogCaster 추천 인증서 구성
# 개발/테스트: Let's Encrypt DV
# 프로덕션: Let's Encrypt Wildcard 또는 상용 OV
# 엔터프라이즈: 상용 EV + HSM
```

---

## 2. Let's Encrypt 무료 인증서

### 2.1 Certbot 설치 및 설정

```bash
#!/bin/bash
# install-certbot.sh

# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y certbot python3-certbot-nginx

# CentOS/RHEL
sudo yum install -y epel-release
sudo yum install -y certbot python3-certbot-nginx

# Docker 방식
docker run -it --rm --name certbot \
    -v "/etc/letsencrypt:/etc/letsencrypt" \
    -v "/var/lib/letsencrypt:/var/lib/letsencrypt" \
    certbot/certbot certonly
```

### 2.2 인증서 발급

```bash
#!/bin/bash
# issue-letsencrypt.sh

DOMAIN="logcaster.example.com"
EMAIL="admin@example.com"

# Standalone 방식 (서버 중지 필요)
sudo certbot certonly --standalone \
    -d $DOMAIN \
    -d www.$DOMAIN \
    --email $EMAIL \
    --agree-tos \
    --no-eff-email

# Webroot 방식 (서버 실행 중)
sudo certbot certonly --webroot \
    -w /var/www/html \
    -d $DOMAIN \
    -d www.$DOMAIN \
    --email $EMAIL \
    --agree-tos

# DNS 방식 (와일드카드 인증서)
sudo certbot certonly --manual \
    --preferred-challenges dns \
    -d "*.logcaster.example.com" \
    -d logcaster.example.com \
    --email $EMAIL \
    --agree-tos
```

### 2.3 자동 갱신 설정

```bash
# /etc/cron.d/certbot-renewal
0 0,12 * * * root certbot renew --quiet --post-hook "systemctl reload nginx"

# 또는 systemd timer 사용
cat > /etc/systemd/system/certbot-renewal.service <<EOF
[Unit]
Description=Certbot Renewal
After=network.target

[Service]
Type=oneshot
ExecStart=/usr/bin/certbot renew --quiet --deploy-hook "/usr/local/bin/reload-services.sh"
EOF

cat > /etc/systemd/system/certbot-renewal.timer <<EOF
[Unit]
Description=Run certbot twice daily

[Timer]
OnCalendar=*-*-* 00,12:00:00
RandomizedDelaySec=3600
Persistent=true

[Install]
WantedBy=timers.target
EOF

systemctl enable --now certbot-renewal.timer
```

---

## 3. 자체 서명 인증서 (개발용)

### 3.1 CA 인증서 생성

```bash
#!/bin/bash
# create-ca.sh

# CA 키 생성
openssl genrsa -out ca.key 4096

# CA 인증서 생성
openssl req -new -x509 -days 3650 -key ca.key -out ca.crt \
    -subj "/C=KR/ST=Seoul/L=Seoul/O=LogCaster/CN=LogCaster CA"

# CA 인증서 시스템에 등록
sudo cp ca.crt /usr/local/share/ca-certificates/logcaster-ca.crt
sudo update-ca-certificates
```

### 3.2 서버 인증서 생성

```bash
#!/bin/bash
# create-server-cert.sh

DOMAIN="logcaster.local"

# 서버 키 생성
openssl genrsa -out server.key 2048

# CSR 생성
cat > server.conf <<EOF
[req]
distinguished_name = req_distinguished_name
req_extensions = v3_req
prompt = no

[req_distinguished_name]
C = KR
ST = Seoul
L = Seoul
O = LogCaster
CN = $DOMAIN

[v3_req]
keyUsage = keyEncipherment, dataEncipherment
extendedKeyUsage = serverAuth
subjectAltName = @alt_names

[alt_names]
DNS.1 = $DOMAIN
DNS.2 = *.$DOMAIN
IP.1 = 127.0.0.1
IP.2 = 192.168.1.100
EOF

openssl req -new -key server.key -out server.csr -config server.conf

# 인증서 서명
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key \
    -CAcreateserial -out server.crt -days 365 \
    -extensions v3_req -extfile server.conf

# PEM 형식으로 결합
cat server.crt server.key > server.pem
```

---

## 4. LogCaster TLS 통합

### 4.1 C 버전 TLS 구현

```c
// tls_server.c
#include <openssl/ssl.h>
#include <openssl/err.h>

SSL_CTX* create_ssl_context() {
    SSL_CTX* ctx;
    
    // OpenSSL 초기화
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    
    // TLS 1.2 이상만 허용
    ctx = SSL_CTX_new(TLS_server_method());
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    
    // 강력한 암호 스위트만 사용
    SSL_CTX_set_cipher_list(ctx, 
        "ECDHE-ECDSA-AES256-GCM-SHA384:"
        "ECDHE-RSA-AES256-GCM-SHA384:"
        "ECDHE-ECDSA-CHACHA20-POLY1305:"
        "ECDHE-RSA-CHACHA20-POLY1305:"
        "ECDHE-ECDSA-AES128-GCM-SHA256:"
        "ECDHE-RSA-AES128-GCM-SHA256");
    
    // Perfect Forward Secrecy
    SSL_CTX_set_ecdh_auto(ctx, 1);
    
    // Session tickets 비활성화 (보안 강화)
    SSL_CTX_set_options(ctx, SSL_OP_NO_TICKET);
    
    return ctx;
}

int configure_ssl_context(SSL_CTX* ctx, const char* cert_file, const char* key_file) {
    // 인증서 로드
    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return -1;
    }
    
    // 개인키 로드
    if (SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return -1;
    }
    
    // 인증서와 키 매칭 확인
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match certificate\n");
        return -1;
    }
    
    // 인증서 체인 로드 (있는 경우)
    SSL_CTX_load_verify_locations(ctx, "ca-bundle.crt", NULL);
    
    // 클라이언트 인증서 요구 (선택사항)
    // SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    
    return 0;
}

void handle_tls_connection(int client_fd, SSL_CTX* ctx) {
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_fd);
    
    if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
    } else {
        // TLS 연결 정보 로깅
        printf("TLS Version: %s\n", SSL_get_version(ssl));
        printf("Cipher: %s\n", SSL_get_cipher(ssl));
        
        // 데이터 읽기/쓰기
        char buffer[1024];
        int bytes = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytes > 0) {
            // 처리
            SSL_write(ssl, "OK\n", 3);
        }
    }
    
    SSL_shutdown(ssl);
    SSL_free(ssl);
}
```

### 4.2 C++ 버전 TLS 구현

```cpp
// TLSServer.cpp
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

class TLSServer {
private:
    boost::asio::io_context& io_context_;
    boost::asio::ssl::context ssl_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    
public:
    TLSServer(boost::asio::io_context& io_context, unsigned short port)
        : io_context_(io_context),
          ssl_context_(boost::asio::ssl::context::tlsv12),
          acceptor_(io_context, boost::asio::ip::tcp::endpoint(
              boost::asio::ip::tcp::v4(), port)) {
        
        configure_ssl();
        accept_connections();
    }
    
    void configure_ssl() {
        // SSL 옵션 설정
        ssl_context_.set_options(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::no_sslv3 |
            boost::asio::ssl::context::no_tlsv1 |
            boost::asio::ssl::context::no_tlsv1_1 |
            boost::asio::ssl::context::single_dh_use);
        
        // 인증서 파일 로드
        ssl_context_.use_certificate_chain_file("server.crt");
        ssl_context_.use_private_key_file("server.key", 
            boost::asio::ssl::context::pem);
        
        // DH 파라미터 설정
        ssl_context_.use_tmp_dh_file("dhparam.pem");
        
        // 암호 스위트 설정
        SSL_CTX_set_cipher_list(ssl_context_.native_handle(),
            "ECDHE+AESGCM:ECDHE+AES256:!aNULL:!MD5:!DSS");
    }
    
    void accept_connections() {
        auto new_session = std::make_shared<TLSSession>(io_context_, ssl_context_);
        
        acceptor_.async_accept(new_session->socket(),
            [this, new_session](std::error_code ec) {
                if (!ec) {
                    new_session->start();
                }
                accept_connections();
            });
    }
};
```

---

## 5. 인증서 모니터링 및 관리

### 5.1 인증서 만료 모니터링

```bash
#!/bin/bash
# monitor-certificates.sh

CHECK_DOMAINS=(
    "logcaster.example.com:443"
    "api.logcaster.example.com:443"
    "admin.logcaster.example.com:443"
)

ALERT_DAYS=30

for DOMAIN in "${CHECK_DOMAINS[@]}"; do
    EXPIRY_DATE=$(echo | openssl s_client -servername ${DOMAIN%:*} \
        -connect $DOMAIN 2>/dev/null | \
        openssl x509 -noout -enddate | cut -d= -f2)
    
    EXPIRY_EPOCH=$(date -d "$EXPIRY_DATE" +%s)
    CURRENT_EPOCH=$(date +%s)
    DAYS_LEFT=$(( ($EXPIRY_EPOCH - $CURRENT_EPOCH) / 86400 ))
    
    if [ $DAYS_LEFT -lt $ALERT_DAYS ]; then
        echo "⚠️ 인증서 만료 경고: $DOMAIN - ${DAYS_LEFT}일 남음"
        # 알림 전송
        ./send-alert.sh "인증서 만료 임박: $DOMAIN (${DAYS_LEFT}일)"
    else
        echo "✅ $DOMAIN: ${DAYS_LEFT}일 남음"
    fi
done
```

### 5.2 인증서 자동 배포

```bash
#!/bin/bash
# deploy-certificates.sh

CERT_SOURCE="/etc/letsencrypt/live/logcaster.example.com"
SERVERS=(
    "server1.example.com"
    "server2.example.com"
    "server3.example.com"
)

echo "🔐 인증서 배포 시작..."

for SERVER in "${SERVERS[@]}"; do
    echo "배포 중: $SERVER"
    
    # 인증서 복사
    rsync -avz --delete \
        $CERT_SOURCE/ \
        $SERVER:/etc/ssl/logcaster/
    
    # 서비스 재시작
    ssh $SERVER "systemctl reload logcaster"
    
    # 검증
    if curl -f https://$SERVER:9999/health > /dev/null 2>&1; then
        echo "✅ $SERVER: 성공"
    else
        echo "❌ $SERVER: 실패"
    fi
done

echo "✅ 인증서 배포 완료"
```

---

## 6. 보안 헤더 설정

### 6.1 NGINX 보안 헤더

```nginx
# /etc/nginx/sites-available/logcaster-ssl
server {
    listen 443 ssl http2;
    server_name logcaster.example.com;
    
    # SSL 인증서
    ssl_certificate /etc/letsencrypt/live/logcaster.example.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/logcaster.example.com/privkey.pem;
    
    # SSL 설정
    ssl_protocols TLSv1.2 TLSv1.3;
    ssl_ciphers ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384;
    ssl_prefer_server_ciphers off;
    
    # OCSP Stapling
    ssl_stapling on;
    ssl_stapling_verify on;
    ssl_trusted_certificate /etc/letsencrypt/live/logcaster.example.com/chain.pem;
    
    # Session 설정
    ssl_session_timeout 1d;
    ssl_session_cache shared:SSL:10m;
    ssl_session_tickets off;
    
    # 보안 헤더
    add_header Strict-Transport-Security "max-age=63072000; includeSubDomains; preload" always;
    add_header X-Frame-Options "DENY" always;
    add_header X-Content-Type-Options "nosniff" always;
    add_header X-XSS-Protection "1; mode=block" always;
    add_header Content-Security-Policy "default-src 'self'" always;
    add_header Referrer-Policy "strict-origin-when-cross-origin" always;
    
    # 프록시 설정
    location / {
        proxy_pass http://localhost:9999;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}

# HTTP → HTTPS 리다이렉트
server {
    listen 80;
    server_name logcaster.example.com;
    return 301 https://$server_name$request_uri;
}
```

---

## 7. 인증서 백업 및 복구

### 7.1 인증서 백업

```bash
#!/bin/bash
# backup-certificates.sh

BACKUP_DIR="/backup/certificates/$(date +%Y%m%d)"
mkdir -p $BACKUP_DIR

# Let's Encrypt 인증서 백업
tar -czf $BACKUP_DIR/letsencrypt.tar.gz /etc/letsencrypt/

# 커스텀 인증서 백업
tar -czf $BACKUP_DIR/custom-certs.tar.gz /etc/ssl/logcaster/

# 암호화 백업
openssl enc -aes-256-cbc -salt \
    -in $BACKUP_DIR/letsencrypt.tar.gz \
    -out $BACKUP_DIR/letsencrypt.tar.gz.enc \
    -k $BACKUP_PASSWORD

# 원격 백업
rsync -avz $BACKUP_DIR/ backup-server:/backups/certificates/

echo "✅ 인증서 백업 완료: $BACKUP_DIR"
```

### 7.2 인증서 복구

```bash
#!/bin/bash
# restore-certificates.sh

RESTORE_DATE=${1:-$(date +%Y%m%d)}
BACKUP_DIR="/backup/certificates/$RESTORE_DATE"

echo "🔐 인증서 복구 시작..."

# 복호화
openssl enc -d -aes-256-cbc \
    -in $BACKUP_DIR/letsencrypt.tar.gz.enc \
    -out $BACKUP_DIR/letsencrypt.tar.gz \
    -k $BACKUP_PASSWORD

# 복원
tar -xzf $BACKUP_DIR/letsencrypt.tar.gz -C /
tar -xzf $BACKUP_DIR/custom-certs.tar.gz -C /

# 권한 복원
chmod 600 /etc/letsencrypt/live/*/privkey.pem
chmod 644 /etc/letsencrypt/live/*/fullchain.pem

# 서비스 재시작
systemctl reload nginx
systemctl restart logcaster

echo "✅ 인증서 복구 완료"
```

---

## 8. 문제 해결

### 8.1 일반적인 문제

```bash
#!/bin/bash
# troubleshoot-ssl.sh

echo "🔍 SSL/TLS 문제 진단..."

# 인증서 검증
openssl x509 -in server.crt -text -noout

# 인증서 체인 검증
openssl verify -CAfile ca.crt server.crt

# SSL 연결 테스트
openssl s_client -connect logcaster.example.com:443 \
    -servername logcaster.example.com

# 프로토콜 테스트
for PROTOCOL in ssl3 tls1 tls1_1 tls1_2 tls1_3; do
    echo "Testing $PROTOCOL..."
    openssl s_client -connect logcaster.example.com:443 \
        -$PROTOCOL 2>&1 | grep -E "Protocol|Cipher"
done

# SSL Labs 테스트
echo "SSL Labs 테스트: https://www.ssllabs.com/ssltest/analyze.html?d=logcaster.example.com"
```

---

## ✅ 체크리스트

### 필수 확인사항
- [ ] 인증서 발급 완료
- [ ] TLS 1.2 이상만 허용
- [ ] 강력한 암호 스위트 설정
- [ ] 인증서 자동 갱신 설정
- [ ] HTTPS 리다이렉트 설정
- [ ] 보안 헤더 구성

### 권장사항
- [ ] OCSP Stapling 활성화
- [ ] Perfect Forward Secrecy
- [ ] CAA DNS 레코드 설정
- [ ] HSTS Preload 등록
- [ ] 인증서 투명성 로그
- [ ] HSM 사용 (엔터프라이즈)

---

## 🎯 다음 단계
→ [09_testing_strategy.md](09_testing_strategy.md) - 테스팅 전략