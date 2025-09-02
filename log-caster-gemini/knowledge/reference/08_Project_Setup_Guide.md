# 4. 프로젝트 개발환경 준비하기 🛠️
## 실제 개발을 위한 첫 걸음

*LogCaster 프로젝트를 위한 사전지식 문서*

---

## 📋 문서 정보
- **난이도**: 🟡 중급 → 🔴 고급
- **예상 학습 시간**: 10-12시간
- **전제 조건**: 
  - 이전 3개 문서의 내용 숙지
  - Linux/macOS 개발 환경
  - 기본적인 빌드 도구 사용 경험
- **실습 환경**: Linux/macOS, GCC/G++, CMake, Git

## 🎯 이 문서에서 배울 내용

**"드디어 실제 프로젝트를 만들어볼 시간이에요! 🎉"**

지금까지 C/C++의 기초, 메모리 관리, 데이터 구조, 객체지향까지 배웠다면, 이제 진짜 실력을 발휘할 때입니다! 

🧑‍💻 **개발자가 되는 여정**을 생각해보세요:
- 첫 번째 단계: 언어 배우기 ✅ (지금까지 완료!)
- 두 번째 단계: **실제 프로젝트 만들기** 👈 (지금 여기!)
- 세 번째 단계: 포트폴리오 완성하기

이 문서에서는 개발환경 설정부터 테스트 방법론, 디버깅 기법, 그리고 단계별 구현 가이드까지 실제 프로젝트 개발에 필요한 모든 실습 정보를 제공합니다. 마치 **요리를 위해 재료와 도구를 준비하는 것**처럼, 프로젝트를 위한 모든 준비를 완벽하게 해보겠습니다!

---

## 🛠️ 1. 개발환경 설정

**"좋은 도구가 좋은 결과를 만든다! 🔧"**

개발환경 설정은 마치 **요리할 때 칼과 도마를 준비하는 것**과 같습니다. 처음에 제대로 준비해두면 나중에 훨씬 편하게 개발할 수 있어요!

### 필수 도구 설치

#### Linux/macOS 환경

**마치 게임에서 아이템을 모으는 것처럼, 하나씩 설치해봅시다! 🎮**

```bash
# 🔧 1. 컴파일러 설치 (가장 중요한 도구!)
## Ubuntu/Debian (리눅스 사용자)
sudo apt update  # 패키지 목록 업데이트
sudo apt install build-essential gcc g++ clang cmake make
# build-essential: C/C++ 개발에 필요한 모든 기본 도구들이 들어있어요!

## macOS (Mac 사용자 - Homebrew 필요)
brew install gcc cmake make
# Homebrew가 없다면: /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 🐛 2. 디버깅 도구 (버그를 잡는 명탐정 도구들!)
sudo apt install gdb valgrind  # Linux
brew install gdb               # macOS
# gdb: 프로그램 내부를 들여다보는 현미경
# valgrind: 메모리 문제를 찾아내는 탐지기

# ⚡ 3. 성능 분석 도구 (프로그램이 얼마나 빠른지 측정!)
sudo apt install perf linux-tools-generic  # Linux
brew install gperftools                     # macOS
# 내 프로그램이 어디서 느려지는지 찾아내는 도구

# 🌐 4. 네트워크 분석 도구 (인터넷 통신 확인!)
sudo apt install netcat-openbsd tcpdump wireshark
brew install netcat tcpdump wireshark
# netcat(nc): 간단한 네트워크 테스트 도구
# tcpdump: 네트워크 패킷 분석기

# 🏗️ 5. 빌드 도구 (프로젝트를 조립하는 도구!)
sudo apt install ninja-build pkg-config
brew install ninja pkg-config
# ninja: 매우 빠른 빌드 시스템
# pkg-config: 라이브러리 정보 관리

# 📁 6. 버전 관리 (코드의 역사를 기록!)
sudo apt install git
brew install git
# git: 코드 변경사항을 추적하고 관리하는 필수 도구
```

#### 개발환경 검증

**"설치가 끝났다면 제대로 동작하는지 확인해봅시다! ✅"**

마치 **새 차를 산 후 시동을 걸어보는 것**처럼, 모든 도구가 정상 작동하는지 테스트해보겠습니다.

```bash
# 🔍 컴파일러 버전 확인 (우리의 주요 도구들이 준비됐나요?)
gcc --version    # C 컴파일러 확인
g++ --version    # C++ 컴파일러 확인 
clang --version  # 또 다른 컴파일러 확인

# 📋 CMake 버전 확인 (3.10 이상이어야 해요!)
cmake --version
# 만약 버전이 낮다면 최신 버전을 설치하세요

# 🧵 스레드 지원 확인 (멀티스레딩이 가능한지 테스트)
# 간단한 테스트 프로그램을 만들어서 실행해봅시다
echo '#include <iostream>
#include <thread>
int main() { 
    std::cout << "🎉 축하합니다! C++17이 정상 작동합니다!" << std::endl;
    std::cout << "💻 사용 가능한 하드웨어 스레드 수: " << std::thread::hardware_concurrency() << std::endl; 
    return 0; 
}' > test_threads.cpp

# 컴파일 및 실행
g++ -std=c++17 -pthread test_threads.cpp -o test_threads
./test_threads

# 테스트 파일 정리
rm test_threads test_threads.cpp

# 🎊 모든 출력이 정상이라면 준비 완료!
```

### IDE 설정

**"코딩을 위한 최고의 작업 공간을 만들어봅시다! 🏠"**

IDE(통합 개발 환경)는 개발자의 **디지털 책상**이에요. 편안하고 효율적으로 설정해두면 생산성이 크게 향상됩니다!

#### Visual Studio Code (무료이면서 강력한 선택! 💪)
```json
// .vscode/settings.json
// 💡 이 파일은 VSCode에게 "C++ 프로젝트를 어떻게 다뤄야 하는지" 알려줍니다
{
    "C_Cpp.default.cppStandard": "c++17",  // 🚀 최신 C++17 표준 사용
    "C_Cpp.default.cStandard": "c11",      // 📜 C11 표준 사용
    "C_Cpp.default.compilerPath": "/usr/bin/g++",  // 🔧 컴파일러 위치
    "C_Cpp.default.includePath": [  // 📁 헤더 파일을 어디서 찾을지
        "${workspaceFolder}/**",     // 프로젝트 폴더 전체
        "/usr/include",              // 시스템 헤더들
        "/usr/local/include"         // 추가 라이브러리들
    ],
    "C_Cpp.clang_format_style": "{ BasedOnStyle: Google, IndentWidth: 4, TabWidth: 4 }",  // 🎨 코드 스타일
    "editor.formatOnSave": true,    // 💾 저장할 때 자동으로 코드 정리
    "files.associations": {         // 📄 파일 확장자 연결
        "*.hpp": "cpp",
        "*.h": "c"
    }
}

// .vscode/tasks.json
// 🛠️ 빌드 작업을 쉽게 실행할 수 있는 단축키를 만듭니다
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build LogCaster-C",        // 🅲 C 버전 빌드
            "type": "shell",
            "command": "make",
            "args": ["-C", "logcaster-c"],       // logcaster-c 폴더에서 make 실행
            "group": {
                "kind": "build",
                "isDefault": true                   // 기본 빌드 작업으로 설정
            },
            "presentation": {
                "echo": true,
                "reveal": "always",                 // 항상 터미널 창 보여주기
                "focus": false,
                "panel": "shared"
            },
            "problemMatcher": "$gcc"               // 컴파일 에러를 자동으로 파싱
        },
        {
            "label": "Build LogCaster-CPP",      // 🅒++ C++ 버전 빌드
            "type": "shell",
            "command": "cmake",
            "args": ["--build", "logcaster-cpp/build"],
            "group": "build",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            },
            "problemMatcher": "$gcc"
        }
    ]
}

// 💡 사용법: Ctrl+Shift+P → "Tasks: Run Task" → 원하는 빌드 선택!

// .vscode/launch.json
// 🐛 디버깅 설정 - 버그를 잡는 도구 설정
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug LogCaster-C",          // 🅲 C 버전 디버깅
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/logcaster-c/logcaster",  // 실행할 프로그램
            "args": ["--port", "9999"],           // 프로그램 실행 인자
            "stopAtEntry": false,                  // main 함수에서 자동 멈춤 안함
            "cwd": "${workspaceFolder}/logcaster-c",  // 작업 디렉토리
            "environment": [],
            "externalConsole": false,              // VSCode 내장 터미널 사용
            "MIMode": "gdb",                       // 디버거 종류
            "setupCommands": [                     // 디버깅 시작 전 실행할 명령들
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "Build LogCaster-C", // 디버깅 전에 빌드 먼저 실행
            "miDebuggerPath": "/usr/bin/gdb"       // gdb 경로
        },
        {
            "name": "Debug LogCaster-CPP",        // 🅒++ C++ 버전 디버깅
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/logcaster-cpp/build/logcaster",
            "args": ["--port", "9999"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}/logcaster-cpp",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "Build LogCaster-CPP"
        }
    ]
}

// 💡 사용법: F5 키를 누르면 디버깅 시작! 브레이크포인트를 설정하고 변수 값을 확인할 수 있어요.
```

### 프로젝트 디렉토리 구조

**"집을 지을 때 설계도가 필요하듯, 프로젝트도 체계적인 구조가 필요해요! 🏗️"**

아래는 우리가 만들 LogCaster 프로젝트의 **청사진**입니다. 마치 **아파트 평면도**를 보는 것처럼, 각 폴더와 파일이 어떤 역할을 하는지 이해해봅시다:

```
LogCaster/                      # 🏠 우리 프로젝트의 메인 폴더
├── README.md                    # 📖 프로젝트 설명서 (가장 먼저 읽는 파일!)
├── .gitignore                   # 🚫 Git에 올리지 않을 파일들 목록
├── docs/                        # 📚 문서 보관함
│   ├── design.md               # 🎨 설계 문서
│   ├── api.md                  # 🔌 API 사용법
│   └── performance.md          # ⚡ 성능 분석 결과
├── scripts/                     # 🤖 자동화 스크립트들
│   ├── build.sh                # 🔨 빌드 자동화
│   ├── test.sh                 # 🧪 테스트 자동화
│   └── benchmark.sh            # 📊 성능 측정 자동화
├── logcaster-c/               # 🅲 C 언어 버전
│   ├── Makefile                # 🔧 C 프로젝트 빌드 설정
│   ├── src/                    # 💻 소스 코드들
│   │   ├── main.c              # 🚀 프로그램 시작점
│   │   ├── server.c            # 🌐 서버 핵심 로직
│   │   ├── log_storage.c       # 💾 로그 저장 관리
│   │   ├── network.c           # 📡 네트워크 통신
│   │   └── utils.c             # 🛠️ 유틸리티 함수들
│   ├── include/                # 📄 헤더 파일들 (함수 선언)
│   │   ├── server.h
│   │   ├── log_storage.h
│   │   ├── network.h
│   │   └── utils.h
│   ├── tests/                  # 🧪 테스트 코드들
│   │   ├── test_server.c       # 서버 기능 테스트
│   │   ├── test_storage.c      # 저장소 기능 테스트
│   │   └── Makefile            # 테스트 빌드 설정
│   └── examples/               # 📝 사용 예제들
│       ├── client.c            # 클라이언트 예제
│       └── stress_test.c       # 부하 테스트
├── logcaster-cpp/             # 🅒++ C++ 언어 버전
│   ├── CMakeLists.txt          # 🔧 C++ 프로젝트 빌드 설정
│   ├── src/                    # 💻 소스 코드들
│   │   ├── main.cpp            # 🚀 프로그램 시작점
│   │   ├── Server.cpp          # 🌐 서버 클래스
│   │   ├── LogStorage.cpp      # 💾 로그 저장 클래스
│   │   ├── LogEntry.cpp        # 📝 로그 엔트리 클래스
│   │   └── NetworkManager.cpp  # 📡 네트워크 관리자 클래스
│   ├── include/                # 📄 헤더 파일들
│   │   ├── Server.hpp
│   │   ├── LogStorage.hpp
│   │   ├── LogEntry.hpp
│   │   └── NetworkManager.hpp
│   ├── tests/                  # 🧪 테스트 코드들
│   │   ├── test_server.cpp
│   │   ├── test_storage.cpp
│   │   └── CMakeLists.txt
│   ├── examples/               # 📝 사용 예제들
│   │   ├── client.cpp
│   │   └── benchmark.cpp
│   └── build/                  # 🏗️ 빌드 결과물들이 저장되는 곳
└── tools/                      # 🛠️ 개발 도구들
    ├── log_generator.py        # 🎲 테스트용 로그 생성기
    ├── performance_monitor.py  # 📈 성능 모니터링 도구
    └── client_simulator.py     # 🎭 클라이언트 시뮬레이터
```

💡 **팁**: 이런 구조를 사용하면 코드를 찾기 쉽고, 다른 개발자들도 프로젝트를 이해하기 쉬워져요!

---

## 🧪 2. 테스트 방법론

### 단위 테스트 프레임워크

#### C 프로젝트용 간단한 테스트 프레임워크
```c
// tests/test_framework.h
#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 테스트 통계
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// 색상 출력
#define COLOR_GREEN  "\x1b[32m"
#define COLOR_RED    "\x1b[31m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_RESET  "\x1b[0m"

// 테스트 매크로
#define TEST_START(name) \
    do { \
        printf(COLOR_YELLOW "Running test: %s" COLOR_RESET "\n", name); \
        tests_run++; \
    } while(0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            printf(COLOR_GREEN "  ✓ ASSERT_EQ passed" COLOR_RESET "\n"); \
        } else { \
            printf(COLOR_RED "  ✗ ASSERT_EQ failed: expected %d, got %d" COLOR_RESET "\n", \
                   (int)(expected), (int)(actual)); \
            tests_failed++; \
            return 0; \
        } \
    } while(0)

#define ASSERT_STR_EQ(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) == 0) { \
            printf(COLOR_GREEN "  ✓ ASSERT_STR_EQ passed" COLOR_RESET "\n"); \
        } else { \
            printf(COLOR_RED "  ✗ ASSERT_STR_EQ failed: expected '%s', got '%s'" COLOR_RESET "\n", \
                   (expected), (actual)); \
            tests_failed++; \
            return 0; \
        } \
    } while(0)

#define ASSERT_TRUE(condition) \
    do { \
        if (condition) { \
            printf(COLOR_GREEN "  ✓ ASSERT_TRUE passed" COLOR_RESET "\n"); \
        } else { \
            printf(COLOR_RED "  ✗ ASSERT_TRUE failed" COLOR_RESET "\n"); \
            tests_failed++; \
            return 0; \
        } \
    } while(0)

#define TEST_END() \
    do { \
        tests_passed++; \
        printf(COLOR_GREEN "Test passed!" COLOR_RESET "\n\n"); \
        return 1; \
    } while(0)

// 테스트 실행 함수
typedef int (*test_func_t)(void);

void run_test(const char* name, test_func_t test) {
    if (test()) {
        printf(COLOR_GREEN "✓ %s PASSED" COLOR_RESET "\n", name);
    } else {
        printf(COLOR_RED "✗ %s FAILED" COLOR_RESET "\n", name);
    }
}

void print_test_summary(void) {
    printf("\n" COLOR_YELLOW "=== Test Summary ===" COLOR_RESET "\n");
    printf("Total tests: %d\n", tests_run);
    printf(COLOR_GREEN "Passed: %d" COLOR_RESET "\n", tests_passed);
    if (tests_failed > 0) {
        printf(COLOR_RED "Failed: %d" COLOR_RESET "\n", tests_failed);
    }
    printf("Success rate: %.1f%%\n", (double)tests_passed / tests_run * 100.0);
}

#endif // TEST_FRAMEWORK_H

// tests/test_log_storage.c
#include "test_framework.h"
#include "../include/log_storage.h"

int test_log_storage_create() {
    TEST_START("log_storage_create");
    
    LogStorage* storage = log_storage_create(100);
    ASSERT_TRUE(storage != NULL);
    ASSERT_EQ(0, log_storage_size(storage));
    ASSERT_EQ(100, log_storage_capacity(storage));
    
    log_storage_destroy(storage);
    TEST_END();
}

int test_log_storage_add() {
    TEST_START("log_storage_add");
    
    LogStorage* storage = log_storage_create(10);
    
    LogEntry entry = {"Test message", "INFO", "localhost", time(NULL)};
    int result = log_storage_add(storage, &entry);
    
    ASSERT_EQ(0, result);
    ASSERT_EQ(1, log_storage_size(storage));
    
    log_storage_destroy(storage);
    TEST_END();
}

int test_log_storage_search() {
    TEST_START("log_storage_search");
    
    LogStorage* storage = log_storage_create(100);
    
    // 테스트 데이터 추가
    LogEntry entries[] = {
        {"Error occurred", "ERROR", "server1", time(NULL)},
        {"Info message", "INFO", "server1", time(NULL)},
        {"Warning message", "WARNING", "server2", time(NULL)}
    };
    
    for (int i = 0; i < 3; i++) {
        log_storage_add(storage, &entries[i]);
    }
    
    // 검색 테스트
    LogEntry* results[10];
    int count = log_storage_search_by_level(storage, "ERROR", results, 10);
    
    ASSERT_EQ(1, count);
    ASSERT_STR_EQ("Error occurred", results[0]->message);
    
    log_storage_destroy(storage);
    TEST_END();
}

int main() {
    printf(COLOR_YELLOW "=== LogCaster Storage Tests ===" COLOR_RESET "\n\n");
    
    run_test("log_storage_create", test_log_storage_create);
    run_test("log_storage_add", test_log_storage_add);
    run_test("log_storage_search", test_log_storage_search);
    
    print_test_summary();
    
    return (tests_failed == 0) ? 0 : 1;
}
```

#### C++ 프로젝트용 Google Test 설정
```cmake
# tests/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# Google Test 다운로드 및 설정
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50bc33767dfd8bcf8dc8ab5db6b3b4.zip
)
FetchContent_MakeAvailable(googletest)

# 테스트 실행파일들
add_executable(test_server test_server.cpp)
target_link_libraries(test_server 
    logcaster_lib 
    gtest_main 
    gmock_main
    pthread
)

add_executable(test_storage test_storage.cpp)
target_link_libraries(test_storage 
    logcaster_lib 
    gtest_main
    gmock_main
    pthread
)

# 테스트 등록
include(GoogleTest)
gtest_discover_tests(test_server)
gtest_discover_tests(test_storage)
```

```cpp
// tests/test_storage.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../include/LogStorage.hpp"
#include "../include/LogEntry.hpp"

class LogStorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        storage = std::make_unique<LogStorage>(100);
    }

    void TearDown() override {
        storage.reset();
    }

    std::unique_ptr<LogStorage> storage;
};

TEST_F(LogStorageTest, CreateEmpty) {
    EXPECT_EQ(0, storage->size());
    EXPECT_EQ(100, storage->capacity());
    EXPECT_TRUE(storage->empty());
}

TEST_F(LogStorageTest, AddSingleLog) {
    LogEntry entry("Test message", "INFO", "localhost");
    storage->addLog(entry);
    
    EXPECT_EQ(1, storage->size());
    EXPECT_FALSE(storage->empty());
}

TEST_F(LogStorageTest, SearchByLevel) {
    // 테스트 데이터 추가
    storage->addLog(LogEntry("Error 1", "ERROR", "server1"));
    storage->addLog(LogEntry("Info 1", "INFO", "server1"));
    storage->addLog(LogEntry("Error 2", "ERROR", "server2"));
    storage->addLog(LogEntry("Warning 1", "WARNING", "server1"));
    
    auto error_logs = storage->getLogsByLevel("ERROR");
    
    EXPECT_EQ(2, error_logs.size());
    EXPECT_EQ("Error 1", error_logs[0].getMessage());
    EXPECT_EQ("Error 2", error_logs[1].getMessage());
}

TEST_F(LogStorageTest, SearchByTimeRange) {
    auto now = std::chrono::system_clock::now();
    auto hour_ago = now - std::chrono::hours(1);
    auto hour_later = now + std::chrono::hours(1);
    
    // 시간을 조작한 로그 엔트리 (테스트용 생성자 필요)
    LogEntry old_entry("Old message", "INFO", "server1");
    LogEntry recent_entry("Recent message", "INFO", "server1");
    
    // 실제 구현에서는 시간 설정 메서드 필요
    storage->addLog(old_entry);
    storage->addLog(recent_entry);
    
    auto recent_logs = storage->getLogsInRange(hour_ago, hour_later);
    
    // 구현에 따라 결과 검증
    EXPECT_GE(recent_logs.size(), 1);
}

// 성능 테스트
TEST_F(LogStorageTest, PerformanceTest) {
    const int LOG_COUNT = 100000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < LOG_COUNT; ++i) {
        storage->addLog(LogEntry("Message " + std::to_string(i), "INFO", "localhost"));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Added " << LOG_COUNT << " logs in " << duration.count() << " ms" << std::endl;
    std::cout << "Rate: " << (LOG_COUNT * 1000.0 / duration.count()) << " logs/sec" << std::endl;
    
    EXPECT_LT(duration.count(), 1000);  // 1초 이내에 완료되어야 함
}

// Mock을 사용한 네트워크 테스트
class MockNetworkManager {
public:
    MOCK_METHOD(bool, sendData, (const std::string& data), ());
    MOCK_METHOD(std::string, receiveData, (), ());
    MOCK_METHOD(bool, isConnected, (), (const));
};

TEST(NetworkTest, SendLogData) {
    MockNetworkManager mock_network;
    
    EXPECT_CALL(mock_network, sendData(::testing::_))
        .WillOnce(::testing::Return(true));
    
    std::string log_data = "Test log message";
    bool result = mock_network.sendData(log_data);
    
    EXPECT_TRUE(result);
}
```

### 통합 테스트

```bash
#!/bin/bash
# scripts/integration_test.sh

set -e  # 오류 시 스크립트 종료

echo "=== LogCaster Integration Test ==="

# 1. 서버 빌드
echo "Building LogCaster server..."
cd logcaster-c
make clean && make
cd ..

# 2. 테스트용 로그 파일 생성
echo "Generating test logs..."
python3 tools/log_generator.py --count 1000 --output test_logs.txt

# 3. 서버 시작 (백그라운드)
echo "Starting LogCaster server..."
./logcaster-c/logcaster --port 9999 --daemon &
SERVER_PID=$!

# 서버 시작 대기
sleep 2

# 4. 서버 상태 확인
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "❌ Server failed to start"
    exit 1
fi

echo "✅ Server started with PID: $SERVER_PID"

# 5. 클라이언트 연결 테스트
echo "Testing client connections..."

# 단일 클라이언트 테스트
echo "Test message 1" | nc localhost 9999
if [ $? -eq 0 ]; then
    echo "✅ Single client test passed"
else
    echo "❌ Single client test failed"
    kill $SERVER_PID
    exit 1
fi

# 다중 클라이언트 테스트
echo "Testing multiple clients..."
for i in {1..10}; do
    echo "Message from client $i" | nc localhost 9999 &
done
wait

echo "✅ Multiple client test completed"

# 6. 대용량 데이터 테스트
echo "Testing bulk data transfer..."
cat test_logs.txt | nc localhost 9999

echo "✅ Bulk data test completed"

# 7. 성능 테스트
echo "Running performance test..."
python3 tools/client_simulator.py --host localhost --port 9999 --clients 50 --messages 100

# 8. 메모리 누수 검사 (Valgrind 있는 경우)
if command -v valgrind &> /dev/null; then
    echo "Running memory leak test..."
    kill $SERVER_PID
    sleep 1
    
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
        ./logcaster-c/logcaster --port 9998 &
    VALGRIND_PID=$!
    
    sleep 3
    echo "Test message for valgrind" | nc localhost 9998
    sleep 2
    
    kill $VALGRIND_PID
    wait $VALGRIND_PID 2>/dev/null || true
else
    echo "Valgrind not available, skipping memory test"
    kill $SERVER_PID
fi

# 9. 정리
rm -f test_logs.txt

echo "=== Integration Test Completed ==="
```

---

## 🚀 3. 단계별 구현 가이드

### Phase 1: 기본 구조 구현 (1-2주)

#### LogCaster-C 기본 구현
```c
// include/server.h
#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "log_storage.h"

typedef struct {
    int port;
    int server_fd;
    int max_clients;
    LogStorage* storage;
    pthread_t* worker_threads;
    int thread_count;
    volatile int running;
} LogServer;

// 서버 생성 및 관리
LogServer* server_create(int port, int max_clients);
int server_start(LogServer* server);
void server_stop(LogServer* server);
void server_destroy(LogServer* server);

// 클라이언트 처리
void* client_handler(void* arg);

#endif

// src/server.c (기본 구현)
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

LogServer* server_create(int port, int max_clients) {
    LogServer* server = malloc(sizeof(LogServer));
    if (!server) return NULL;
    
    server->port = port;
    server->max_clients = max_clients;
    server->server_fd = -1;
    server->storage = log_storage_create(10000);  // 1만 개 로그 저장
    server->thread_count = 4;  // 기본 4개 워커 스레드
    server->worker_threads = malloc(sizeof(pthread_t) * server->thread_count);
    server->running = 0;
    
    if (!server->storage || !server->worker_threads) {
        server_destroy(server);
        return NULL;
    }
    
    return server;
}

int server_start(LogServer* server) {
    // 1. 소켓 생성
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd < 0) {
        perror("socket creation failed");
        return -1;
    }
    
    // 2. 소켓 옵션 설정
    int opt = 1;
    setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 3. 주소 바인딩
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(server->port);
    
    if (bind(server->server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server->server_fd);
        return -1;
    }
    
    // 4. 리스닝 시작
    if (listen(server->server_fd, server->max_clients) < 0) {
        perror("listen failed");
        close(server->server_fd);
        return -1;
    }
    
    server->running = 1;
    printf("LogCaster server started on port %d\n", server->port);
    
    // 5. 메인 서버 루프
    while (server->running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server->server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (server->running) {
                perror("accept failed");
            }
            continue;
        }
        
        // 클라이언트 정보 구조체 생성
        ClientInfo* client_info = malloc(sizeof(ClientInfo));
        client_info->fd = client_fd;
        client_info->addr = client_addr;
        client_info->server = server;
        
        // 새 스레드에서 클라이언트 처리
        pthread_t client_thread;
        pthread_create(&client_thread, NULL, client_handler, client_info);
        pthread_detach(client_thread);  // 자동 정리
    }
    
    return 0;
}

typedef struct {
    int fd;
    struct sockaddr_in addr;
    LogServer* server;
} ClientInfo;

void* client_handler(void* arg) {
    ClientInfo* client = (ClientInfo*)arg;
    char buffer[1024];
    char client_ip[INET_ADDRSTRLEN];
    
    inet_ntop(AF_INET, &client->addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("Client connected: %s:%d\n", client_ip, ntohs(client->addr.sin_port));
    
    while (1) {
        ssize_t bytes = recv(client->fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;
        
        buffer[bytes] = '\0';
        
        // 로그 엔트리 생성 및 저장
        LogEntry entry;
        strncpy(entry.message, buffer, sizeof(entry.message) - 1);
        strncpy(entry.level, "INFO", sizeof(entry.level) - 1);  // 기본 레벨
        strncpy(entry.source, client_ip, sizeof(entry.source) - 1);
        entry.timestamp = time(NULL);
        
        log_storage_add(client->server->storage, &entry);
        
        // 응답 전송
        send(client->fd, "ACK\n", 4, 0);
    }
    
    printf("Client disconnected: %s:%d\n", client_ip, ntohs(client->addr.sin_port));
    close(client->fd);
    free(client);
    return NULL;
}
```

#### LogCaster-CPP 기본 구현
```cpp
// include/Server.hpp
#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include "LogStorage.hpp"
#include "NetworkManager.hpp"

class Server {
private:
    int port_;
    std::unique_ptr<LogStorage> storage_;
    std::unique_ptr<NetworkManager> network_manager_;
    std::vector<std::thread> worker_threads_;
    std::atomic<bool> running_{false};
    
public:
    explicit Server(int port);
    ~Server();
    
    // 복사/이동 제어
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&) = default;
    Server& operator=(Server&&) = default;
    
    bool start();
    void stop();
    bool isRunning() const { return running_.load(); }
    
    // 통계 정보
    size_t getLogCount() const;
    void printStats() const;
    
private:
    void serverLoop();
    void handleClient(int client_fd, const std::string& client_address);
};

// src/Server.cpp (기본 구현)
#include "Server.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

Server::Server(int port) 
    : port_(port),
      storage_(std::make_unique<LogStorage>(10000)),
      network_manager_(std::make_unique<NetworkManager>()) {
}

Server::~Server() {
    stop();
}

bool Server::start() {
    if (running_.exchange(true)) {
        return false;  // 이미 실행 중
    }
    
    if (!network_manager_->bind(port_)) {
        running_ = false;
        return false;
    }
    
    if (!network_manager_->listen(128)) {
        running_ = false;
        return false;
    }
    
    std::cout << "LogCaster C++ server started on port " << port_ << std::endl;
    
    // 워커 스레드 시작
    size_t thread_count = std::thread::hardware_concurrency();
    worker_threads_.reserve(thread_count);
    
    for (size_t i = 0; i < thread_count; ++i) {
        worker_threads_.emplace_back(&Server::serverLoop, this);
    }
    
    return true;
}

void Server::stop() {
    running_ = false;
    
    // 모든 워커 스레드 종료 대기
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    worker_threads_.clear();
    network_manager_->close();
}

void Server::serverLoop() {
    while (running_) {
        auto client_info = network_manager_->acceptClient(1000);  // 1초 타임아웃
        
        if (client_info.has_value()) {
            auto [client_fd, client_address] = client_info.value();
            
            // 새 스레드에서 클라이언트 처리
            std::thread client_thread(&Server::handleClient, this, client_fd, client_address);
            client_thread.detach();
        }
    }
}

void Server::handleClient(int client_fd, const std::string& client_address) {
    std::cout << "Client connected: " << client_address << std::endl;
    
    char buffer[1024];
    while (running_) {
        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;
        
        buffer[bytes] = '\0';
        
        // 로그 엔트리 생성 및 저장
        LogEntry entry(std::string(buffer), "INFO", client_address);
        storage_->addLog(std::move(entry));
        
        // 응답 전송
        const char* ack = "ACK\n";
        send(client_fd, ack, strlen(ack), 0);
    }
    
    std::cout << "Client disconnected: " << client_address << std::endl;
    close(client_fd);
}

size_t Server::getLogCount() const {
    return storage_->size();
}

void Server::printStats() const {
    std::cout << "=== Server Statistics ===" << std::endl;
    std::cout << "Total logs: " << getLogCount() << std::endl;
    std::cout << "Server running: " << (running_ ? "Yes" : "No") << std::endl;
    std::cout << "Worker threads: " << worker_threads_.size() << std::endl;
}
```

### Phase 2: 고급 기능 구현 (2-3주)

```cpp
// 로그 조회 서버 추가
class QueryServer {
private:
    std::shared_ptr<LogStorage> storage_;
    int query_port_;
    std::thread query_thread_;
    std::atomic<bool> running_{false};
    
public:
    QueryServer(std::shared_ptr<LogStorage> storage, int port)
        : storage_(storage), query_port_(port) {}
    
    void start() {
        running_ = true;
        query_thread_ = std::thread(&QueryServer::queryLoop, this);
    }
    
    void stop() {
        running_ = false;
        if (query_thread_.joinable()) {
            query_thread_.join();
        }
    }
    
private:
    void queryLoop() {
        // select() 기반 쿼리 서버 구현
        // 클라이언트로부터 검색 요청 수신
        // 결과를 JSON 형태로 반환
    }
    
    void handleQuery(int client_fd, const std::string& query) {
        // 쿼리 파싱 및 실행
        // 예: "level:ERROR", "source:192.168.1.100", "time:2024-01-01~2024-01-02"
    }
};
```

### Phase 3: 성능 최적화 (1-2주)

```cpp
// 고성능 버전 구현
class HighPerformanceServer {
private:
    std::unique_ptr<EpollNetworkManager> network_;
    std::unique_ptr<LockFreeLogBuffer> buffer_;
    std::unique_ptr<MemoryPool> memory_pool_;
    
public:
    // epoll() 기반 네트워킹
    // Lock-free 데이터 구조
    // 메모리 풀 활용
    // SIMD 최적화 (가능한 경우)
};
```

---

## 🐛 4. 디버깅 도구와 기법

### GDB 디버깅

```bash
# 디버그 빌드
gcc -g -O0 -DDEBUG src/*.c -o logcaster_debug

# GDB 시작
gdb ./logcaster_debug

# 기본 GDB 명령어
(gdb) break main          # main 함수에 브레이크포인트
(gdb) break server.c:45   # 특정 파일의 45번째 줄
(gdb) run --port 9999     # 프로그램 실행
(gdb) bt                  # 백트레이스 출력
(gdb) print variable_name # 변수 값 출력
(gdb) step                # 한 줄씩 실행
(gdb) continue            # 계속 실행
(gdb) info threads        # 스레드 정보
(gdb) thread 2            # 스레드 2로 전환
(gdb) watch variable      # 변수 변경 감시

# 고급 명령어
(gdb) set follow-fork-mode child    # 자식 프로세스 추적
(gdb) set scheduler-locking on      # 스레드 잠금
(gdb) call function_name()          # 함수 직접 호출
```

### Valgrind 메모리 분석

```bash
# 메모리 누수 검사
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind_log.txt \
         ./logcaster --port 9999

# 스레드 에러 검사
valgrind --tool=helgrind ./logcaster --port 9999

# 캐시 성능 분석
valgrind --tool=cachegrind ./logcaster --port 9999
callgrind_annotate cachegrind.out.*
```

### AddressSanitizer 사용

```bash
# 컴파일 시 AddressSanitizer 활성화
gcc -fsanitize=address -g -O1 src/*.c -o logcaster_asan

# 환경 변수 설정
export ASAN_OPTIONS=verbosity=3:halt_on_error=1:detect_leaks=1

# 실행
./logcaster_asan --port 9999
```

### 성능 프로파일링

```bash
# perf 사용 (Linux)
perf record -g ./logcaster --port 9999
perf report

# gprof 사용
gcc -pg src/*.c -o logcaster_prof
./logcaster_prof --port 9999
# 프로그램 종료 후
gprof logcaster_prof gmon.out > analysis.txt

# htop으로 실시간 모니터링
htop -p $(pgrep logcaster)
```

---

## 📊 5. 벤치마크와 성능 측정

### 성능 테스트 스크립트

```python
#!/usr/bin/env python3
# tools/benchmark.py

import socket
import threading
import time
import argparse
import statistics
from concurrent.futures import ThreadPoolExecutor, as_completed

class LogCasterBenchmark:
    def __init__(self, host='localhost', port=9999):
        self.host = host
        self.port = port
        self.results = []
        
    def send_log_message(self, message):
        """단일 로그 메시지 전송 및 응답 시간 측정"""
        start_time = time.perf_counter()
        
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.settimeout(5.0)
                sock.connect((self.host, self.port))
                
                # 메시지 전송
                sock.send(message.encode('utf-8'))
                
                # 응답 수신
                response = sock.recv(1024)
                
                end_time = time.perf_counter()
                return end_time - start_time, len(response) > 0
                
        except Exception as e:
            return None, False
    
    def connection_throughput_test(self, num_connections=100, messages_per_connection=10):
        """연결 처리량 테스트"""
        print(f"Connection throughput test: {num_connections} connections, {messages_per_connection} messages each")
        
        def worker(connection_id):
            times = []
            for i in range(messages_per_connection):
                message = f"Benchmark message {connection_id}-{i} " + "x" * 100
                duration, success = self.send_log_message(message)
                if success and duration:
                    times.append(duration)
            return times
        
        start_time = time.perf_counter()
        
        with ThreadPoolExecutor(max_workers=50) as executor:
            futures = [executor.submit(worker, i) for i in range(num_connections)]
            
            all_times = []
            for future in as_completed(futures):
                times = future.result()
                all_times.extend(times)
        
        end_time = time.perf_counter()
        
        total_messages = num_connections * messages_per_connection
        total_time = end_time - start_time
        
        print(f"Total time: {total_time:.2f} seconds")
        print(f"Total messages: {total_messages}")
        print(f"Messages per second: {total_messages / total_time:.2f}")
        print(f"Average response time: {statistics.mean(all_times) * 1000:.2f} ms")
        print(f"95th percentile: {statistics.quantiles(all_times, n=20)[18] * 1000:.2f} ms")
        
    def latency_test(self, num_requests=1000):
        """지연시간 테스트"""
        print(f"Latency test: {num_requests} sequential requests")
        
        latencies = []
        
        for i in range(num_requests):
            message = f"Latency test message {i}"
            duration, success = self.send_log_message(message)
            
            if success and duration:
                latencies.append(duration * 1000)  # 밀리초로 변환
            
            if i % 100 == 0:
                print(f"Progress: {i}/{num_requests}")
        
        print(f"Average latency: {statistics.mean(latencies):.2f} ms")
        print(f"Median latency: {statistics.median(latencies):.2f} ms")
        print(f"95th percentile: {statistics.quantiles(latencies, n=20)[18]:.2f} ms")
        print(f"99th percentile: {statistics.quantiles(latencies, n=100)[98]:.2f} ms")
        print(f"Max latency: {max(latencies):.2f} ms")
        
    def stress_test(self, duration_seconds=60, concurrent_connections=20):
        """스트레스 테스트"""
        print(f"Stress test: {concurrent_connections} concurrent connections for {duration_seconds} seconds")
        
        start_time = time.perf_counter()
        end_time = start_time + duration_seconds
        message_count = 0
        error_count = 0
        
        def stress_worker():
            nonlocal message_count, error_count
            worker_message_count = 0
            
            while time.perf_counter() < end_time:
                message = f"Stress test message {worker_message_count}"
                duration, success = self.send_log_message(message)
                
                if success:
                    message_count += 1
                else:
                    error_count += 1
                
                worker_message_count += 1
                time.sleep(0.01)  # 10ms 간격
        
        threads = []
        for _ in range(concurrent_connections):
            thread = threading.Thread(target=stress_worker)
            thread.start()
            threads.append(thread)
        
        for thread in threads:
            thread.join()
        
        actual_duration = time.perf_counter() - start_time
        
        print(f"Actual duration: {actual_duration:.2f} seconds")
        print(f"Total messages sent: {message_count}")
        print(f"Errors: {error_count}")
        print(f"Success rate: {message_count / (message_count + error_count) * 100:.2f}%")
        print(f"Messages per second: {message_count / actual_duration:.2f}")

def main():
    parser = argparse.ArgumentParser(description='LogCaster Benchmark Tool')
    parser.add_argument('--host', default='localhost', help='Server host')
    parser.add_argument('--port', type=int, default=9999, help='Server port')
    parser.add_argument('--test', choices=['throughput', 'latency', 'stress', 'all'], 
                       default='all', help='Test type to run')
    
    args = parser.parse_args()
    
    benchmark = LogCasterBenchmark(args.host, args.port)
    
    print("=== LogCaster Benchmark ===")
    print(f"Target: {args.host}:{args.port}")
    print()
    
    if args.test in ['throughput', 'all']:
        benchmark.connection_throughput_test()
        print()
    
    if args.test in ['latency', 'all']:
        benchmark.latency_test()
        print()
    
    if args.test in ['stress', 'all']:
        benchmark.stress_test()
        print()

if __name__ == '__main__':
    main()
```

### 자동화된 성능 모니터링

```bash
#!/bin/bash
# scripts/performance_monitor.sh

# 시스템 리소스 모니터링
monitor_resources() {
    local pid=$1
    local duration=$2
    local output_file=$3
    
    echo "timestamp,cpu_percent,memory_mb,threads,fds" > "$output_file"
    
    for ((i=0; i<duration; i++)); do
        local timestamp=$(date +%s)
        local cpu=$(ps -p $pid -o %cpu --no-headers | tr -d ' ')
        local memory=$(ps -p $pid -o rss --no-headers | tr -d ' ')
        local threads=$(ps -p $pid -o nlwp --no-headers | tr -d ' ')
        local fds=$(ls /proc/$pid/fd 2>/dev/null | wc -l)
        
        echo "$timestamp,$cpu,$memory,$threads,$fds" >> "$output_file"
        sleep 1
    done
}

# 네트워크 통계 모니터링
monitor_network() {
    local duration=$1
    local output_file=$2
    
    ss -tuln > "${output_file}_before.txt"
    
    sleep $duration
    
    ss -tuln > "${output_file}_after.txt"
    netstat -i > "${output_file}_interface.txt"
}

# 메인 모니터링 실행
main() {
    local server_pid=$(pgrep logcaster)
    
    if [ -z "$server_pid" ]; then
        echo "LogCaster server not running"
        exit 1
    fi
    
    echo "Monitoring LogCaster (PID: $server_pid)"
    
    monitor_resources $server_pid 300 "resources.csv" &
    monitor_network 300 "network" &
    
    wait
    
    echo "Monitoring completed. Check resources.csv and network files."
}

main
```

---

## ✅ 6. 최종 체크리스트

### 구현 완료 체크리스트

#### 기본 기능 (MVP 1)
- [ ] TCP 소켓 서버 구현
- [ ] 클라이언트 연결 수락
- [ ] 로그 메시지 수신 및 출력
- [ ] 기본 에러 처리
- [ ] Makefile/CMake 설정

#### 고급 기능 (MVP 2)
- [ ] 멀티스레딩 지원
- [ ] 로그 메모리 저장
- [ ] Thread-safe 동기화
- [ ] 연결 관리 (연결/해제)
- [ ] 성능 모니터링

#### 전문 기능 (MVP 3)
- [ ] 로그 검색 기능
- [ ] 쿼리 서버 구현
- [ ] 필터링 및 정렬
- [ ] 통계 정보 제공
- [ ] API 문서화

### 성능 목표
- [ ] 1000+ 동시 연결 지원
- [ ] 10,000+ 메시지/초 처리
- [ ] 100MB+ 메모리 효율적 사용
- [ ] 1ms 미만 평균 응답 시간
- [ ] 24시간 안정성 테스트 통과

### 코드 품질
- [ ] 단위 테스트 커버리지 80%+
- [ ] 메모리 누수 없음 (Valgrind 검증)
- [ ] 코딩 스타일 일관성
- [ ] API 문서 완성
- [ ] 예제 코드 제공

### 배포 준비
- [ ] 설치 스크립트 작성
- [ ] 설정 파일 템플릿
- [ ] 로그 로테이션 지원
- [ ] 서비스 데몬화
- [ ] 모니터링 대시보드

---

## 🎉 마무리

LogCaster 프로젝트를 통해 다음을 달성할 수 있습니다:

1. **C/C++ 언어 숙련도 향상**: 저수준 시스템 프로그래밍부터 고수준 객체 지향 설계까지
2. **시스템 프로그래밍 이해**: 네트워킹, 멀티스레딩, 메모리 관리
3. **성능 최적화 경험**: 프로파일링, 병목점 분석, 고성능 코드 작성
4. **소프트웨어 엔지니어링**: 테스트, 디버깅, 문서화, 배포

### 다음 단계 제안
1. **기본 구현 완성** → GitHub에 코드 공개
2. **고급 기능 추가** → 포트폴리오 강화
3. **오픈소스 기여** → 유명 로깅 라이브러리에 기여
4. **확장 프로젝트** → 분산 로깅 시스템, 로그 분석 엔진 등

## 🚨 자주 하는 실수와 해결법

### 1. 빌드 시스템 설정 오류
**문제**: CMakeLists.txt에서 라이브러리 링크 순서 오류
```cmake
# ❌ 잘못된 순서
target_link_libraries(logcaster pthread logcraft_lib)

# ✅ 올바른 순서 (의존성 있는 라이브러리가 먼저)
target_link_libraries(logcaster logcraft_lib pthread)
```

### 2. 네트워크 프로그래밍 실수
**문제**: TIME_WAIT 상태로 인한 "Address already in use" 오류
```c
// ❌ SO_REUSEADDR 설정 누락
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));

// ✅ SO_REUSEADDR 설정
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
int opt = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
```

### 3. 스레드 동기화 누락
**문제**: 데이터 경쟁 상태 발생
```cpp
// ❌ 동기화 없는 접근
class LogStorage {
    std::vector<LogEntry> logs_;
public:
    void addLog(const LogEntry& log) {
        logs_.push_back(log);  // 여러 스레드가 동시 접근 시 문제!
    }
};

// ✅ 뮤텍스로 보호
class LogStorage {
    std::vector<LogEntry> logs_;
    mutable std::mutex mutex_;
public:
    void addLog(const LogEntry& log) {
        std::lock_guard<std::mutex> lock(mutex_);
        logs_.push_back(log);
    }
};
```

### 4. 파일 디스크립터 누수
**문제**: 클라이언트 소켓을 닫지 않음
```c
// ❌ 에러 시 리턴만 하고 소켓 닫지 않음
void handle_client(int client_fd) {
    if (recv(client_fd, buffer, size, 0) < 0) {
        return;  // 소켓 누수!
    }
}

// ✅ 모든 경로에서 소켓 닫기
void handle_client(int client_fd) {
    if (recv(client_fd, buffer, size, 0) < 0) {
        close(client_fd);
        return;
    }
    // ... 처리 ...
    close(client_fd);
}
```

## 💡 실습 프로젝트

### 프로젝트 1: 미니 LogCaster (1-2일)
```c
// 다음 기능을 구현하세요:
// 1. 단일 스레드 TCP 서버
// 2. 10개 로그 메모리 저장
// 3. 간단한 출력 기능

// mini_logcaster.c
typedef struct {
    char message[256];
    time_t timestamp;
} SimpleLog;

typedef struct {
    SimpleLog logs[10];
    int count;
    int server_fd;
} MiniLogCaster;

// 구현해야 할 함수들
MiniLogCaster* create_mini_logcaster(int port);
void run_mini_logcaster(MiniLogCaster* lc);
void destroy_mini_logcaster(MiniLogCaster* lc);
```

### 프로젝트 2: 멀티스레드 LogCaster (3-4일)
```cpp
// C++ 버전으로 다음 구현:
// 1. Thread pool 패턴
// 2. Lock-free 큐
// 3. 비동기 로깅

class AsyncLogCaster {
    // 구현 요구사항:
    // - Worker thread pool
    // - Concurrent queue for logs
    // - Async file writer
    // - Performance metrics
};
```

### 프로젝트 3: 고성능 LogCaster (1주)
```cpp
// 최종 버전 구현:
// 1. epoll/kqueue 사용
// 2. Zero-copy 기법
// 3. Memory pool
// 4. 분산 로깅 지원

class DistributedLogCaster {
    // 요구사항:
    // - 100k+ connections
    // - 1M+ logs/sec
    // - Clustering support
};
```

## 📚 추가 학습 자료

### 책 추천
- "Unix Network Programming" - W. Richard Stevens
- "The Linux Programming Interface" - Michael Kerrisk
- "C++ Concurrency in Action" - Anthony Williams

### 온라인 자료
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [Linux System Programming](https://man7.org/tlpi/)
- [Modern CMake Tutorial](https://cliutils.gitlab.io/modern-cmake/)

### 도구 문서
- [GDB Documentation](https://www.gnu.org/software/gdb/documentation/)
- [Valgrind User Manual](https://valgrind.org/docs/manual/)
- [Perf Tutorial](https://perf.wiki.kernel.org/index.php/Tutorial)

## ✅ 학습 체크리스트

### 환경 설정 (1일)
- [ ] 개발 도구 설치 완료
- [ ] IDE/에디터 설정
- [ ] 빌드 시스템 이해
- [ ] Git 저장소 생성

### 기본 구현 (1주)
- [ ] TCP 서버 구현
- [ ] 클라이언트 처리
- [ ] 로그 저장 구조
- [ ] 기본 테스트 작성

### 고급 기능 (2주)
- [ ] 멀티스레딩 적용
- [ ] 동기화 구현
- [ ] 성능 최적화
- [ ] 쿼리 기능 추가

### 프로덕션 준비 (1주)
- [ ] 통합 테스트
- [ ] 성능 벤치마크
- [ ] 문서화 완성
- [ ] 배포 스크립트

## 🔄 다음 학습 단계

이 문서를 완료했다면 다음으로 진행하세요:

1. **실제 프로젝트 구현** - LogCaster 코딩 시작
   - GitHub 저장소 생성
   - 단계별 커밋
   - CI/CD 설정

2. **포트폴리오 작성** - 프로젝트 문서화
   - README 작성
   - 기술 블로그 포스팅
   - 성능 분석 리포트

3. **오픈소스 기여** - 실력 향상
   - 유명 C/C++ 프로젝트 분석
   - 이슈 해결
   - Pull Request 제출

---

*🎯 성공적인 LogCaster 구현을 위해 단계별로 천천히 진행하세요. 각 단계에서 충분히 테스트하고 검증한 후 다음 단계로 넘어가는 것이 중요합니다!*