# 02. 클라우드 리소스 설정

## 🎯 목표
AWS, GCP, Azure 등 주요 클라우드 플랫폼에서 LogCaster를 위한 리소스를 프로비저닝하고 설정합니다.

## 📋 Prerequisites
- 클라우드 계정 (AWS/GCP/Azure)
- CLI 도구 설치 (aws-cli, gcloud, az)
- Terraform 0.14+ (선택사항)
- kubectl 1.21+ (Kubernetes 사용 시)

---

## 1. AWS 배포

### 1.1 EC2 인스턴스 설정

```bash
# AWS CLI를 사용한 EC2 인스턴스 생성
aws ec2 run-instances \
    --image-id ami-0c94855ba95c574c8 \
    --instance-type t3.medium \
    --key-name logcaster-key \
    --security-group-ids sg-0123456789abcdef0 \
    --subnet-id subnet-0123456789abcdef0 \
    --tag-specifications 'ResourceType=instance,Tags=[{Key=Name,Value=logcaster-server}]' \
    --user-data file://user-data.sh
```

### 1.2 보안 그룹 설정

```bash
# 보안 그룹 생성
aws ec2 create-security-group \
    --group-name logcaster-sg \
    --description "Security group for LogCaster"

# 인바운드 규칙 추가
aws ec2 authorize-security-group-ingress \
    --group-name logcaster-sg \
    --protocol tcp --port 9999 --cidr 0.0.0.0/0 \
    --protocol tcp --port 9998 --cidr 0.0.0.0/0 \
    --protocol tcp --port 8999 --cidr 0.0.0.0/0 \
    --protocol tcp --port 8998 --cidr 0.0.0.0/0 \
    --protocol tcp --port 6667 --cidr 0.0.0.0/0 \
    --protocol tcp --port 22 --cidr YOUR_IP/32
```

### 1.3 EKS 클러스터 설정 (Kubernetes)

```yaml
# eks-cluster.yaml
apiVersion: eksctl.io/v1alpha5
kind: ClusterConfig

metadata:
  name: logcaster-cluster
  region: ap-northeast-2
  version: "1.27"

nodeGroups:
  - name: logcaster-nodes
    instanceType: t3.medium
    desiredCapacity: 3
    minSize: 1
    maxSize: 5
    volumeSize: 50
    ssh:
      allow: true
      publicKeyName: logcaster-key
    labels:
      app: logcaster
    tags:
      Environment: production
```

```bash
# 클러스터 생성
eksctl create cluster -f eks-cluster.yaml
```

### 1.4 RDS 데이터베이스 (선택사항)

```bash
# RDS PostgreSQL 인스턴스 생성 (로그 메타데이터 저장용)
aws rds create-db-instance \
    --db-instance-identifier logcaster-db \
    --db-instance-class db.t3.micro \
    --engine postgres \
    --engine-version 14.7 \
    --master-username admin \
    --master-user-password ${DB_PASSWORD} \
    --allocated-storage 20 \
    --backup-retention-period 7 \
    --multi-az
```

### 1.5 S3 버킷 (로그 아카이빙)

```bash
# S3 버킷 생성
aws s3 mb s3://logcaster-logs-archive

# 라이프사이클 정책 설정
aws s3api put-bucket-lifecycle-configuration \
    --bucket logcaster-logs-archive \
    --lifecycle-configuration file://s3-lifecycle.json
```

```json
// s3-lifecycle.json
{
  "Rules": [
    {
      "Id": "ArchiveOldLogs",
      "Status": "Enabled",
      "Transitions": [
        {
          "Days": 30,
          "StorageClass": "STANDARD_IA"
        },
        {
          "Days": 90,
          "StorageClass": "GLACIER"
        }
      ],
      "Expiration": {
        "Days": 365
      }
    }
  ]
}
```

---

## 2. GCP 배포

### 2.1 Compute Engine 인스턴스

```bash
# GCE 인스턴스 생성
gcloud compute instances create logcaster-server \
    --zone=asia-northeast3-a \
    --machine-type=e2-medium \
    --image-family=ubuntu-2204-lts \
    --image-project=ubuntu-os-cloud \
    --boot-disk-size=50GB \
    --tags=logcaster \
    --metadata-from-file startup-script=startup.sh
```

### 2.2 방화벽 규칙

```bash
# 방화벽 규칙 생성
gcloud compute firewall-rules create logcaster-allow-ports \
    --allow tcp:9999,tcp:9998,tcp:8999,tcp:8998,tcp:6667 \
    --source-ranges 0.0.0.0/0 \
    --target-tags logcaster
```

### 2.3 GKE 클러스터

```bash
# GKE 클러스터 생성
gcloud container clusters create logcaster-cluster \
    --zone asia-northeast3-a \
    --num-nodes 3 \
    --machine-type e2-medium \
    --enable-autoscaling \
    --min-nodes 1 \
    --max-nodes 5 \
    --enable-autorepair \
    --enable-autoupgrade
```

### 2.4 Cloud Storage (로그 아카이빙)

```bash
# Storage 버킷 생성
gsutil mb -c STANDARD -l asia-northeast3 gs://logcaster-logs-archive

# 라이프사이클 설정
gsutil lifecycle set lifecycle.json gs://logcaster-logs-archive
```

---

## 3. Azure 배포

### 3.1 Virtual Machine

```bash
# 리소스 그룹 생성
az group create --name logcaster-rg --location koreacentral

# VM 생성
az vm create \
    --resource-group logcaster-rg \
    --name logcaster-vm \
    --image UbuntuLTS \
    --size Standard_B2s \
    --admin-username azureuser \
    --generate-ssh-keys \
    --custom-data cloud-init.yaml
```

### 3.2 네트워크 보안 그룹

```bash
# NSG 규칙 추가
az network nsg rule create \
    --resource-group logcaster-rg \
    --nsg-name logcaster-vmNSG \
    --name AllowLogCasterPorts \
    --priority 100 \
    --destination-port-ranges 9999 9998 8999 8998 6667 \
    --access Allow \
    --protocol Tcp
```

### 3.3 AKS 클러스터

```bash
# AKS 클러스터 생성
az aks create \
    --resource-group logcaster-rg \
    --name logcaster-aks \
    --node-count 3 \
    --node-vm-size Standard_B2s \
    --enable-cluster-autoscaler \
    --min-count 1 \
    --max-count 5 \
    --generate-ssh-keys
```

---

## 4. Terraform을 사용한 IaC

### 4.1 AWS Terraform 구성

```hcl
# main.tf
terraform {
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 5.0"
    }
  }
}

provider "aws" {
  region = var.aws_region
}

# VPC
resource "aws_vpc" "logcaster" {
  cidr_block           = "10.0.0.0/16"
  enable_dns_hostnames = true
  enable_dns_support   = true

  tags = {
    Name = "logcaster-vpc"
  }
}

# Subnets
resource "aws_subnet" "public" {
  count                   = 2
  vpc_id                  = aws_vpc.logcaster.id
  cidr_block              = "10.0.${count.index + 1}.0/24"
  availability_zone       = data.aws_availability_zones.available.names[count.index]
  map_public_ip_on_launch = true

  tags = {
    Name = "logcaster-public-${count.index + 1}"
  }
}

# EC2 Instances
resource "aws_instance" "logcaster" {
  count         = var.instance_count
  ami           = data.aws_ami.ubuntu.id
  instance_type = var.instance_type
  subnet_id     = aws_subnet.public[count.index % 2].id
  
  user_data = templatefile("${path.module}/user-data.sh", {
    instance_id = count.index
  })

  tags = {
    Name = "logcaster-${count.index}"
  }
}

# Load Balancer
resource "aws_lb" "logcaster" {
  name               = "logcaster-lb"
  internal           = false
  load_balancer_type = "network"
  subnets            = aws_subnet.public[*].id

  tags = {
    Name = "logcaster-lb"
  }
}
```

### 4.2 변수 정의

```hcl
# variables.tf
variable "aws_region" {
  description = "AWS region"
  type        = string
  default     = "ap-northeast-2"
}

variable "instance_count" {
  description = "Number of EC2 instances"
  type        = number
  default     = 2
}

variable "instance_type" {
  description = "EC2 instance type"
  type        = string
  default     = "t3.medium"
}
```

---

## 5. Kubernetes 매니페스트

### 5.1 Deployment

```yaml
# logcaster-deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: logcaster-c
  namespace: logcaster
spec:
  replicas: 3
  selector:
    matchLabels:
      app: logcaster-c
  template:
    metadata:
      labels:
        app: logcaster-c
    spec:
      containers:
      - name: logcaster
        image: logcaster/logcaster-c:latest
        ports:
        - containerPort: 9999
          name: log-input
        - containerPort: 9998
          name: query
        env:
        - name: PERSISTENCE_ENABLED
          value: "true"
        - name: LOG_LEVEL
          value: "INFO"
        resources:
          requests:
            memory: "256Mi"
            cpu: "250m"
          limits:
            memory: "1Gi"
            cpu: "1000m"
        livenessProbe:
          tcpSocket:
            port: 9999
          initialDelaySeconds: 10
          periodSeconds: 10
        readinessProbe:
          tcpSocket:
            port: 9998
          initialDelaySeconds: 5
          periodSeconds: 5
        volumeMounts:
        - name: log-storage
          mountPath: /var/log/logcaster
      volumes:
      - name: log-storage
        persistentVolumeClaim:
          claimName: logcaster-pvc
```

### 5.2 Service

```yaml
# logcaster-service.yaml
apiVersion: v1
kind: Service
metadata:
  name: logcaster-c-service
  namespace: logcaster
spec:
  type: LoadBalancer
  selector:
    app: logcaster-c
  ports:
  - name: log-input
    port: 9999
    targetPort: 9999
    protocol: TCP
  - name: query
    port: 9998
    targetPort: 9998
    protocol: TCP
```

### 5.3 PersistentVolume

```yaml
# logcaster-pv.yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: logcaster-pvc
  namespace: logcaster
spec:
  accessModes:
    - ReadWriteMany
  storageClassName: standard
  resources:
    requests:
      storage: 50Gi
```

---

## 6. 비용 최적화

### 6.1 예상 비용 (AWS 기준)

#### 최소 구성 (개발/스테이징)
- **월 $150-200**
  - EC2 t3.medium (1대): $30
  - EBS 50GB: $5
  - 네트워크 전송: $20
  - S3 스토리지: $5
  - CloudWatch: $10

#### 프로덕션 구성
- **월 $500-800**
  - EC2 t3.large (3대): $180
  - Application Load Balancer: $25
  - EBS 100GB x3: $30
  - RDS t3.micro: $15
  - S3 스토리지 (1TB): $25
  - 네트워크 전송: $100
  - CloudWatch 상세 모니터링: $30

### 6.2 비용 절감 팁

```bash
# Spot 인스턴스 사용 (최대 70% 절감)
aws ec2 request-spot-instances \
    --instance-count 1 \
    --type "persistent" \
    --launch-specification file://spot-spec.json

# 예약 인스턴스 구매 (최대 40% 절감)
aws ec2 purchase-reserved-instances-offering \
    --reserved-instances-offering-id offering-id \
    --instance-count 1

# 자동 스케일링 설정
aws autoscaling create-auto-scaling-group \
    --auto-scaling-group-name logcaster-asg \
    --min-size 1 \
    --max-size 5 \
    --desired-capacity 2
```

---

## ✅ 체크리스트

### 필수 확인사항
- [ ] 클라우드 계정 및 권한 설정
- [ ] 네트워크 구성 (VPC, 서브넷)
- [ ] 보안 그룹/방화벽 규칙
- [ ] 컴퓨팅 리소스 프로비저닝
- [ ] 스토리지 설정
- [ ] 로드 밸런서 구성

### 권장사항
- [ ] 자동 스케일링 설정
- [ ] 백업 정책 구성
- [ ] 모니터링 대시보드
- [ ] 비용 알림 설정
- [ ] 태그 정책 적용

---

## 🎯 다음 단계
→ [03_cicd_pipeline.md](03_cicd_pipeline.md) - CI/CD 파이프라인 구축