# LogCaster Knowledge Base

이 폴더는 LogCaster 프로젝트의 지식 베이스를 체계적으로 정리한 문서들을 포함합니다.

## 🚀 빠른 시작
- **처음이신가요?** → [📚 학습 로드맵](LEARNING_ROADMAP.md)을 먼저 확인하세요!
- **문서 검토 현황** → [📋 문서 리뷰 결과](DOCUMENTATION_REVIEW.md)를 참고하세요.

## 📁 폴더 구조

### core/ - LogCaster 핵심 문서
LogCaster 프로젝트에 특화된 핵심 구현 문서들입니다.

1. **1_LogCaster_Architecture.md** - 전체 시스템 아키텍처
2. **2_TCP_Server_Implementation.md** - TCP 서버 구현 상세
3. **3_Concurrent_Log_Processing.md** - 동시성 처리 및 스레드 안전성
4. **4_IRC_Protocol_Integration.md** - IRC 프로토콜 통합
5. **5_Performance_and_Optimization.md** - 성능 최적화 기법

### reference/ - 참고 문서
프로젝트 구현에 사용된 일반적인 C++ 및 시스템 프로그래밍 지식들입니다.

- **Basic.md** - C++ 기초
- **Memory.md** - 메모리 관리
- **Networking.md** - 네트워크 프로그래밍
- **Multithreading.md** - 멀티스레딩
- **IRC_Protocol_Guide.md** - IRC 프로토콜 상세
- 기타 C++ 고급 주제들

### appendix/ - 부록
프로젝트와 직접적인 관련은 적지만 참고할 만한 내용들입니다.

- **PhysiSim.md** - 물리 시뮬레이션 예제

## 🎯 문서 활용 가이드

### LogCaster를 이해하려면
1. `core/1_LogCaster_Architecture.md`부터 시작하세요
2. 순서대로 core 문서들을 읽으며 구현을 이해하세요
3. 필요시 reference 문서를 참고하세요

### 특정 기술을 학습하려면
- TCP 서버 구현 → `core/2_TCP_Server_Implementation.md`
- 동시성 처리 → `core/3_Concurrent_Log_Processing.md`
- IRC 프로토콜 → `core/4_IRC_Protocol_Integration.md`
- 성능 최적화 → `core/5_Performance_and_Optimization.md`

### C++ 기초가 필요하면
- `reference/` 폴더의 문서들을 참고하세요
- 특히 Memory, Multithreading, Networking 문서가 유용합니다

## 📝 문서 작성 원칙

1. **핵심 문서는 프로젝트 특화적으로**
   - LogCaster 코드와 직접 연결
   - 실제 구현 예제 포함
   - SEQUENCE 번호로 코드 추적 가능

2. **참고 문서는 재사용 가능하게**
   - 일반적인 개념 설명
   - 다양한 예제 제공
   - 다른 프로젝트에도 활용 가능

3. **중복 최소화**
   - 핵심 문서에서 참고 문서 링크
   - 개념 설명은 한 곳에만
   - 프로젝트별 적용만 핵심 문서에