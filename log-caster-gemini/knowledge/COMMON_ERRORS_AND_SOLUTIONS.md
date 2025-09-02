# LogCaster 자주 발생하는 에러와 해결방법

## 🚨 이 문서의 목적

LogCaster 프로젝트를 진행하면서 자주 마주치는 에러들과 그 해결방법을 정리했습니다. 에러 메시지를 보고 당황하지 말고 이 문서를 참고하세요!

## 📌 에러 해결의 기본 원칙

1. **에러 메시지를 꼼꼼히 읽기** - 대부분의 답이 에러 메시지에 있습니다
2. **에러가 발생한 위치 확인** - 파일명과 줄 번호를 확인하세요
3. **최근 변경사항 검토** - 무엇을 바꾸고 나서 에러가 발생했나요?
4. **구글링하기** - 에러 메시지로 검색하면 많은 도움을 받을 수 있습니다

## 🔧 컴파일 에러

### 1. "undefined reference to `pthread_create'"

**에러 메시지:**
```
/usr/bin/ld: server.o: undefined reference to `pthread_create'
collect2: error: ld returned 1 exit status
```

**원인:** pthread 라이브러리가 링크되지 않음

**해결방법:**
```bash
# Makefile에 -pthread 플래그 추가
gcc main.c -o logcaster -pthread

# 또는 CMakeLists.txt에 추가
target_link_libraries(logcaster pthread)
```

### 2. "fatal error: sys/socket.h: No such file or directory"

**에러 메시지:**
```
server.c:1:10: fatal error: sys/socket.h: No such file or directory
    1 | #include <sys/socket.h>
      |          ^~~~~~~~~~~~~~
```

**원인:** Windows에서 POSIX 헤더 사용 시도

**해결방법:**
- Windows: WSL(Windows Subsystem for Linux) 사용
- 또는 Windows 소켓 API 사용:
```c
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif
```

### 3. "error: invalid use of incomplete type"

**에러 메시지:**
```
error: invalid use of incomplete type 'struct log_entry'
```

**원인:** 구조체 정의 전에 사용

**해결방법:**
```c
// 잘못된 코드
struct log_entry* entry;  // 구조체 정의 전에 사용
entry->message = "test";

// 올바른 코드
struct log_entry {
    char message[256];
    time_t timestamp;
};
struct log_entry* entry = malloc(sizeof(struct log_entry));
```

## 💥 런타임 에러

### 1. "Segmentation fault (core dumped)"

**증상:** 프로그램이 갑자기 종료됨

**일반적인 원인들:**
1. NULL 포인터 역참조
2. 배열 범위 초과
3. 해제된 메모리 접근
4. 스택 오버플로우

**디버깅 방법:**
```bash
# GDB로 디버깅
gdb ./logcaster
(gdb) run
# 크래시 발생 시
(gdb) bt  # 백트레이스 확인
(gdb) print 변수명  # 변수 값 확인
```

**예방법:**
```c
// NULL 체크 습관화
if (ptr != NULL) {
    // 안전하게 사용
}

// 배열 범위 체크
if (index >= 0 && index < array_size) {
    array[index] = value;
}
```

### 2. "Address already in use"

**에러 메시지:**
```
bind: Address already in use
```

**원인:** 포트가 이미 사용 중

**해결방법:**
```bash
# 사용 중인 포트 확인
lsof -i :9999
# 또는
netstat -tulpn | grep 9999

# 프로세스 종료
kill -9 [PID]
```

**코드에서 해결:**
```c
// SO_REUSEADDR 옵션 설정
int opt = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

### 3. "Too many open files"

**원인:** 파일 디스크립터 한계 도달

**해결방법:**
```bash
# 현재 한계 확인
ulimit -n

# 한계 늘리기 (임시)
ulimit -n 4096

# 영구적으로 변경 (/etc/security/limits.conf)
* soft nofile 4096
* hard nofile 8192
```

**코드에서 해결:**
```c
// 사용 후 반드시 닫기
close(client_fd);
fclose(file);
```

## 🐛 논리적 에러

### 1. 메모리 누수

**증상:** 프로그램이 점점 느려지고 메모리 사용량 증가

**검사 방법:**
```bash
valgrind --leak-check=full ./logcaster
```

**일반적인 원인:**
```c
// malloc 후 free 안 함
char* buffer = malloc(1024);
// ... 사용 ...
// free(buffer); // 잊어버림!

// 올바른 패턴
char* buffer = malloc(1024);
if (buffer != NULL) {
    // 사용
    free(buffer);
    buffer = NULL;  // 이중 해제 방지
}
```

### 2. 경쟁 상태 (Race Condition)

**증상:** 가끔씩만 발생하는 이상한 동작

**예시:**
```c
// 잘못된 코드
if (count < MAX_COUNT) {  // 스레드 A가 체크
    // 스레드 B가 count 증가시킬 수 있음
    count++;  // 스레드 A가 증가
}

// 올바른 코드
pthread_mutex_lock(&mutex);
if (count < MAX_COUNT) {
    count++;
}
pthread_mutex_unlock(&mutex);
```

### 3. 데드락 (Deadlock)

**증상:** 프로그램이 멈춰서 진행 안 됨

**일반적인 원인:**
```c
// 스레드 1
pthread_mutex_lock(&mutex1);
pthread_mutex_lock(&mutex2);

// 스레드 2
pthread_mutex_lock(&mutex2);
pthread_mutex_lock(&mutex1);
```

**해결방법:** 항상 같은 순서로 락 획득

## 🔍 디버깅 팁

### 1. 로그 추가하기
```c
#ifdef DEBUG
    #define LOG_DEBUG(fmt, ...) \
        fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", \
                __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define LOG_DEBUG(fmt, ...)
#endif

// 사용
LOG_DEBUG("Client connected, fd=%d", client_fd);
```

### 2. assert 활용
```c
#include <assert.h>

void process_data(char* data, size_t size) {
    assert(data != NULL);
    assert(size > 0);
    // 이후 안전하게 처리
}
```

### 3. 컴파일러 경고 활용
```bash
# 모든 경고 켜기
gcc -Wall -Wextra -Werror -pedantic

# 더 엄격한 검사
gcc -fsanitize=address  # 메모리 에러 검출
gcc -fsanitize=thread   # 스레드 에러 검출
```

## 📝 체크리스트

에러가 발생했을 때 확인할 사항들:

- [ ] 에러 메시지를 정확히 읽었나요?
- [ ] 컴파일러 경고는 없나요?
- [ ] 포인터를 사용하기 전에 NULL 체크를 했나요?
- [ ] malloc한 메모리를 free했나요?
- [ ] 파일이나 소켓을 열었으면 닫았나요?
- [ ] 배열 인덱스가 범위를 벗어나지 않나요?
- [ ] 멀티스레드 환경에서 공유 자원을 보호했나요?

## 🆘 추가 도움말

### 유용한 도구들
- **GDB**: GNU 디버거
- **Valgrind**: 메모리 검사 도구
- **strace**: 시스템 콜 추적
- **ltrace**: 라이브러리 콜 추적
- **AddressSanitizer**: 메모리 에러 검출

### 도움을 요청할 때
1. 에러 메시지 전체를 포함하세요
2. 관련 코드 부분을 보여주세요
3. 무엇을 시도했는지 설명하세요
4. 운영체제와 컴파일러 버전을 명시하세요

---

💡 **Remember**: 에러는 배움의 기회입니다. 포기하지 마세요!