*LogCaster 프로젝트를 위한 사전지식 문서*

---

## 📋 문서 정보
- **난이도**: 🟢 초급
- **예상 학습 시간**: 3-4시간
- **전제 조건**: 기본적인 프로그래밍 개념 (변수, 함수, 조건문)

## 🎯 이 문서에서 배울 내용

**"프로그래밍이 처음이신가요? 걱정하지 마세요!"**

프로그래밍의 본질은 문제 해결입니다. 이 문서는 LogCaster 프로젝트를 시작하기 전에 반드시 알아야 할 C와 C++의 기초 개념을 **마치 친구가 옆에서 천천히 설명해주는 것처럼** 다룹니다.

🎓 **학습 목표:**
- C와 C++가 무엇인지, 왜 이 언어들을 배워야 하는지 이해하기
- 컴퓨터가 우리의 코드를 어떻게 실행 가능한 프로그램으로 만드는지 알아보기  
- 기본 문법을 통해 간단한 프로그램을 직접 만들어보기
- LogCaster라는 실제 프로젝트에서 이 지식들이 어떻게 활용되는지 체험하기

💡 **이 문서의 특별한 점:**
- 복잡한 용어는 쉬운 예시로 설명합니다
- 모든 코드에는 "왜 이렇게 작성했는지" 이유를 함께 설명합니다
- 실수하기 쉬운 부분은 미리 알려드립니다
- 단계별로 따라하면서 자연스럽게 이해할 수 있도록 구성했습니다

---

## 1. C와 C++의 차이점과 특징

### 🤔 "프로그래밍 언어가 뭔가요?"

프로그래밍 언어는 **인간과 컴퓨터 사이의 번역기** 같은 역할을 합니다. 

**일상 예시로 이해해보기:**
여러분이 외국인 친구와 대화한다고 생각해보세요. 여러분은 한국어로 생각하지만, 친구는 영어만 알아들어요. 이때 번역기가 필요하겠죠?

- **여러분의 생각 (한국어)**: "사용자가 로그인하면 환영 메시지를 보여줘"
- **번역기 역할 (프로그래밍 언어)**: `if (user_login) { print("환영합니다!"); }`
- **컴퓨터가 이해하는 말 (기계어)**: `01001000 01100101 01101100 01101100 01101111`

**왜 C/C++을 배워야 할까요?**

C와 C++은 특히 **컴퓨터의 하드웨어에 가깝게 접근**할 수 있어서, 마치 자동차의 엔진룸에 직접 들어가서 조작하는 것처럼 **매우 세밀한 제어**가 가능합니다.

**실제 사용 예시:**
- **게임 엔진** (Unreal Engine, Unity 내부): 초당 60번의 화면 갱신이 필요한 게임에서는 0.1초의 지연도 큰 문제가 됩니다
- **운영체제** (Windows, Linux): 컴퓨터의 모든 자원을 관리해야 하므로 효율성이 생명입니다
- **웹 브라우저** (Chrome, Firefox): 복잡한 웹페이지를 빠르게 렌더링해야 합니다
- **LogCaster**: 대량의 로그 데이터를 실시간으로 처리해야 하므로 성능이 중요합니다

그래서 **"성능이 생명인 프로그램"**들이 이 언어들로 만들어집니다!

### 🔍 "그럼 C와 C++는 뭐가 다른가요?"

간단히 말하면 **C++는 C의 업그레이드 버전**입니다. C가 1972년에 태어났고, C++가 1985년에 C를 기반으로 더 많은 기능을 추가해서 만들어졌습니다.

### 🅲 C언어: "단순하지만 강력한 도구"

C 언어를 **스위스 아미 나이프**에 비유할 수 있습니다. 기본적이지만 정확하고, 어떤 상황에서든 믿을 수 있는 도구죠.

**📝 C언어의 특징을 쉽게 이해해보기:**

**1. 절차 지향 프로그래밍 = 요리 레시피 방식**
```
🍳 계란후라이 만들기 (C언어 스타일)
1. prepare_pan() - 팬을 준비한다
2. heat_oil() - 기름을 데운다  
3. crack_egg() - 계란을 깬다
4. cook() - 요리한다
5. serve() - 완성!
```
위에서 아래로, 순서대로 함수를 실행합니다. 매우 직관적이고 이해하기 쉽죠!

**2. 저수준 제어 = 자동차 엔진에 직접 접근**
- 다른 언어들이 "가속해줘"라고 말한다면
- C언어는 "엔진 1번 실린더에 연료를 더 넣어줘"라고 구체적으로 지시할 수 있습니다
- 그래서 **운영체제**(Windows, Linux), **임베디드 시스템**(TV, 냉장고의 두뇌) 같은 걸 만들 때 사용합니다

**3. 작고 빠름 = 경량 스포츠카**
- 불필요한 장식이 없어서 프로그램이 가볍습니다
- 실행 속도가 매우 빠릅니다
- 메모리를 적게 사용합니다

**예시 코드:**
```c
#include <stdio.h>  // 표준 입출력 함수 (예: printf)를 사용하기 위한 헤더 파일 포함
#include <stdlib.h> // 메모리 할당/해제 함수 (예: malloc/free)를 사용하기 위한 헤더 파일 포함

// 구조체 정의
// struct 키워드는 여러 종류의 데이터를 하나로 묶어 새로운 데이터 타입을 만드는 데 사용됩니다.
// typedef는 새로운 데이터 타입을 정의하는 키워드입니다.
typedef struct LogEntry {
    char message[256];
    int timestamp;
} LogEntry;

// 함수 정의
void print_log(const LogEntry* log) {
    printf("Log: %s (Time: %d)\n", log->message, log->timestamp);
    log->message[0] = '\0';  // 로그 메시지를 비웁니다 (예시로, 원본 데이터를 수정)
}

int main() {
    // 메모리 직접 할당
    // malloc을 통해 할당된 메모리는 프로그램이 종료될 때까지 유지됩니다.
    // 이 메모리는 수동으로 해제해야 합니다 (free 함수 사용).
    LogEntry* log = malloc(sizeof(LogEntry));
    if (log == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;  // 메모리 할당 실패 시 프로그램 종료
    }

    // 데이터 설정
    // strcpy 함수는 문자열을 복사하는 함수입니다.
    // strncpy를 사용하면 버퍼 오버플로우를 방지할 수 있습니다
    strcpy(log->message, "Server started");
    log->timestamp = 1234567890;
    
    // 함수 호출
    // pass by reference: 포인터를 사용하여 메모리 주소를 전달
    // 이렇게 하면 함수 내에서 원본 데이터를 수정할 수 있습니다.
    print_log(log);
    if (log->message[0] == '\0') {
        printf("Print log message cleared.\n");
    }
    
    // 메모리 해제
    // malloc으로 할당한 메모리는 사용 후 반드시 free로 해제해야 합니다.
    // 프로그램이 종료되면 운영체제가 자동으로 메모리를 해제하지만, 명시적으로 해제하는 것이 좋습니다.
    free(log);
    return 0;
}
```

### 🅲➕ C++: "C언어에 날개를 달아준 언어"

C++를 **스마트폰**에 비유할 수 있습니다. 기본 전화 기능(C언어)은 그대로 유지하면서, 카메라, 인터넷, 게임 등 다양한 앱(새로운 기능들)을 추가한 거죠!

**📱 C++의 특징을 쉽게 이해해보기:**

**1. 객체 지향 프로그래밍 = 레고 블록 조립**
```
🏠 집 짓기 (C++ 스타일)
🧱 벽돌 클래스: 색깔, 크기, 강도 + 쌓기(), 페인트칠하기()
🚪 문 클래스: 재질, 크기, 잠금상태 + 열기(), 닫기(), 잠그기()
🏠 집 클래스: 벽돌들, 문들, 지붕 + 건축하기(), 입주하기()
```
각각이 독립적인 **"객체"**로, 필요에 따라 조합해서 사용할 수 있습니다!

**2. 고수준 추상화 = TV 리모컨**
- 리모컨의 "전원" 버튼을 누르면 TV가 켜집니다
- 하지만 우리는 TV 내부에서 어떤 복잡한 전자회로가 동작하는지 몰라도 됩니다
- C++도 마찬가지로 복잡한 내부 구조를 감추고 **간단한 버튼(함수)**만 제공합니다

**3. 다양한 프로그래밍 방식 지원 = 멀티 플레이어**
- **절차 지향**: C언어처럼 순서대로 (요리 레시피 방식)
- **객체 지향**: 레고 블록 조립 방식
- **제네릭**: 하나의 틀로 여러 종류 만들기 (붕어빵 틀로 다양한 모양 만들기)

**4. 풍부한 표준 라이브러리(STL) = 만능 도구상자**
- 필요한 도구들이 이미 다 들어있어서 바로 꺼내 쓸 수 있습니다
- 동적 배열, 정렬, 검색 등등... 바퀴를 다시 발명할 필요가 없어요!

**예시 코드:**
```cpp
#include <iostream> // C++ 표준 입출력 (예: std::cout)을 위한 헤더 파일 포함
#include <string>   // 문자열을 쉽게 다룰 수 있는 std::string 클래스 사용을 위한 헤더 파일 포함
#include <memory>   // 스마트 포인터 (예: std::make_unique)를 사용하기 위한 헤더 파일 포함

// 클래스 정의
// class 키워드는 객체를 만들기 위한 틀(설계도)를 정의합니다.
// LogEntry 클래스는 로그 메시지와 타임스탬프를 저장하는 역할을 하며, 이를 다루는 메서드를 포함합니다.
class LogEntry {
// private 접근 지정자: 클래스 내부에서만 접근 가능하도록 데이터를 숨깁니다. (캡슐화)
private:
    // C++ 표준 라이브러리의 문자열 클래스 (자동으로 메모리 관리)
    std::string message;
    int timestamp;

public:
    // 생성자: 객체가 생성될 떄 자동으로 호출되는 특별한 함수입니다.
    // 객체의 초기 상태를 설정하는 역할을 합니다.
    LogEntry(const std::string& msg, int time) 
        // 초기화 리스트를 사용하여 멤버 변수를 초기화합니다.
        // 초기화 리스트는 생성자 본문보다 먼저 실행되어, 멤버 변수를 초기화하는 데 더 효율적입니다.
        : message(msg), timestamp(time) {}
    
    // 메서드 (클래스 내부의 함수)
    // const 키워드는 이 메서드가 객체의 상태를 변경하지 않음을 나타냅니다.
    void print() const {
        // std::cout은 C++의 표준 출력 스트림입니다.
        // << 연산자는 출력 스트림에 데이터를 삽입하는 데 사용됩니다.
        // std::endl은 줄바꿈을 하고 출력 버퍼를 비웁니다.
        std::cout << "Log: " << message << " (Time: " << timestamp << ")" << std::endl;
    }
    
    // Getter 메서드: private 멤버 변수의 값을 읽어올 때 사용됩니다.
    // 첫 번째 const는 반환되는 참조가 원본 데이터를 수정하지 않음을 보장하고,
    // 두 번째 const는 메서드가 객체의 상태를 변경하지 않음을 보장합니다.
    const std::string& getMessage() const { return message; }
    int getTimestamp() const { return timestamp; }
};

// 프로그램의 시작점 함수
int main() {
    // 스마트 포인터로 자동 메모리 관리
    // std::make_unique는 C++14부터 제공되는 함수로, unique_ptr라는 스마트 포인터를 생성합니다.
    // 스마트 포인터는 자신이 가리키는 메모리 영역이 더 이상 사용되지 않을 때 자동으로 메모리를 해제하며 이로 인해 메모리 누수를 방지합니다.
    // auto 키워드는 변수의 타입을 컴파일러가 자동으로 추론하도록 합니다.
    auto log = std::make_unique<LogEntry>("Server started", 1234567890);
    
    // 메서드 호출
    log->print();
    
    // 메모리 자동 해제 (소멸자가 자동 호출)
    // unique_ptr는 스코프를 벗어나면 자동으로 메모리를 해제합니다.
    // 따라서 별도로 메모리를 해제할 필요가 없으며 덕분에 C++에서는 메모리 관리가 더 안전하고 편리합니다.
    return 0;
}
```

### 🔍 주요 차이점 비교

| 항목 | C | C++ |
|----|---|---|
| **프로그래밍 패러다임** | 절차 지향 (함수와 순서 중심) | 객체 지향 (클래스와 객체 중심) + 절차 지향 |
| **메모리 관리** | 수동 (`malloc`/`free`) | 자동 + 수동 (스마트 포인터) |
| **코드 구조** | 함수 중심 | 클래스 중심 |
| **표준 라이브러리** | 제한적 | 풍부 (STL) |
| **컴파일 속도** | 빠름 | (추상화 및 객체 지향 기능으로 인해) 상대적으로 느림 |
| **실행 속도** | 매우 빠름 | 빠름 (C에 비해 약간의 오버헤드가 있을 수 있지만 대부분 고성능) |
| **학습 곡선** | 가파름 (저수준) | 점진적 (추상화) |

---

## 2. 컴파일 과정과 빌드 시스템

### 🤔 "컴퓨터는 우리 말을 못 알아듣나요?"

네, 맞습니다! 컴퓨터는 우리가 작성한 C/C++ 코드를 직접 이해하지 못합니다. 

**예시로 이해해보기:**
- **우리가 쓴 코드**: `printf("Hello World");`
- **컴퓨터가 이해하는 언어**: `01001000 01100101 01101100 01101100 01101111`

따라서 **"번역"** 과정이 필요합니다. 이 번역 과정을 **컴파일(Compile)**이라고 하고, 번역기를 **컴파일러(Compiler)**라고 합니다.

### 🏭 컴파일 과정: "코드 제작 공장의 4단계"

우리 코드가 실행 파일이 되기까지, 마치 **자동차 제작 공장**의 컨베이어 벨트처럼 4단계를 거칩니다:

```
📝 우리 코드 → 🔧 전처리 → ⚙️ 컴파일 → 🔩 어셈블리 → 📦 링킹 → 🚀 실행파일
```

**실제 예시로 따라해보기:**
우리가 이런 간단한 코드를 작성했다고 가정해봅시다:

```c
#include <stdio.h>
#define GREETING "안녕하세요"
// 메인 함수입니다
int main() {
    printf("%s, 세계여!\n", GREETING);
    return 0;
}
```

이 코드가 어떻게 변해가는지 단계별로 보겠습니다!

#### 🔧 1단계: 전처리 (Preprocessing) - "준비 작업"

**🍳 요리 준비에 비유하면:**
- 냉장고에서 재료 꺼내기 (`#include` - 필요한 헤더파일 가져오기)
- 레시피 메모지 붙이기 (`#define` - 상수 정의하기)  
- 불필요한 포장지 버리기 (주석 제거)

**전처리 과정에서 일어나는 일:**

```c
// 원본 코드
#include <stdio.h>     // 📋 "printf 사용법이 적힌 설명서를 가져와줘"
#define GREETING "안녕하세요"   // 🏷️ "GREETING이라고 쓰면 "안녕하세요"라는 뜻이야"
// 메인 함수입니다 <- 이 주석은 사라집니다
int main() {
    printf("%s, 세계여!\n", GREETING);
    return 0;
}
```

**전처리 후 결과:**
```c
// <stdio.h>의 내용이 여기에 수백 줄로 붙여넣어집니다!
// printf, scanf 등의 함수 선언들...
extern int printf(const char *format, ...);
// ... 더 많은 선언들 ...

int main() {
    printf("%s, 세계여!\n", "안녕하세요");  // GREETING → "안녕하세요"로 치환됨
    return 0;
}
```

**터미널에서 직접 확인해보기:**
```bash
gcc -E hello.c -o hello.i    # 전처리만 수행하고 결과를 hello.i에 저장
cat hello.i                  # 전처리 결과를 확인
```

#### ⚙️ 2단계: 컴파일 (Compilation) - "번역 작업"

**🌍 언어 번역에 비유하면:**  
우리가 쓴 C/C++ 코드를 **어셈블리어**라는 중간 언어로 번역합니다.

**실제 변환 과정:**
```c
// 전처리된 C 코드
int main() {
    printf("%s, 세계여!\n", "안녕하세요");
    return 0;
}
```

**↓ 어셈블리어로 번역**
```assembly
main:
    push    rbp                    # 스택 프레임 설정
    mov     rbp, rsp
    mov     rsi, OFFSET "안녕하세요"    # 두 번째 인자 (문자열)
    mov     rdi, OFFSET "%s, 세계여!\n" # 첫 번째 인자 (포맷 문자열)
    call    printf                 # printf 함수 호출
    mov     eax, 0                 # 반환값 0 설정
    pop     rbp                    # 스택 프레임 복원
    ret                           # 함수 종료
```

**터미널에서 확인하기:**
```bash
gcc -S hello.c -o hello.s    # 어셈블리어로 컴파일
cat hello.s                  # 어셈블리 코드 확인
```

#### 🔩 3단계: 어셈블리 (Assembly) - "기계어 변환"

**🔢 암호화에 비유하면:**  
어셈블리 코드를 컴퓨터가 직접 이해하는 **0과 1의 기계어**로 변환합니다.

**실제 변환 과정:**
```assembly
mov rsi, OFFSET "안녕하세요"
```
**↓ 기계어로 변환**
```
48 be 10 20 40 00 00 00 00 00    # 16진수 기계어
01001000 10111110 00010000 ...    # 2진수 기계어 (실제 0과 1)
```

이때 만들어지는 파일을 **오브젝트 파일(.o, .obj)**이라고 합니다.

**터미널에서 확인하기:**
```bash
gcc -c hello.c -o hello.o    # 오브젝트 파일 생성
hexdump -C hello.o | head    # 기계어를 16진수로 확인
```

#### 📦 4단계: 링킹 (Linking) - "최종 조립"

**🧩 퍼즐 맞추기에 비유하면:**  
여러 조각들(오브젝트 파일들, 라이브러리들)을 하나로 연결해서 완성된 실행 파일을 만듭니다.

**링킹 과정 상세 설명:**

1. **내가 만든 오브젝트 파일** (hello.o)
   - main 함수의 기계어 코드
   - "printf를 어디서 찾을지는 모르겠어..." 라는 빈 칸

2. **printf 라이브러리** (libc.so 또는 libc.a)
   - printf 함수의 실제 구현 코드
   - 수천 개의 다른 표준 함수들

3. **링커의 작업:**
   - "아! printf는 여기 있구나!" 하며 빈 칸을 채워넣음
   - 메모리 주소를 정확하게 계산해서 연결
   - 실행 가능한 파일 형식으로 포장

**결과:**
```
내가 만든 코드 🧩 + printf 라이브러리 🧩 + 시작코드 🧩 + 기타 라이브러리들 🧩 
= 완성된 실행파일 🎯 (hello 또는 hello.exe)
```

**터미널에서 전체 과정 확인:**
```bash
# 한 번에 컴파일하고 링킹
gcc hello.c -o hello

# 실행해보기
./hello                      # 출력: 안녕하세요, 세계여!

# 실행파일 정보 확인
file hello                   # 파일 타입 확인
ldd hello                    # 링크된 라이브러리 확인 (Linux)
```

### C 프로젝트 - Makefile

`Makefile`은 `make`라는 빌드 도구가 소스 코드를 컴파일하고 링크하는 과정을 자동화하기 위해 사용하는 스크립트 파일입니다. 복잡한 명령어를 매번 직접 입력하지 않고, `Makefile`에 정의된 규칙을 사용하여 간편하게 빌드할 수 있습니다.

#### LogCaster-C 프로젝트에서 사용할 Makefile 예시:

```makefile
# 컴파일러 설정
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread
    # -Wall: 모든 경고 메시지 출력
    # -Wextra: 추가적인 경고 메시지 출력
    # -std=c20: C20 표준을 사용
    # -pthread: POSIX 스레드 라이브러리 사용
# 최종 실행 파일 이름
TARGET = logcaster_c
SRCDIR = src
# 'src' 디렉토리 안의 모든 '.c' 파일을 찾아 'SOURCES' 변수에 저장합니다.
SOURCES = $(wildcard $(SRCDIR)/*.c)
# 'SOURCES'에 있는 각 '.c' 파일의 확장자를 '.o'로 변경하여 'OBJECTS' 변수에 저장합니다.
# (예: src/main.c -> src/main.o)
OBJECTS = $(SOURCES:.c=.o)

# 기본 타겟: make 명령만 입력했을 때 실행되는 타겟
all: $(TARGET)

# TARGET: RECIPE
#   COMMAND

# 최종 실행 파일을 생성 규칙
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(CFLAGS)

# 오브젝트 파일 생성
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# 정리
clean:
	rm -f $(OBJECTS) $(TARGET)

# 실행
run: $(TARGET)
	./$(TARGET)

# .PHONY: makefile에서 정의된 타겟이 실제 파일이 아님을 명시
# make가 항상 이 타겟을 실행하도록 합니다.
.PHONY: all clean run
```

#### 사용법:
`Makefile`이 있는 디렉토리에서 터미널을 열고 다음 명령어를 입력합니다.

```bash
make          # all 타겟을 실행하여 프로젝트를 컴파일하고 실행파일을 만듭니다.
make run      # 프로젝트를 컴파일한 후 바로 실행합니다.
make clean    # 컴파일된 오브젝트 파일과 실행 파일을 삭제합니다.
```

### C++ 프로젝트 - CMake

CMake는 C++ 프로젝트에서 가장 널리 사용되는 빌드 시스템 생성기입니다. `Makefile`과 같은 특정 빌드 도구에 종속되지 않고, 다양한 운영체제(Windows, Linux, macOS)와 컴파일러 환경에서 프로젝트를 빌드할 수 있도록 도와줍니다. `CMake`는 `CMakeLists.txt`라는 설정 파일을 기반으로 각 환경에 맞는 빌드 스크립트(예: `Makefile`, `Visual Studio` 프로젝트 파일)를 자동으로 생성해줍니다.

#### LogCaster-CPP 프로젝트에서 사용할 CMakeLists.txt 예시:

```cmake
# 최소 CMake 버전
cmake_minimum_required(VERSION 3.10) # 이 프로젝트를 빌드하기 위해 필요한 최소 CMake 버전을 지정합니다.

# 프로젝트 정보
project(LogCaster_CPP VERSION 1.0) # 프로젝트의 이름과 버전을 설정합니다.

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17) # C++ 컴파일러가 C++17 표준을 따르도록 설정합니다.
set(CMAKE_CXX_STANDARD_REQUIRED ON) # C++17 표준이 반드시 필요함을 명시합니다.

# 컴파일러 옵션
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") # C++ 컴파일러 옵션을 설정합니다.
                                                        # 기존 옵션에 -Wall, -Wextra를 추가합니다 (C와 동일).

# 소스 파일 찾기
file(GLOB_RECURSE SOURCES "src/*.cpp") # 'src' 디렉토리와 그 하위 디렉토리에서 모든 '.cpp' 파일을 찾아 'SOURCES' 변수에 저장합니다.
file(GLOB_RECURSE HEADERS "include/*.hpp") # 'include' 디렉토리와 그 하위 디렉토리에서 모든 '.hpp' 파일을 찾아 'HEADERS' 변수에 저장합니다.

# 헤더 파일 디렉토리
include_directories(include) # 컴파일러가 헤더 파일을 찾을 때 'include' 디렉토리를 포함하도록 설정합니다.

# 실행 파일 생성
add_executable(logcaster_cpp ${SOURCES}) # 'logcaster_cpp'라는 이름의 실행 파일을 생성하며,
                                            # 이때 'SOURCES' 변수에 있는 모든 '.cpp' 파일을 사용합니다.

# 스레드 라이브러리 링크
find_package(Threads REQUIRED) # 'Threads' 라이브러리를 찾습니다. 'REQUIRED'는 이 라이브러리가 필수적임을 의미합니다.
target_link_libraries(logcaster_cpp Threads::Threads) # 'logcaster_cpp' 실행 파일에 찾은 'Threads' 라이브러리를 링크합니다.
                                                        # 스레드를 사용하는 프로그램을 만들 때 필요합니다.

# 선택적: Boost 라이브러리 (필요에 따라 주석 해제하여 사용)
# find_package(Boost REQUIRED COMPONENTS system) # Boost 라이브러리를 찾고, 'system' 컴포넌트가 필요하다고 명시합니다.
# target_link_libraries(logcaster_cpp ${Boost_LIBRARIES}) # Boost 라이브러리를 실행 파일에 링크합니다.
```

#### 사용법:

`CMakeLists.txt`가 있는 디렉토리에서 터미널을 열고 다음 명령어를 입력합니다.

```bash
mkdir build
cd build
cmake ..    # CMakeLists.txt 파일을 읽고 현재 환경에 맞는 빌드 시스템을 생성합니다.
make        # 생성된 빌드 시스템을 사용하여 프로젝트를 컴파일합니다. (이 경우 Makefile을 예시로 사용)
./logcaster_cpp
```

---

## 3. 기본 문법과 데이터 타입

모든 프로그래밍 언어에는 데이터를 표현하고 조작하기 위한 기본적인 규칙과 요소들이 있습니다. 이를 '문법'이라고 합니다.

### C 기본 문법

C 언어는 데이터를 저장하기 위한 다양한 '데이터 타입'을 제공하며, 함수를 사용하여 프로그램을 구성합니다.

#### 변수 선언과 초기화

**변수:** 데이터를 저장할 수 있는 메모리 공간에 이름을 붙인 것입니다. 변수를 사용하기 전에는 어떤 종류의 데이터를 저장할 지 (데이터 타입) 그리고 그 변수 이름을 무엇으로 할지(선언) 지정해야 합니다. '초기화'는 변수를 선언함과 동시에 첫 값을 할당하는 것을 의미합니다.

```c
// 기본 데이터 타입: C 언어가 기본적으로 제공하는 데이터 종류입니다.
int port = 9999;              // 'int'는 정수(소수점이 없는 숫자)를 저장하는 데이터 타입입니다.
                              // 'port'라는 이름의 정수형 변수를 선언하고 9999로 초기화했습니다.
float timeout = 5.5f;         // 'float'는 단정밀도 부동소수점 숫자(소수점이 있는 실수)를 저장하는 데이터 타입입니다.
                              // 숫자 뒤에 'f'를 붙여 float 타입임을 명시합니다.
char buffer[256];             // 'char'는 하나의 문자(예: 'a', 'B', '1')를 저장하는 데이터 타입입니다.
                              // 'char buffer[256]'는 256개의 문자를 저장할 수 있는 '문자 배열'입니다.
                              // 문자 배열은 C에서 문자열(여러 문자가 나열된 것, 예: "Hello")을 표현할 때 사용됩니다.
char* message = "Hello";      // 'char*'는 '문자 포인터'입니다. 문자열 리터럴("Hello")은 메모리에 저장되고,
                              // 그 시작 주소가 'message' 포인터 변수에 저장됩니다.
                              // 이 포인터를 통해 문자열에 접근할 수 있습니다.

// 상수 정의: 프로그램 실행 중에 값이 변하지 않는 고정된 값입니다.
const int MAX_CONNECTIONS = 100; // 'const' 키워드는 변수를 상수로 만듭니다. 한 번 초기화되면 값을 변경할 수 없습니다.
#define BUFFER_SIZE 1024         // '#define' 전처리기 지시문으로 상수를 정의할 수도 있습니다.
                                 // 컴파일 전에 'BUFFER_SIZE'가 '1024'로 모두 치환됩니다.

// 구조체 (struct): 서로 다른 데이터 타입을 하나로 묶어 새로운 데이터 타입을 정의할 때 사용합니다.
// 마치 사용자가 직접 만드는 '복합 데이터 타입'과 같습니다.
struct ServerConfig {
    int port;            // 서버 포트 번호 (정수)
    char host[64];       // 호스트 이름 (문자 배열)
    int max_connections; // 최대 연결 수 (정수)
};

// 열거형 (enum): 정수형 상수들에 이름을 부여하여 코드의 가독성을 높일 때 사용합니다.
// 관련된 값들을 묶어서 관리하기 편리합니다.
enum LogLevel {
    LOG_DEBUG = 0, // 'LOG_DEBUG'는 0이라는 정수 값을 가집니다.
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3
};
```

#### 함수 정의

**함수:** 특정 작업을 수행하는 코드 블록에 이름을 붙인 것입니다. 함수를 사용하면 코드를 재사용하고, 프로그램을 작은 단위로 나누어 관리하기 용이합니다.

```c
// 함수 선언 (헤더 파일에 주로 작성): 함수를 사용하기 전에 컴파일러에게 함수의 존재,
// 이름, 매개변수(입력), 반환 타입(출력)을 미리 알려주는 것입니다.
int start_server(int port); // 'start_server'라는 함수는 정수형 'port'를 입력받고, 정수 값을 반환합니다.
void log_message(enum LogLevel level, const char* message); // 'log_message' 함수는 'LogLevel' 열거형과 문자 포인터를 입력받고,
                                                            // 어떤 값도 반환하지 않습니다('void').

// 함수 구현 (소스 파일에 작성): 함수가 실제로 어떤 작업을 수행할지 코드로 작성하는 부분입니다.
int start_server(int port) {
    printf("Starting server on port %d\n", port); // 'printf'는 표준 출력 함수로, 화면에 텍스트를 출력합니다.
                                                  // '%d'는 뒤에 오는 정수형 변수 'port'의 값이 들어갈 자리입니다.
    return 0;  // 함수가 성공적으로 완료되었음을 나타내기 위해 0을 반환합니다. (관례)
}

void log_message(enum LogLevel level, const char* message) {
    // 'level_names'는 'LogLevel' 열거형의 각 값에 대응하는 문자열 배열입니다.
    // 예를 들어, level이 'LOG_INFO' (값 1)이면 'level_names[1]'은 "INFO"가 됩니다.
    const char* level_names[] = {"DEBUG", "INFO", "WARNING", "ERROR"};
    printf("[%s] %s\n", level_names[level], message); // 로그 레벨과 메시지를 출력합니다.
}
```

### C++ 기본 문법

C++은 C의 문법을 대부분 그대로 사용하면서, '클래스', '객체', '네임스페이스' 등의 객체 지향 요소를 추가하여 더 강력하고 유연한 프로그래밍이 가능합니다.

#### 클래스와 객체

**클래스(Class):** 객체를 생성하기 위한 '설계도' 또는 '틀' 입니다. 데이터(속성)와 데이터를 조작하는 함수(메서드)를 함꼐 묶어 하나의 단위로 정의합니다.

**객체(Object):** 클래스라는 설계도를 바탕으로 실제로 만들어진 '실체', 즉 클래스의 인스턴스입니다. 객체는 클래스에서 정의한 속성과 메서드를 가집니다. 예를 들어 '자동차'가 클래스라면 '내 자동차'는 객체가 됩니다.

```cpp
#include <string>   // C++ 문자열 클래스 std::string을 사용하기 위해 포함
#include <vector>   // C++ 동적 배열 std::vector를 사용하기 위해 포함 (예시 코드에는 직접 사용되지 않지만 언급됨)
#include <iostream> // C++ 표준 입출력 std::cout을 사용하기 위해 포함

// Server 클래스 정의
class Server {
private: // private 멤버는 클래스 외부에서 직접 접근할 수 없으며, 클래스 내부의 메서드를 통해서만 접근할 수 있습니다.
         // 이는 데이터를 보호하고, 클래스의 내부 구현을 숨겨 유지보수를 쉽게 합니다 (정보 은닉).
    int port_;             // 서버 포트 번호
    std::string host_;     // 호스트 이름 (C++의 안전하고 편리한 문자열 클래스)
    bool running_;         // 서버 실행 상태 (true/false)

public: // public 멤버는 클래스 외부에서 자유롭게 접근할 수 있습니다. 이들은 클래스의 '인터페이스' 역할을 합니다.
    // 생성자: 'Server' 객체가 생성될 때 자동으로 호출되는 특별한 메서드입니다.
    // 주로 멤버 변수를 초기화하는 데 사용됩니다.
    // 'Server(int port, const std::string& host)'는 두 개의 매개변수를 받아 멤버 변수 'port_'와 'host_'를 초기화합니다.
    Server(int port, const std::string& host) 
        : port_(port), host_(host), running_(false) {} // '멤버 초기화 리스트'를 사용하여 멤버 변수를 효율적으로 초기화합니다.
    
    // 소멸자: 객체가 소멸될 때 (메모리에서 해제될 때) 자동으로 호출되는 특별한 메서드입니다.
    // 주로 객체가 사용했던 자원(메모리, 파일 핸들 등)을 정리하는 데 사용됩니다.
    ~Server() {
        if (running_) { // 서버가 실행 중이었다면
            stop();     // 서버를 중지하는 메서드를 호출합니다.
        }
        std::cout << "Server object destroyed." << std::endl; // 소멸 메시지 출력 (예시 추가)
    }
    
    // 메서드 (멤버 함수): 클래스에 속한 함수로, 해당 클래스의 객체가 수행할 수 있는 '행동'을 정의합니다.
    bool start() {
        std::cout << "Starting server on " << host_ << ":" << port_ << std::endl;
        running_ = true; // 서버 상태를 '실행 중'으로 변경
        return true;     // 시작 성공을 반환
    }
    
    void stop() {
        std::cout << "Stopping server" << std::endl;
        running_ = false; // 서버 상태를 '정지'로 변경
    }
    
    // Getter/Setter 메서드: private 멤버 변수에 안전하게 접근하기 위한 public 메서드입니다.
    // 'getPort()'는 'port_' 값을 반환하고, 'const'는 이 메서드가 객체의 상태를 변경하지 않음을 의미합니다.
    int getPort() const { return port_; }
    bool isRunning() const { return running_; }
};

// 네임스페이스 (namespace): 코드의 이름 충돌을 방지하고, 관련된 코드들을 그룹화하는 데 사용됩니다.
// 예를 들어, 'LogCaster' 네임스페이스 안에 'LogLevel'과 'Logger'를 정의하여
// 다른 라이브러리에 동일한 이름이 있더라도 충돌 없이 사용할 수 있도록 합니다.
namespace LogCaster {
    // 'enum class' (범위 지정 열거형): C++11부터 도입된 열거형으로,
    // 열거자(Debug, Info 등)가 해당 열거형 스코프 안에서만 유효하며,
    // 다른 열거형이나 전역 이름과 이름 충돌을 일으키지 않습니다.
    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };
    
    class Logger {
    public:
        // 'static' 키워드: 이 메서드가 특정 'Logger' 객체에 속하지 않고,
        // 'Logger' 클래스 자체에 속함을 의미합니다.
        // 객체를 생성하지 않고도 'LogCaster::Logger::log()'와 같이 직접 호출할 수 있습니다.
        static void log(LogLevel level, const std::string& message) {
            std::cout << "[" << levelToString(level) << "] " << message << std::endl;
        }
        
    private:
        // 'static' 및 'private' 메서드: 클래스 내부에서만 사용되는 헬퍼(도우미) 함수입니다.
        static std::string levelToString(LogLevel level) {
            switch (level) {
                case LogLevel::Debug: return "DEBUG";
                case LogLevel::Info: return "INFO";
                case LogLevel::Warning: return "WARNING";
                case LogLevel::Error: return "ERROR";
                default: return "UNKNOWN";
            }
        }
    };
} // end namespace LogCaster
```

#### STL 컨테이너 사용

**STL(Standard Template Library):** C++ 표준 라이브러리의 핵심 부분으로, 데이터를 효율적으로 저장하고 관리하는 '컨테이너(Container)'와 데이터를 처리하는 '알고리즘(Algorithm)'등을 제공합니다. STL 컨테이너는 다양한 상황에 맞춰 최적화된 자료 구조를 제공하며, 대부분 자동으로 메모리를 관리해줍니다.

```cpp
#include <vector> // 동적 배열인 std::vector를 사용하기 위해 포함
#include <string> // 문자열 클래스 std::string을 사용하기 위해 포함
#include <map>    // 키-값 쌍을 저장하는 연관 컨테이너 std::map을 사용하기 위해 포함
#include <queue>  // FIFO(선입선출) 구조의 큐 컨테이너 std::queue를 사용하기 위해 포함

// 동적 배열: 'std::vector'는 크기가 변할 수 있는 배열입니다.
// 필요한 만큼 메모리를 자동으로 할당하고 해제하므로, C의 고정 크기 배열이나 수동 메모리 관리보다 편리하고 안전합니다.
std::vector<std::string> log_messages; // 문자열을 저장할 수 있는 동적 배열 선언
log_messages.push_back("Server started"); // 배열의 끝에 요소 추가
log_messages.push_back("Client connected"); // 또 다른 요소 추가

// 연관 컨테이너: 'std::map'은 '키(Key)'와 '값(Value)'의 쌍을 저장하는 컨테이너입니다.
// 키를 통해 빠르게 해당 값에 접근할 수 있습니다. 키는 중복될 수 없습니다.
std::map<int, std::string> client_map; // 정수형 키와 문자열 값으로 구성된 맵 선언
client_map[1] = "192.168.1.100"; // 키 1에 "192.168.1.100" 값 저장
client_map[2] = "192.168.1.101"; // 키 2에 "192.168.1.101" 값 저장

// 큐 (로그 버퍼용): 'std::queue'는 '선입선출(FIFO: First-In, First-Out)' 방식으로 동작하는 컨테이너입니다.
// 먼저 추가된 요소가 먼저 제거됩니다. 대기열(줄 서기)과 유사합니다.
std::queue<std::string> log_buffer; // 문자열을 저장할 수 있는 큐 선언
log_buffer.push("New log entry"); // 큐의 뒤쪽에 요소 추가
std::string next_log = log_buffer.front(); // 큐의 가장 앞에 있는 요소 (제거하지 않고 읽기)
log_buffer.pop(); // 큐의 가장 앞에 있는 요소 제거
```

---

## 4. LogCaster 프로젝트에서의 활용

LogCaster 프로젝트는 로그를 효율적으로 관리하고 처리하는 시스템입니다. C/C++ 각각의 특성을 활용하여 프로젝트를 구현할 수 있습니다.

### C 버전에서 중요한 개념

1. **구조체(`struct`)**: 다양한 속성(메시지, 시간, 로그 레벨 등)을 가진 '로그 엔트리'나 '클라이언트 정보'와 같은 복합적인 데이터를 하나의 단위로 묶어 저장하는 데 사용됩니다.
2. **포인터(`*`)**:
    - **동적 메모리 할당**: `malloc()`과 `free()`를 사용하여 런타임에 메모리를 할당하고 해제합니다.
    - **참조 전달**: 함수에 포인터를 전달하여 원본 데이터를 수정하거나, 메모리 사용을 최적화합니다.
    - **연결 리스트**: 필요한 데이터셋을 동적으로 관리하기 위해 포인터를 사용하여 연결된 데이터 구조를 만듭니다. 이는 고정된 배열보다 유연하게 데이터를 추가하거나 삭제할 수 있게 합니다.
3. **함수**: '로그 기록', '로그 출력', '서버 시작'등 특정 기능을 수행하는 작은 단위로 프로그램을 모듈화하여 관리하고 재사용성을 높이는 데 활용됩니다.
4. **전처리기(`#define`, `#include`)**:
    - **상수 정의**: `#define`을 사용하여 프로그램 전반에서 사용할 상수를 정의합니다. 예를 들어, 로그 버퍼 크기나 최대 연결 수 등을 정의할 수 있습니다.
    - **조건부 컴파일**: 특정 조건에 따라 코드의 일부를 컴파일하거나 제외할 때 사용될 수 있어, 여러 환경에 맞는 코드를 만들 때 유용합니다.

### C++ 버전에서 중요한 개념

1. **클래스(`class`)**:
    - **Logger, Server, Client 클래스 설계**: '로그를 기록하는 주체', '서버 기능', '클라이언트 정보'와 같이 독립적인 역할과 데이터를 가지는 객체를 클래스로 설계하여, 실제 세계의 대상을 모델링하고 코드를 체계적으로 구성합니다.
    - **캡슐화**: 클래스 내부에 데이터를 숨기고 (`private`), 외부에 공개된 메서드를 통해서만 접근하게 하여 (`public`) 데이터의 무결성을 보호하고 코드의 변경이 다른 부분에 미치는 영향을 최소화합니다.
2. **STL 컨테이너(`std::vector`, `std::map`, `std::queue` 등)**:
    - **로그 저장**: `std::vector`와 같은 동적 배열을 사용하여 가변적인 수의 로그 엔트리를 효율적으로 저장하고 관리합니다.
    - **클라이언트 관리**: `std::map`과 같은 연관 컨테이너를 사용하여 클라이언트 ID와 같은 키를 통해 클라이언트 정보를 빠르게 찾아 관리할 수 있습니다.
    - **로그 버퍼**: `std::queue`를 사용하여 로그 이벤트를 순서대로 처리하는 버퍼를 구현할 수 있습니다.
3. **스마트 포인터(`std::unique_ptr`, `std::shared_ptr`)**:
    - **메모리 관리**: C++의 스마트 포인터를 사용하여 동적 메모리 할당과 해제를 자동으로 관리합니다. 이는 메모리 누수를 방지하고, 코드의 안전성을 높이는 데 기여합니다.
4. **네임스페이스(`namespace`)**:
    - **코드 구조화**: 관련된 클래스, 함수, 변수 등을 논리적으로 묶어 이름 충돌을 방지하고, 코드의 가독성과 유지보수성을 향상시킵니다.

---

## 5. 실습 예제

개념 학습 후에 직접 코드를 작성하며 익히는 것이 중요합니다. 

**🎯 실습 목표:**
- 지금까지 배운 개념들을 실제 코드로 구현해보기
- C와 C++의 차이점을 직접 체험하기
- LogCaster 프로젝트의 기초가 되는 로그 시스템 만들어보기

**💡 실습 팁:**
1. **따라 치지 말고 이해하며 작성하세요** - 각 줄이 왜 필요한지 생각해보세요
2. **에러를 두려워하지 마세요** - 컴파일 에러는 여러분을 도와주는 친구입니다
3. **작은 변화를 시도해보세요** - 코드를 조금씩 바꿔보며 어떻게 동작하는지 관찰하세요

간단한 로그 시스템 구현 예제를 통해 앞서 배운 개념을 적용해봅시다.

### 간단한 로그 시스템 구현 - C 버전

이 예제는 '연결 리스트'와 '구조체'를 활용하여 로그를 추가하고 출력하는 기본적인 C 스타일의 로그 시스템을 구현합니다.

```c
#include <stdio.h>    // 표준 입출력 함수 (printf) 사용
#include <stdlib.h>   // 메모리 할당 (malloc, free) 사용
#include <string.h>   // 문자열 복사 (strcpy) 사용
#include <time.h>     // 시간 정보 (time_t, time) 사용

// 로그 엔트리를 나타내는 구조체 정의
struct LogEntry {
    char message[256];       // 로그 메시지를 저장할 문자 배열
    time_t timestamp;        // 로그가 기록된 시간을 저장할 변수
    struct LogEntry* next;   // 다음 LogEntry를 가리키는 포인터 (연결 리스트 구현용)
};

// 로그 연결 리스트의 시작(헤드)을 가리키는 전역 포인터
// 전역 변수로 선언하여 어떤 함수에서든 접근하고 변경할 수 있도록 합니다.
struct LogEntry* log_head = NULL; // 처음에는 아무 로그도 없으므로 NULL로 초기화

// 새로운 로그를 연결 리스트에 추가하는 함수
void add_log(const char* message) {
    // 새 로그 엔트리를 위한 메모리 동적 할당
    // malloc으로 LogEntry 구조체 크기만큼의 메모리를 요청하고, 그 주소를 new_entry 포인터에 저장합니다.
    struct LogEntry* new_entry = malloc(sizeof(struct LogEntry));
    // 메모리 할당에 실패한 경우 (메모리가 부족할 때)
    if (new_entry == NULL) {
        perror("Failed to allocate memory for new log entry"); // 오류 메시지 출력
        return; // 함수 종료
    }
    
    // 할당된 메모리에 로그 메시지 복사
    // strcpy_s (보안 버전) 대신 strcpy를 사용했지만, 실제 프로젝트에서는 더 안전한 함수를 고려해야 합니다.
    strncpy(new_entry->message, message, sizeof(new_entry->message) - 1); // 버퍼 오버플로우 방지
    new_entry->message[sizeof(new_entry->message) - 1] = '\0'; // 문자열 끝에 널 문자 보장
    
    // 현재 시간 저장
    new_entry->timestamp = time(NULL); // time(NULL)은 현재 시간을 초 단위로 반환합니다.
    
    // 새 엔트리를 연결 리스트의 맨 앞에 추가
    new_entry->next = log_head; // 새 엔트리의 next 포인터가 현재 리스트의 첫 번째 엔트리를 가리키게 합니다.
    log_head = new_entry;       // 리스트의 헤드(시작)를 새 엔트리로 업데이트합니다.
}

// 저장된 모든 로그를 출력하는 함수
void print_logs() {
    // 리스트의 시작부터 끝까지 순회하기 위한 임시 포인터
    struct LogEntry* current = log_head;
    printf("\n--- Log Entries ---\n");
    while (current != NULL) { // 현재 포인터가 NULL이 아닐 때까지 (리스트의 끝에 도달할 때까지) 반복
        // 로그 메시지와 시간을 출력합니다.
        // %ld는 long int 타입의 정수를 출력할 때 사용합니다. time_t는 보통 long int로 정의됩니다.
        printf("Message: \"%s\" (Time: %ld)\n", current->message, current->timestamp);
        current = current->next; // 다음 엔트리로 이동
    }
    printf("-------------------\n");
}

// 할당된 모든 로그 메모리를 해제하는 함수
void cleanup_logs() {
    struct LogEntry* current = log_head;
    while (current != NULL) {
        struct LogEntry* next = current->next; // 다음 엔트리의 주소를 미리 저장해둡니다.
        free(current); // 현재 엔트리의 메모리를 해제합니다.
        current = next; // 다음 엔트리로 이동합니다.
    }
    log_head = NULL; // 모든 메모리를 해제했으므로 헤드 포인터를 NULL로 설정하여 빈 리스트로 만듭니다.
    printf("All log entries cleaned up.\n");
}

int main_c_example() {
    add_log("Server started successfully.");
    add_log("Client connected from 192.168.1.10.");
    add_log("Data processing complete.");

    print_logs();

    cleanup_logs();
    return 0;
}
```

### 간단한 로그 시스템 구현 - C++ 버전

이 예제는 '클래스', '스마트 포인터', 'STL 컨테이너(`std::vector`)'를 활용하여 로그를 추가하고 출력하는 C++ 스타일의 로그 시스템을 구현합니다.

```cpp
#include <iostream>   // 표준 입출력 (std::cout) 사용
#include <vector>     // 동적 배열 (std::vector) 사용
#include <string>     // 문자열 (std::string) 사용
#include <chrono>     // 시간 관련 기능 (std::chrono) 사용
#include <ctime>      // 시간을 문자열로 변환 (std::ctime) 사용
#include <memory>     // 스마트 포인터 (std::unique_ptr) 사용 (main 함수에서 활용)

// 로그 엔트리 클래스 정의
class LogEntry {
private:
    std::string message_; // 로그 메시지
    // 'std::chrono::time_point'는 C++11부터 표준화된 시간 표현 방식으로, 정밀하고 유연합니다.
    std::chrono::time_point<std::chrono::system_clock> timestamp_; 

public:
    // 생성자: LogEntry 객체 생성 시 메시지와 현재 시간을 설정합니다.
    LogEntry(const std::string& message) 
        : message_(message), timestamp_(std::chrono::system_clock::now()) {}
    
    // Getter 메서드: private 멤버 변수인 메시지에 접근할 수 있도록 합니다.
    const std::string& getMessage() const { return message_; }
    
    // Getter 메서드: 시간을 읽기 좋은 형식의 문자열로 변환하여 반환합니다.
    std::string getFormattedTime() const {
        // 'std::chrono::system_clock::to_time_t'는 time_point를 C 스타일의 time_t로 변환합니다.
        auto time_t = std::chrono::system_clock::to_time_t(timestamp_);
        // 'std::ctime'은 time_t 값을 사람이 읽을 수 있는 날짜/시간 문자열로 변환합니다.
        // 이 함수는 개행 문자('\n')를 포함하여 반환하므로 주의해야 합니다.
        std::string s = std::ctime(&time_t);
        // std::ctime이 반환하는 문자열 끝의 개행 문자를 제거합니다.
        if (!s.empty() && s.back() == '\n') {
            s.pop_back(); 
        }
        return s;
    }
};

// 간단한 로거 클래스 정의
class SimpleLogger {
private:
    // 'std::vector<LogEntry>'를 사용하여 여러 LogEntry 객체를 저장합니다.
    // std::vector는 동적 배열이므로 필요한 만큼 자동으로 크기가 조절됩니다.
    std::vector<LogEntry> logs_; 

public:
    // 로그를 추가하는 메서드
    void addLog(const std::string& message) {
        // 'emplace_back'은 벡터의 끝에 새로운 LogEntry 객체를 생성하고 추가합니다.
        // 이는 생성자를 직접 호출하여 객체를 만든 후 push_back하는 것보다 효율적일 수 있습니다.
        logs_.emplace_back(message); 
    }
    
    // 저장된 모든 로그를 출력하는 메서드
    void printLogs() const {
        std::cout << "\n--- Log Entries (C++) ---\n";
        // 'range-based for loop' (범위 기반 for 루프): C++11부터 도입된 편리한 반복문입니다.
        // 'logs_' 벡터의 각 'log' 요소에 대해 반복합니다.
        for (const auto& log : logs_) { // 'const auto&'는 각 요소를 읽기 전용 참조로 가져옵니다.
            std::cout << "Message: \"" << log.getMessage() << "\" (Time: " << log.getFormattedTime() << ")\n";
        }
        std::cout << "-------------------------\n";
    }
    
    // 현재 로그 개수를 반환하는 메서드
    size_t getLogCount() const { return logs_.size(); }
};

int main_cpp_example() {
    // SimpleLogger 객체를 스택(지역 변수)에 생성합니다.
    // 스택에 할당된 객체는 함수가 끝나면 자동으로 소멸됩니다.
    SimpleLogger logger;

    logger.addLog("C++ Server started.");
    logger.addLog("C++ Client connected from 10.0.0.5.");
    logger.addLog("C++ Data processing initiated.");

    logger.printLogs();

    std::cout << "Total logs: " << logger.getLogCount() << std::endl;

    // 추가적으로 스마트 포인터 사용 예시 (LogEntry 객체 하나에 대해)
    // std::unique_ptr는 LogEntry 객체를 동적으로 할당하고, main 함수 종료 시 자동으로 해제합니다.
    auto uniqueLog = std::make_unique<LogEntry>("Using unique_ptr for a single log entry.");
    std::cout << "\nUnique Log: \"" << uniqueLog->getMessage() << "\" (Time: " << uniqueLog->getFormattedTime() << ")\n";

    return 0;
}

// 실제 메인 함수 (둘 중 하나를 선택하거나, 둘 다 호출하려면 별도로 작성해야 함)
int main() {
    main_c_example();
    // main_cpp_example(); // C++ 예제를 실행하고 싶으면 이 주석을 해제하세요.
    return 0;
}
```

---

## 6. 고급 개념

### 함수 포인터와 콜백

LogCaster에서 다양한 로그 출력 방식이나 이벤트 처리를 위해 함수 포인터를 활용할 수 있습니다.

#### C에서의 함수 포인터

```c
#include <stdio.h>

// 로그 출력 함수들
void console_logger(const char* message) {
    printf("[CONSOLE] %s\n", message);
}

void file_logger(const char* message) {
    printf("[FILE] %s\n", message);  // 실제로는 파일에 기록
}

// 함수 포인터 타입 정의
typedef void (*LoggerFunction)(const char*);

// 로그 시스템 구조체
struct LogSystem {
    LoggerFunction logger;  // 함수 포인터
    char name[32];
};

// 로그 출력 함수
void log_with_system(struct LogSystem* system, const char* message) {
    printf("Using %s: ", system->name);
    system->logger(message);  // 함수 포인터를 통한 간접 호출
}

int main() {
    struct LogSystem console_system = {console_logger, "Console Logger"};
    struct LogSystem file_system = {file_logger, "File Logger"};
    
    log_with_system(&console_system, "Server started");
    log_with_system(&file_system, "Data saved");
    
    return 0;
}
```

#### C++에서의 함수 객체와 람다

```cpp
#include <iostream>
#include <functional>
#include <vector>

class Logger {
private:
    std::function<void(const std::string&)> output_func_;
    std::string name_;

public:
    Logger(const std::string& name, std::function<void(const std::string&)> func)
        : name_(name), output_func_(func) {}
    
    void log(const std::string& message) {
        std::cout << "[" << name_ << "] ";
        output_func_(message);
    }
};

int main() {
    // 람다 함수를 사용한 로거
    Logger console_logger("CONSOLE", [](const std::string& msg) {
        std::cout << msg << std::endl;
    });
    
    Logger file_logger("FILE", [](const std::string& msg) {
        // 실제로는 파일에 기록
        std::cout << "Writing to file: " << msg << std::endl;
    });
    
    console_logger.log("Application started");
    file_logger.log("Configuration loaded");
    
    return 0;
}
```

### 가변 인자 함수 (printf 스타일 로그)

로그 시스템에서 printf와 같은 형식 지정자를 사용하여 동적으로 메시지를 생성할 수 있습니다.

#### C에서의 가변 인자

```c
#include <stdio.h>
#include <stdarg.h>  // 가변 인자 처리를 위한 헤더
#include <time.h>

// printf 스타일의 로그 함수
void log_printf(const char* level, const char* format, ...) {
    // 현재 시간 출력
    time_t now = time(NULL);
    printf("[%ld][%s] ", now, level);
    
    // 가변 인자 처리
    va_list args;
    va_start(args, format);  // 가변 인자 시작
    vprintf(format, args);   // 가변 인자를 printf에 전달
    va_end(args);            // 가변 인자 종료
    
    printf("\n");
}

int main() {
    log_printf("INFO", "Server started on port %d", 8080);
    log_printf("ERROR", "Failed to connect to %s:%d", "192.168.1.1", 3306);
    log_printf("DEBUG", "User %s logged in with ID %d", "admin", 1001);
    
    return 0;
}
```

#### C++에서의 가변 템플릿 (C++11 이후)

```cpp
#include <iostream>
#include <sstream>
#include <string>

class ModernLogger {
public:
    // 가변 템플릿을 사용한 로그 함수
    template<typename... Args>
    void log(const std::string& level, const std::string& format, Args... args) {
        std::cout << "[" << level << "] ";
        print_formatted(format, args...);
        std::cout << std::endl;
    }

private:
    // 재귀적으로 인자를 처리하는 헬퍼 함수
    template<typename T>
    void print_formatted(const std::string& format, T&& t) {
        std::cout << format;
        replace_placeholder(std::forward<T>(t));
    }
    
    template<typename T, typename... Args>
    void print_formatted(const std::string& format, T&& t, Args&&... args) {
        size_t pos = format.find("{}");
        if (pos != std::string::npos) {
            std::cout << format.substr(0, pos);
            std::cout << std::forward<T>(t);
            print_formatted(format.substr(pos + 2), std::forward<Args>(args)...);
        }
    }
    
    template<typename T>
    void replace_placeholder(T&& value) {
        std::cout << std::forward<T>(value);
    }
};

int main() {
    ModernLogger logger;
    logger.log("INFO", "Server started on port {}", 8080);
    logger.log("ERROR", "Connection failed: {}:{}", "192.168.1.1", 3306);
    
    return 0;
}
```

### 에러 처리 패턴

#### C에서의 에러 처리

```c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// 에러 코드 정의
typedef enum {
    LOG_SUCCESS = 0,
    LOG_ERROR_FILE_OPEN = 1,
    LOG_ERROR_MEMORY = 2,
    LOG_ERROR_INVALID_PARAM = 3
} LogResult;

// 에러 메시지 배열
const char* log_error_messages[] = {
    "Success",
    "Failed to open log file",
    "Memory allocation failed", 
    "Invalid parameter"
};

// 에러 처리를 포함한 로그 파일 열기 함수
LogResult open_log_file(const char* filename, FILE** file) {
    if (filename == NULL || file == NULL) {
        return LOG_ERROR_INVALID_PARAM;
    }
    
    *file = fopen(filename, "a");
    if (*file == NULL) {
        // errno를 사용한 시스템 에러 정보 출력
        fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
        return LOG_ERROR_FILE_OPEN;
    }
    
    return LOG_SUCCESS;
}

// 안전한 로그 기록 함수
LogResult write_log_safe(const char* filename, const char* message) {
    FILE* log_file = NULL;
    LogResult result = open_log_file(filename, &log_file);
    
    if (result != LOG_SUCCESS) {
        fprintf(stderr, "Log error: %s\n", log_error_messages[result]);
        return result;
    }
    
    // 파일에 로그 기록
    if (fprintf(log_file, "%s\n", message) < 0) {
        fclose(log_file);
        return LOG_ERROR_FILE_OPEN;
    }
    
    fclose(log_file);
    return LOG_SUCCESS;
}

int main() {
    LogResult result = write_log_safe("app.log", "Application started successfully");
    
    if (result != LOG_SUCCESS) {
        printf("Failed to write log: %s\n", log_error_messages[result]);
        return 1;
    }
    
    printf("Log written successfully\n");
    return 0;
}
```

#### C++에서의 예외 처리

```cpp
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>

// 커스텀 예외 클래스
class LogException : public std::runtime_error {
public:
    LogException(const std::string& message) : std::runtime_error(message) {}
};

class FileLogger {
private:
    std::string filename_;

public:
    FileLogger(const std::string& filename) : filename_(filename) {}
    
    void writeLog(const std::string& message) {
        try {
            std::ofstream file(filename_, std::ios::app);
            
            if (!file.is_open()) {
                throw LogException("Failed to open log file: " + filename_);
            }
            
            file << message << std::endl;
            
            if (file.fail()) {
                throw LogException("Failed to write to log file: " + filename_);
            }
            
        } catch (const std::ios_base::failure& e) {
            throw LogException("I/O error: " + std::string(e.what()));
        }
    }
};

// RAII 패턴을 사용한 자원 관리
class RAIIFileLogger {
private:
    std::ofstream file_;

public:
    RAIIFileLogger(const std::string& filename) : file_(filename, std::ios::app) {
        if (!file_.is_open()) {
            throw LogException("Cannot open file: " + filename);
        }
    }
    
    // 소멸자에서 자동으로 파일 닫기 (RAII)
    ~RAIIFileLogger() {
        if (file_.is_open()) {
            file_.close();
        }
    }
    
    void log(const std::string& message) {
        file_ << message << std::endl;
        if (file_.fail()) {
            throw LogException("Write operation failed");
        }
    }
};

int main() {
    try {
        FileLogger logger("app.log");
        logger.writeLog("Application started");
        
        // RAII 패턴 사용
        {
            RAIIFileLogger raii_logger("detailed.log");
            raii_logger.log("Detailed logging started");
            raii_logger.log("Processing data...");
            // 블록을 벗어나면 자동으로 파일이 닫힘
        }
        
        std::cout << "Logging completed successfully" << std::endl;
        
    } catch (const LogException& e) {
        std::cerr << "Log error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

---

## 7. 다음 단계 준비

이 문서를 완전히 이해했다면, LogCaster 프로젝트를 위한 기본적인 지식을 갖춘 것입니다. 다음 사항들을 스스로 설명하거나 적용할 수 있는지 확인해보세요.

1. **C와 C++의 기본 차이점**을 설명할 수 있어야 합니다. (예: 패러다임, 메모리 관리 방식 등)
2. **Makefile과 CMake**의 기본 사용법 (빌드, 실행, 정리)을 알아야 합니다.
3. **기본 문법** (변수, 함수, 구조체/클래스)을 활용하여 간단한 프로그램을 작성하고 이해할 수 있어야 합니다.
4. **C의 구조체/ C++의 클래스**가 각각 어떻게 데이터를 묶고 관리하는지 주요 차이점(예: 메서드, 접근 제어자)을 이해해야 합니다.

### 확인 문제

1. C에서 `malloc()`으로 할당한 메모리를 명시적으로 해제하지 않으면 어떤 문제가 발생할까요?
2. C++에서 클래스의 **생성자**와 **소멸자**는 언제 호출될까요?
3. C++의 `std::vector`와 C 언어의 일반 배열의 차이점은 무엇일까요?

---

## 🚨 일반적인 실수와 해결방법

초보자들이 자주 마주치는 문제들과 해결책을 알아봅시다. 이런 실수들은 누구나 하는 것이니 당황하지 마세요!

### 1. 헤더 파일 중복 포함 - "같은 책을 두 번 읽는 문제"

**🤔 문제 상황:**
여러 파일에서 같은 헤더를 포함할 때 발생하는 문제입니다.

```c
// ❌ 잘못된 예: 헤더 가드 없음
// myheader.h
struct LogEntry {
    char message[256];
    int timestamp;
};

// main.c
#include "myheader.h"
#include "myheader.h"  // 중복 포함으로 에러 발생!

int main() {
    struct LogEntry log;  // 에러: LogEntry가 중복 정의됨!
    return 0;
}
```

**📝 컴파일러 에러 메시지:**
```
error: redefinition of 'struct LogEntry'
error: previous definition of 'struct LogEntry' was here
```

**✅ 해결책 1: 헤더 가드 사용**
```c
// myheader.h
#ifndef MYHEADER_H    // "MYHEADER_H가 정의되지 않았다면"
#define MYHEADER_H    // "MYHEADER_H를 정의하고"

struct LogEntry {
    char message[256];
    int timestamp;
};

#endif // MYHEADER_H  // "이 부분을 포함시켜라"
```

**✅ 해결책 2: #pragma once 사용 (현대적 방법)**
```c
// myheader.h
#pragma once  // 이 파일을 한 번만 포함하라고 컴파일러에게 지시

struct LogEntry {
    char message[256];
    int timestamp;
};
```

**🔍 왜 이런 일이 일어날까요?**
전처리기가 `#include`를 만나면 해당 파일의 내용을 **그대로 붙여넣기** 때문입니다. 같은 구조체가 두 번 정의되면 컴파일러가 혼란스러워해요!

### 2. 문자열 버퍼 오버플로우 - "작은 컵에 물을 너무 많이 따르는 문제"

**🤔 문제 상황:**
작은 공간에 큰 데이터를 억지로 넣으려고 할 때 발생합니다.

```c
// ❌ 위험한 예: strcpy 사용
char buffer[10];  // 10글자만 들어갈 수 있는 공간
strcpy(buffer, "This is a very long string");  // 27글자를 넣으려고 시도!

printf("Buffer: %s\n", buffer);  // 예측 불가능한 결과!
```

**😱 뭐가 문제일까요?**
```
buffer[10] = ['T','h','i','s',' ','i','s',' ','a',' ']  // 여기까지는 OK
               0   1   2   3   4   5   6   7   8   9     // 인덱스

하지만 실제로는...
buffer: ['T','h','i','s',' ','i','s',' ','a',' ','v','e','r','y',' ','l','o','n','g',' ','s','t','r','i','n','g','\0']
         0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27

                                              ↑ 여기부터는 다른 메모리 영역을 침범!
```

**💥 가능한 결과:**
- 프로그램 크래시
- 다른 변수의 값이 망가짐
- 보안 취약점 발생

**✅ 해결책 1: strncpy 사용**
```c
char buffer[10];
strncpy(buffer, "This is a very long string", sizeof(buffer) - 1);  // 최대 9글자만 복사
buffer[sizeof(buffer) - 1] = '\0';  // 마지막에 반드시 널 문자 추가

printf("Buffer: %s\n", buffer);  // 출력: "This is a"
```

**✅ 해결책 2: snprintf 사용 (더 안전)**
```c
char buffer[10];
snprintf(buffer, sizeof(buffer), "%s", "This is a very long string");

printf("Buffer: %s\n", buffer);  // 출력: "This is a"
```

**✅ 해결책 3: 동적 메모리 할당**
```c
const char* source = "This is a very long string";
char* buffer = malloc(strlen(source) + 1);  // 필요한 만큼 메모리 할당
if (buffer != NULL) {
    strcpy(buffer, source);  // 이제 strcpy 사용 가능
    printf("Buffer: %s\n", buffer);
    free(buffer);  // 사용 후 메모리 해제
}
```

### 3. 초기화되지 않은 변수 사용 - "빈 상자를 열어보는 문제"

**🤔 문제 상황:**
변수에 값을 넣지 않고 사용하려고 할 때 발생합니다.

```c
// ❌ 잘못된 예
int count;  // 값을 지정하지 않음
printf("Count: %d\n", count);  // 뭐가 나올까요?
```

**😱 결과:**
```
Count: 32767
Count: -1
Count: 0
Count: 1847592

... 실행할 때마다 다른 값이 나옴!
```

**🔍 왜 이런 일이 일어날까요?**
C/C++에서 지역 변수는 **자동으로 초기화되지 않습니다**. 메모리에 이전에 있던 값(쓰레기값)이 그대로 남아있어요!

```c
// 메모리 상황 예시
이전에 사용된 메모리: [42] [0] [98765] [12]
                        ↑
                    count 변수가 
                   여기에 할당됨
```

**✅ 해결책: 항상 초기화하기**
```c
int count = 0;           // 명시적 초기화
printf("Count: %d\n", count);  // 출력: Count: 0

// 다른 초기화 방법들
int port = 8080;         // 특정 값으로 초기화
char buffer[256] = {0};  // 배열을 0으로 초기화
char name[50] = "";      // 빈 문자열로 초기화
```

**💡 초보자 팁:**
```c
// 좋은 습관: 선언과 동시에 초기화
int log_count = 0;
float temperature = 0.0f;
char message[100] = "";

// 나쁜 습관: 나중에 초기화
int log_count;      // 쓰레기값
// ... 코드 100줄 ...
log_count = 0;      // 너무 늦었어요!
```

### 4. C++에서 new/delete와 malloc/free 혼용 - "서로 다른 언어 섞어쓰기"

**🤔 문제 상황:**
C와 C++의 메모리 관리 방식을 섞어서 사용할 때 발생합니다.

```cpp
// ❌ 잘못된 예: 할당과 해제 방식이 다름
int* arr = new int[10];  // C++ 방식으로 할당
free(arr);              // C 방식으로 해제 - 위험!

char* str = (char*)malloc(100);  // C 방식으로 할당
delete[] str;                    // C++ 방식으로 해제 - 위험!
```

**💥 왜 문제가 될까요?**
1. **내부 구현이 달라요**: `new`는 생성자를 호출하지만 `malloc`은 호출하지 않아요
2. **메모리 관리 방식이 달라요**: 각각 다른 힙(heap) 영역을 사용할 수 있어요
3. **예측 불가능한 동작**: 크래시, 메모리 손상, 또는 조용한 버그 발생

**✅ 올바른 사용법:**

**C 스타일 (malloc/free):**
```c
// C에서 메모리 관리
int* arr = (int*)malloc(10 * sizeof(int));  // 할당
if (arr != NULL) {
    // 배열 사용
    for (int i = 0; i < 10; i++) {
        arr[i] = i;
    }
    free(arr);  // 해제
    arr = NULL; // 중요: 포인터를 NULL로 설정
}
```

**C++ 스타일 (new/delete):**
```cpp
// C++에서 메모리 관리
int* arr = new int[10];  // 할당
// 배열 사용
for (int i = 0; i < 10; i++) {
    arr[i] = i;
}
delete[] arr;  // 배열은 delete[]로 해제
arr = nullptr; // 포인터를 nullptr로 설정
```

**🌟 더 나은 방법: 스마트 포인터 사용 (C++11 이후)**
```cpp
#include <memory>

// 자동으로 메모리 관리 - 추천!
auto arr = std::make_unique<int[]>(10);  // 할당
// 배열 사용
for (int i = 0; i < 10; i++) {
    arr[i] = i;
}
// 자동으로 메모리 해제됨 (delete[] 자동 호출)

// 단일 객체의 경우
auto number = std::make_unique<int>(42);
std::cout << *number << std::endl;  // 출력: 42
// 자동으로 메모리 해제됨
```

**📝 기억하기 쉬운 규칙:**
- `malloc` ↔ `free`
- `new` ↔ `delete`
- `new[]` ↔ `delete[]`
- **섞지 마세요!** 🚫

---

## 💪 연습 문제

### 1. 간단한 로그 구조체 만들기 (C)
다음 요구사항을 만족하는 C 프로그램을 작성하세요:
- 로그 레벨(INFO, WARNING, ERROR)을 가진 구조체 정의
- 로그를 출력하는 함수 구현
- 동적 메모리 할당을 사용하여 로그 배열 관리

```c
// 힌트: 구조 예시
typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

typedef struct {
    LogLevel level;
    char message[256];
    time_t timestamp;
} Log;

// 구현해야 할 함수들:
// Log* create_log(LogLevel level, const char* message);
// void print_log(const Log* log);
// void free_log(Log* log);
```

### 2. 로그 클래스 구현 (C++)
C++로 다음 기능을 가진 Logger 클래스를 구현하세요:
- 로그 레벨별 메시지 출력
- 파일과 콘솔 동시 출력 옵션
- 싱글톤 패턴 적용

```cpp
// 사용 예시:
Logger& logger = Logger::getInstance();
logger.setOutput(Logger::CONSOLE | Logger::FILE);
logger.info("Server started");
logger.error("Connection failed");
```

### 3. Makefile 작성하기
위에서 작성한 C 프로그램을 위한 Makefile을 작성하세요:
- 컴파일 규칙
- clean 타겟
- 디버그/릴리즈 모드 지원

---

## ✅ 학습 체크리스트

### 기본 개념
- [ ] C와 C++의 주요 차이점을 3가지 이상 설명할 수 있다
- [ ] 컴파일 과정의 4단계를 순서대로 나열할 수 있다
- [ ] 헤더 파일의 역할과 사용법을 이해한다
- [ ] 전처리기 지시문(#include, #define 등)의 동작을 이해한다

### 빌드 시스템
- [ ] Makefile의 기본 구조를 이해한다
- [ ] make 명령어로 프로젝트를 빌드할 수 있다
- [ ] CMakeLists.txt의 기본 구조를 이해한다
- [ ] 의존성 관계를 이해하고 설명할 수 있다

### 문법과 기능
- [ ] C의 구조체와 C++의 클래스 차이를 이해한다
- [ ] 포인터 기본 문법을 사용할 수 있다
- [ ] 동적 메모리 할당의 필요성을 이해한다
- [ ] 예외 처리의 기본 개념을 이해한다

### 실습 능력
- [ ] 간단한 C 프로그램을 작성하고 컴파일할 수 있다
- [ ] 간단한 C++ 프로그램을 작성하고 컴파일할 수 있다
- [ ] 컴파일 에러 메시지를 읽고 해결할 수 있다
- [ ] 기본적인 디버깅을 수행할 수 있다

다음 문서에서는 C/C++에서 가장 중요하고 심도 있는 개념 중 하나인 **메모리 관리**에 대해 자세히 알아보겠습니다!

---

## 🔄 다음 학습 단계

이 문서를 완료했다면 다음 문서로 진행하세요:

1. **[01_Memory.md](01.%20Memory.md)** - 메모리 관리의 심화 학습
   - 포인터와 참조의 차이
   - 동적 메모리 할당
   - 메모리 누수 방지

2. **[04. Project_Setup_Guide.md](04.%20Project_Setup_Guide.md)** - 실제 프로젝트 환경 구축
   - 개발 도구 설치
   - 프로젝트 구조 설정
   - 첫 번째 프로그램 실행

3. **[02_DataStructures.md](02.%20DataStructures.md)** - 자료구조 이해
   - 배열과 연결 리스트
   - 스택과 큐
   - LogCaster에서 사용할 자료구조

---

*💡 팁: 이 내용들은 LogCaster 프로젝트의 핵심 기반이 됩니다. 충분히 연습해보세요!*