# 11. 롤백 계획

## 🎯 목표
배포 실패 시 신속하고 안전하게 이전 버전으로 롤백하기 위한 종합적인 계획과 절차입니다.

## 📋 Prerequisites
- 백업 시스템 구성
- 이전 버전 이미지 보관
- 롤백 권한 설정
- 모니터링 시스템 준비

---

## 1. 롤백 트리거 조건

### 1.1 자동 롤백 조건

```yaml
# rollback-rules.yaml
rollback_triggers:
  - name: high_error_rate
    condition: "error_rate > 5%"
    duration: "5m"
    action: immediate_rollback
    
  - name: health_check_failure
    condition: "health_check_fails > 3"
    duration: "2m"
    action: immediate_rollback
    
  - name: response_time_degradation
    condition: "p95_latency > 2s"
    duration: "10m"
    action: alert_then_rollback
    
  - name: memory_leak
    condition: "memory_usage > 90%"
    duration: "15m"
    action: gradual_rollback
    
  - name: crash_loop
    condition: "restart_count > 5"
    duration: "5m"
    action: immediate_rollback
```

### 1.2 모니터링 스크립트

```bash
#!/bin/bash
# monitor-rollback-triggers.sh

while true; do
    # 에러율 체크
    ERROR_RATE=$(curl -s http://prometheus:9090/api/v1/query \
        --data-urlencode 'query=rate(http_requests_total{status!="200"}[5m])' \
        | jq '.data.result[0].value[1]' | tr -d '"')
    
    if (( $(echo "$ERROR_RATE > 0.05" | bc -l) )); then
        echo "⚠️ 높은 에러율 감지: $ERROR_RATE"
        ./execute-rollback.sh "high_error_rate"
        break
    fi
    
    # 헬스체크
    if ! curl -f http://logcaster:9998/health > /dev/null 2>&1; then
        HEALTH_FAIL_COUNT=$((HEALTH_FAIL_COUNT + 1))
        if [ $HEALTH_FAIL_COUNT -gt 3 ]; then
            echo "⚠️ 헬스체크 실패"
            ./execute-rollback.sh "health_check_failure"
            break
        fi
    else
        HEALTH_FAIL_COUNT=0
    fi
    
    # 응답 시간 체크
    RESPONSE_TIME=$(curl -s -w "%{time_total}" -o /dev/null http://logcaster:9999)
    if (( $(echo "$RESPONSE_TIME > 2.0" | bc -l) )); then
        echo "⚠️ 느린 응답 시간: ${RESPONSE_TIME}s"
        SLOW_RESPONSE_COUNT=$((SLOW_RESPONSE_COUNT + 1))
        if [ $SLOW_RESPONSE_COUNT -gt 10 ]; then
            ./execute-rollback.sh "slow_response"
            break
        fi
    else
        SLOW_RESPONSE_COUNT=0
    fi
    
    sleep 30
done
```

---

## 2. 롤백 전략

### 2.1 즉시 롤백 (Immediate Rollback)

```bash
#!/bin/bash
# immediate-rollback.sh

echo "🚨 즉시 롤백 시작..."

# 현재 버전 저장
CURRENT_VERSION=$(kubectl get deployment logcaster-prod \
    -o jsonpath='{.spec.template.spec.containers[0].image}' | cut -d: -f2)

# 이전 버전 확인
PREVIOUS_VERSION=$(kubectl rollout history deployment/logcaster-prod \
    | tail -2 | head -1 | awk '{print $1}')

echo "현재 버전: $CURRENT_VERSION"
echo "롤백 대상 버전: $PREVIOUS_VERSION"

# 즉시 롤백 실행
kubectl rollout undo deployment/logcaster-prod

# 롤백 상태 확인
kubectl rollout status deployment/logcaster-prod

# 알림 발송
curl -X POST $SLACK_WEBHOOK_URL \
    -H 'Content-Type: application/json' \
    -d '{
        "text": "🚨 긴급 롤백 실행",
        "blocks": [{
            "type": "section",
            "text": {
                "type": "mrkdwn",
                "text": "*롤백 정보*\n• 원인: 즉시 롤백 조건 충족\n• 현재 버전: '$CURRENT_VERSION'\n• 롤백 버전: '$PREVIOUS_VERSION'"
            }
        }]
    }'

echo "✅ 즉시 롤백 완료"
```

### 2.2 점진적 롤백 (Gradual Rollback)

```bash
#!/bin/bash
# gradual-rollback.sh

echo "📉 점진적 롤백 시작..."

# 카나리 배포 역순으로 진행
CURRENT_REPLICAS=$(kubectl get deployment logcaster-prod -o jsonpath='{.spec.replicas}')
ROLLBACK_REPLICAS=1

# 이전 버전으로 새 deployment 생성
kubectl create deployment logcaster-rollback \
    --image=logcaster/logcaster-c:$PREVIOUS_VERSION \
    --replicas=$ROLLBACK_REPLICAS

# 점진적으로 트래픽 이동
for PERCENTAGE in 10 25 50 75 100; do
    echo "🔄 롤백 진행률: $PERCENTAGE%"
    
    NEW_REPLICAS=$((CURRENT_REPLICAS * PERCENTAGE / 100))
    OLD_REPLICAS=$((CURRENT_REPLICAS - NEW_REPLICAS))
    
    kubectl scale deployment logcaster-rollback --replicas=$NEW_REPLICAS
    kubectl scale deployment logcaster-prod --replicas=$OLD_REPLICAS
    
    # 메트릭 확인
    sleep 60
    ERROR_RATE=$(curl -s http://prometheus:9090/api/v1/query \
        --data-urlencode 'query=rate(http_requests_total{status!="200",version="rollback"}[1m])' \
        | jq '.data.result[0].value[1]' | tr -d '"')
    
    if (( $(echo "$ERROR_RATE > 0.01" | bc -l) )); then
        echo "⚠️ 롤백 버전도 문제가 있습니다. 다른 버전으로 롤백 시도..."
        ./fallback-to-stable.sh
        exit 1
    fi
done

# 최종 전환
kubectl delete deployment logcaster-prod
kubectl patch deployment logcaster-rollback -p '{"metadata":{"name":"logcaster-prod"}}'

echo "✅ 점진적 롤백 완료"
```

### 2.3 블루-그린 롤백

```bash
#!/bin/bash
# blue-green-rollback.sh

echo "🔄 블루-그린 롤백 시작..."

# 현재 활성 환경 확인
CURRENT_ENV=$(kubectl get service logcaster-active \
    -o jsonpath='{.spec.selector.environment}')

if [ "$CURRENT_ENV" == "blue" ]; then
    ROLLBACK_ENV="green"
else
    ROLLBACK_ENV="blue"
fi

echo "현재 환경: $CURRENT_ENV → 롤백 환경: $ROLLBACK_ENV"

# 롤백 환경 헬스체크
if ! kubectl get deployment logcaster-$ROLLBACK_ENV > /dev/null 2>&1; then
    echo "❌ 롤백 환경이 없습니다. 백업에서 복구..."
    ./restore-from-backup.sh
    exit 1
fi

# 트래픽 즉시 전환
kubectl patch service logcaster-active \
    -p '{"spec":{"selector":{"environment":"'$ROLLBACK_ENV'"}}}'

echo "✅ 블루-그린 롤백 완료"
```

---

## 3. 데이터 롤백

### 3.1 데이터베이스 롤백

```bash
#!/bin/bash
# database-rollback.sh

echo "🗄️ 데이터베이스 롤백 시작..."

# 현재 데이터 백업
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
pg_dump -h $DB_HOST -U $DB_USER -d logcaster > backup_before_rollback_$TIMESTAMP.sql

# 마이그레이션 롤백
for MIGRATION in $(ls -r migrations/*.down.sql); do
    echo "실행: $MIGRATION"
    psql -h $DB_HOST -U $DB_USER -d logcaster -f $MIGRATION
done

# 데이터 검증
RECORD_COUNT=$(psql -h $DB_HOST -U $DB_USER -d logcaster -t -c "SELECT COUNT(*) FROM logs")
echo "레코드 수: $RECORD_COUNT"

echo "✅ 데이터베이스 롤백 완료"
```

### 3.2 파일 시스템 롤백

```bash
#!/bin/bash
# filesystem-rollback.sh

echo "📁 파일 시스템 롤백 시작..."

BACKUP_DIR="/backup/last_stable"
PRODUCTION_DIR="/var/lib/logcaster"

# 현재 상태 백업
cp -r $PRODUCTION_DIR ${PRODUCTION_DIR}_rollback_backup

# 백업에서 복원
if [ -d "$BACKUP_DIR" ]; then
    rsync -av --delete $BACKUP_DIR/ $PRODUCTION_DIR/
    echo "✅ 파일 복원 완료"
else
    echo "❌ 백업 디렉토리를 찾을 수 없습니다"
    exit 1
fi

# 권한 복원
chown -R logcaster:logcaster $PRODUCTION_DIR
chmod -R 755 $PRODUCTION_DIR

echo "✅ 파일 시스템 롤백 완료"
```

---

## 4. 설정 롤백

### 4.1 ConfigMap/Secret 롤백

```bash
#!/bin/bash
# config-rollback.sh

echo "⚙️ 설정 롤백 시작..."

# ConfigMap 이력 확인
kubectl get configmap logcaster-config -o yaml > current-config.yaml

# 이전 버전 복원
kubectl apply -f backups/configmap-$PREVIOUS_VERSION.yaml

# Secret 롤백
kubectl apply -f backups/secret-$PREVIOUS_VERSION.yaml

# Pod 재시작으로 설정 적용
kubectl rollout restart deployment/logcaster-prod

echo "✅ 설정 롤백 완료"
```

### 4.2 환경 변수 롤백

```bash
#!/bin/bash
# env-rollback.sh

# 이전 환경 변수 복원
source /backup/env/production-$PREVIOUS_VERSION.env

# Deployment 업데이트
kubectl set env deployment/logcaster-prod \
    LOG_LEVEL=$LOG_LEVEL \
    MAX_CONNECTIONS=$MAX_CONNECTIONS \
    BUFFER_SIZE=$BUFFER_SIZE

echo "✅ 환경 변수 롤백 완료"
```

---

## 5. 완전 복구 (Disaster Recovery)

### 5.1 전체 시스템 복구

```bash
#!/bin/bash
# disaster-recovery.sh

echo "🚨 재해 복구 모드 시작..."

# 1. 모든 서비스 중지
kubectl scale deployment --all --replicas=0 -n production

# 2. 백업에서 전체 복원
LATEST_BACKUP=$(ls -t /backup/full/*.tar.gz | head -1)
tar -xzf $LATEST_BACKUP -C /

# 3. 데이터베이스 복원
psql -h $DB_HOST -U $DB_USER -d postgres -c "DROP DATABASE IF EXISTS logcaster"
psql -h $DB_HOST -U $DB_USER -d postgres -c "CREATE DATABASE logcaster"
psql -h $DB_HOST -U $DB_USER -d logcaster < /backup/db/latest.sql

# 4. 파일 시스템 복원
rsync -av /backup/files/ /var/lib/logcaster/

# 5. 안정 버전으로 배포
kubectl apply -f /backup/deployments/last-stable-deployment.yaml

# 6. 서비스 시작
kubectl scale deployment logcaster-prod --replicas=3

# 7. 헬스체크
for i in {1..30}; do
    if curl -f http://logcaster:9998/health; then
        echo "✅ 시스템 복구 완료"
        break
    fi
    sleep 10
done
```

---

## 6. 롤백 후 작업

### 6.1 원인 분석

```bash
#!/bin/bash
# post-rollback-analysis.sh

echo "🔍 롤백 원인 분석 시작..."

# 로그 수집
mkdir -p /tmp/rollback-analysis
kubectl logs deployment/logcaster-prod --since=1h > /tmp/rollback-analysis/app.log
kubectl describe deployment/logcaster-prod > /tmp/rollback-analysis/deployment.txt
kubectl get events --sort-by='.lastTimestamp' > /tmp/rollback-analysis/events.txt

# 메트릭 수집
curl -s http://prometheus:9090/api/v1/query_range \
    --data-urlencode 'query=up{job="logcaster"}' \
    --data-urlencode "start=$(date -d '1 hour ago' +%s)" \
    --data-urlencode "end=$(date +%s)" \
    --data-urlencode 'step=60' > /tmp/rollback-analysis/metrics.json

# 분석 리포트 생성
cat > /tmp/rollback-analysis/report.md <<EOF
# 롤백 분석 리포트

## 발생 시간
$(date)

## 롤백 원인
- 트리거: $ROLLBACK_TRIGGER
- 에러율: $ERROR_RATE
- 응답 시간: $RESPONSE_TIME

## 영향도
- 영향 받은 사용자: 약 X명
- 다운타임: X분
- 데이터 손실: 없음/있음

## 근본 원인
[분석 중...]

## 개선 사항
1. 
2. 
3. 

EOF

echo "✅ 분석 리포트 생성: /tmp/rollback-analysis/report.md"
```

### 6.2 팀 알림

```bash
#!/bin/bash
# notify-team.sh

# Slack 알림
curl -X POST $SLACK_WEBHOOK_URL \
    -H 'Content-Type: application/json' \
    -d '{
        "text": "🔄 롤백 완료 및 분석 리포트",
        "attachments": [{
            "color": "warning",
            "fields": [
                {"title": "롤백 시간", "value": "'$(date)'", "short": true},
                {"title": "원인", "value": "'$ROLLBACK_REASON'", "short": true},
                {"title": "현재 버전", "value": "'$CURRENT_VERSION'", "short": true},
                {"title": "상태", "value": "안정화됨", "short": true}
            ]
        }]
    }'

# 이메일 알림
cat <<EOF | mail -s "LogCaster 롤백 알림" team@company.com
LogCaster 프로덕션 롤백이 실행되었습니다.

시간: $(date)
원인: $ROLLBACK_REASON
조치: 이전 안정 버전으로 롤백 완료

상세 리포트: http://monitoring.company.com/rollback-report

감사합니다.
DevOps 팀
EOF
```

---

## 7. 롤백 자동화 통합 스크립트

```bash
#!/bin/bash
# auto-rollback.sh

set -e

# 설정
ROLLBACK_REASON=${1:-"manual"}
ROLLBACK_STRATEGY=${2:-"immediate"}  # immediate, gradual, blue-green

echo "╔════════════════════════════════════╗"
echo "║         롤백 프로세스 시작          ║"
echo "╠════════════════════════════════════╣"
echo "║ 원인: $ROLLBACK_REASON             ║"
echo "║ 전략: $ROLLBACK_STRATEGY           ║"
echo "╚════════════════════════════════════╝"

# 현재 상태 저장
./save-current-state.sh

# 롤백 전략 실행
case $ROLLBACK_STRATEGY in
    immediate)
        ./immediate-rollback.sh
        ;;
    gradual)
        ./gradual-rollback.sh
        ;;
    blue-green)
        ./blue-green-rollback.sh
        ;;
    *)
        echo "❌ 알 수 없는 롤백 전략: $ROLLBACK_STRATEGY"
        exit 1
        ;;
esac

# 데이터 롤백 (필요한 경우)
if [ "$ROLLBACK_DATA" == "true" ]; then
    ./database-rollback.sh
    ./filesystem-rollback.sh
fi

# 설정 롤백
./config-rollback.sh

# 검증
./verify-rollback.sh

# 후속 작업
./post-rollback-analysis.sh
./notify-team.sh

echo "✅ 롤백 프로세스 완료"
```

---

## ✅ 체크리스트

### 필수 확인사항
- [ ] 롤백 트리거 설정
- [ ] 백업 확인
- [ ] 롤백 권한 설정
- [ ] 모니터링 구성
- [ ] 알림 채널 설정
- [ ] 롤백 테스트 완료

### 권장사항
- [ ] 자동 롤백 구성
- [ ] 다단계 롤백 계획
- [ ] RCA 템플릿 준비
- [ ] 롤백 드릴 실시
- [ ] 문서화

---

## 🎯 다음 단계
→ [12_operation_runbook.md](12_operation_runbook.md) - 운영 런북