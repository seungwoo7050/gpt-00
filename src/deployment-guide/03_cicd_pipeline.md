# 03. CI/CD 파이프라인 구축

## 🎯 목표
GitHub Actions, GitLab CI, Jenkins 등을 사용하여 LogCaster의 자동 빌드, 테스트, 배포 파이프라인을 구축합니다.

## 📋 Prerequisites
- Git 저장소 (GitHub/GitLab/Bitbucket)
- Docker Hub 또는 프라이빗 레지스트리 계정
- 클라우드 배포 환경 준비
- 테스트 스위트 준비

---

## 1. GitHub Actions 파이프라인

### 1.1 빌드 및 테스트 워크플로우

```yaml
# .github/workflows/build-test.yml
name: Build and Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

env:
  REGISTRY: docker.io
  IMAGE_NAME_C: logcaster/logcaster-c
  IMAGE_NAME_CPP: logcaster/logcaster-cpp

jobs:
  build-c:
    runs-on: ubuntu-latest
    steps:
    - name: Checkout code
      uses: actions/checkout@v3
    
    - name: Setup CMake
      uses: jwlawson/actions-setup-cmake@v1.13
      with:
        cmake-version: '3.25'
    
    - name: Build LogCaster-C
      run: |
        cd LogCaster-C
        mkdir build && cd build
        cmake -DCMAKE_BUILD_TYPE=Release ..
        make -j$(nproc)
    
    - name: Run C tests
      run: |
        cd LogCaster-C
        python3 tests/test_client.py
        python3 tests/test_mvp2.py
        python3 tests/test_mvp3.py
    
    - name: Upload C artifacts
      uses: actions/upload-artifact@v3
      with:
        name: logcaster-c-binary
        path: LogCaster-C/build/logcaster-c

  build-cpp:
    runs-on: ubuntu-latest
    steps:
    - name: Checkout code
      uses: actions/checkout@v3
    
    - name: Setup CMake
      uses: jwlawson/actions-setup-cmake@v1.13
      with:
        cmake-version: '3.25'
    
    - name: Build LogCaster-CPP
      run: |
        cd LogCaster-CPP
        mkdir build && cd build
        cmake -DCMAKE_BUILD_TYPE=Release ..
        make -j$(nproc)
    
    - name: Run CPP tests
      run: |
        cd LogCaster-CPP
        python3 tests/test_client.py
        python3 tests/test_mvp2.py
        python3 tests/test_mvp3.py
        python3 test_irc.py
    
    - name: Upload CPP artifacts
      uses: actions/upload-artifact@v3
      with:
        name: logcaster-cpp-binary
        path: LogCaster-CPP/build/logcaster-cpp

  security-scan:
    runs-on: ubuntu-latest
    needs: [build-c, build-cpp]
    steps:
    - name: Checkout code
      uses: actions/checkout@v3
    
    - name: Run Trivy security scan
      uses: aquasecurity/trivy-action@master
      with:
        scan-type: 'fs'
        scan-ref: '.'
        format: 'sarif'
        output: 'trivy-results.sarif'
    
    - name: Upload Trivy results
      uses: github/codeql-action/upload-sarif@v2
      with:
        sarif_file: 'trivy-results.sarif'

  code-quality:
    runs-on: ubuntu-latest
    steps:
    - name: Checkout code
      uses: actions/checkout@v3
    
    - name: Static analysis with cppcheck
      run: |
        sudo apt-get update
        sudo apt-get install -y cppcheck
        cppcheck --enable=all --xml --xml-version=2 \
          LogCaster-C/src LogCaster-CPP/src 2> cppcheck.xml
    
    - name: SonarCloud Scan
      uses: SonarSource/sonarcloud-github-action@master
      env:
        GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
        SONAR_TOKEN: ${{ secrets.SONAR_TOKEN }}
```

### 1.2 Docker 빌드 및 푸시

```yaml
# .github/workflows/docker-publish.yml
name: Docker Build and Push

on:
  push:
    tags:
      - 'v*'
  workflow_dispatch:

jobs:
  docker-build:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        include:
          - context: LogCaster-C
            image: logcaster-c
            dockerfile: Dockerfile.c
          - context: LogCaster-CPP
            image: logcaster-cpp
            dockerfile: Dockerfile.cpp
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v3
    
    - name: Set up Docker Buildx
      uses: docker/setup-buildx-action@v2
    
    - name: Log in to Docker Hub
      uses: docker/login-action@v2
      with:
        username: ${{ secrets.DOCKER_USERNAME }}
        password: ${{ secrets.DOCKER_PASSWORD }}
    
    - name: Extract metadata
      id: meta
      uses: docker/metadata-action@v4
      with:
        images: ${{ secrets.DOCKER_USERNAME }}/${{ matrix.image }}
        tags: |
          type=ref,event=branch
          type=ref,event=pr
          type=semver,pattern={{version}}
          type=semver,pattern={{major}}.{{minor}}
          type=sha
    
    - name: Build and push Docker image
      uses: docker/build-push-action@v4
      with:
        context: .
        file: ${{ matrix.dockerfile }}
        platforms: linux/amd64,linux/arm64
        push: true
        tags: ${{ steps.meta.outputs.tags }}
        labels: ${{ steps.meta.outputs.labels }}
        cache-from: type=gha
        cache-to: type=gha,mode=max
```

### 1.3 배포 워크플로우

```yaml
# .github/workflows/deploy.yml
name: Deploy to Production

on:
  workflow_dispatch:
    inputs:
      environment:
        description: 'Environment to deploy to'
        required: true
        default: 'staging'
        type: choice
        options:
          - staging
          - production
      version:
        description: 'Version to deploy'
        required: true
        default: 'latest'

jobs:
  deploy:
    runs-on: ubuntu-latest
    environment: ${{ github.event.inputs.environment }}
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v3
    
    - name: Configure AWS credentials
      uses: aws-actions/configure-aws-credentials@v2
      with:
        aws-access-key-id: ${{ secrets.AWS_ACCESS_KEY_ID }}
        aws-secret-access-key: ${{ secrets.AWS_SECRET_ACCESS_KEY }}
        aws-region: ap-northeast-2
    
    - name: Deploy to ECS
      run: |
        # Update task definition
        aws ecs register-task-definition \
          --cli-input-json file://ecs-task-definition.json
        
        # Update service
        aws ecs update-service \
          --cluster logcaster-cluster \
          --service logcaster-service \
          --task-definition logcaster:${{ github.event.inputs.version }}
        
        # Wait for deployment
        aws ecs wait services-stable \
          --cluster logcaster-cluster \
          --services logcaster-service
    
    - name: Verify deployment
      run: |
        ./scripts/health-check.sh ${{ github.event.inputs.environment }}
    
    - name: Notify Slack
      uses: 8398a7/action-slack@v3
      with:
        status: ${{ job.status }}
        text: |
          Deployment to ${{ github.event.inputs.environment }} completed!
          Version: ${{ github.event.inputs.version }}
          Actor: ${{ github.actor }}
        webhook_url: ${{ secrets.SLACK_WEBHOOK }}
```

---

## 2. GitLab CI 파이프라인

### 2.1 .gitlab-ci.yml

```yaml
# .gitlab-ci.yml
stages:
  - build
  - test
  - security
  - package
  - deploy

variables:
  DOCKER_DRIVER: overlay2
  DOCKER_TLS_CERTDIR: "/certs"
  IMAGE_TAG: $CI_REGISTRY_IMAGE:$CI_COMMIT_SHA

before_script:
  - echo "Starting CI/CD Pipeline..."

# Build Stage
build:c:
  stage: build
  image: gcc:latest
  script:
    - apt-get update && apt-get install -y cmake
    - cd LogCaster-C
    - mkdir build && cd build
    - cmake -DCMAKE_BUILD_TYPE=Release ..
    - make -j$(nproc)
  artifacts:
    paths:
      - LogCaster-C/build/logcaster-c
    expire_in: 1 day

build:cpp:
  stage: build
  image: gcc:latest
  script:
    - apt-get update && apt-get install -y cmake
    - cd LogCaster-CPP
    - mkdir build && cd build
    - cmake -DCMAKE_BUILD_TYPE=Release ..
    - make -j$(nproc)
  artifacts:
    paths:
      - LogCaster-CPP/build/logcaster-cpp
    expire_in: 1 day

# Test Stage
test:unit:
  stage: test
  image: python:3.9
  dependencies:
    - build:c
    - build:cpp
  script:
    - pip install -r requirements-test.txt
    - pytest tests/ --junitxml=report.xml
  artifacts:
    reports:
      junit: report.xml

test:integration:
  stage: test
  image: python:3.9
  services:
    - docker:dind
  script:
    - docker-compose up -d
    - sleep 10
    - python3 tests/integration_test.py
    - docker-compose down

# Security Stage
security:scan:
  stage: security
  image: 
    name: aquasec/trivy:latest
    entrypoint: [""]
  script:
    - trivy fs --security-checks vuln,config --format json -o security-report.json .
  artifacts:
    reports:
      container_scanning: security-report.json

# Package Stage
package:docker:
  stage: package
  image: docker:latest
  services:
    - docker:dind
  script:
    - docker login -u $CI_REGISTRY_USER -p $CI_REGISTRY_PASSWORD $CI_REGISTRY
    - docker build -f Dockerfile.c -t $CI_REGISTRY_IMAGE/logcaster-c:$CI_COMMIT_SHA .
    - docker build -f Dockerfile.cpp -t $CI_REGISTRY_IMAGE/logcaster-cpp:$CI_COMMIT_SHA .
    - docker push $CI_REGISTRY_IMAGE/logcaster-c:$CI_COMMIT_SHA
    - docker push $CI_REGISTRY_IMAGE/logcaster-cpp:$CI_COMMIT_SHA
  only:
    - main
    - tags

# Deploy Stage
deploy:staging:
  stage: deploy
  image: alpine/helm:latest
  script:
    - helm upgrade --install logcaster-staging ./helm \
        --set image.tag=$CI_COMMIT_SHA \
        --namespace staging \
        --create-namespace
  environment:
    name: staging
    url: https://staging.logcaster.example.com
  only:
    - main

deploy:production:
  stage: deploy
  image: alpine/helm:latest
  script:
    - helm upgrade --install logcaster ./helm \
        --set image.tag=$CI_COMMIT_SHA \
        --namespace production \
        --create-namespace
  environment:
    name: production
    url: https://logcaster.example.com
  when: manual
  only:
    - tags
```

---

## 3. Jenkins 파이프라인

### 3.1 Jenkinsfile

```groovy
// Jenkinsfile
pipeline {
    agent any
    
    environment {
        DOCKER_REGISTRY = 'docker.io'
        DOCKER_CREDENTIALS = credentials('docker-hub-credentials')
        SONAR_TOKEN = credentials('sonar-token')
        KUBECONFIG = credentials('kubeconfig')
    }
    
    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }
        
        stage('Build') {
            parallel {
                stage('Build C') {
                    steps {
                        sh '''
                            cd LogCaster-C
                            mkdir -p build && cd build
                            cmake -DCMAKE_BUILD_TYPE=Release ..
                            make -j$(nproc)
                        '''
                    }
                }
                stage('Build CPP') {
                    steps {
                        sh '''
                            cd LogCaster-CPP
                            mkdir -p build && cd build
                            cmake -DCMAKE_BUILD_TYPE=Release ..
                            make -j$(nproc)
                        '''
                    }
                }
            }
        }
        
        stage('Test') {
            steps {
                sh '''
                    python3 -m venv venv
                    . venv/bin/activate
                    pip install -r requirements-test.txt
                    pytest tests/ --junitxml=test-results.xml
                '''
                junit 'test-results.xml'
            }
        }
        
        stage('Code Quality') {
            steps {
                withSonarQubeEnv('SonarQube') {
                    sh '''
                        sonar-scanner \
                            -Dsonar.projectKey=logcaster \
                            -Dsonar.sources=. \
                            -Dsonar.host.url=$SONAR_HOST_URL \
                            -Dsonar.login=$SONAR_TOKEN
                    '''
                }
                timeout(time: 1, unit: 'HOURS') {
                    waitForQualityGate abortPipeline: true
                }
            }
        }
        
        stage('Build Docker Images') {
            steps {
                script {
                    docker.withRegistry('https://registry.hub.docker.com', 'docker-hub-credentials') {
                        def imageC = docker.build("logcaster/logcaster-c:${env.BUILD_NUMBER}", "-f Dockerfile.c .")
                        def imageCpp = docker.build("logcaster/logcaster-cpp:${env.BUILD_NUMBER}", "-f Dockerfile.cpp .")
                        imageC.push()
                        imageC.push('latest')
                        imageCpp.push()
                        imageCpp.push('latest')
                    }
                }
            }
        }
        
        stage('Deploy to Staging') {
            steps {
                sh '''
                    kubectl --kubeconfig=$KUBECONFIG apply -f k8s/staging/
                    kubectl --kubeconfig=$KUBECONFIG set image deployment/logcaster-c \
                        logcaster-c=logcaster/logcaster-c:${BUILD_NUMBER} -n staging
                    kubectl --kubeconfig=$KUBECONFIG set image deployment/logcaster-cpp \
                        logcaster-cpp=logcaster/logcaster-cpp:${BUILD_NUMBER} -n staging
                '''
            }
        }
        
        stage('Integration Tests') {
            steps {
                sh '''
                    sleep 30
                    python3 tests/integration/test_deployed.py --env staging
                '''
            }
        }
        
        stage('Deploy to Production') {
            when {
                branch 'main'
            }
            input {
                message "Deploy to production?"
                ok "Yes, deploy to production"
            }
            steps {
                sh '''
                    kubectl --kubeconfig=$KUBECONFIG apply -f k8s/production/
                    kubectl --kubeconfig=$KUBECONFIG set image deployment/logcaster-c \
                        logcaster-c=logcaster/logcaster-c:${BUILD_NUMBER} -n production
                    kubectl --kubeconfig=$KUBECONFIG set image deployment/logcaster-cpp \
                        logcaster-cpp=logcaster/logcaster-cpp:${BUILD_NUMBER} -n production
                '''
            }
        }
    }
    
    post {
        success {
            slackSend(
                color: 'good',
                message: "Build #${env.BUILD_NUMBER} deployed successfully!"
            )
        }
        failure {
            slackSend(
                color: 'danger',
                message: "Build #${env.BUILD_NUMBER} failed!"
            )
        }
        always {
            cleanWs()
        }
    }
}
```

---

## 4. 자동화 스크립트

### 4.1 빌드 스크립트

```bash
#!/bin/bash
# scripts/build.sh

set -e

VERSION=${1:-latest}
REGISTRY=${2:-docker.io}

echo "Building LogCaster version: $VERSION"

# Build C version
echo "Building LogCaster-C..."
docker build -f Dockerfile.c -t $REGISTRY/logcaster/logcaster-c:$VERSION .

# Build CPP version
echo "Building LogCaster-CPP..."
docker build -f Dockerfile.cpp -t $REGISTRY/logcaster/logcaster-cpp:$VERSION .

# Run tests
echo "Running tests..."
docker run --rm $REGISTRY/logcaster/logcaster-c:$VERSION /usr/local/bin/logcaster-c --test
docker run --rm $REGISTRY/logcaster/logcaster-cpp:$VERSION /usr/local/bin/logcaster-cpp --test

echo "Build completed successfully!"
```

### 4.2 배포 스크립트

```bash
#!/bin/bash
# scripts/deploy.sh

set -e

ENVIRONMENT=${1:-staging}
VERSION=${2:-latest}

echo "Deploying LogCaster $VERSION to $ENVIRONMENT"

# Update Kubernetes deployments
kubectl set image deployment/logcaster-c \
    logcaster-c=logcaster/logcaster-c:$VERSION \
    -n $ENVIRONMENT

kubectl set image deployment/logcaster-cpp \
    logcaster-cpp=logcaster/logcaster-cpp:$VERSION \
    -n $ENVIRONMENT

# Wait for rollout
kubectl rollout status deployment/logcaster-c -n $ENVIRONMENT
kubectl rollout status deployment/logcaster-cpp -n $ENVIRONMENT

# Verify deployment
./scripts/health-check.sh $ENVIRONMENT

echo "Deployment completed successfully!"
```

### 4.3 롤백 스크립트

```bash
#!/bin/bash
# scripts/rollback.sh

set -e

ENVIRONMENT=${1:-staging}

echo "Rolling back LogCaster in $ENVIRONMENT"

# Rollback deployments
kubectl rollout undo deployment/logcaster-c -n $ENVIRONMENT
kubectl rollout undo deployment/logcaster-cpp -n $ENVIRONMENT

# Wait for rollback
kubectl rollout status deployment/logcaster-c -n $ENVIRONMENT
kubectl rollout status deployment/logcaster-cpp -n $ENVIRONMENT

# Verify rollback
./scripts/health-check.sh $ENVIRONMENT

echo "Rollback completed successfully!"
```

---

## 5. 모니터링 및 알림

### 5.1 Slack 통합

```yaml
# .github/workflows/notify.yml
name: Deployment Notifications

on:
  workflow_run:
    workflows: ["Deploy to Production"]
    types:
      - completed

jobs:
  notify:
    runs-on: ubuntu-latest
    steps:
    - name: Send Slack notification
      uses: slackapi/slack-github-action@v1.24.0
      with:
        payload: |
          {
            "text": "Deployment Status",
            "blocks": [
              {
                "type": "section",
                "text": {
                  "type": "mrkdwn",
                  "text": "*Deployment Status:* ${{ github.event.workflow_run.conclusion }}\n*Version:* ${{ github.event.workflow_run.head_sha }}\n*Environment:* Production"
                }
              }
            ]
          }
      env:
        SLACK_WEBHOOK_URL: ${{ secrets.SLACK_WEBHOOK_URL }}
```

---

## ✅ 체크리스트

### 필수 확인사항
- [ ] CI/CD 도구 선택 및 설정
- [ ] 빌드 파이프라인 구성
- [ ] 테스트 자동화
- [ ] Docker 이미지 빌드 및 푸시
- [ ] 배포 자동화
- [ ] 롤백 메커니즘

### 권장사항
- [ ] 코드 품질 검사
- [ ] 보안 스캔
- [ ] 성능 테스트
- [ ] 알림 설정
- [ ] 배포 승인 프로세스
- [ ] 아티팩트 저장소

---

## 🎯 다음 단계
→ [04_docker_registry.md](04_docker_registry.md) - Docker 레지스트리 설정