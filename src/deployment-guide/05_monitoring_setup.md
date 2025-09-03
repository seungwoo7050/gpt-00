# 05. 모니터링 시스템 구축

## 🎯 목표
Prometheus, Grafana, ELK Stack을 사용하여 LogCaster의 포괄적인 모니터링 시스템을 구축합니다.

## 📋 Prerequisites
- Docker 및 docker-compose 설치
- 최소 8GB RAM (모니터링 스택용)
- 네트워크 포트 접근 (3000, 9090, 5601 등)

---

## 1. Prometheus 설정

### 1.1 Prometheus 구성

```yaml
# prometheus.yml
global:
  scrape_interval: 15s
  evaluation_interval: 15s
  external_labels:
    monitor: 'logcaster-monitor'

alerting:
  alertmanagers:
    - static_configs:
        - targets:
          - alertmanager:9093

rule_files:
  - "alerts/*.yml"

scrape_configs:
  # LogCaster 메트릭 (추가 구현 필요)
  - job_name: 'logcaster-c'
    static_configs:
      - targets: ['logcaster-c:9090']
    metrics_path: '/metrics'
    relabel_configs:
      - source_labels: [__address__]
        target_label: instance
        replacement: 'logcaster-c'

  - job_name: 'logcaster-cpp'
    static_configs:
      - targets: ['logcaster-cpp:9090']
    metrics_path: '/metrics'

  # Node Exporter
  - job_name: 'node'
    static_configs:
      - targets: ['node-exporter:9100']

  # cAdvisor (컨테이너 메트릭)
  - job_name: 'cadvisor'
    static_configs:
      - targets: ['cadvisor:8080']
```

### 1.2 Alert Rules

```yaml
# alerts/logcaster.yml
groups:
  - name: logcaster_alerts
    interval: 30s
    rules:
      - alert: HighConnectionCount
        expr: logcaster_active_connections > 900
        for: 5m
        labels:
          severity: warning
        annotations:
          summary: "High connection count on {{ $labels.instance }}"
          description: "Connection count is {{ $value }} (threshold: 900)"

      - alert: HighMemoryUsage
        expr: (node_memory_MemTotal_bytes - node_memory_MemAvailable_bytes) / node_memory_MemTotal_bytes > 0.9
        for: 5m
        labels:
          severity: critical
        annotations:
          summary: "High memory usage on {{ $labels.instance }}"
          description: "Memory usage is above 90% (current: {{ $value | humanizePercentage }})"

      - alert: ServiceDown
        expr: up{job=~"logcaster.*"} == 0
        for: 1m
        labels:
          severity: critical
        annotations:
          summary: "LogCaster service down"
          description: "{{ $labels.job }} has been down for more than 1 minute"

      - alert: HighLogIngestionRate
        expr: rate(logcaster_logs_received_total[5m]) > 10000
        for: 5m
        labels:
          severity: warning
        annotations:
          summary: "High log ingestion rate"
          description: "Log ingestion rate is {{ $value }} logs/sec"
```

---

## 2. Grafana 대시보드

### 2.1 Docker Compose 설정

```yaml
# docker-compose-monitoring.yml
version: '3.8'

services:
  prometheus:
    image: prom/prometheus:latest
    container_name: prometheus
    command:
      - '--config.file=/etc/prometheus/prometheus.yml'
      - '--storage.tsdb.path=/prometheus'
      - '--web.console.libraries=/etc/prometheus/console_libraries'
      - '--web.console.templates=/etc/prometheus/consoles'
      - '--web.enable-lifecycle'
    ports:
      - "9090:9090"
    volumes:
      - ./prometheus:/etc/prometheus
      - prometheus_data:/prometheus
    networks:
      - monitoring

  grafana:
    image: grafana/grafana:latest
    container_name: grafana
    ports:
      - "3000:3000"
    environment:
      - GF_SECURITY_ADMIN_USER=admin
      - GF_SECURITY_ADMIN_PASSWORD=admin
      - GF_INSTALL_PLUGINS=grafana-piechart-panel
    volumes:
      - grafana_data:/var/lib/grafana
      - ./grafana/provisioning:/etc/grafana/provisioning
      - ./grafana/dashboards:/var/lib/grafana/dashboards
    networks:
      - monitoring

  alertmanager:
    image: prom/alertmanager:latest
    container_name: alertmanager
    ports:
      - "9093:9093"
    volumes:
      - ./alertmanager:/etc/alertmanager
      - alertmanager_data:/alertmanager
    command:
      - '--config.file=/etc/alertmanager/config.yml'
      - '--storage.path=/alertmanager'
    networks:
      - monitoring

  node-exporter:
    image: prom/node-exporter:latest
    container_name: node-exporter
    ports:
      - "9100:9100"
    volumes:
      - /proc:/host/proc:ro
      - /sys:/host/sys:ro
      - /:/rootfs:ro
    command:
      - '--path.procfs=/host/proc'
      - '--path.sysfs=/host/sys'
      - '--collector.filesystem.mount-points-exclude=^/(sys|proc|dev|host|etc)($$|/)'
    networks:
      - monitoring

  cadvisor:
    image: gcr.io/cadvisor/cadvisor:latest
    container_name: cadvisor
    ports:
      - "8080:8080"
    volumes:
      - /:/rootfs:ro
      - /var/run:/var/run:ro
      - /sys:/sys:ro
      - /var/lib/docker:/var/lib/docker:ro
      - /dev/disk:/dev/disk:ro
    privileged: true
    devices:
      - /dev/kmsg
    networks:
      - monitoring

volumes:
  prometheus_data:
  grafana_data:
  alertmanager_data:

networks:
  monitoring:
    driver: bridge
```

### 2.2 Grafana 대시보드 JSON

```json
{
  "dashboard": {
    "title": "LogCaster Monitoring",
    "panels": [
      {
        "id": 1,
        "title": "Active Connections",
        "type": "graph",
        "targets": [
          {
            "expr": "logcaster_active_connections",
            "legendFormat": "{{ instance }}"
          }
        ],
        "gridPos": { "h": 8, "w": 12, "x": 0, "y": 0 }
      },
      {
        "id": 2,
        "title": "Logs Ingestion Rate",
        "type": "graph",
        "targets": [
          {
            "expr": "rate(logcaster_logs_received_total[5m])",
            "legendFormat": "{{ instance }}"
          }
        ],
        "gridPos": { "h": 8, "w": 12, "x": 12, "y": 0 }
      },
      {
        "id": 3,
        "title": "Query Response Time",
        "type": "graph",
        "targets": [
          {
            "expr": "histogram_quantile(0.95, rate(logcaster_query_duration_seconds_bucket[5m]))",
            "legendFormat": "p95"
          },
          {
            "expr": "histogram_quantile(0.99, rate(logcaster_query_duration_seconds_bucket[5m]))",
            "legendFormat": "p99"
          }
        ],
        "gridPos": { "h": 8, "w": 12, "x": 0, "y": 8 }
      },
      {
        "id": 4,
        "title": "Memory Usage",
        "type": "graph",
        "targets": [
          {
            "expr": "container_memory_usage_bytes{name=~\"logcaster.*\"}",
            "legendFormat": "{{ name }}"
          }
        ],
        "gridPos": { "h": 8, "w": 12, "x": 12, "y": 8 }
      }
    ]
  }
}
```

---

## 3. ELK Stack 설정

### 3.1 Elasticsearch

```yaml
# elasticsearch.yml
cluster.name: logcaster-cluster
node.name: es-node-1
network.host: 0.0.0.0
discovery.type: single-node
xpack.security.enabled: false
xpack.monitoring.collection.enabled: true
```

### 3.2 Logstash 파이프라인

```ruby
# logstash.conf
input {
  tcp {
    port => 5000
    codec => json
  }
  
  file {
    path => "/var/log/logcaster/*.log"
    start_position => "beginning"
    codec => multiline {
      pattern => "^\d{4}-\d{2}-\d{2}"
      negate => true
      what => "previous"
    }
  }
}

filter {
  if [type] == "logcaster" {
    grok {
      match => {
        "message" => "%{TIMESTAMP_ISO8601:timestamp} \[%{LOGLEVEL:level}\] %{GREEDYDATA:log_message}"
      }
    }
    
    date {
      match => [ "timestamp", "ISO8601" ]
      target => "@timestamp"
    }
    
    mutate {
      add_field => {
        "service" => "logcaster"
        "environment" => "${ENV:production}"
      }
    }
  }
}

output {
  elasticsearch {
    hosts => ["elasticsearch:9200"]
    index => "logcaster-%{+YYYY.MM.dd}"
  }
  
  stdout {
    codec => rubydebug
  }
}
```

### 3.3 Kibana 설정

```yaml
# kibana.yml
server.name: kibana
server.host: "0.0.0.0"
elasticsearch.hosts: ["http://elasticsearch:9200"]
monitoring.ui.container.elasticsearch.enabled: true
```

### 3.4 ELK Docker Compose

```yaml
# docker-compose-elk.yml
version: '3.8'

services:
  elasticsearch:
    image: docker.elastic.co/elasticsearch/elasticsearch:8.11.0
    container_name: elasticsearch
    environment:
      - discovery.type=single-node
      - "ES_JAVA_OPTS=-Xms512m -Xmx512m"
      - xpack.security.enabled=false
    ports:
      - "9200:9200"
      - "9300:9300"
    volumes:
      - es_data:/usr/share/elasticsearch/data
    networks:
      - elk

  logstash:
    image: docker.elastic.co/logstash/logstash:8.11.0
    container_name: logstash
    ports:
      - "5000:5000"
      - "9600:9600"
    volumes:
      - ./logstash/pipeline:/usr/share/logstash/pipeline
      - ./logs:/var/log/logcaster:ro
    environment:
      - "LS_JAVA_OPTS=-Xmx256m -Xms256m"
    networks:
      - elk
    depends_on:
      - elasticsearch

  kibana:
    image: docker.elastic.co/kibana/kibana:8.11.0
    container_name: kibana
    ports:
      - "5601:5601"
    environment:
      - ELASTICSEARCH_HOSTS=http://elasticsearch:9200
    networks:
      - elk
    depends_on:
      - elasticsearch

volumes:
  es_data:

networks:
  elk:
    driver: bridge
```

---

## 4. LogCaster 메트릭 익스포터 구현

### 4.1 C 버전 메트릭 익스포터

```c
// metrics_exporter.c
#include <stdio.h>
#include <string.h>
#include <microhttpd.h>
#include "log_buffer.h"
#include "server.h"

#define METRICS_PORT 9090

static int metrics_handler(void *cls, struct MHD_Connection *connection,
                          const char *url, const char *method,
                          const char *version, const char *upload_data,
                          size_t *upload_data_size, void **con_cls) {
    
    server_stats_t *stats = (server_stats_t *)cls;
    char metrics[4096];
    
    snprintf(metrics, sizeof(metrics),
        "# HELP logcaster_active_connections Number of active connections\n"
        "# TYPE logcaster_active_connections gauge\n"
        "logcaster_active_connections %d\n"
        "\n"
        "# HELP logcaster_logs_received_total Total number of logs received\n"
        "# TYPE logcaster_logs_received_total counter\n"
        "logcaster_logs_received_total %ld\n"
        "\n"
        "# HELP logcaster_queries_processed_total Total number of queries processed\n"
        "# TYPE logcaster_queries_processed_total counter\n"
        "logcaster_queries_processed_total %ld\n"
        "\n"
        "# HELP logcaster_buffer_usage Current buffer usage\n"
        "# TYPE logcaster_buffer_usage gauge\n"
        "logcaster_buffer_usage %d\n",
        stats->active_connections,
        stats->logs_received,
        stats->queries_processed,
        stats->buffer_usage
    );
    
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(metrics), (void *)metrics, MHD_RESPMEM_MUST_COPY);
    
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    
    return ret;
}

void start_metrics_exporter(server_stats_t *stats) {
    struct MHD_Daemon *daemon = MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY,
        METRICS_PORT, NULL, NULL,
        &metrics_handler, stats,
        MHD_OPTION_END
    );
}
```

### 4.2 C++ 버전 메트릭 익스포터

```cpp
// MetricsExporter.cpp
#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>

class MetricsExporter {
private:
    std::shared_ptr<prometheus::Registry> registry;
    prometheus::Exposer exposer;
    
    prometheus::Gauge& active_connections;
    prometheus::Counter& logs_received;
    prometheus::Counter& queries_processed;
    prometheus::Histogram& query_duration;
    
public:
    MetricsExporter(const std::string& bind_address = "0.0.0.0:9090")
        : registry(std::make_shared<prometheus::Registry>()),
          exposer(bind_address),
          active_connections(prometheus::BuildGauge()
              .Name("logcaster_active_connections")
              .Help("Number of active connections")
              .Register(*registry)
              .Add({})),
          logs_received(prometheus::BuildCounter()
              .Name("logcaster_logs_received_total")
              .Help("Total number of logs received")
              .Register(*registry)
              .Add({})),
          queries_processed(prometheus::BuildCounter()
              .Name("logcaster_queries_processed_total")
              .Help("Total number of queries processed")
              .Register(*registry)
              .Add({})),
          query_duration(prometheus::BuildHistogram()
              .Name("logcaster_query_duration_seconds")
              .Help("Query processing duration")
              .Register(*registry)
              .Add({}, prometheus::Histogram::BucketBoundaries{
                  0.001, 0.01, 0.1, 0.5, 1.0, 5.0})) {
        
        exposer.RegisterCollectable(registry);
    }
    
    void incrementConnections() { active_connections.Increment(); }
    void decrementConnections() { active_connections.Decrement(); }
    void incrementLogsReceived() { logs_received.Increment(); }
    void incrementQueriesProcessed() { queries_processed.Increment(); }
    void observeQueryDuration(double seconds) { query_duration.Observe(seconds); }
};
```

---

## 5. 통합 모니터링 스크립트

### 5.1 시작 스크립트

```bash
#!/bin/bash
# start-monitoring.sh

echo "Starting LogCaster Monitoring Stack..."

# Start Prometheus stack
docker-compose -f docker-compose-monitoring.yml up -d

# Wait for services to be ready
echo "Waiting for services to start..."
sleep 10

# Start ELK stack
docker-compose -f docker-compose-elk.yml up -d

# Import Grafana dashboards
echo "Importing Grafana dashboards..."
curl -X POST http://admin:admin@localhost:3000/api/dashboards/db \
  -H "Content-Type: application/json" \
  -d @grafana/dashboards/logcaster.json

# Create Kibana index pattern
echo "Creating Kibana index pattern..."
curl -X POST http://localhost:5601/api/saved_objects/index-pattern \
  -H "Content-Type: application/json" \
  -H "kbn-xsrf: true" \
  -d '{
    "attributes": {
      "title": "logcaster-*",
      "timeFieldName": "@timestamp"
    }
  }'

echo "Monitoring stack started successfully!"
echo "Access points:"
echo "  - Grafana: http://localhost:3000 (admin/admin)"
echo "  - Prometheus: http://localhost:9090"
echo "  - Kibana: http://localhost:5601"
echo "  - Alertmanager: http://localhost:9093"
```

---

## ✅ 체크리스트

### 필수 확인사항
- [ ] Prometheus 설정 및 실행
- [ ] Grafana 대시보드 구성
- [ ] 메트릭 익스포터 구현
- [ ] 알림 규칙 설정
- [ ] 로그 수집 파이프라인
- [ ] 기본 대시보드 생성

### 권장사항
- [ ] ELK Stack 설정
- [ ] 분산 추적 (Jaeger)
- [ ] APM 도구 통합
- [ ] 커스텀 메트릭 추가
- [ ] SLA 모니터링
- [ ] 비용 모니터링

---

## 🎯 다음 단계
→ [06_logging_system.md](06_logging_system.md) - 로깅 시스템 설정