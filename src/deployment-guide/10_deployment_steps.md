# 10. 실제 배포 단계

## 🎯 목표
LogCaster를 프로덕션 환경에 안전하고 체계적으로 배포하기 위한 단계별 가이드입니다.

## 📋 Prerequisites
- 모든 인프라 준비 완료
- CI/CD 파이프라인 구성 완료
- 보안 체크리스트 통과
- 백업 시스템 준비

---

## 1. 배포 전 준비

### 1.1 체크리스트 확인

```bash
#!/bin/bash
# pre-deployment-check.sh

echo "🔍 배포 전 체크리스트 확인 중..."

# 필수 도구 확인
check_tool() {
    if ! command -v $1 &> /dev/null; then
        echo "❌ $1이 설치되지 않았습니다."
        exit 1
    fi
    echo "✅ $1 확인됨"
}

check_tool docker
check_tool docker-compose
check_tool kubectl
check_tool helm

# 환경 변수 확인
check_env() {
    if [ -z "${!1}" ]; then
        echo "❌ 환경 변수 $1이 설정되지 않았습니다."
        exit 1
    fi
    echo "✅ $1 설정됨"
}

check_env DOCKER_REGISTRY
check_env DEPLOYMENT_ENV
check_env AWS_REGION

# 디스크 공간 확인
DISK_USAGE=$(df -h / | awk 'NR==2 {print $5}' | sed 's/%//')
if [ $DISK_USAGE -gt 80 ]; then
    echo "⚠️ 디스크 사용률이 높습니다: ${DISK_USAGE}%"
fi

# 네트워크 연결 확인
if ! ping -c 1 google.com &> /dev/null; then
    echo "❌ 네트워크 연결 실패"
    exit 1
fi

echo "✅ 모든 사전 체크 통과"
```

### 1.2 백업 수행

```bash
#!/bin/bash
# backup-before-deploy.sh

BACKUP_DIR="/backup/$(date +%Y%m%d_%H%M%S)"
mkdir -p $BACKUP_DIR

echo "📦 배포 전 백업 시작..."

# 데이터베이스 백업 (있는 경우)
if [ -f /var/lib/logcaster/data.db ]; then
    cp -r /var/lib/logcaster $BACKUP_DIR/
    echo "✅ 데이터 백업 완료"
fi

# 설정 파일 백업
cp -r /etc/logcaster $BACKUP_DIR/config
echo "✅ 설정 파일 백업 완료"

# 현재 컨테이너 이미지 백업
docker save logcaster/logcaster-c:current > $BACKUP_DIR/logcaster-c.tar
docker save logcaster/logcaster-cpp:current > $BACKUP_DIR/logcaster-cpp.tar
echo "✅ Docker 이미지 백업 완료"

# 백업 검증
tar -czf $BACKUP_DIR.tar.gz $BACKUP_DIR
echo "✅ 백업 아카이브 생성: $BACKUP_DIR.tar.gz"
```

---

## 2. 스테이징 배포

### 2.1 스테이징 환경 설정

```yaml
# staging-deploy.yaml
apiVersion: v1
kind: Namespace
metadata:
  name: staging
---
apiVersion: apps/v1
kind: Deployment
metadata:
  name: logcaster-staging
  namespace: staging
spec:
  replicas: 2
  strategy:
    type: RollingUpdate
    rollingUpdate:
      maxSurge: 1
      maxUnavailable: 0
  selector:
    matchLabels:
      app: logcaster
      env: staging
  template:
    metadata:
      labels:
        app: logcaster
        env: staging
    spec:
      containers:
      - name: logcaster-c
        image: logcaster/logcaster-c:staging
        ports:
        - containerPort: 9999
        - containerPort: 9998
        env:
        - name: ENV
          value: "staging"
        - name: LOG_LEVEL
          value: "DEBUG"
        livenessProbe:
          tcpSocket:
            port: 9999
          initialDelaySeconds: 30
          periodSeconds: 10
        readinessProbe:
          httpGet:
            path: /health
            port: 9998
          initialDelaySeconds: 5
          periodSeconds: 5
        resources:
          requests:
            memory: "256Mi"
            cpu: "250m"
          limits:
            memory: "512Mi"
            cpu: "500m"
```

### 2.2 스테이징 배포 스크립트

```bash
#!/bin/bash
# deploy-staging.sh

echo "🚀 스테이징 환경 배포 시작..."

# 이미지 빌드 및 푸시
VERSION=$(git rev-parse --short HEAD)
docker build -f Dockerfile.c -t logcaster/logcaster-c:$VERSION .
docker build -f Dockerfile.cpp -t logcaster/logcaster-cpp:$VERSION .

docker tag logcaster/logcaster-c:$VERSION logcaster/logcaster-c:staging
docker tag logcaster/logcaster-cpp:$VERSION logcaster/logcaster-cpp:staging

docker push logcaster/logcaster-c:staging
docker push logcaster/logcaster-cpp:staging

# Kubernetes 배포
kubectl apply -f staging-deploy.yaml

# 배포 상태 확인
kubectl rollout status deployment/logcaster-staging -n staging

# 스모크 테스트
echo "🧪 스모크 테스트 실행..."
python3 tests/smoke_test.py --env staging

if [ $? -eq 0 ]; then
    echo "✅ 스테이징 배포 성공"
else
    echo "❌ 스테이징 배포 실패"
    kubectl rollout undo deployment/logcaster-staging -n staging
    exit 1
fi
```

---

## 3. 카나리 배포

### 3.1 카나리 배포 설정

```yaml
# canary-deployment.yaml
apiVersion: v1
kind: Service
metadata:
  name: logcaster-service
spec:
  selector:
    app: logcaster
  ports:
  - name: log-input
    port: 9999
  - name: query
    port: 9998
---
# 안정 버전 (90% 트래픽)
apiVersion: apps/v1
kind: Deployment
metadata:
  name: logcaster-stable
spec:
  replicas: 9
  selector:
    matchLabels:
      app: logcaster
      version: stable
  template:
    metadata:
      labels:
        app: logcaster
        version: stable
    spec:
      containers:
      - name: logcaster
        image: logcaster/logcaster-c:v1.0.0
---
# 카나리 버전 (10% 트래픽)
apiVersion: apps/v1
kind: Deployment
metadata:
  name: logcaster-canary
spec:
  replicas: 1
  selector:
    matchLabels:
      app: logcaster
      version: canary
  template:
    metadata:
      labels:
        app: logcaster
        version: canary
    spec:
      containers:
      - name: logcaster
        image: logcaster/logcaster-c:v1.1.0
```

### 3.2 카나리 배포 진행

```bash
#!/bin/bash
# canary-deploy.sh

echo "🐤 카나리 배포 시작..."

# 초기 카나리 배포 (10% 트래픽)
kubectl set image deployment/logcaster-canary \
    logcaster=logcaster/logcaster-c:$NEW_VERSION

# 메트릭 모니터링
echo "📊 메트릭 모니터링 중... (5분)"
sleep 300

# 성공률 체크
SUCCESS_RATE=$(curl -s http://prometheus:9090/api/v1/query \
    --data-urlencode 'query=rate(http_requests_total{version="canary",status="200"}[5m])' \
    | jq '.data.result[0].value[1]' | tr -d '"')

ERROR_RATE=$(curl -s http://prometheus:9090/api/v1/query \
    --data-urlencode 'query=rate(http_requests_total{version="canary",status!="200"}[5m])' \
    | jq '.data.result[0].value[1]' | tr -d '"')

if (( $(echo "$ERROR_RATE > 0.01" | bc -l) )); then
    echo "❌ 에러율이 임계값을 초과했습니다. 롤백합니다."
    kubectl delete deployment logcaster-canary
    exit 1
fi

# 점진적 롤아웃
echo "📈 카나리 트래픽 증가 (25%)"
kubectl scale deployment logcaster-canary --replicas=3
kubectl scale deployment logcaster-stable --replicas=7
sleep 300

echo "📈 카나리 트래픽 증가 (50%)"
kubectl scale deployment logcaster-canary --replicas=5
kubectl scale deployment logcaster-stable --replicas=5
sleep 300

echo "📈 카나리 트래픽 증가 (100%)"
kubectl scale deployment logcaster-canary --replicas=10
kubectl scale deployment logcaster-stable --replicas=0

echo "✅ 카나리 배포 완료"
```

---

## 4. 블루-그린 배포

### 4.1 블루-그린 설정

```bash
#!/bin/bash
# blue-green-deploy.sh

CURRENT_ENV=$(kubectl get service logcaster-active -o jsonpath='{.spec.selector.environment}')
echo "현재 활성 환경: $CURRENT_ENV"

if [ "$CURRENT_ENV" == "blue" ]; then
    NEW_ENV="green"
else
    NEW_ENV="blue"
fi

echo "🔄 $NEW_ENV 환경으로 배포 시작..."

# 새 환경에 배포
kubectl set image deployment/logcaster-$NEW_ENV \
    logcaster=logcaster/logcaster-c:$NEW_VERSION

# 헬스체크 대기
kubectl wait --for=condition=ready pod \
    -l app=logcaster,environment=$NEW_ENV \
    --timeout=300s

# 테스트 실행
echo "🧪 새 환경 테스트..."
TEST_ENDPOINT=$(kubectl get service logcaster-$NEW_ENV -o jsonpath='{.status.loadBalancer.ingress[0].ip}')
python3 tests/integration_test.py --endpoint $TEST_ENDPOINT

if [ $? -ne 0 ]; then
    echo "❌ 테스트 실패. 배포 중단."
    exit 1
fi

# 트래픽 전환
echo "🔀 트래픽을 $NEW_ENV 환경으로 전환..."
kubectl patch service logcaster-active \
    -p '{"spec":{"selector":{"environment":"'$NEW_ENV'"}}}'

echo "✅ 블루-그린 배포 완료. 새 활성 환경: $NEW_ENV"

# 이전 환경은 롤백을 위해 유지
echo "💡 이전 환경($CURRENT_ENV)은 롤백을 위해 유지됩니다."
```

---

## 5. 프로덕션 배포

### 5.1 최종 체크리스트

```bash
#!/bin/bash
# final-checks.sh

echo "🔍 프로덕션 배포 최종 체크..."

# 스테이징 테스트 결과 확인
if [ ! -f "staging-test-passed.flag" ]; then
    echo "❌ 스테이징 테스트를 통과하지 못했습니다."
    exit 1
fi

# 보안 스캔 결과 확인
VULNERABILITIES=$(trivy image --severity HIGH,CRITICAL \
    --format json logcaster/logcaster-c:$VERSION \
    | jq '.Results[].Vulnerabilities | length')

if [ "$VULNERABILITIES" -gt 0 ]; then
    echo "❌ 보안 취약점이 발견되었습니다."
    exit 1
fi

# 승인 요청
echo "⚠️ 프로덕션 배포를 진행하시겠습니까? (yes/no)"
read APPROVAL

if [ "$APPROVAL" != "yes" ]; then
    echo "❌ 배포가 취소되었습니다."
    exit 1
fi

echo "✅ 모든 체크 통과. 프로덕션 배포를 시작합니다."
```

### 5.2 프로덕션 배포 실행

```bash
#!/bin/bash
# deploy-production.sh

set -e

echo "🚀 프로덕션 배포 시작..."
START_TIME=$(date +%s)

# 배포 알림
curl -X POST $SLACK_WEBHOOK_URL \
    -H 'Content-Type: application/json' \
    -d '{"text":"🚀 LogCaster 프로덕션 배포 시작 (v'$VERSION')"}'

# 데이터베이스 마이그레이션 (필요한 경우)
# kubectl exec -it db-pod -- psql -U admin -d logcaster -f /migrations/v$VERSION.sql

# 롤링 업데이트 시작
kubectl set image deployment/logcaster-prod \
    logcaster-c=logcaster/logcaster-c:$VERSION \
    logcaster-cpp=logcaster/logcaster-cpp:$VERSION \
    --record

# 배포 진행 모니터링
kubectl rollout status deployment/logcaster-prod --timeout=600s

# 헬스체크
for i in {1..10}; do
    echo "헬스체크 시도 $i/10..."
    if curl -f http://logcaster.production.com/health; then
        echo "✅ 헬스체크 통과"
        break
    fi
    sleep 10
done

# 성능 테스트
echo "🏃 성능 테스트 실행..."
ab -n 10000 -c 100 http://logcaster.production.com:9999/ > performance.txt

# 메트릭 확인
RESPONSE_TIME=$(curl -s http://prometheus:9090/api/v1/query \
    --data-urlencode 'query=histogram_quantile(0.95, http_request_duration_seconds_bucket)' \
    | jq '.data.result[0].value[1]')

if (( $(echo "$RESPONSE_TIME > 1.0" | bc -l) )); then
    echo "⚠️ 응답 시간이 느립니다: ${RESPONSE_TIME}s"
fi

END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))

# 배포 완료 알림
curl -X POST $SLACK_WEBHOOK_URL \
    -H 'Content-Type: application/json' \
    -d '{"text":"✅ LogCaster 프로덕션 배포 완료 (소요시간: '$DURATION'초)"}'

echo "✅ 프로덕션 배포 완료!"
```

---

## 6. 배포 후 작업

### 6.1 모니터링 강화

```bash
#!/bin/bash
# post-deployment-monitoring.sh

echo "📊 배포 후 모니터링 시작..."

# 실시간 로그 모니터링
kubectl logs -f deployment/logcaster-prod --all-containers=true &

# 메트릭 대시보드 열기
echo "📈 Grafana 대시보드: http://grafana.monitoring.com/d/logcaster"

# 알림 규칙 활성화
curl -X POST http://alertmanager:9093/api/v1/alerts \
    -H "Content-Type: application/json" \
    -d '[{
        "labels": {
            "alertname": "DeploymentComplete",
            "severity": "info",
            "version": "'$VERSION'"
        },
        "annotations": {
            "description": "LogCaster '$VERSION' deployed to production"
        }
    }]'
```

### 6.2 문서 업데이트

```bash
#!/bin/bash
# update-documentation.sh

echo "📝 배포 문서 업데이트..."

cat >> deployment-history.md <<EOF

## $(date '+%Y-%m-%d %H:%M:%S') - Version $VERSION

### 변경 사항
- $(git log --oneline -n 5)

### 배포 정보
- 환경: Production
- 배포자: $(whoami)
- 소요 시간: $DURATION 초
- 이전 버전: $PREVIOUS_VERSION

### 테스트 결과
- 헬스체크: ✅ 통과
- 성능 테스트: ✅ 통과
- 보안 스캔: ✅ 통과

---
EOF

git add deployment-history.md
git commit -m "Update deployment history for v$VERSION"
git push
```

---

## 7. 배포 자동화 스크립트

### 7.1 통합 배포 스크립트

```bash
#!/bin/bash
# deploy.sh - 통합 배포 스크립트

set -e

# 설정
VERSION=${1:-latest}
ENVIRONMENT=${2:-staging}
STRATEGY=${3:-rolling}  # rolling, canary, blue-green

echo "╔════════════════════════════════════╗"
echo "║     LogCaster 배포 시작           ║"
echo "╠════════════════════════════════════╣"
echo "║ Version: $VERSION                  ║"
echo "║ Environment: $ENVIRONMENT          ║"
echo "║ Strategy: $STRATEGY                ║"
echo "╚════════════════════════════════════╝"

# 사전 체크
./pre-deployment-check.sh

# 백업
./backup-before-deploy.sh

# 배포 전략 선택
case $STRATEGY in
    canary)
        ./canary-deploy.sh $VERSION $ENVIRONMENT
        ;;
    blue-green)
        ./blue-green-deploy.sh $VERSION $ENVIRONMENT
        ;;
    rolling|*)
        ./rolling-deploy.sh $VERSION $ENVIRONMENT
        ;;
esac

# 배포 후 작업
./post-deployment-monitoring.sh
./update-documentation.sh

echo "✅ 배포가 성공적으로 완료되었습니다!"
```

---

## ✅ 체크리스트

### 필수 확인사항
- [ ] 모든 테스트 통과
- [ ] 백업 완료
- [ ] 롤백 계획 준비
- [ ] 모니터링 설정
- [ ] 알림 채널 구성
- [ ] 문서 업데이트

### 권장사항
- [ ] 부하 테스트 실행
- [ ] 보안 스캔 완료
- [ ] 성능 메트릭 수집
- [ ] 사용자 공지
- [ ] 팀 브리핑

---

## 🎯 다음 단계
→ [11_rollback_plan.md](11_rollback_plan.md) - 롤백 계획