# 01. 인프라 구축 가이드

## 🎯 목표
LogCaster를 프로덕션 환경에 배포하기 위한 기본 인프라를 구축합니다.

## 📋 Prerequisites
- Docker 20.10+ 설치
- docker-compose 1.29+ 설치
- 최소 4GB RAM, 20GB 디스크 공간
- Linux 또는 macOS 환경

---

## 1. Docker 컨테이너화

### 1.1 LogCaster-C Dockerfile

```dockerfile
# Dockerfile.c
# 멀티스테이지 빌드로 최종 이미지 크기 최소화
FROM ubuntu:22.04 AS builder

# 빌드 의존성 설치
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# 소스 코드 복사
WORKDIR /build
COPY LogCaster-C/ .

# 빌드 수행
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

# 런타임 이미지
FROM ubuntu:22.04

# 런타임 의존성만 설치
RUN apt-get update && apt-get install -y \
    libpthread-stubs0-dev \
    && rm -rf /var/lib/apt/lists/*

# 실행 파일 복사
COPY --from=builder /build/build/logcaster-c /usr/local/bin/

# 로그 디렉토리 생성
RUN mkdir -p /var/log/logcaster

# 비root 사용자 생성
RUN useradd -m -s /bin/bash logcaster
USER logcaster

# 포트 노출
EXPOSE 9999 9998

# 헬스체크
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD echo "PING" | nc localhost 9998 || exit 1

# 실행
ENTRYPOINT ["logcaster-c"]
CMD ["-P", "-d", "/var/log/logcaster"]
```

### 1.2 LogCaster-CPP Dockerfile

```dockerfile
# Dockerfile.cpp
FROM ubuntu:22.04 AS builder

# 빌드 의존성 설치
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# 소스 코드 복사
WORKDIR /build
COPY LogCaster-CPP/ .

# 빌드 수행
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

# 런타임 이미지
FROM ubuntu:22.04

# 런타임 의존성
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# 실행 파일 복사
COPY --from=builder /build/build/logcaster-cpp /usr/local/bin/

# 로그 디렉토리 생성
RUN mkdir -p /var/log/logcaster

# 비root 사용자 생성
RUN useradd -m -s /bin/bash logcaster
USER logcaster

# 포트 노출
EXPOSE 9999 9998 6667

# 헬스체크
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD echo "PING" | nc localhost 9998 || exit 1

# 실행
ENTRYPOINT ["logcaster-cpp"]
CMD ["-P", "-d", "/var/log/logcaster"]
```

### 1.3 Docker Compose 설정

```yaml
# docker-compose.yml
version: '3.8'

services:
  logcaster-c:
    build:
      context: .
      dockerfile: Dockerfile.c
    container_name: logcaster-c
    restart: unless-stopped
    ports:
      - "9999:9999"  # 로그 수신 포트
      - "9998:9998"  # 쿼리 포트
    volumes:
      - ./logs/c:/var/log/logcaster
      - ./config/c:/etc/logcaster
    environment:
      - LOG_LEVEL=INFO
      - MAX_CONNECTIONS=1000
      - BUFFER_SIZE=10000
      - PERSISTENCE_ENABLED=true
    networks:
      - logcaster-net
    deploy:
      resources:
        limits:
          cpus: '2'
          memory: 1G
        reservations:
          cpus: '0.5'
          memory: 256M

  logcaster-cpp:
    build:
      context: .
      dockerfile: Dockerfile.cpp
    container_name: logcaster-cpp
    restart: unless-stopped
    ports:
      - "8999:9999"  # 로그 수신 포트 (다른 포트로 매핑)
      - "8998:9998"  # 쿼리 포트
      - "6667:6667"  # IRC 포트
    volumes:
      - ./logs/cpp:/var/log/logcaster
      - ./config/cpp:/etc/logcaster
    environment:
      - LOG_LEVEL=INFO
      - MAX_CONNECTIONS=1000
      - BUFFER_SIZE=10000
      - PERSISTENCE_ENABLED=true
      - IRC_ENABLED=true
    networks:
      - logcaster-net
    deploy:
      resources:
        limits:
          cpus: '2'
          memory: 1G
        reservations:
          cpus: '0.5'
          memory: 256M

  # 모니터링 사이드카 컨테이너
  prometheus-exporter:
    image: prom/node-exporter:latest
    container_name: node-exporter
    restart: unless-stopped
    ports:
      - "9100:9100"
    networks:
      - logcaster-net
    volumes:
      - /proc:/host/proc:ro
      - /sys:/host/sys:ro
      - /:/rootfs:ro
    command:
      - '--path.procfs=/host/proc'
      - '--path.sysfs=/host/sys'
      - '--collector.filesystem.mount-points-exclude=^/(sys|proc|dev|host|etc)($$|/)'

networks:
  logcaster-net:
    driver: bridge
    ipam:
      config:
        - subnet: 172.20.0.0/16
```

---

## 2. 환경별 설정

### 2.1 개발 환경 설정

```yaml
# docker-compose.dev.yml
version: '3.8'

services:
  logcaster-c:
    build:
      context: .
      dockerfile: Dockerfile.c
      args:
        BUILD_TYPE: Debug
    environment:
      - LOG_LEVEL=DEBUG
      - ENABLE_PROFILING=true
    volumes:
      - ./LogCaster-C/src:/usr/src/app
    ports:
      - "9999:9999"
      - "9998:9998"
      - "4000:4000"  # 디버거 포트

  logcaster-cpp:
    build:
      context: .
      dockerfile: Dockerfile.cpp
      args:
        BUILD_TYPE: Debug
    environment:
      - LOG_LEVEL=DEBUG
      - ENABLE_PROFILING=true
    volumes:
      - ./LogCaster-CPP/src:/usr/src/app
    ports:
      - "8999:9999"
      - "8998:9998"
      - "6667:6667"
      - "4001:4000"  # 디버거 포트
```

### 2.2 프로덕션 환경 설정

```yaml
# docker-compose.prod.yml
version: '3.8'

services:
  logcaster-c:
    image: logcaster/logcaster-c:latest
    deploy:
      replicas: 3
      update_config:
        parallelism: 1
        delay: 10s
      restart_policy:
        condition: on-failure
        delay: 5s
        max_attempts: 3
    environment:
      - LOG_LEVEL=WARN
      - MAX_CONNECTIONS=5000
      - BUFFER_SIZE=50000

  logcaster-cpp:
    image: logcaster/logcaster-cpp:latest
    deploy:
      replicas: 3
      update_config:
        parallelism: 1
        delay: 10s
      restart_policy:
        condition: on-failure
        delay: 5s
        max_attempts: 3
    environment:
      - LOG_LEVEL=WARN
      - MAX_CONNECTIONS=5000
      - BUFFER_SIZE=50000
```

---

## 3. 네트워크 구성

### 3.1 방화벽 규칙

```bash
# UFW 설정 (Ubuntu)
sudo ufw allow 9999/tcp comment 'LogCaster-C Log Input'
sudo ufw allow 9998/tcp comment 'LogCaster-C Query'
sudo ufw allow 8999/tcp comment 'LogCaster-CPP Log Input'
sudo ufw allow 8998/tcp comment 'LogCaster-CPP Query'
sudo ufw allow 6667/tcp comment 'LogCaster-CPP IRC'
sudo ufw allow 9100/tcp comment 'Prometheus Node Exporter'
```

### 3.2 로드 밸런서 설정 (nginx)

```nginx
# /etc/nginx/sites-available/logcaster
upstream logcaster_c {
    least_conn;
    server 127.0.0.1:9999 max_fails=3 fail_timeout=30s;
    server 127.0.0.1:19999 max_fails=3 fail_timeout=30s backup;
}

upstream logcaster_cpp {
    least_conn;
    server 127.0.0.1:8999 max_fails=3 fail_timeout=30s;
    server 127.0.0.1:18999 max_fails=3 fail_timeout=30s backup;
}

# TCP 스트림 프록시
stream {
    server {
        listen 10999;
        proxy_pass logcaster_c;
        proxy_connect_timeout 1s;
        proxy_timeout 300s;
    }
    
    server {
        listen 10998;
        proxy_pass logcaster_cpp;
        proxy_connect_timeout 1s;
        proxy_timeout 300s;
    }
}
```

---

## 4. 스토리지 설정

### 4.1 볼륨 마운트 구조

```bash
# 디렉토리 구조 생성
mkdir -p /opt/logcaster/{logs,config,data}/{c,cpp}
mkdir -p /opt/logcaster/backup

# 권한 설정
chown -R 1000:1000 /opt/logcaster
chmod -R 755 /opt/logcaster
```

### 4.2 로그 로테이션 설정

```bash
# /etc/logrotate.d/logcaster
/opt/logcaster/logs/*/*.log {
    daily
    rotate 30
    compress
    delaycompress
    missingok
    notifempty
    create 0644 logcaster logcaster
    sharedscripts
    postrotate
        docker exec logcaster-c kill -USR1 1
        docker exec logcaster-cpp kill -USR1 1
    endscript
}
```

---

## 5. 빌드 및 배포 스크립트

### 5.1 setup.sh

```bash
#!/bin/bash
# setup.sh - 초기 환경 설정 스크립트

set -e

echo "🚀 LogCaster 인프라 설정 시작..."

# 디렉토리 생성
echo "📁 디렉토리 구조 생성..."
mkdir -p /opt/logcaster/{logs,config,data,backup}/{c,cpp}

# Docker 이미지 빌드
echo "🔨 Docker 이미지 빌드..."
docker-compose build --parallel

# 네트워크 생성
echo "🌐 Docker 네트워크 생성..."
docker network create logcaster-net 2>/dev/null || true

# 설정 파일 복사
echo "📋 설정 파일 복사..."
cp -r config/* /opt/logcaster/config/

# 권한 설정
echo "🔒 권한 설정..."
chown -R 1000:1000 /opt/logcaster
chmod -R 755 /opt/logcaster

echo "✅ 인프라 설정 완료!"
```

### 5.2 deploy.sh

```bash
#!/bin/bash
# deploy.sh - 배포 스크립트

set -e

ENV=${1:-dev}

echo "🚀 LogCaster 배포 시작 (환경: $ENV)..."

# 이전 컨테이너 정지
echo "🛑 기존 컨테이너 정지..."
docker-compose down

# 환경별 배포
if [ "$ENV" = "prod" ]; then
    echo "🚀 프로덕션 배포..."
    docker-compose -f docker-compose.yml -f docker-compose.prod.yml up -d
else
    echo "🚀 개발 환경 배포..."
    docker-compose -f docker-compose.yml -f docker-compose.dev.yml up -d
fi

# 헬스체크
echo "❤️ 헬스체크 수행..."
sleep 5
./health-check.sh

echo "✅ 배포 완료!"
```

---

## ✅ 체크리스트

### 필수 확인사항
- [ ] Docker 및 docker-compose 설치 확인
- [ ] 필요한 포트가 열려있는지 확인
- [ ] 디렉토리 구조 및 권한 설정 완료
- [ ] Docker 이미지 빌드 성공
- [ ] 컨테이너 정상 실행 확인
- [ ] 헬스체크 통과

### 권장사항
- [ ] 로그 로테이션 설정
- [ ] 백업 디렉토리 설정
- [ ] 모니터링 컨테이너 실행
- [ ] 네트워크 보안 그룹 설정

---

## 🎯 다음 단계
→ [02_cloud_resources.md](02_cloud_resources.md) - 클라우드 리소스 설정