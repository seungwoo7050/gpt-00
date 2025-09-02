# LogCaster - C vs C++ Learning Guide

## 🎯 학습 목표
이 가이드는 동일한 로그 수집 서버를 C와 C++로 구현하면서 두 언어의 차이점과 각각의 장단점을 학습하는 방법을 안내합니다.

## 📚 필수 전제 지식
- C 언어 기본 문법
- 포인터와 메모리 관리
- 기본적인 네트워크 프로그래밍
- Make/CMake 빌드 시스템

## 🗺️ 학습 순서

### Phase 1: 프로젝트 이해 (1일차)

1. **전체 구조 파악**
   - 📖 `README.md` - 프로젝트 개요
   - 📖 `subject.md` - 요구사항 명세
   - 📖 `DEVELOPMENT_JOURNEY.md` - 개발 과정

2. **C vs C++ 비교 기초**
   - 📖 `knowledge/reference/00_Basic.md` - 언어 기초 차이점
   - 📖 `knowledge/reference/01_Memory.md` - 메모리 관리 비교

3. **네트워킹 기초**
   - 📖 `knowledge/reference/13_Networking.md` - TCP 서버 기초
   - 📖 `knowledge/reference/14_IOMultiplexing.md` - I/O 다중화

### Phase 2: MVP별 구현 학습 (2-4일차)

#### MVP1: 기본 TCP 서버
**C 버전**
- 📁 `LogCaster-C/src/server_mvp1.c.bak`
- 📁 `versions/mvp1-basic-tcp-server/`
- 핵심: `socket()`, `bind()`, `listen()`, `accept()`

**C++ 버전**
- 📁 `LogCaster-CPP/src/LogServer_mvp1.cpp.bak`
- 📁 `versions/mvp1-cpp-oop-design/`
- 핵심: 클래스 설계, RAII 패턴

**비교 학습 포인트**
```bash
# 두 구현 비교
diff LogCaster-C/src/server_mvp1.c.bak LogCaster-CPP/src/LogServer_mvp1.cpp.bak
```

#### MVP2: 스레드 풀과 순환 버퍼
**C 버전**
- 📁 `versions/mvp2-c-thread-pool/`
- 📖 `knowledge/reference/11_Multithreading.md`
- 핵심 코드: `src/thread_pool.c`, `src/log_buffer.c`

**C++ 버전**
- 📁 `versions/mvp2-cpp-modern-threading/`
- 핵심: `std::thread`, `std::mutex`, `std::deque`

**학습 포인트**
- C: pthread 직접 사용, 수동 동기화
- C++: STL 컨테이너, 스마트 포인터

#### MVP3: 쿼리 인터페이스
**구현 파일**
- C: `src/query_parser.c`, `src/query_handler.c`
- C++: `src/QueryParser.cpp`, `src/QueryHandler.cpp`

**테스트로 학습**
```bash
# 쿼리 기능 테스트
python tests/test_mvp2.py
python tests/test_mvp3.py
```

#### MVP4: 디스크 영속성
**구현 파일**
- C: `src/persistence.c`
- C++: `src/Persistence.cpp`

**테스트**
```bash
python tests/test_persistence.py
```

### Phase 3: 고급 주제 (5일차)

1. **동기화와 동시성**
   - 📖 `knowledge/reference/12_Synchronization.md`
   - C: mutex, condition variable 직접 사용
   - C++: `std::lock_guard`, RAII 활용

2. **성능 최적화**
   - 📖 `knowledge/reference/15_Performance_Guide.md`
   - 메모리 풀, lock-free 구조

3. **디버깅 기법**
   - 📖 `knowledge/reference/16_DebuggingTools.md`
   - Valgrind, GDB, AddressSanitizer

### Phase 4: 통합 및 비교 (6-7일차)

1. **전체 시스템 테스트**
   ```bash
   # C 버전 빌드 및 실행
   cd LogCaster-C
   make
   ./bin/log_server
   
   # C++ 버전 빌드 및 실행
   cd ../LogCaster-CPP
   mkdir build && cd build
   cmake ..
   make
   ./log_server
   ```

2. **성능 비교**
   ```bash
   # 스트레스 테스트
   python tests/test_client.py --stress
   ```

3. **코드 품질 비교**
   - 메모리 사용량
   - 코드 라인 수
   - 유지보수성

### Phase 5: 현대적 C++ 마스터 (8-10일차) 🚀

새로 추가된 핵심 문서들로 C++ 전문가 수준까지 도달

1. **예외 처리와 에러 관리** (1일)
   - 📖 `knowledge/reference/03_Exception_Handling.md`
   - LogCaster에 견고한 예외 처리 시스템 적용
   - RAII 패턴과 예외 안전성 보장
   - 커스텀 예외 클래스 설계

2. **Modern C++ 기능 적용** (2일)
   - 📖 `knowledge/reference/05_Modern_CPP_Features.md`
   - auto, lambda, smart pointers로 코드 개선
   - structured bindings, concepts 활용
   - C++20 ranges로 데이터 처리 최적화

3. **함수형 프로그래밍 도입** (1일)
   - 📖 `knowledge/reference/06_Lambda_Functional_Programming.md`
   - 로그 처리 파이프라인을 함수형으로 재구성
   - Map-Filter-Reduce 패턴으로 데이터 분석
   - 고차 함수와 커링 기법 적용

4. **STL 알고리즘 최적화** (1일)
   - 📖 `knowledge/reference/07_STL_Algorithms_Deep_Dive.md`
   - 로그 검색, 정렬, 분석을 STL 알고리즘으로 최적화
   - 병렬 알고리즘으로 성능 향상
   - 커스텀 알고리즘 구현

**실습 프로젝트: LogCaster Modern 버전 구현**
```bash
# Modern C++ 기능을 모두 활용한 최신 버전 구현
mkdir LogCaster-Modern
# 새로 학습한 모든 기법을 종합 적용
```

## 💻 실습 가이드

### 단계별 구현 실습

1. **MVP1 직접 구현**
   ```bash
   # 빈 파일에서 시작
   touch my_server.c
   # versions/mvp1-basic-tcp-server/ 참고하여 구현
   ```

2. **기능 추가 실습**
   - 로그 레벨 필터링 추가
   - 정규식 검색 구현
   - 압축 기능 추가

3. **C에서 C++로 포팅**
   - C 코드를 C++로 변환
   - STL 활용하여 개선
   - RAII 패턴 적용

### 테스트 작성 실습

1. **Python 테스트 분석**
   ```python
   # tests/test_client.py 구조 이해
   # 새로운 테스트 케이스 추가
   ```

2. **단위 테스트 추가**
   - 쿼리 파서 테스트
   - 버퍼 동작 테스트

## 🔧 디버깅 실습

### 메모리 누수 찾기
```bash
# C 버전
valgrind --leak-check=full ./bin/log_server

# C++ 버전  
valgrind --leak-check=full ./log_server
```

### 성능 프로파일링
```bash
# gprof 사용
gcc -pg -o log_server *.c
./log_server
gprof log_server gmon.out
```

## 📊 학습 체크포인트

### 기초 레벨
- [ ] TCP 서버를 C로 구현할 수 있다
- [ ] pthread를 사용하여 멀티스레딩을 구현할 수 있다
- [ ] 동적 메모리를 안전하게 관리할 수 있다
- [ ] Makefile을 작성할 수 있다

### 중급 레벨
- [ ] 순환 버퍼를 구현할 수 있다
- [ ] 스레드 풀을 구현할 수 있다
- [ ] 파일 I/O와 로그 로테이션을 구현할 수 있다
- [ ] C++로 동일 기능을 구현할 수 있다

### 고급 레벨
- [ ] lock-free 자료구조를 이해한다
- [ ] 성능 최적화를 할 수 있다
- [ ] 메모리 누수를 찾고 수정할 수 있다
- [ ] C와 C++의 장단점을 설명할 수 있다

### 전문가 레벨 (새로 추가) 🎯
- [ ] C++ 예외 처리를 안전하게 설계할 수 있다
- [ ] Modern C++ (C++17/20/23) 기능을 적절히 활용할 수 있다
- [ ] Lambda와 함수형 프로그래밍 패턴을 구현할 수 있다
- [ ] STL 알고리즘으로 복잡한 데이터 처리를 최적화할 수 있다
- [ ] 병렬 프로그래밍과 성능 분석을 수행할 수 있다

## 🎓 추가 학습 자료

### 프로젝트 확장 아이디어
1. **네트워크 프로토콜**
   - 바이너리 프로토콜 설계
   - 압축 알고리즘 적용

2. **보안 기능**
   - TLS/SSL 지원
   - 인증 메커니즘

3. **분산 시스템**
   - 여러 서버로 확장
   - 로드 밸런싱

### 관련 문서
- 📖 `knowledge/reference/04_OOP_CPP_Guide.md` - C++ OOP 가이드
- 📖 `knowledge/reference/08_Project_Setup_Guide.md` - 프로젝트 구성

### 고급 학습 자료 (체계적으로 재정리된 문서들)
**🎯 핵심 언어 기능 (00-07):**
- 📖 `knowledge/reference/03_Exception_Handling.md` - C++ 예외 처리 완벽 가이드
- 📖 `knowledge/reference/05_Modern_CPP_Features.md` - 최신 C++ 기능들 (C++11/14/17/20/23)
- 📖 `knowledge/reference/06_Lambda_Functional_Programming.md` - Lambda와 함수형 프로그래밍
- 📖 `knowledge/reference/07_STL_Algorithms_Deep_Dive.md` - STL 알고리즘 완벽 분석

**🔧 시스템 프로그래밍 (10-19):**
- 📖 `knowledge/reference/10_SystemProgramming.md` - 시스템 프로그래밍 기초
- 📖 `knowledge/reference/17_DesignPatterns.md` - 디자인 패턴
- 📖 `knowledge/reference/18_Advanced.md` - 고급 주제
- 📖 `knowledge/reference/19_Production_Ready.md` - 프로덕션 준비

**🚀 전문 분야 (20-24):**
- 📖 `knowledge/reference/20_Security_Operations.md` - 보안 운영
- 📖 `knowledge/reference/22_IRC_Protocol_Guide.md` - IRC 프로토콜
- 📖 `knowledge/reference/24_Chat_Server_Security.md` - 채팅 서버 보안

## ❓ 자주 묻는 질문

**Q: C와 C++ 중 어떤 걸 먼저 봐야 하나요?**
A: C 버전을 먼저 이해한 후 C++ 버전을 보면 차이점이 명확해집니다.

**Q: 왜 Python으로 테스트를 작성했나요?**
A: 클라이언트 로직에 집중하고, 언어 중립적인 테스트를 위해서입니다.

**Q: 실제 프로덕션에서는 어떤 언어를 선택해야 하나요?**
A: README의 "When to Use Which" 섹션을 참고하세요. 일반적으로 응용 프로그램은 C++, 시스템 프로그래밍은 C를 선택합니다.