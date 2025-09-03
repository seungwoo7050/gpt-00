# 13. 스케일링 전략

## 🎯 목표
LogCaster를 수평/수직으로 확장하여 증가하는 트래픽과 데이터를 효과적으로 처리하기 위한 전략입니다.

## 📋 Prerequisites
- 로드 밸런서 구성
- 컨테이너 오케스트레이션 (Kubernetes)
- 모니터링 시스템
- 자동 스케일링 도구

---

## 1. 스케일링 아키텍처

### 1.1 수평 확장 (Horizontal Scaling)

```yaml
# horizontal-scaling-architecture.yaml
architecture:
  load_balancer:
    type: nginx/haproxy
    strategy: least_connections
    health_check_interval: 5s
    
  application_tier:
    min_instances: 3
    max_instances: 20
    target_cpu: 70%
    target_memory: 80%
    
  data_tier:
    sharding_strategy: consistent_hashing
    replication_factor: 3
    partition_count: 128
```

### 1.2 수직 확장 (Vertical Scaling)

```yaml
# vertical-scaling-tiers.yaml
instance_types:
  small:
    cpu: 2
    memory: 4GB
    network: 1Gbps
    iops: 3000
    
  medium:
    cpu: 4
    memory: 16GB
    network: 5Gbps
    iops: 10000
    
  large:
    cpu: 16
    memory: 64GB
    network: 10Gbps
    iops: 30000
    
  xlarge:
    cpu: 32
    memory: 128GB
    network: 25Gbps
    iops: 60000
```

---

## 2. Kubernetes 자동 스케일링

### 2.1 Horizontal Pod Autoscaler (HPA)

```yaml
# hpa-logcaster.yaml
apiVersion: autoscaling/v2
kind: HorizontalPodAutoscaler
metadata:
  name: logcaster-hpa
  namespace: production
spec:
  scaleTargetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: logcaster
  minReplicas: 3
  maxReplicas: 50
  metrics:
  - type: Resource
    resource:
      name: cpu
      target:
        type: Utilization
        averageUtilization: 70
  - type: Resource
    resource:
      name: memory
      target:
        type: Utilization
        averageUtilization: 80
  - type: Pods
    pods:
      metric:
        name: logcaster_active_connections
      target:
        type: AverageValue
        averageValue: "100"
  - type: External
    external:
      metric:
        name: queue_size
        selector:
          matchLabels:
            queue: logs
      target:
        type: Value
        value: "1000"
  behavior:
    scaleDown:
      stabilizationWindowSeconds: 300
      policies:
      - type: Percent
        value: 50
        periodSeconds: 60
      - type: Pods
        value: 2
        periodSeconds: 60
      selectPolicy: Min
    scaleUp:
      stabilizationWindowSeconds: 0
      policies:
      - type: Percent
        value: 100
        periodSeconds: 15
      - type: Pods
        value: 5
        periodSeconds: 15
      selectPolicy: Max
```

### 2.2 Vertical Pod Autoscaler (VPA)

```yaml
# vpa-logcaster.yaml
apiVersion: autoscaling.k8s.io/v1
kind: VerticalPodAutoscaler
metadata:
  name: logcaster-vpa
  namespace: production
spec:
  targetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: logcaster
  updatePolicy:
    updateMode: "Auto"  # Off, Initial, Auto
  resourcePolicy:
    containerPolicies:
    - containerName: logcaster
      minAllowed:
        cpu: 100m
        memory: 128Mi
      maxAllowed:
        cpu: 4
        memory: 8Gi
      controlledResources: ["cpu", "memory"]
```

### 2.3 Cluster Autoscaler

```yaml
# cluster-autoscaler.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: cluster-autoscaler
  namespace: kube-system
spec:
  template:
    spec:
      containers:
      - image: k8s.gcr.io/autoscaling/cluster-autoscaler:v1.27.0
        name: cluster-autoscaler
        command:
        - ./cluster-autoscaler
        - --v=4
        - --stderrthreshold=info
        - --cloud-provider=aws
        - --skip-nodes-with-local-storage=false
        - --expander=least-waste
        - --node-group-auto-discovery=asg:tag=k8s.io/cluster-autoscaler/enabled,k8s.io/cluster-autoscaler/logcaster-cluster
        - --balance-similar-node-groups
        - --skip-nodes-with-system-pods=false
        env:
        - name: AWS_REGION
          value: ap-northeast-2
```

---

## 3. 로드 밸런싱 전략

### 3.1 NGINX 로드 밸런서

```nginx
# nginx-load-balancer.conf
upstream logcaster_backend {
    least_conn;  # 또는 ip_hash, random
    
    server backend1.example.com:9999 weight=3 max_fails=3 fail_timeout=30s;
    server backend2.example.com:9999 weight=2 max_fails=3 fail_timeout=30s;
    server backend3.example.com:9999 weight=1 max_fails=3 fail_timeout=30s;
    
    # 백업 서버
    server backup1.example.com:9999 backup;
    
    # 헬스체크
    keepalive 32;
    keepalive_requests 100;
    keepalive_timeout 60s;
}

server {
    listen 80;
    server_name logcaster.example.com;
    
    location / {
        proxy_pass http://logcaster_backend;
        proxy_next_upstream error timeout http_500 http_502 http_503;
        proxy_connect_timeout 1s;
        proxy_send_timeout 30s;
        proxy_read_timeout 30s;
        
        # 연결 재사용
        proxy_http_version 1.1;
        proxy_set_header Connection "";
        
        # 헤더 설정
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        
        # 버퍼링
        proxy_buffering on;
        proxy_buffer_size 4k;
        proxy_buffers 8 4k;
        proxy_busy_buffers_size 8k;
    }
    
    # 헬스체크 엔드포인트
    location /health {
        access_log off;
        return 200 "healthy\n";
        add_header Content-Type text/plain;
    }
}
```

### 3.2 HAProxy 로드 밸런서

```conf
# haproxy.cfg
global
    maxconn 50000
    log stdout local0
    nbproc 4
    nbthread 4
    cpu-map auto:1/1-4 0-3

defaults
    mode tcp
    timeout connect 5s
    timeout client 30s
    timeout server 30s
    option tcplog

frontend logcaster_frontend
    bind *:9999
    mode tcp
    default_backend logcaster_backend

backend logcaster_backend
    mode tcp
    balance leastconn
    
    # 헬스체크
    option tcp-check
    tcp-check connect
    tcp-check send "PING\r\n"
    tcp-check expect string "PONG"
    
    # 서버 풀
    server backend1 10.0.1.10:9999 check inter 2s rise 2 fall 3 weight 100
    server backend2 10.0.1.11:9999 check inter 2s rise 2 fall 3 weight 100
    server backend3 10.0.1.12:9999 check inter 2s rise 2 fall 3 weight 100
    
    # 동적 서버 추가
    server-template srv 1-10 _backend._tcp.service.consul resolvers consul resolve-opts allow-dup-ip resolve-prefer ipv4 check
```

---

## 4. 데이터 샤딩

### 4.1 샤딩 구현

```c
// sharding.c
#include <stdint.h>
#include <string.h>

// Consistent Hashing을 위한 구조체
typedef struct {
    uint32_t hash;
    int server_id;
} hash_node_t;

typedef struct {
    hash_node_t* nodes;
    int node_count;
    int server_count;
    int virtual_nodes;  // 가상 노드 수
} consistent_hash_t;

// MurmurHash3
uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed) {
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    const uint32_t r1 = 15;
    const uint32_t r2 = 13;
    const uint32_t m = 5;
    const uint32_t n = 0xe6546b64;
    
    uint32_t hash = seed;
    const int nblocks = len / 4;
    const uint32_t* blocks = (const uint32_t*)key;
    
    for (int i = 0; i < nblocks; i++) {
        uint32_t k = blocks[i];
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;
        
        hash ^= k;
        hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
    }
    
    const uint8_t* tail = (const uint8_t*)(key + nblocks * 4);
    uint32_t k1 = 0;
    
    switch (len & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
            k1 *= c1;
            k1 = (k1 << r1) | (k1 >> (32 - r1));
            k1 *= c2;
            hash ^= k1;
    }
    
    hash ^= len;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);
    
    return hash;
}

// 샤드 선택
int get_shard(consistent_hash_t* ch, const char* key) {
    uint32_t hash = murmur3_32((uint8_t*)key, strlen(key), 0);
    
    // Binary search for the right node
    int left = 0, right = ch->node_count - 1;
    while (left < right) {
        int mid = left + (right - left) / 2;
        if (ch->nodes[mid].hash < hash) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    // Wrap around if necessary
    if (left == ch->node_count) {
        left = 0;
    }
    
    return ch->nodes[left].server_id;
}

// 샤드 재분배
void rebalance_shards(consistent_hash_t* ch, int new_server_id) {
    // 새 서버를 위한 가상 노드 추가
    for (int i = 0; i < ch->virtual_nodes; i++) {
        char vnode[64];
        snprintf(vnode, sizeof(vnode), "server%d-%d", new_server_id, i);
        uint32_t hash = murmur3_32((uint8_t*)vnode, strlen(vnode), 0);
        
        // 정렬된 위치에 삽입
        // ... (삽입 로직)
    }
}
```

### 4.2 데이터 마이그레이션

```bash
#!/bin/bash
# data-migration.sh

echo "📦 데이터 마이그레이션 시작..."

OLD_SHARD=$1
NEW_SHARD=$2
KEY_RANGE_START=$3
KEY_RANGE_END=$4

# 1. 새 샤드 준비
ssh $NEW_SHARD "mkdir -p /var/lib/logcaster/data"

# 2. 데이터 복사 (온라인)
redis-cli --scan --pattern "log:*" | while read key; do
    HASH=$(echo -n "$key" | md5sum | cut -d' ' -f1)
    if [[ "$HASH" > "$KEY_RANGE_START" && "$HASH" < "$KEY_RANGE_END" ]]; then
        # 데이터 이동
        redis-cli --raw dump "$key" | head -c-1 | \
            redis-cli -h $NEW_SHARD --pipe
    fi
done

# 3. 검증
COUNT_OLD=$(redis-cli -h $OLD_SHARD dbsize | awk '{print $1}')
COUNT_NEW=$(redis-cli -h $NEW_SHARD dbsize | awk '{print $1}')
echo "이동된 키: $COUNT_NEW"

# 4. 라우팅 테이블 업데이트
curl -X POST http://config-server/api/shards \
    -d "{\"shard\": \"$NEW_SHARD\", \"range\": [\"$KEY_RANGE_START\", \"$KEY_RANGE_END\"]}"

echo "✅ 마이그레이션 완료"
```

---

## 5. 캐싱 전략

### 5.1 다층 캐싱

```c
// multi_tier_cache.c
typedef struct {
    // L1: 로컬 메모리 캐시
    struct {
        void* data;
        size_t size;
        time_t expiry;
    } l1_cache[1000];
    
    // L2: Redis 캐시
    redisContext* redis_conn;
    
    // L3: CDN 캐시
    char* cdn_endpoint;
} multi_tier_cache_t;

void* get_from_cache(multi_tier_cache_t* cache, const char* key) {
    // L1 캐시 확인
    uint32_t hash = hash_function(key) % 1000;
    if (cache->l1_cache[hash].data && 
        cache->l1_cache[hash].expiry > time(NULL)) {
        return cache->l1_cache[hash].data;
    }
    
    // L2 캐시 확인
    redisReply* reply = redisCommand(cache->redis_conn, "GET %s", key);
    if (reply && reply->type == REDIS_REPLY_STRING) {
        // L1 캐시에 저장
        cache->l1_cache[hash].data = strdup(reply->str);
        cache->l1_cache[hash].expiry = time(NULL) + 60;
        
        freeReplyObject(reply);
        return cache->l1_cache[hash].data;
    }
    
    // L3 캐시 (CDN) 확인
    // ... CDN API 호출
    
    return NULL;
}
```

### 5.2 Redis Cluster 설정

```bash
#!/bin/bash
# setup-redis-cluster.sh

# Redis 노드 시작
for port in 7000 7001 7002 7003 7004 7005; do
    mkdir -p /var/lib/redis/$port
    cat > /var/lib/redis/$port/redis.conf <<EOF
port $port
cluster-enabled yes
cluster-config-file nodes-$port.conf
cluster-node-timeout 5000
appendonly yes
maxmemory 2gb
maxmemory-policy allkeys-lru
EOF
    redis-server /var/lib/redis/$port/redis.conf &
done

sleep 5

# 클러스터 생성
redis-cli --cluster create \
    127.0.0.1:7000 127.0.0.1:7001 \
    127.0.0.1:7002 127.0.0.1:7003 \
    127.0.0.1:7004 127.0.0.1:7005 \
    --cluster-replicas 1 \
    --cluster-yes

echo "✅ Redis 클러스터 구성 완료"
```

---

## 6. 자동 스케일링 정책

### 6.1 메트릭 기반 스케일링

```python
# autoscaling_policy.py
import boto3
import time
from datetime import datetime, timedelta

class AutoScaler:
    def __init__(self):
        self.cloudwatch = boto3.client('cloudwatch')
        self.autoscaling = boto3.client('autoscaling')
        self.asg_name = 'logcaster-asg'
        
    def get_metrics(self):
        """최근 5분간 메트릭 수집"""
        end_time = datetime.utcnow()
        start_time = end_time - timedelta(minutes=5)
        
        response = self.cloudwatch.get_metric_statistics(
            Namespace='LogCaster',
            MetricName='ActiveConnections',
            Dimensions=[{'Name': 'ServiceName', 'Value': 'logcaster'}],
            StartTime=start_time,
            EndTime=end_time,
            Period=300,
            Statistics=['Average'],
            Unit='Count'
        )
        
        if response['Datapoints']:
            return response['Datapoints'][0]['Average']
        return 0
    
    def scale_decision(self, current_metric):
        """스케일링 결정"""
        # 현재 인스턴스 수
        response = self.autoscaling.describe_auto_scaling_groups(
            AutoScalingGroupNames=[self.asg_name]
        )
        current_capacity = response['AutoScalingGroups'][0]['DesiredCapacity']
        
        # 스케일링 규칙
        connections_per_instance = 1000
        required_instances = max(3, int(current_metric / connections_per_instance) + 1)
        
        if required_instances > current_capacity * 1.2:
            # Scale up
            new_capacity = min(50, current_capacity + 2)
            self.scale_to(new_capacity)
            return f"Scaled up to {new_capacity} instances"
            
        elif required_instances < current_capacity * 0.6:
            # Scale down
            new_capacity = max(3, current_capacity - 1)
            self.scale_to(new_capacity)
            return f"Scaled down to {new_capacity} instances"
            
        return "No scaling needed"
    
    def scale_to(self, capacity):
        """인스턴스 수 조정"""
        self.autoscaling.set_desired_capacity(
            AutoScalingGroupName=self.asg_name,
            DesiredCapacity=capacity,
            HonorCooldown=True
        )
    
    def predictive_scaling(self):
        """예측 기반 스케일링"""
        # 과거 패턴 분석
        historical_data = self.get_historical_patterns()
        
        # 시간대별 예측
        current_hour = datetime.now().hour
        if current_hour in [9, 10, 11, 14, 15, 16]:  # 피크 시간
            self.scale_to(10)
        elif current_hour in [0, 1, 2, 3, 4, 5]:  # 저조한 시간
            self.scale_to(3)
        else:
            self.scale_to(5)

# 실행
if __name__ == "__main__":
    scaler = AutoScaler()
    
    while True:
        metric = scaler.get_metrics()
        result = scaler.scale_decision(metric)
        print(f"[{datetime.now()}] Metric: {metric}, Action: {result}")
        time.sleep(60)
```

### 6.2 비용 최적화 스케일링

```bash
#!/bin/bash
# cost-optimized-scaling.sh

# Spot 인스턴스 혼합 사용
aws autoscaling update-auto-scaling-group \
    --auto-scaling-group-name logcaster-asg \
    --mixed-instances-policy '{
        "LaunchTemplate": {
            "LaunchTemplateSpecification": {
                "LaunchTemplateName": "logcaster-template",
                "Version": "$Latest"
            },
            "Overrides": [
                {"InstanceType": "t3.medium"},
                {"InstanceType": "t3a.medium"},
                {"InstanceType": "t2.medium"}
            ]
        },
        "InstancesDistribution": {
            "OnDemandAllocationStrategy": "prioritized",
            "OnDemandBaseCapacity": 2,
            "OnDemandPercentageAboveBaseCapacity": 30,
            "SpotAllocationStrategy": "lowest-price",
            "SpotInstancePools": 3
        }
    }'

# 예약 인스턴스 활용
aws ec2 purchase-reserved-instances-offering \
    --reserved-instances-offering-id offering-id \
    --instance-count 3

echo "✅ 비용 최적화 스케일링 설정 완료"
```

---

## 7. 글로벌 스케일링

### 7.1 지역별 배포

```yaml
# global-deployment.yaml
regions:
  us-east-1:
    instances: 5
    load_balancer: alb-us-east
    database: rds-us-east
    cache: elasticache-us-east
    
  eu-west-1:
    instances: 3
    load_balancer: alb-eu-west
    database: rds-eu-west
    cache: elasticache-eu-west
    
  ap-northeast-1:
    instances: 4
    load_balancer: alb-ap-northeast
    database: rds-ap-northeast
    cache: elasticache-ap-northeast

cross_region_replication:
  enabled: true
  consistency: eventual
  lag_threshold: 1000ms
```

### 7.2 GeoDNS 설정

```bash
# Route53 GeoDNS 설정
aws route53 change-resource-record-sets \
    --hosted-zone-id Z123456789 \
    --change-batch '{
        "Changes": [{
            "Action": "CREATE",
            "ResourceRecordSet": {
                "Name": "logcaster.example.com",
                "Type": "A",
                "SetIdentifier": "US",
                "GeoLocation": {
                    "CountryCode": "US"
                },
                "AliasTarget": {
                    "HostedZoneId": "Z215JYRZR8TBV",
                    "DNSName": "us-alb.example.com",
                    "EvaluateTargetHealth": true
                }
            }
        }]
    }'
```

---

## 8. 모니터링 및 알림

### 8.1 스케일링 이벤트 모니터링

```yaml
# scaling-alerts.yaml
alerts:
  - name: ScalingFrequent
    condition: count(scaling_events) > 10 in 1h
    action: notify_ops
    
  - name: ScalingFailed
    condition: scaling_status == "failed"
    action: page_oncall
    
  - name: CapacityLimit
    condition: current_instances >= max_instances * 0.9
    action: alert_capacity_planning
```

### 8.2 스케일링 대시보드

```json
{
  "dashboard": {
    "title": "LogCaster Scaling Dashboard",
    "panels": [
      {
        "title": "Instance Count",
        "query": "sum(up{job='logcaster'})"
      },
      {
        "title": "Scaling Events",
        "query": "rate(autoscaling_events_total[5m])"
      },
      {
        "title": "Cost Projection",
        "query": "instance_count * instance_cost_per_hour * 24 * 30"
      }
    ]
  }
}
```

---

## ✅ 체크리스트

### 필수 확인사항
- [ ] HPA/VPA 구성
- [ ] 로드 밸런서 설정
- [ ] 자동 스케일링 정책
- [ ] 모니터링 메트릭
- [ ] 비용 최적화 전략
- [ ] 장애 복구 계획

### 권장사항
- [ ] 예측 기반 스케일링
- [ ] 멀티 리전 배포
- [ ] 캐싱 계층 구성
- [ ] 데이터 샤딩
- [ ] Spot 인스턴스 활용
- [ ] 글로벌 로드 밸런싱

---

## 🎯 완료
이제 LogCaster의 포괄적인 배포 가이드가 완성되었습니다!