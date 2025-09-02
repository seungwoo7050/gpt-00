# 06. 로깅 시스템 설정

## 🎯 목표
LogCaster의 구조화된 로깅 시스템을 구축하고 중앙 집중식 로그 관리를 구현합니다.

## 📋 Prerequisites
- Elasticsearch 7.x+ 또는 Loki
- Fluentd 또는 Filebeat
- 충분한 스토리지 (최소 100GB)
- 로그 보존 정책 결정

---

## 1. 로깅 아키텍처

### 1.1 로그 레벨 정의

```c
// log_levels.h
typedef enum {
    LOG_TRACE = 0,    // 상세 디버깅 정보
    LOG_DEBUG = 1,    // 디버깅 정보
    LOG_INFO = 2,     // 일반 정보
    LOG_WARN = 3,     // 경고
    LOG_ERROR = 4,    // 에러
    LOG_FATAL = 5     // 치명적 에러
} log_level_t;

// 구조화된 로그 포맷
typedef struct {
    time_t timestamp;
    log_level_t level;
    char* component;
    char* message;
    char* trace_id;
    char* user_id;
    char* client_ip;
    int response_time_ms;
    char* extra_json;
} log_entry_t;
```

### 1.2 로그 출력 구현

```c
// structured_logger.c
#include <json-c/json.h>

void log_structured(log_entry_t* entry) {
    json_object* jobj = json_object_new_object();
    
    // 타임스탬프
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S%z", 
             localtime(&entry->timestamp));
    json_object_object_add(jobj, "@timestamp", 
                           json_object_new_string(timestamp));
    
    // 로그 레벨
    const char* level_str[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    json_object_object_add(jobj, "level", 
                           json_object_new_string(level_str[entry->level]));
    
    // 컴포넌트
    json_object_object_add(jobj, "component", 
                           json_object_new_string(entry->component));
    
    // 메시지
    json_object_object_add(jobj, "message", 
                           json_object_new_string(entry->message));
    
    // 추적 정보
    if (entry->trace_id) {
        json_object_object_add(jobj, "trace_id", 
                               json_object_new_string(entry->trace_id));
    }
    
    // 사용자 정보
    if (entry->user_id) {
        json_object_object_add(jobj, "user_id", 
                               json_object_new_string(entry->user_id));
    }
    
    // 클라이언트 IP
    if (entry->client_ip) {
        json_object_object_add(jobj, "client_ip", 
                               json_object_new_string(entry->client_ip));
    }
    
    // 응답 시간
    if (entry->response_time_ms > 0) {
        json_object_object_add(jobj, "response_time_ms", 
                               json_object_new_int(entry->response_time_ms));
    }
    
    // JSON 출력
    fprintf(log_file, "%s\n", json_object_to_json_string_ext(jobj, 
            JSON_C_TO_STRING_PLAIN));
    fflush(log_file);
    
    json_object_put(jobj);
}
```

---

## 2. Fluentd 설정

### 2.1 Fluentd 설치 및 구성

```dockerfile
# Dockerfile.fluentd
FROM fluent/fluentd:v1.16-debian

USER root

# 플러그인 설치
RUN gem install fluent-plugin-elasticsearch \
                fluent-plugin-prometheus \
                fluent-plugin-s3 \
                fluent-plugin-kafka

USER fluent
```

### 2.2 Fluentd 설정 파일

```ruby
# fluent.conf
# LogCaster 로그 수집
<source>
  @type tail
  path /var/log/logcaster/*.log
  pos_file /var/log/fluentd/logcaster.pos
  tag logcaster.*
  <parse>
    @type json
    time_key @timestamp
    time_format %Y-%m-%dT%H:%M:%S%z
  </parse>
</source>

# 로그 필터링 및 변환
<filter logcaster.**>
  @type record_transformer
  <record>
    hostname ${hostname}
    environment ${ENV}
    service logcaster
  </record>
</filter>

# 에러 로그 특별 처리
<filter logcaster.**>
  @type grep
  <regexp>
    key level
    pattern ERROR|FATAL
  </regexp>
</filter>

# Elasticsearch 출력
<match logcaster.**>
  @type elasticsearch
  host elasticsearch
  port 9200
  logstash_format true
  logstash_prefix logcaster
  logstash_dateformat %Y.%m.%d
  include_timestamp true
  <buffer>
    @type file
    path /var/log/fluentd/buffer/elasticsearch
    flush_interval 10s
    chunk_limit_size 5M
    retry_limit 5
    retry_wait 10s
  </buffer>
</match>

# S3 백업 (선택사항)
<match logcaster.**>
  @type s3
  aws_key_id YOUR_AWS_KEY_ID
  aws_sec_key YOUR_AWS_SECRET_KEY
  s3_bucket logcaster-logs
  s3_region ap-northeast-2
  path logs/%Y/%m/%d/
  <buffer time>
    @type file
    path /var/log/fluentd/buffer/s3
    timekey 3600  # 1시간마다
    timekey_wait 10m
    chunk_limit_size 256m
  </buffer>
</match>

# 메트릭 수집
<source>
  @type prometheus
  bind 0.0.0.0
  port 24231
  metrics_path /metrics
</source>

<source>
  @type prometheus_monitor
</source>
```

---

## 3. Filebeat 대안 설정

### 3.1 Filebeat 구성

```yaml
# filebeat.yml
filebeat.inputs:
- type: log
  enabled: true
  paths:
    - /var/log/logcaster/*.log
  json.keys_under_root: true
  json.add_error_key: true
  fields:
    service: logcaster
    environment: ${ENV:production}
  fields_under_root: true

- type: log
  enabled: true
  paths:
    - /var/log/logcaster/error.log
  multiline.pattern: '^\d{4}-\d{2}-\d{2}'
  multiline.negate: true
  multiline.match: after
  fields:
    log_type: error

processors:
  - add_host_metadata:
      when.not.contains:
        tags: forwarded
  - add_docker_metadata: ~
  - add_kubernetes_metadata: ~
  - drop_event:
      when:
        or:
          - regexp:
              message: "^DEBUG"
          - contains:
              message: "health_check"

output.elasticsearch:
  hosts: ["elasticsearch:9200"]
  index: "logcaster-%{+yyyy.MM.dd}"
  template.name: "logcaster"
  template.pattern: "logcaster-*"
  template.settings:
    index.number_of_shards: 2
    index.number_of_replicas: 1

logging.level: info
logging.to_files: true
logging.files:
  path: /var/log/filebeat
  name: filebeat
  keepfiles: 7
  permissions: 0640
```

---

## 4. 중앙 집중식 로깅 (Loki)

### 4.1 Loki 스택 설정

```yaml
# docker-compose-loki.yml
version: '3.8'

services:
  loki:
    image: grafana/loki:2.9.0
    ports:
      - "3100:3100"
    command: -config.file=/etc/loki/local-config.yaml
    volumes:
      - ./loki-config.yaml:/etc/loki/local-config.yaml
      - loki_data:/loki
    networks:
      - loki

  promtail:
    image: grafana/promtail:2.9.0
    volumes:
      - /var/log:/var/log:ro
      - ./promtail-config.yaml:/etc/promtail/config.yml
      - /var/lib/docker/containers:/var/lib/docker/containers:ro
    command: -config.file=/etc/promtail/config.yml
    networks:
      - loki

  grafana:
    image: grafana/grafana:latest
    ports:
      - "3000:3000"
    environment:
      - GF_SECURITY_ADMIN_PASSWORD=admin
    volumes:
      - grafana_data:/var/lib/grafana
      - ./grafana-datasources.yaml:/etc/grafana/provisioning/datasources/datasources.yaml
    networks:
      - loki

volumes:
  loki_data:
  grafana_data:

networks:
  loki:
    driver: bridge
```

### 4.2 Promtail 설정

```yaml
# promtail-config.yaml
server:
  http_listen_port: 9080
  grpc_listen_port: 0

positions:
  filename: /tmp/positions.yaml

clients:
  - url: http://loki:3100/loki/api/v1/push

scrape_configs:
  - job_name: logcaster
    static_configs:
      - targets:
          - localhost
        labels:
          job: logcaster
          __path__: /var/log/logcaster/*.log
    pipeline_stages:
      - json:
          expressions:
            level: level
            component: component
            message: message
            timestamp: "@timestamp"
      - labels:
          level:
          component:
      - timestamp:
          source: timestamp
          format: "2006-01-02T15:04:05Z07:00"
      - metrics:
          log_lines_total:
            type: Counter
            description: "Total number of log lines"
            source: level
            config:
              action: inc
```

---

## 5. 로그 분석 및 알림

### 5.1 로그 분석 규칙

```yaml
# log-analysis-rules.yaml
rules:
  - name: high_error_rate
    query: |
      sum(rate({job="logcaster"} |= "ERROR" [5m])) > 10
    duration: 5m
    annotations:
      summary: "High error rate detected"
      description: "Error rate is {{ $value }} errors/min"
    
  - name: slow_queries
    query: |
      avg({job="logcaster"} | json | response_time_ms > 1000 [5m]) > 100
    duration: 10m
    annotations:
      summary: "Slow queries detected"
      
  - name: security_events
    query: |
      {job="logcaster"} |~ "unauthorized|forbidden|injection|malicious"
    duration: 1m
    annotations:
      summary: "Security event detected"
      severity: critical
```

### 5.2 로그 집계 대시보드

```json
{
  "dashboard": {
    "title": "LogCaster Log Analytics",
    "panels": [
      {
        "title": "Log Volume",
        "targets": [{
          "expr": "sum(rate({job=\"logcaster\"}[5m])) by (level)"
        }]
      },
      {
        "title": "Error Logs",
        "targets": [{
          "expr": "{job=\"logcaster\", level=\"ERROR\"}"
        }]
      },
      {
        "title": "Response Time Distribution",
        "targets": [{
          "expr": "histogram_quantile(0.95, sum(rate({job=\"logcaster\"} | json | __error__=\"\" | response_time_ms > 0 [5m])) by (le))"
        }]
      },
      {
        "title": "Top Errors",
        "targets": [{
          "expr": "topk(10, count by (message) ({job=\"logcaster\", level=\"ERROR\"}))"
        }]
      }
    ]
  }
}
```

---

## 6. 로그 보존 및 아카이빙

### 6.1 로그 로테이션

```bash
# /etc/logrotate.d/logcaster
/var/log/logcaster/*.log {
    daily
    rotate 30
    compress
    delaycompress
    missingok
    notifempty
    create 0644 logcaster logcaster
    sharedscripts
    postrotate
        # 시그널 전송으로 로그 파일 재오픈
        kill -USR1 $(cat /var/run/logcaster.pid) 2>/dev/null || true
        
        # S3에 아카이브 업로드
        for file in /var/log/logcaster/*.gz; do
            aws s3 cp $file s3://logcaster-archive/logs/$(date +%Y/%m/%d)/
            rm $file
        done
    endscript
}
```

### 6.2 로그 아카이빙 스크립트

```bash
#!/bin/bash
# archive-logs.sh

ARCHIVE_DIR="/archive/logs"
S3_BUCKET="logcaster-logs-archive"
RETENTION_DAYS=90

echo "📦 로그 아카이빙 시작..."

# 1. 로컬 아카이브 생성
find /var/log/logcaster -name "*.log" -mtime +7 | while read file; do
    ARCHIVE_NAME=$(basename $file .log)_$(date -r $file +%Y%m%d).tar.gz
    tar -czf $ARCHIVE_DIR/$ARCHIVE_NAME $file
    rm $file
done

# 2. S3 업로드
aws s3 sync $ARCHIVE_DIR s3://$S3_BUCKET/archives/ \
    --storage-class GLACIER \
    --delete

# 3. 오래된 아카이브 삭제
find $ARCHIVE_DIR -name "*.tar.gz" -mtime +$RETENTION_DAYS -delete

# 4. Elasticsearch 인덱스 정리
curl -X DELETE "elasticsearch:9200/logcaster-$(date -d '90 days ago' +%Y.%m.%d)"

echo "✅ 로그 아카이빙 완료"
```

---

## 7. 로그 보안

### 7.1 민감 정보 마스킹

```c
// log_masking.c
char* mask_sensitive_data(const char* message) {
    char* masked = strdup(message);
    
    // 패턴 정의
    struct {
        const char* pattern;
        const char* replacement;
    } patterns[] = {
        {"password=([^\\s]+)", "password=***"},
        {"token=([^\\s]+)", "token=***"},
        {"\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Z|a-z]{2,}\\b", "***@***.***"},
        {"\\b(?:\\d{4}[-\\s]?){3}\\d{4}\\b", "****-****-****-****"},  // 카드번호
        {"\\b\\d{3}-\\d{2}-\\d{4}\\b", "***-**-****"},  // SSN
        {NULL, NULL}
    };
    
    for (int i = 0; patterns[i].pattern; i++) {
        regex_t regex;
        regcomp(&regex, patterns[i].pattern, REG_EXTENDED);
        
        regmatch_t match;
        if (regexec(&regex, masked, 1, &match, 0) == 0) {
            // 마스킹 적용
            memset(masked + match.rm_so, '*', match.rm_eo - match.rm_so);
        }
        
        regfree(&regex);
    }
    
    return masked;
}
```

### 7.2 로그 암호화

```bash
#!/bin/bash
# encrypt-logs.sh

# 로그 암호화 (전송 중)
openssl enc -aes-256-cbc -salt -in /var/log/logcaster/sensitive.log \
    -out /var/log/logcaster/sensitive.log.enc -k $LOG_ENCRYPTION_KEY

# 로그 서명 (무결성 보장)
openssl dgst -sha256 -sign private_key.pem \
    -out /var/log/logcaster/sensitive.log.sig \
    /var/log/logcaster/sensitive.log
```

---

## 8. 성능 최적화

### 8.1 비동기 로깅

```c
// async_logger.c
#include <pthread.h>

typedef struct {
    log_entry_t* buffer;
    int capacity;
    int head;
    int tail;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} log_queue_t;

void* logger_thread(void* arg) {
    log_queue_t* queue = (log_queue_t*)arg;
    
    while (1) {
        pthread_mutex_lock(&queue->mutex);
        
        while (queue->head == queue->tail) {
            pthread_cond_wait(&queue->not_empty, &queue->mutex);
        }
        
        log_entry_t entry = queue->buffer[queue->head];
        queue->head = (queue->head + 1) % queue->capacity;
        
        pthread_cond_signal(&queue->not_full);
        pthread_mutex_unlock(&queue->mutex);
        
        // 실제 로깅
        log_structured(&entry);
    }
    
    return NULL;
}

void async_log(log_queue_t* queue, log_entry_t* entry) {
    pthread_mutex_lock(&queue->mutex);
    
    int next_tail = (queue->tail + 1) % queue->capacity;
    while (next_tail == queue->head) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    
    queue->buffer[queue->tail] = *entry;
    queue->tail = next_tail;
    
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);
}
```

---

## ✅ 체크리스트

### 필수 확인사항
- [ ] 구조화된 로깅 구현
- [ ] 로그 수집 에이전트 설치
- [ ] 중앙 로그 저장소 구성
- [ ] 로그 로테이션 설정
- [ ] 민감 정보 마스킹
- [ ] 로그 보존 정책 수립

### 권장사항
- [ ] 로그 분석 대시보드
- [ ] 실시간 알림 설정
- [ ] 로그 아카이빙
- [ ] 암호화 적용
- [ ] 비동기 로깅
- [ ] 로그 검색 최적화

---

## 🎯 다음 단계
→ [07_security_checklist.md](07_security_checklist.md) - 보안 체크리스트