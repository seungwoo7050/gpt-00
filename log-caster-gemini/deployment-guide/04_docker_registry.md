# 04. Docker 레지스트리 설정

## 🎯 목표
프라이빗 Docker 레지스트리를 구축하고 이미지 관리 전략을 수립합니다.

## 📋 Prerequisites
- Docker 20.10+ 설치
- 충분한 스토리지 공간 (최소 50GB)
- HTTPS 인증서 (프로덕션용)
- 클라우드 스토리지 계정 (선택사항)

---

## 1. Docker Hub 사용

### 1.1 Docker Hub 설정

```bash
# Docker Hub 로그인
docker login

# 이미지 태깅
docker tag logcaster-c:latest yourusername/logcaster-c:latest
docker tag logcaster-cpp:latest yourusername/logcaster-cpp:latest

# 이미지 푸시
docker push yourusername/logcaster-c:latest
docker push yourusername/logcaster-cpp:latest
```

### 1.2 자동화된 빌드 설정

```yaml
# docker-hub-build.yml
version: 1
build:
  logcaster-c:
    context: ./LogCaster-C
    dockerfile: Dockerfile
    tags:
      - latest
      - "{sourceref}"
  logcaster-cpp:
    context: ./LogCaster-CPP
    dockerfile: Dockerfile
    tags:
      - latest
      - "{sourceref}"
```

---

## 2. 프라이빗 레지스트리 구축

### 2.1 기본 레지스트리 설정

```yaml
# docker-compose-registry.yml
version: '3.8'

services:
  registry:
    image: registry:2
    container_name: docker-registry
    restart: always
    ports:
      - "5000:5000"
    environment:
      REGISTRY_HTTP_TLS_CERTIFICATE: /certs/domain.crt
      REGISTRY_HTTP_TLS_KEY: /certs/domain.key
      REGISTRY_AUTH: htpasswd
      REGISTRY_AUTH_HTPASSWD_PATH: /auth/htpasswd
      REGISTRY_AUTH_HTPASSWD_REALM: Registry Realm
      REGISTRY_STORAGE_DELETE_ENABLED: "true"
    volumes:
      - ./registry/data:/var/lib/registry
      - ./registry/certs:/certs
      - ./registry/auth:/auth
    networks:
      - registry-net

  registry-ui:
    image: joxit/docker-registry-ui:latest
    container_name: registry-ui
    restart: always
    ports:
      - "8080:80"
    environment:
      REGISTRY_TITLE: LogCaster Registry
      REGISTRY_URL: https://registry.logcaster.local:5000
      DELETE_IMAGES: "true"
      SINGLE_REGISTRY: "true"
    depends_on:
      - registry
    networks:
      - registry-net

networks:
  registry-net:
    driver: bridge
```

### 2.2 인증 설정

```bash
# htpasswd 파일 생성
mkdir -p registry/auth
docker run --rm --entrypoint htpasswd \
  httpd:2 -Bbn admin yourpassword > registry/auth/htpasswd

# 추가 사용자 생성
docker run --rm --entrypoint htpasswd \
  httpd:2 -Bbn developer devpassword >> registry/auth/htpasswd
```

### 2.3 TLS/SSL 설정

```bash
# 자체 서명 인증서 생성 (개발용)
mkdir -p registry/certs
openssl req -newkey rsa:4096 -nodes -sha256 \
  -keyout registry/certs/domain.key \
  -x509 -days 365 \
  -out registry/certs/domain.crt \
  -subj "/C=KR/ST=Seoul/L=Seoul/O=LogCaster/CN=registry.logcaster.local"

# Let's Encrypt 인증서 (프로덕션용)
certbot certonly --standalone \
  -d registry.logcaster.com \
  --agree-tos \
  --email admin@logcaster.com
```

---

## 3. 클라우드 레지스트리

### 3.1 AWS ECR

```bash
# ECR 레포지토리 생성
aws ecr create-repository \
  --repository-name logcaster-c \
  --image-scanning-configuration scanOnPush=true \
  --region ap-northeast-2

aws ecr create-repository \
  --repository-name logcaster-cpp \
  --image-scanning-configuration scanOnPush=true \
  --region ap-northeast-2

# Docker 로그인
aws ecr get-login-password --region ap-northeast-2 | \
  docker login --username AWS \
  --password-stdin 123456789012.dkr.ecr.ap-northeast-2.amazonaws.com

# 이미지 푸시
docker tag logcaster-c:latest \
  123456789012.dkr.ecr.ap-northeast-2.amazonaws.com/logcaster-c:latest
docker push 123456789012.dkr.ecr.ap-northeast-2.amazonaws.com/logcaster-c:latest
```

### 3.2 Google Container Registry (GCR)

```bash
# GCR 인증 설정
gcloud auth configure-docker

# 이미지 태깅 및 푸시
docker tag logcaster-c:latest gcr.io/project-id/logcaster-c:latest
docker push gcr.io/project-id/logcaster-c:latest

docker tag logcaster-cpp:latest gcr.io/project-id/logcaster-cpp:latest
docker push gcr.io/project-id/logcaster-cpp:latest
```

### 3.3 Azure Container Registry (ACR)

```bash
# ACR 생성
az acr create \
  --resource-group logcaster-rg \
  --name logcasterregistry \
  --sku Basic

# 로그인
az acr login --name logcasterregistry

# 이미지 푸시
docker tag logcaster-c:latest \
  logcasterregistry.azurecr.io/logcaster-c:latest
docker push logcasterregistry.azurecr.io/logcaster-c:latest
```

---

## 4. Harbor 엔터프라이즈 레지스트리

### 4.1 Harbor 설치

```yaml
# harbor-values.yaml
expose:
  type: ingress
  tls:
    enabled: true
    certSource: secret
    secret:
      secretName: harbor-tls
  ingress:
    hosts:
      core: harbor.logcaster.com
      notary: notary.logcaster.com

externalURL: https://harbor.logcaster.com

persistence:
  enabled: true
  persistentVolumeClaim:
    registry:
      size: 100Gi
    chartmuseum:
      size: 5Gi
    database:
      size: 10Gi
    redis:
      size: 1Gi

harborAdminPassword: "Harbor12345"

database:
  type: internal

redis:
  type: internal

trivy:
  enabled: true
```

```bash
# Helm을 사용한 Harbor 설치
helm repo add harbor https://helm.goharbor.io
helm install harbor harbor/harbor \
  -f harbor-values.yaml \
  --namespace harbor \
  --create-namespace
```

### 4.2 Harbor 프로젝트 설정

```bash
# Harbor CLI 사용
curl -X POST "https://harbor.logcaster.com/api/v2.0/projects" \
  -H "Content-Type: application/json" \
  -H "Authorization: Basic $(echo -n admin:Harbor12345 | base64)" \
  -d '{
    "project_name": "logcaster",
    "public": false,
    "metadata": {
      "enable_content_trust": "true",
      "auto_scan": "true",
      "severity": "high",
      "reuse_sys_cve_whitelist": "true"
    }
  }'
```

---

## 5. 이미지 관리 전략

### 5.1 태깅 전략

```bash
# 태깅 규칙
# 1. 최신 개발 버전
docker tag logcaster-c:build logcaster/logcaster-c:dev

# 2. 스테이징 버전
docker tag logcaster-c:build logcaster/logcaster-c:staging

# 3. 프로덕션 버전 (시맨틱 버저닝)
docker tag logcaster-c:build logcaster/logcaster-c:1.2.3

# 4. Git 커밋 해시
docker tag logcaster-c:build logcaster/logcaster-c:git-abc123

# 5. 날짜 기반
docker tag logcaster-c:build logcaster/logcaster-c:2024-01-15
```

### 5.2 이미지 정리 정책

```yaml
# cleanup-policy.yaml
apiVersion: batch/v1
kind: CronJob
metadata:
  name: registry-cleanup
spec:
  schedule: "0 2 * * *"  # 매일 새벽 2시
  jobTemplate:
    spec:
      template:
        spec:
          containers:
          - name: cleanup
            image: registry:2
            command:
            - /bin/registry
            - garbage-collect
            - /etc/docker/registry/config.yml
            volumeMounts:
            - name: config
              mountPath: /etc/docker/registry
          volumes:
          - name: config
            configMap:
              name: registry-config
          restartPolicy: OnFailure
```

### 5.3 보존 정책

```json
// retention-policy.json
{
  "rules": [
    {
      "tag_selectors": [
        {
          "kind": "doublestar",
          "pattern": "**"
        }
      ],
      "scope_selectors": {
        "repository": [
          {
            "kind": "doublestar",
            "pattern": "**"
          }
        ]
      },
      "action": "retain",
      "params": {
        "latestPushedK": 10,
        "latestPulledN": 5
      }
    }
  ]
}
```

---

## 6. 보안 스캔 및 서명

### 6.1 Trivy 스캔 통합

```bash
# 이미지 스캔
trivy image logcaster/logcaster-c:latest

# CI/CD 파이프라인에 통합
docker run --rm \
  -v /var/run/docker.sock:/var/run/docker.sock \
  aquasec/trivy:latest \
  image --exit-code 1 --severity HIGH,CRITICAL \
  logcaster/logcaster-c:latest
```

### 6.2 이미지 서명 (Cosign)

```bash
# Cosign 설치
curl -O -L https://github.com/sigstore/cosign/releases/latest/download/cosign-linux-amd64
chmod +x cosign-linux-amd64
sudo mv cosign-linux-amd64 /usr/local/bin/cosign

# 키 생성
cosign generate-key-pair

# 이미지 서명
cosign sign --key cosign.key logcaster/logcaster-c:latest

# 서명 검증
cosign verify --key cosign.pub logcaster/logcaster-c:latest
```

---

## 7. 미러링 및 복제

### 7.1 레지스트리 미러링

```yaml
# mirror-config.yml
version: 0.1
log:
  level: info
storage:
  filesystem:
    rootdirectory: /var/lib/registry
http:
  addr: :5000
proxy:
  remoteurl: https://registry-1.docker.io
  username: dockerhub_username
  password: dockerhub_password
```

### 7.2 다중 레지스트리 동기화

```bash
# Skopeo를 사용한 이미지 동기화
skopeo sync \
  --src docker --dest docker \
  registry.source.com/logcaster \
  registry.target.com/logcaster
```

---

## 8. 모니터링 및 알림

### 8.1 Prometheus 메트릭

```yaml
# prometheus-config.yml
global:
  scrape_interval: 15s

scrape_configs:
  - job_name: 'docker-registry'
    static_configs:
      - targets: ['registry:5001']
    metrics_path: '/metrics'
```

### 8.2 Grafana 대시보드

```json
{
  "dashboard": {
    "title": "Docker Registry Metrics",
    "panels": [
      {
        "title": "Push/Pull Rate",
        "targets": [
          {
            "expr": "rate(registry_http_requests_total[5m])"
          }
        ]
      },
      {
        "title": "Storage Usage",
        "targets": [
          {
            "expr": "registry_storage_blob_bytes"
          }
        ]
      }
    ]
  }
}
```

---

## ✅ 체크리스트

### 필수 확인사항
- [ ] 레지스트리 선택 (Docker Hub/프라이빗/클라우드)
- [ ] 인증 메커니즘 설정
- [ ] TLS/SSL 구성
- [ ] 이미지 태깅 전략 수립
- [ ] 백업 정책 설정
- [ ] 접근 권한 관리

### 권장사항
- [ ] 이미지 스캔 도구 통합
- [ ] 이미지 서명 구현
- [ ] 정리 정책 설정
- [ ] 미러링/복제 구성
- [ ] 모니터링 대시보드
- [ ] 알림 설정

---

## 🎯 다음 단계
→ [05_monitoring_setup.md](05_monitoring_setup.md) - 모니터링 시스템 구축