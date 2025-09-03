# 12. 운영 런북 (Operation Runbook)

## 🎯 목표
LogCaster의 일상적인 운영, 문제 해결, 비상 상황 대응을 위한 종합 가이드입니다.

## 📋 Prerequisites
- 시스템 접근 권한
- 모니터링 도구 접근
- 비상 연락망
- 운영 도구 설치

---

## 1. 일일 운영 작업

### 1.1 일일 점검 체크리스트

```bash
#!/bin/bash
# daily-checks.sh

echo "📋 일일 운영 점검 시작 - $(date)"

# 1. 서비스 상태 확인
echo "1️⃣ 서비스 상태 확인..."
kubectl get pods -n production | grep logcaster
systemctl status logcaster-* 2>/dev/null || echo "Systemd 서비스 없음"

# 2. 헬스체크
echo "2️⃣ 헬스체크..."
for endpoint in 9999 9998 8999 8998 6667; do
    if curl -f -s http://logcaster:$endpoint/health > /dev/null; then
        echo "  ✅ 포트 $endpoint: 정상"
    else
        echo "  ❌ 포트 $endpoint: 실패"
    fi
done

# 3. 디스크 사용량
echo "3️⃣ 디스크 사용량..."
df -h | grep -E "Filesystem|logcaster|/$"

# 4. 메모리 사용량
echo "4️⃣ 메모리 사용량..."
free -h

# 5. 로그 에러 확인
echo "5️⃣ 최근 에러 로그..."
grep -i error /var/log/logcaster/*.log | tail -10

# 6. 백업 상태
echo "6️⃣ 백업 상태..."
ls -lh /backup/daily/ | tail -5

# 7. 인증서 만료일
echo "7️⃣ SSL 인증서 만료일..."
echo | openssl s_client -servername logcaster.com -connect logcaster.com:443 2>/dev/null | openssl x509 -noout -dates

echo "✅ 일일 점검 완료"
```

### 1.2 로그 로테이션 확인

```bash
#!/bin/bash
# check-log-rotation.sh

echo "🔄 로그 로테이션 상태 확인..."

# 로그 파일 크기 확인
find /var/log/logcaster -type f -name "*.log" -exec ls -lh {} \; | \
    awk '{if($5 ~ /G$/ || ($5 ~ /M$/ && $5+0 > 100)) print "⚠️ 큰 로그 파일: " $9 " (" $5 ")"}'

# 오래된 로그 파일 확인
find /var/log/logcaster -type f -name "*.log" -mtime +30 -exec echo "⚠️ 30일 이상된 로그: {}" \;

# logrotate 실행
/usr/sbin/logrotate -f /etc/logrotate.d/logcaster

echo "✅ 로그 로테이션 완료"
```

---

## 2. 모니터링 및 알림

### 2.1 주요 메트릭 모니터링

```bash
#!/bin/bash
# monitor-metrics.sh

while true; do
    clear
    echo "📊 LogCaster 실시간 모니터링 - $(date)"
    echo "=========================================="
    
    # CPU 사용률
    echo -n "CPU: "
    top -bn1 | grep "Cpu(s)" | awk '{print $2"%"}'
    
    # 메모리 사용률
    echo -n "Memory: "
    free | grep Mem | awk '{printf "%.1f%%\n", $3/$2 * 100.0}'
    
    # 활성 연결 수
    echo -n "Active Connections: "
    netstat -an | grep -E ":9999|:9998" | grep ESTABLISHED | wc -l
    
    # 초당 로그 수
    echo -n "Logs/sec: "
    tail -1000 /var/log/logcaster/access.log | \
        awk '{print $4}' | sort | uniq -c | tail -1 | awk '{print $1}'
    
    # 에러율
    echo -n "Error Rate: "
    ERROR_COUNT=$(tail -1000 /var/log/logcaster/error.log | wc -l)
    TOTAL_COUNT=$(tail -1000 /var/log/logcaster/access.log | wc -l)
    echo "scale=2; $ERROR_COUNT / $TOTAL_COUNT * 100" | bc
    
    echo "=========================================="
    sleep 5
done
```

### 2.2 알림 규칙

```yaml
# alert-rules.yaml
alerts:
  - name: high_cpu
    metric: cpu_usage
    threshold: 80
    duration: 5m
    severity: warning
    action: |
      echo "⚠️ CPU 사용률 높음: ${VALUE}%" | \
        mail -s "LogCaster CPU Alert" ops@company.com
      
  - name: low_disk_space
    metric: disk_free
    threshold: 10  # GB
    severity: critical
    action: |
      ./emergency-cleanup.sh
      ./notify-slack.sh "🚨 디스크 공간 부족: ${VALUE}GB 남음"
      
  - name: service_down
    metric: health_check
    threshold: failed
    severity: critical
    action: |
      ./restart-service.sh
      ./page-oncall.sh "LogCaster 서비스 다운"
```

---

## 3. 문제 해결 가이드

### 3.1 일반적인 문제와 해결법

```markdown
## 🔧 문제 해결 가이드

### 문제: 서비스가 시작되지 않음
**증상**: 
- logcaster 프로세스가 실행되지 않음
- 포트 9999/9998에 연결 불가

**해결**:
1. 포트 충돌 확인
   ```bash
   netstat -tulpn | grep -E "9999|9998"
   ```
2. 로그 확인
   ```bash
   tail -100 /var/log/logcaster/error.log
   ```
3. 권한 확인
   ```bash
   ls -la /var/lib/logcaster
   chown -R logcaster:logcaster /var/lib/logcaster
   ```
4. 서비스 재시작
   ```bash
   systemctl restart logcaster
   ```

### 문제: 높은 메모리 사용
**증상**:
- 메모리 사용률 90% 이상
- OOM Killer 발생

**해결**:
1. 메모리 누수 확인
   ```bash
   valgrind --leak-check=full ./logcaster-c
   ```
2. 버퍼 크기 조정
   ```bash
   export BUFFER_SIZE=5000  # 기본값: 10000
   ```
3. 서비스 재시작
   ```bash
   systemctl restart logcaster
   ```

### 문제: 느린 쿼리 응답
**증상**:
- 쿼리 응답 시간 > 2초
- 타임아웃 발생

**해결**:
1. 인덱스 확인
   ```bash
   ./check-indexes.sh
   ```
2. 쿼리 최적화
   ```bash
   ./optimize-queries.sh
   ```
3. 캐시 재구성
   ```bash
   redis-cli FLUSHALL
   ```
```

### 3.2 디버깅 스크립트

```bash
#!/bin/bash
# debug-helper.sh

echo "🔍 LogCaster 디버깅 도우미"
echo "=========================="

# 프로세스 정보
echo "📊 프로세스 정보:"
ps aux | grep logcaster | grep -v grep

# 네트워크 연결
echo -e "\n🌐 네트워크 연결:"
netstat -tulpn | grep logcaster

# 최근 로그
echo -e "\n📝 최근 에러 로그:"
tail -20 /var/log/logcaster/error.log

# 시스템 리소스
echo -e "\n💻 시스템 리소스:"
top -bn1 | head -10

# 코어 덤프
echo -e "\n💥 코어 덤프:"
ls -la /var/crash/ | grep logcaster

# strace 실행 (선택적)
read -p "strace를 실행하시겠습니까? (y/n) " -n 1 -r
if [[ $REPLY =~ ^[Yy]$ ]]; then
    PID=$(pgrep logcaster | head -1)
    strace -p $PID -o strace.out &
    sleep 10
    kill %1
    echo -e "\nstrace 출력이 strace.out에 저장되었습니다."
fi
```

---

## 4. 성능 튜닝

### 4.1 시스템 튜닝

```bash
#!/bin/bash
# system-tuning.sh

echo "⚡ 시스템 성능 튜닝..."

# 커널 파라미터 최적화
cat >> /etc/sysctl.conf <<EOF
# Network tuning
net.core.somaxconn = 65535
net.ipv4.tcp_max_syn_backlog = 8192
net.core.netdev_max_backlog = 16384
net.ipv4.tcp_fin_timeout = 15
net.ipv4.tcp_tw_reuse = 1
net.ipv4.tcp_keepalive_time = 300
net.ipv4.tcp_keepalive_probes = 3
net.ipv4.tcp_keepalive_intvl = 30

# Memory tuning
vm.swappiness = 10
vm.dirty_ratio = 15
vm.dirty_background_ratio = 5

# File descriptors
fs.file-max = 2097152
fs.nr_open = 2097152
EOF

sysctl -p

# ulimit 설정
cat >> /etc/security/limits.conf <<EOF
logcaster soft nofile 65535
logcaster hard nofile 65535
logcaster soft nproc 32768
logcaster hard nproc 32768
EOF

echo "✅ 시스템 튜닝 완료"
```

### 4.2 애플리케이션 튜닝

```bash
#!/bin/bash
# app-tuning.sh

# 스레드 풀 크기 조정
export THREAD_POOL_SIZE=$(nproc)

# 버퍼 크기 최적화
TOTAL_MEM=$(free -m | awk 'NR==2{print $2}')
export BUFFER_SIZE=$((TOTAL_MEM * 10))  # 전체 메모리의 1%

# I/O 최적화
export USE_DIRECT_IO=1
export WRITE_BATCH_SIZE=1000

# 재시작
systemctl restart logcaster

echo "✅ 애플리케이션 튜닝 완료"
```

---

## 5. 백업 및 복구

### 5.1 백업 절차

```bash
#!/bin/bash
# backup-procedure.sh

BACKUP_DIR="/backup/$(date +%Y%m%d)"
mkdir -p $BACKUP_DIR

echo "💾 백업 시작..."

# 1. 애플리케이션 데이터
tar -czf $BACKUP_DIR/app-data.tar.gz /var/lib/logcaster

# 2. 설정 파일
tar -czf $BACKUP_DIR/config.tar.gz /etc/logcaster

# 3. 로그 파일
tar -czf $BACKUP_DIR/logs.tar.gz /var/log/logcaster

# 4. 데이터베이스 (있는 경우)
if [ -f /var/lib/logcaster/data.db ]; then
    sqlite3 /var/lib/logcaster/data.db ".backup $BACKUP_DIR/database.bak"
fi

# 5. 백업 검증
for file in $BACKUP_DIR/*.tar.gz; do
    if tar -tzf $file > /dev/null 2>&1; then
        echo "✅ $file 검증 완료"
    else
        echo "❌ $file 검증 실패"
    fi
done

# 6. 원격 백업
rsync -avz $BACKUP_DIR/ backup-server:/backups/logcaster/

echo "✅ 백업 완료: $BACKUP_DIR"
```

### 5.2 복구 절차

```bash
#!/bin/bash
# restore-procedure.sh

RESTORE_DATE=${1:-$(date +%Y%m%d)}
BACKUP_DIR="/backup/$RESTORE_DATE"

echo "♻️ 복구 시작 (날짜: $RESTORE_DATE)..."

# 1. 서비스 중지
systemctl stop logcaster

# 2. 기존 데이터 백업
mv /var/lib/logcaster /var/lib/logcaster.old

# 3. 데이터 복원
tar -xzf $BACKUP_DIR/app-data.tar.gz -C /

# 4. 설정 복원
tar -xzf $BACKUP_DIR/config.tar.gz -C /

# 5. 권한 복원
chown -R logcaster:logcaster /var/lib/logcaster

# 6. 서비스 시작
systemctl start logcaster

# 7. 검증
sleep 5
if systemctl is-active logcaster; then
    echo "✅ 복구 완료"
else
    echo "❌ 복구 실패. 롤백 중..."
    mv /var/lib/logcaster.old /var/lib/logcaster
    systemctl start logcaster
fi
```

---

## 6. 비상 대응 절차

### 6.1 서비스 장애 대응

```bash
#!/bin/bash
# emergency-response.sh

echo "🚨 비상 대응 모드 활성화"

# 1단계: 즉시 조치
echo "1️⃣ 즉시 조치 실행..."
# 서비스 재시작 시도
systemctl restart logcaster || {
    echo "재시작 실패. 강제 종료 후 재시작..."
    pkill -9 logcaster
    sleep 2
    systemctl start logcaster
}

# 2단계: 임시 우회
if ! systemctl is-active logcaster; then
    echo "2️⃣ 임시 우회 서비스 시작..."
    docker run -d --name logcaster-emergency \
        -p 9999:9999 -p 9998:9998 \
        logcaster/logcaster-c:stable
fi

# 3단계: 알림
echo "3️⃣ 관련자 알림..."
./notify-emergency.sh "LogCaster 비상 상황 발생"

# 4단계: 로그 수집
echo "4️⃣ 진단 정보 수집..."
./collect-diagnostics.sh

# 5단계: 상태 보고
echo "5️⃣ 상태 보고서 생성..."
cat > emergency-report.txt <<EOF
비상 상황 보고서
================
시간: $(date)
서비스 상태: $(systemctl is-active logcaster)
임시 서비스: $(docker ps | grep emergency | wc -l)개 실행 중
영향도: 높음
예상 복구 시간: 30분
EOF

echo "✅ 비상 대응 초기 조치 완료"
```

### 6.2 데이터 손실 대응

```bash
#!/bin/bash
# data-recovery.sh

echo "💾 데이터 복구 프로세스 시작..."

# 1. 손실 범위 파악
LAST_BACKUP=$(ls -t /backup/*/app-data.tar.gz | head -1)
LAST_BACKUP_TIME=$(stat -c %Y $LAST_BACKUP)
CURRENT_TIME=$(date +%s)
LOST_MINUTES=$(( (CURRENT_TIME - LAST_BACKUP_TIME) / 60 ))

echo "예상 데이터 손실: 약 $LOST_MINUTES 분"

# 2. WAL 로그 확인 (있는 경우)
if [ -d /var/lib/logcaster/wal ]; then
    echo "WAL 로그에서 복구 시도..."
    ./replay-wal.sh
fi

# 3. 부분 복구
echo "부분 데이터 복구 시도..."
for file in /var/log/logcaster/persistence-*.log; do
    if [ -f "$file" ]; then
        ./parse-persistence-log.sh $file
    fi
done

# 4. 무결성 검사
./check-data-integrity.sh

echo "✅ 데이터 복구 완료"
```

---

## 7. 정기 유지보수

### 7.1 주간 유지보수

```bash
#!/bin/bash
# weekly-maintenance.sh

echo "🔧 주간 유지보수 시작 - $(date)"

# 1. 디스크 정리
echo "1️⃣ 디스크 정리..."
find /var/log/logcaster -name "*.log" -mtime +7 -delete
find /tmp -name "logcaster-*" -mtime +1 -delete

# 2. 인덱스 재구성
echo "2️⃣ 인덱스 최적화..."
./optimize-indexes.sh

# 3. 통계 업데이트
echo "3️⃣ 통계 정보 업데이트..."
./update-statistics.sh

# 4. 보안 패치 확인
echo "4️⃣ 보안 업데이트 확인..."
apt-get update && apt-get upgrade -s | grep -i security

# 5. 성능 리포트 생성
echo "5️⃣ 주간 성능 리포트..."
./generate-performance-report.sh > weekly-report-$(date +%Y%m%d).txt

echo "✅ 주간 유지보수 완료"
```

### 7.2 월간 유지보수

```bash
#!/bin/bash
# monthly-maintenance.sh

echo "🔧 월간 유지보수 시작 - $(date)"

# 1. 전체 백업
./full-backup.sh

# 2. 보안 감사
./security-audit.sh

# 3. 용량 계획 검토
./capacity-planning.sh

# 4. 로그 아카이빙
./archive-old-logs.sh

# 5. 인증서 갱신 확인
./check-certificates.sh

echo "✅ 월간 유지보수 완료"
```

---

## 8. 운영 체크리스트

### 8.1 교대 근무 인수인계

```markdown
## 🔄 교대 근무 인수인계 체크리스트

### 이전 근무자 확인 사항
- [ ] 미해결 알림 및 이슈
- [ ] 진행 중인 유지보수 작업
- [ ] 특이사항 및 주의사항
- [ ] 예정된 변경 작업

### 신규 근무자 확인 사항
- [ ] 모니터링 대시보드 정상
- [ ] 알림 채널 연결 확인
- [ ] 비상 연락망 확인
- [ ] 운영 도구 접근 권한

### 시스템 상태 확인
- [ ] 모든 서비스 정상 작동
- [ ] 디스크 공간 충분 (>20%)
- [ ] 메모리 사용률 정상 (<80%)
- [ ] 백업 정상 수행
- [ ] 보안 이벤트 없음
```

---

## ✅ 최종 체크리스트

### 필수 확인사항
- [ ] 일일 점검 스크립트 설정
- [ ] 모니터링 알림 구성
- [ ] 백업 자동화
- [ ] 비상 대응 계획
- [ ] 팀 교육 완료

### 권장사항
- [ ] 자동화 스크립트 검증
- [ ] 재해 복구 드릴
- [ ] 성능 베이스라인 설정
- [ ] 운영 문서 최신화
- [ ] 정기 리뷰 일정

---

## 🎯 다음 단계
→ [13_scaling_strategy.md](13_scaling_strategy.md) - 스케일링 전략