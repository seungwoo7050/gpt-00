# LogCaster C/C++ - DevOps Guide

## 🎯 Overview
Complete DevOps implementation for the LogCaster TCP log collection server, covering both C and C++ implementations with emphasis on systems programming best practices, performance optimization, and reliability.

## 📋 Table of Contents
1. [CI/CD Pipeline](#cicd-pipeline)
2. [Build Automation](#build-automation)
3. [Containerization](#containerization)
4. [Performance Testing](#performance-testing)
5. [Memory & Security Testing](#memory--security-testing)
6. [Deployment Strategies](#deployment-strategies)
7. [Monitoring & Logging](#monitoring--logging)
8. [Benchmarking](#benchmarking)

## CI/CD Pipeline

### GitHub Actions Workflow

**.github/workflows/logcaster-ci.yml**
```yaml
name: LogCaster CI/CD

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

env:
  CMAKE_VERSION: 3.25
  REGISTRY: ghcr.io
  IMAGE_NAME: ${{ github.repository }}

jobs:
  build-and-test:
    strategy:
      matrix:
        implementation: [c, cpp]
        compiler: [gcc-11, gcc-12, clang-14, clang-15]
        build-type: [Debug, Release]
    
    runs-on: ubuntu-22.04
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup build environment
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake valgrind cppcheck clang-tidy \
          libasan6 libubsan1 python3-pip
        pip3 install pytest asyncio
        
    - name: Configure CMake (${{ matrix.implementation }})
      run: |
        cd LogCaster-${{ matrix.implementation == 'c' && 'C' || 'CPP' }}
        mkdir build && cd build
        cmake .. \
          -DCMAKE_BUILD_TYPE=${{ matrix.build-type }} \
          -DCMAKE_C_COMPILER=${{ contains(matrix.compiler, 'gcc') && 'gcc' || 'clang' }} \
          -DCMAKE_CXX_COMPILER=${{ contains(matrix.compiler, 'gcc') && 'g++' || 'clang++' }} \
          -DENABLE_SANITIZERS=ON \
          -DENABLE_COVERAGE=ON
          
    - name: Build
      run: |
        cd LogCaster-${{ matrix.implementation == 'c' && 'C' || 'CPP' }}/build
        make -j$(nproc)
        
    - name: Run tests
      run: |
        cd LogCaster-${{ matrix.implementation == 'c' && 'C' || 'CPP' }}
        python3 tests/test_client.py
        python3 tests/test_mvp2.py
        python3 tests/test_mvp3.py
        python3 tests/test_persistence.py
        
    - name: Memory leak check
      if: matrix.build-type == 'Debug'
      run: |
        cd LogCaster-${{ matrix.implementation == 'c' && 'C' || 'CPP' }}
        valgrind --leak-check=full --error-exitcode=1 \
          ./build/logcaster-${{ matrix.implementation }} -t &
        SERVER_PID=$!
        sleep 5
        python3 tests/test_client.py --stress --duration 30
        kill $SERVER_PID
        
    - name: Static analysis
      run: |
        cd LogCaster-${{ matrix.implementation == 'c' && 'C' || 'CPP' }}
        if [ "${{ matrix.implementation }}" = "c" ]; then
          cppcheck --enable=all --error-exitcode=1 src/
        else
          clang-tidy src/*.cpp -- -Iinclude
        fi
        
    - name: Upload coverage
      if: matrix.build-type == 'Debug' && matrix.compiler == 'gcc-11'
      uses: codecov/codecov-action@v3
      with:
        directory: ./LogCaster-${{ matrix.implementation == 'c' && 'C' || 'CPP' }}/build
        flags: ${{ matrix.implementation }}

  performance-test:
    needs: build-and-test
    runs-on: ubuntu-22.04
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup environment
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake gcc g++ python3-pip siege apache2-utils
        pip3 install pytest asyncio matplotlib pandas
        
    - name: Build optimized versions
      run: |
        for impl in C CPP; do
          cd LogCaster-$impl
          mkdir build-perf && cd build-perf
          cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native"
          make -j$(nproc)
          cd ../..
        done
        
    - name: Run performance tests
      run: |
        # Start C server
        ./LogCaster-C/build-perf/logcaster-c -P &
        C_PID=$!
        sleep 2
        
        # Start C++ server on different port
        ./LogCaster-CPP/build-perf/logcaster-cpp -p 8999 -q 8998 -P &
        CPP_PID=$!
        sleep 2
        
        # Run benchmarks
        python3 tests/benchmark.py --output results.json
        
        # Cleanup
        kill $C_PID $CPP_PID
        
    - name: Upload performance results
      uses: actions/upload-artifact@v3
      with:
        name: performance-results
        path: results.json

  docker-build:
    needs: build-and-test
    runs-on: ubuntu-latest
    permissions:
      contents: read
      packages: write
      
    strategy:
      matrix:
        implementation: [c, cpp]
        
    steps:
    - uses: actions/checkout@v3
    
    - name: Set up Docker Buildx
      uses: docker/setup-buildx-action@v2
      
    - name: Log in to Container Registry
      uses: docker/login-action@v2
      with:
        registry: ${{ env.REGISTRY }}
        username: ${{ github.actor }}
        password: ${{ secrets.GITHUB_TOKEN }}
        
    - name: Build and push Docker image
      uses: docker/build-push-action@v4
      with:
        context: ./LogCaster-${{ matrix.implementation == 'c' && 'C' || 'CPP' }}
        push: true
        tags: |
          ${{ env.REGISTRY }}/${{ env.IMAGE_NAME }}-${{ matrix.implementation }}:latest
          ${{ env.REGISTRY }}/${{ env.IMAGE_NAME }}-${{ matrix.implementation }}:${{ github.sha }}
        cache-from: type=gha
        cache-to: type=gha,mode=max
```

### GitLab CI Alternative

**.gitlab-ci.yml**
```yaml
stages:
  - build
  - test
  - benchmark
  - deploy

variables:
  GIT_SUBMODULE_STRATEGY: recursive

.build-template:
  stage: build
  image: gcc:11
  before_script:
    - apt-get update -qq
    - apt-get install -y cmake
  artifacts:
    paths:
      - build/
    expire_in: 1 hour

build-c-debug:
  extends: .build-template
  script:
    - cd LogCaster-C
    - mkdir build && cd build
    - cmake .. -DCMAKE_BUILD_TYPE=Debug
    - make -j$(nproc)

build-cpp-release:
  extends: .build-template
  script:
    - cd LogCaster-CPP
    - mkdir build && cd build
    - cmake .. -DCMAKE_BUILD_TYPE=Release
    - make -j$(nproc)

test-valgrind:
  stage: test
  image: ubuntu:22.04
  needs: ["build-c-debug"]
  before_script:
    - apt-get update && apt-get install -y valgrind python3
  script:
    - cd LogCaster-C
    - valgrind --leak-check=full --track-origins=yes ./build/logcaster-c -t &
    - sleep 5
    - python3 tests/test_client.py
  artifacts:
    reports:
      junit: test-results.xml
```

## Build Automation

### CMake Configuration

**LogCaster-C/CMakeLists.txt**
```cmake
cmake_minimum_required(VERSION 3.20)
project(LogCaster-C VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(BUILD_STATIC "Build static binary" OFF)

# Compiler flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -DNDEBUG)
        
        # Link-time optimization
        include(CheckIPOSupported)
        check_ipo_supported(RESULT ipo_supported)
        if(ipo_supported)
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        endif()
    endif()
    
    if(ENABLE_SANITIZERS)
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    endif()
    
    if(ENABLE_COVERAGE)
        add_compile_options(--coverage)
        add_link_options(--coverage)
    endif()
endif()

# Source files
set(SOURCES
    src/main.c
    src/server.c
    src/thread_pool.c
    src/log_buffer.c
    src/query_parser.c
    src/query_handler.c
    src/persistence.c
)

# Headers
include_directories(include)

# Find packages
find_package(Threads REQUIRED)

# Executable
add_executable(logcaster-c ${SOURCES})
target_link_libraries(logcaster-c PRIVATE Threads::Threads m)

if(BUILD_STATIC)
    set_target_properties(logcaster-c PROPERTIES LINK_FLAGS "-static")
endif()

# Installation
install(TARGETS logcaster-c
    RUNTIME DESTINATION bin
)

install(FILES README.md
    DESTINATION share/doc/logcaster-c
)

# CPack configuration
set(CPACK_PACKAGE_NAME "LogCaster-C")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "DEB;RPM;TGZ")
include(CPack)

# Testing
enable_testing()
add_test(NAME basic_test COMMAND python3 ${CMAKE_SOURCE_DIR}/tests/test_client.py)
add_test(NAME stress_test COMMAND python3 ${CMAKE_SOURCE_DIR}/tests/test_client.py --stress)
```

### Makefile for Quick Builds

**Makefile**
```makefile
.PHONY: all clean test debug release profile sanitize

CC ?= gcc
CXX ?= g++
CFLAGS = -Wall -Wextra -Wpedantic -Iinclude -pthread
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++17 -Iinclude -pthread

# Directories
BUILD_DIR = build
BIN_DIR = bin
OBJ_DIR = obj

# Default target
all: release

# Debug build
debug: CFLAGS += -g -O0 -DDEBUG
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(BIN_DIR)/logcaster-c-debug $(BIN_DIR)/logcaster-cpp-debug

# Release build
release: CFLAGS += -O3 -DNDEBUG
release: CXXFLAGS += -O3 -DNDEBUG
release: $(BIN_DIR)/logcaster-c $(BIN_DIR)/logcaster-cpp

# Profile build
profile: CFLAGS += -O2 -pg
profile: CXXFLAGS += -O2 -pg
profile: $(BIN_DIR)/logcaster-c-profile $(BIN_DIR)/logcaster-cpp-profile

# Sanitizer build
sanitize: CFLAGS += -g -fsanitize=address,undefined
sanitize: CXXFLAGS += -g -fsanitize=address,undefined
sanitize: LDFLAGS += -fsanitize=address,undefined
sanitize: $(BIN_DIR)/logcaster-c-sanitize $(BIN_DIR)/logcaster-cpp-sanitize

# Testing
test: debug
	python3 tests/test_client.py
	python3 tests/test_mvp2.py
	python3 tests/test_mvp3.py

# Benchmarking
benchmark: release
	python3 tests/benchmark.py

# Static analysis
analyze:
	cppcheck --enable=all LogCaster-C/src/
	clang-tidy LogCaster-CPP/src/*.cpp -- -ILogCaster-CPP/include

# Clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(OBJ_DIR)
	find . -name "*.gcda" -delete
	find . -name "*.gcno" -delete
```

## Containerization

### Multi-stage Dockerfile for C Implementation

**LogCaster-C/Dockerfile**
```dockerfile
# Build stage
FROM gcc:11 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . .

# Build with optimizations
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON && \
    make -j$(nproc)

# Runtime stage
FROM scratch

# Copy static binary
COPY --from=builder /build/build/logcaster-c /logcaster

# Create necessary directories
WORKDIR /data

# Expose ports
EXPOSE 9999 9998

# Run the server
ENTRYPOINT ["/logcaster", "-P", "-d", "/data"]
```

### C++ Implementation with Alpine

**LogCaster-CPP/Dockerfile**
```dockerfile
# Build stage
FROM alpine:3.18 AS builder

RUN apk add --no-cache \
    g++ \
    cmake \
    make \
    linux-headers

WORKDIR /build
COPY . .

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Runtime stage
FROM alpine:3.18

RUN apk add --no-cache \
    libstdc++ \
    libgcc

WORKDIR /app
COPY --from=builder /build/build/logcaster-cpp .

# Health check
HEALTHCHECK --interval=30s --timeout=3s --retries=3 \
    CMD echo "PING" | nc localhost 9998 || exit 1

EXPOSE 9999 9998

CMD ["./logcaster-cpp", "-P"]
```

### Docker Compose for Testing

**docker-compose.yml**
```yaml
version: '3.8'

services:
  logcaster-c:
    build:
      context: ./LogCaster-C
      dockerfile: Dockerfile
    ports:
      - "9999:9999"
      - "9998:9998"
    volumes:
      - c-logs:/data
    environment:
      - LOG_LEVEL=INFO
      - MAX_CLIENTS=1000
    deploy:
      resources:
        limits:
          cpus: '2'
          memory: 1G
        reservations:
          cpus: '1'
          memory: 512M
          
  logcaster-cpp:
    build:
      context: ./LogCaster-CPP
      dockerfile: Dockerfile
    ports:
      - "8999:9999"
      - "8998:9998"
    volumes:
      - cpp-logs:/data
    environment:
      - LOG_LEVEL=INFO
      - MAX_CLIENTS=1000
      
  test-client:
    build:
      context: ./tests
      dockerfile: Dockerfile.client
    depends_on:
      - logcaster-c
      - logcaster-cpp
    environment:
      - TEST_SERVERS=logcaster-c:9999,logcaster-cpp:9999
      - TEST_DURATION=300
      - TEST_CLIENTS=100
      
  prometheus:
    image: prom/prometheus
    ports:
      - "9090:9090"
    volumes:
      - ./monitoring/prometheus.yml:/etc/prometheus/prometheus.yml
      
volumes:
  c-logs:
  cpp-logs:
```

## Performance Testing

### Load Testing Script

**tests/benchmark.py**
```python
#!/usr/bin/env python3
import asyncio
import time
import statistics
import json
import argparse
from concurrent.futures import ThreadPoolExecutor
import socket
import matplotlib.pyplot as plt

class BenchmarkClient:
    def __init__(self, host, port, query_port):
        self.host = host
        self.port = port
        self.query_port = query_port
        self.results = {
            'throughput': [],
            'latency': [],
            'errors': 0
        }
    
    async def send_log(self, message):
        start_time = time.time()
        try:
            reader, writer = await asyncio.open_connection(self.host, self.port)
            writer.write(f"{message}\n".encode())
            await writer.drain()
            writer.close()
            await writer.wait_closed()
            
            latency = (time.time() - start_time) * 1000  # ms
            self.results['latency'].append(latency)
            return True
        except Exception as e:
            self.results['errors'] += 1
            return False
    
    async def benchmark_throughput(self, duration, concurrent_clients):
        start_time = time.time()
        sent = 0
        
        async def client_task(client_id):
            nonlocal sent
            while time.time() - start_time < duration:
                message = f"Client {client_id} - Log message {sent}"
                if await self.send_log(message):
                    sent += 1
                await asyncio.sleep(0.001)  # 1ms between messages
        
        tasks = [client_task(i) for i in range(concurrent_clients)]
        await asyncio.gather(*tasks)
        
        elapsed = time.time() - start_time
        throughput = sent / elapsed
        self.results['throughput'].append(throughput)
        
        return throughput
    
    async def benchmark_query_performance(self, num_queries):
        query_times = []
        
        for _ in range(num_queries):
            start_time = time.time()
            try:
                reader, writer = await asyncio.open_connection(
                    self.host, self.query_port
                )
                writer.write(b"COUNT\n")
                await writer.drain()
                
                response = await reader.readline()
                query_time = (time.time() - start_time) * 1000
                query_times.append(query_time)
                
                writer.close()
                await writer.wait_closed()
            except:
                self.results['errors'] += 1
        
        return query_times

async def run_benchmark(host='localhost', c_port=9999, cpp_port=8999):
    results = {}
    
    # Test C implementation
    print("Benchmarking C implementation...")
    c_client = BenchmarkClient(host, c_port, c_port - 1)
    
    # Throughput test
    throughput = await c_client.benchmark_throughput(30, 100)
    print(f"C Throughput: {throughput:.2f} msgs/sec")
    
    # Query performance
    query_times = await c_client.benchmark_query_performance(1000)
    print(f"C Query latency: {statistics.mean(query_times):.2f}ms (p99: {statistics.quantiles(query_times, n=100)[98]:.2f}ms)")
    
    results['c'] = {
        'throughput': throughput,
        'latency_mean': statistics.mean(c_client.results['latency']),
        'latency_p99': statistics.quantiles(c_client.results['latency'], n=100)[98],
        'query_latency_mean': statistics.mean(query_times),
        'errors': c_client.results['errors']
    }
    
    # Test C++ implementation
    print("\nBenchmarking C++ implementation...")
    cpp_client = BenchmarkClient(host, cpp_port, cpp_port - 1)
    
    throughput = await cpp_client.benchmark_throughput(30, 100)
    print(f"C++ Throughput: {throughput:.2f} msgs/sec")
    
    query_times = await cpp_client.benchmark_query_performance(1000)
    print(f"C++ Query latency: {statistics.mean(query_times):.2f}ms (p99: {statistics.quantiles(query_times, n=100)[98]:.2f}ms)")
    
    results['cpp'] = {
        'throughput': throughput,
        'latency_mean': statistics.mean(cpp_client.results['latency']),
        'latency_p99': statistics.quantiles(cpp_client.results['latency'], n=100)[98],
        'query_latency_mean': statistics.mean(query_times),
        'errors': cpp_client.results['errors']
    }
    
    return results

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', default='benchmark_results.json')
    args = parser.parse_args()
    
    results = asyncio.run(run_benchmark())
    
    # Save results
    with open(args.output, 'w') as f:
        json.dump(results, f, indent=2)
    
    # Generate comparison chart
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))
    
    # Throughput comparison
    implementations = ['C', 'C++']
    throughputs = [results['c']['throughput'], results['cpp']['throughput']]
    ax1.bar(implementations, throughputs)
    ax1.set_ylabel('Messages/second')
    ax1.set_title('Throughput Comparison')
    
    # Latency comparison
    latencies = [
        [results['c']['latency_mean'], results['c']['latency_p99']],
        [results['cpp']['latency_mean'], results['cpp']['latency_p99']]
    ]
    x = range(len(implementations))
    width = 0.35
    ax2.bar([i - width/2 for i in x], [l[0] for l in latencies], width, label='Mean')
    ax2.bar([i + width/2 for i in x], [l[1] for l in latencies], width, label='P99')
    ax2.set_ylabel('Latency (ms)')
    ax2.set_title('Latency Comparison')
    ax2.set_xticks(x)
    ax2.set_xticklabels(implementations)
    ax2.legend()
    
    plt.tight_layout()
    plt.savefig('benchmark_comparison.png')
    print(f"\nResults saved to {args.output}")
```

### Stress Testing

**tests/stress_test.sh**
```bash
#!/bin/bash

# Stress test configuration
DURATION=300  # 5 minutes
CLIENTS=500
LOG_RATE=1000  # logs per second per client

echo "Starting stress test..."
echo "Duration: $DURATION seconds"
echo "Clients: $CLIENTS"
echo "Target rate: $((CLIENTS * LOG_RATE)) logs/second"

# Start monitoring
./monitor.sh &
MONITOR_PID=$!

# Run stress test
python3 tests/stress_client.py \
    --host localhost \
    --port 9999 \
    --clients $CLIENTS \
    --rate $LOG_RATE \
    --duration $DURATION \
    --output stress_results.json

# Stop monitoring
kill $MONITOR_PID

# Generate report
python3 tests/generate_report.py stress_results.json
```

## Memory & Security Testing

### Valgrind Test Suite

**tests/valgrind_test.sh**
```bash
#!/bin/bash

echo "Running Valgrind memory tests..."

# Test 1: Basic memory leaks
echo "Test 1: Memory leak detection"
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind-leaks.log \
         ./LogCaster-C/build/logcaster-c -t &

SERVER_PID=$!
sleep 5

# Send test data
python3 tests/test_client.py --duration 60

kill $SERVER_PID
wait $SERVER_PID

# Test 2: Helgrind for thread issues
echo "Test 2: Thread safety analysis"
valgrind --tool=helgrind \
         --log-file=valgrind-threads.log \
         ./LogCaster-C/build/logcaster-c -t &

SERVER_PID=$!
sleep 5

# Concurrent stress test
python3 tests/test_client.py --stress --clients 50 --duration 30

kill $SERVER_PID

# Test 3: Massif for heap profiling
echo "Test 3: Heap profiling"
valgrind --tool=massif \
         --massif-out-file=massif.out \
         ./LogCaster-C/build/logcaster-c -P &

SERVER_PID=$!
sleep 5

# Generate load
python3 tests/test_persistence.py

kill $SERVER_PID

# Generate heap profile graph
ms_print massif.out > heap_profile.txt

echo "Valgrind tests completed. Check log files for results."
```

### Fuzzing

**tests/fuzz_test.c**
```c
#include <stdint.h>
#include <string.h>
#include "../include/query_parser.h"

// LibFuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) return 0;
    
    // Null-terminate the input
    char *input = malloc(size + 1);
    memcpy(input, data, size);
    input[size] = '\0';
    
    // Test query parser
    query_t query;
    parse_query(input, &query);
    
    free(input);
    return 0;
}
```

**Build fuzzer:**
```bash
clang -g -O1 -fsanitize=fuzzer,address \
    tests/fuzz_test.c \
    LogCaster-C/src/query_parser.c \
    -I LogCaster-C/include \
    -o query_fuzzer

# Run fuzzer
./query_fuzzer -max_total_time=300 -max_len=1024
```

## Deployment Strategies

### Systemd Service

**/etc/systemd/system/logcaster.service**
```ini
[Unit]
Description=LogCaster Log Collection Server
After=network.target

[Service]
Type=simple
User=logcaster
Group=logcaster
WorkingDirectory=/opt/logcaster
ExecStart=/opt/logcaster/bin/logcaster-c -P -d /var/lib/logcaster
Restart=always
RestartSec=5

# Security
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ReadWritePaths=/var/lib/logcaster

# Resource limits
LimitNOFILE=65536
LimitNPROC=4096
MemoryLimit=2G
CPUQuota=200%

[Install]
WantedBy=multi-user.target
```

### Kubernetes Deployment

**k8s/deployment.yaml**
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: logcaster
  labels:
    app: logcaster
spec:
  replicas: 3
  selector:
    matchLabels:
      app: logcaster
  template:
    metadata:
      labels:
        app: logcaster
    spec:
      containers:
      - name: logcaster
        image: ghcr.io/yourorg/logcaster-c:latest
        ports:
        - containerPort: 9999
          name: logs
          protocol: TCP
        - containerPort: 9998
          name: query
          protocol: TCP
        env:
        - name: MAX_BUFFER_SIZE
          value: "50000"
        - name: PERSISTENCE_PATH
          value: "/data"
        resources:
          requests:
            memory: "512Mi"
            cpu: "500m"
          limits:
            memory: "2Gi"
            cpu: "2000m"
        livenessProbe:
          tcpSocket:
            port: 9999
          initialDelaySeconds: 10
          periodSeconds: 10
        readinessProbe:
          exec:
            command:
            - /bin/sh
            - -c
            - "echo 'PING' | nc localhost 9998"
          initialDelaySeconds: 5
          periodSeconds: 5
        volumeMounts:
        - name: data
          mountPath: /data
      volumes:
      - name: data
        persistentVolumeClaim:
          claimName: logcaster-data
---
apiVersion: v1
kind: Service
metadata:
  name: logcaster
spec:
  selector:
    app: logcaster
  ports:
  - name: logs
    port: 9999
    targetPort: 9999
  - name: query
    port: 9998
    targetPort: 9998
  type: LoadBalancer
```

### Ansible Playbook

**ansible/deploy-logcaster.yml**
```yaml
---
- name: Deploy LogCaster
  hosts: logcaster_servers
  become: yes
  
  vars:
    logcaster_version: "1.0.0"
    logcaster_user: "logcaster"
    logcaster_group: "logcaster"
    logcaster_home: "/opt/logcaster"
    
  tasks:
    - name: Create logcaster user
      user:
        name: "{{ logcaster_user }}"
        group: "{{ logcaster_group }}"
        system: yes
        shell: /bin/false
        home: "{{ logcaster_home }}"
        
    - name: Install dependencies
      package:
        name:
          - gcc
          - make
          - cmake
        state: present
        
    - name: Download and extract LogCaster
      unarchive:
        src: "https://github.com/yourorg/logcaster/releases/download/v{{ logcaster_version }}/logcaster-{{ logcaster_version }}.tar.gz"
        dest: "{{ logcaster_home }}"
        remote_src: yes
        owner: "{{ logcaster_user }}"
        group: "{{ logcaster_group }}"
        
    - name: Build LogCaster
      command: |
        cmake -B build -DCMAKE_BUILD_TYPE=Release
        cmake --build build
      args:
        chdir: "{{ logcaster_home }}"
        
    - name: Install systemd service
      template:
        src: logcaster.service.j2
        dest: /etc/systemd/system/logcaster.service
        
    - name: Start and enable LogCaster
      systemd:
        name: logcaster
        state: started
        enabled: yes
        daemon_reload: yes
```

## Monitoring & Logging

### Prometheus Metrics Export

**src/metrics.c**
```c
// Simple Prometheus metrics exporter
#include <stdio.h>
#include <time.h>
#include "metrics.h"

typedef struct {
    atomic_uint_fast64_t total_connections;
    atomic_uint_fast64_t active_connections;
    atomic_uint_fast64_t total_logs_received;
    atomic_uint_fast64_t total_queries_processed;
    atomic_uint_fast64_t buffer_size;
    atomic_uint_fast64_t disk_writes;
} metrics_t;

static metrics_t metrics = {0};

void metrics_export(int client_fd) {
    char buffer[4096];
    time_t now = time(NULL);
    
    snprintf(buffer, sizeof(buffer),
        "# HELP logcaster_connections_total Total number of connections\n"
        "# TYPE logcaster_connections_total counter\n"
        "logcaster_connections_total %lu\n"
        "\n"
        "# HELP logcaster_connections_active Current active connections\n"
        "# TYPE logcaster_connections_active gauge\n"
        "logcaster_connections_active %lu\n"
        "\n"
        "# HELP logcaster_logs_total Total logs received\n"
        "# TYPE logcaster_logs_total counter\n"
        "logcaster_logs_total %lu\n"
        "\n"
        "# HELP logcaster_queries_total Total queries processed\n"
        "# TYPE logcaster_queries_total counter\n"
        "logcaster_queries_total %lu\n"
        "\n"
        "# HELP logcaster_buffer_size_bytes Current buffer size\n"
        "# TYPE logcaster_buffer_size_bytes gauge\n"
        "logcaster_buffer_size_bytes %lu\n",
        metrics.total_connections,
        metrics.active_connections,
        metrics.total_logs_received,
        metrics.total_queries_processed,
        metrics.buffer_size * sizeof(log_entry_t)
    );
    
    send(client_fd, buffer, strlen(buffer), 0);
}
```

### Grafana Dashboard

**monitoring/grafana-dashboard.json**
```json
{
  "dashboard": {
    "title": "LogCaster Performance",
    "panels": [
      {
        "title": "Throughput",
        "targets": [{
          "expr": "rate(logcaster_logs_total[5m])"
        }]
      },
      {
        "title": "Active Connections",
        "targets": [{
          "expr": "logcaster_connections_active"
        }]
      },
      {
        "title": "Buffer Usage",
        "targets": [{
          "expr": "logcaster_buffer_size_bytes / 1024 / 1024"
        }]
      },
      {
        "title": "Query Rate",
        "targets": [{
          "expr": "rate(logcaster_queries_total[5m])"
        }]
      }
    ]
  }
}
```

## Benchmarking

### Performance Comparison Script

**benchmark/compare.sh**
```bash
#!/bin/bash

echo "LogCaster C vs C++ Performance Comparison"
echo "=========================================="

# Build optimized versions
echo "Building optimized binaries..."
cd LogCaster-C && mkdir -p build-bench && cd build-bench
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O3 -march=native"
make -j$(nproc)
cd ../..

cd LogCaster-CPP && mkdir -p build-bench && cd build-bench
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native"
make -j$(nproc)
cd ../..

# Memory usage comparison
echo -e "\nMemory Usage Test:"
/usr/bin/time -v ./LogCaster-C/build-bench/logcaster-c -t &
C_PID=$!
sleep 60
kill $C_PID

/usr/bin/time -v ./LogCaster-CPP/build-bench/logcaster-cpp -t &
CPP_PID=$!
sleep 60
kill $CPP_PID

# CPU profiling
echo -e "\nCPU Profiling:"
perf record -g ./LogCaster-C/build-bench/logcaster-c -t &
C_PID=$!
python3 tests/test_client.py --stress --duration 60
kill $C_PID
perf report > c_perf_report.txt

# Binary size comparison
echo -e "\nBinary Size Comparison:"
ls -lh LogCaster-C/build-bench/logcaster-c
ls -lh LogCaster-CPP/build-bench/logcaster-cpp

# Startup time
echo -e "\nStartup Time:"
time ./LogCaster-C/build-bench/logcaster-c --version
time ./LogCaster-CPP/build-bench/logcaster-cpp --version
```

## 🚀 Quick Commands

```bash
# Build all versions
make all

# Run tests
make test

# Performance benchmark
make benchmark

# Security scan
make security-scan

# Docker build
docker build -t logcaster:latest .

# Deploy to Kubernetes
kubectl apply -f k8s/

# View metrics
curl http://localhost:9090/metrics

# Stress test
./tests/stress_test.sh

# Memory profiling
valgrind --tool=massif ./logcaster-c
```

## 📊 Performance Targets

1. **Throughput**: > 50,000 logs/second
2. **Query Latency**: < 10ms (p99)
3. **Memory Usage**: < 100MB for 10,000 logs
4. **CPU Usage**: < 50% on single core
5. **Startup Time**: < 100ms

## 🔒 Security Checklist

- [ ] Input validation for all queries
- [ ] Buffer overflow protection
- [ ] Resource limits enforced
- [ ] No memory leaks (Valgrind clean)
- [ ] Thread-safe operations
- [ ] Secure file permissions
- [ ] Rate limiting implemented

## 📚 Additional Resources

- [C Best Practices](./docs/C_BEST_PRACTICES.md)
- [Performance Tuning Guide](./docs/PERFORMANCE.md)
- [Security Hardening](./docs/SECURITY.md)
- [Troubleshooting Guide](./docs/TROUBLESHOOTING.md)