*LogCaster 프로젝트를 위한 사전지식 문서*

---

## 📋 문서 정보
- **난이도**: 🟢 초급~🟡 중급
- **예상 학습 시간**: 4-5시간
- **전제 조건**: C 기본 문법 (변수, 함수, 포인터 개념)
- **실습 환경**: GCC/Clang 컴파일러, Valgrind (선택사항)

## 🎯 이 문서에서 배울 내용

**"메모리? 그게 뭔가요? 컴퓨터 용량이랑 다른 건가요?"**

좋은 질문입니다! 많은 분들이 헷갈려하시는 부분이에요. 이 문서에서는 메모리가 무엇인지부터 차근차근 설명드리겠습니다.

🧠 **메모리를 쉽게 이해해보기:**
- **메모리(RAM)** = 책상 (작업할 때 필요한 물건들을 올려두는 공간)
- **저장장치(하드디스크)** = 서랍장 (오래 보관할 물건들을 넣어두는 공간)

프로그램이 실행될 때는 하드디스크(서랍장)에서 필요한 데이터를 꺼내서 메모리(책상) 위에 올려놓고 작업합니다.

🎓 **이 문서에서 배울 것들:**
- 메모리가 어떻게 생겼고, 프로그램이 어떻게 사용하는지 📚
- C언어의 "직접 관리" vs C++의 "자동 관리" 방식 비교 🆚  
- 메모리 누수(Memory Leak) 같은 흔한 실수들과 해결법 🐛
- 대용량 데이터를 효율적으로 다루는 고급 기술들 🚀

💡 **친절한 설명 방식:**
- 어려운 개념은 일상생활 예시로 설명합니다
- "왜 이렇게 해야 하는지" 이유를 항상 설명합니다  
- 자주 하는 실수들을 미리 알려드립니다
- 실제 LogCaster 프로젝트 예시로 실무 감각을 기릅니다

---

## 1. 메모리 구조 이해

### 🏠 "메모리는 체계적으로 정리된 아파트 같아요!"

컴퓨터 메모리를 **아파트**에 비유해보겠습니다:

🏢 **메모리 = 아파트 건물**
- 각 방마다 고유한 **호수(주소)**가 있습니다 (예: 101호, 102호...)
- 프로그램은 이 **주소**를 사용해서 정확한 방을 찾아갑니다
- 각 방에는 **데이터**(숫자, 글자, 객체 등)가 살고 있습니다

🔍 **예시로 이해해보기:**
```
🏠 메모리 주소 0x1000: "Hello" (문자열이 살고 있는 방)
🏠 메모리 주소 0x1004: 42 (숫자가 살고 있는 방)  
🏠 메모리 주소 0x1008: true (불린값이 살고 있는 방)
```

프로그램이 "Hello"를 찾으려면? → "주소 0x1000번 방을 확인해줘!"라고 말합니다.

### 🏢 "메모리 아파트는 4개 층으로 나뉘어져 있어요!"

우리 프로그램이 사는 메모리 아파트는 **4개 층**으로 구성되어 있습니다:

```
🏢 메모리 아파트 구조

🔝 4층: Stack (스택) ← 높은 주소  
   📦 임시 작업실 (지역변수, 함수 매개변수)
   ⚡ 빠르고 자동 관리, 하지만 공간이 좁음
   
🌬️ 3층: 여유 공간 (Spacer)
   🛗 스택과 힙 사이의 안전 구역
   
🏗️ 2층: Heap (힙)  
   🗂️ 자유 창고 (동적 할당 메모리)
   🔧 수동 관리, 넓지만 느림
   
📊 1층: Data (데이터)
   🏪 상점 (전역변수, 정적변수)
   
🏛️ 지하: Text (텍스트)  ← 낮은 주소
   📜 도서관 (실행 가능한 프로그램 코드)
```

### 🎯 "가장 중요한 두 층: 스택 vs 힙"

개발자가 **직접 관리**해야 하는 가장 중요한 두 층이 있습니다:
- **4층 스택(Stack)**: 빠르고 자동이지만 좁은 임시 작업실 
- **2층 힙(Heap)**: 넓고 자유롭지만 수동 관리가 필요한 창고

### 📦 스택 메모리: "정리정돈 끝판왕의 임시 작업실"

스택을 **책상 위에 책을 쌓는 것**에 비유해보겠습니다.

📚 **스택 = 책 더미**
- 새 책은 항상 **맨 위**에만 올릴 수 있어요
- 책을 가져갈 때도 **맨 위**에서만 가져갈 수 있어요  
- 이걸 **LIFO**(Last In, First Out: 마지막에 들어간 게 먼저 나옴)라고 해요

🔍 **스택은 언제 사용되나요?**

**함수 호출 = 새 방 만들기**

스택은 함수가 호출될 때마다 **새로운 작업 공간**을 만들어줍니다. 마치 도서관에서 책을 읽을 때마다 새 책상을 배정받는 것과 같아요!

**실생활 예시로 이해하기:**
```
📱 카톡으로 대화하는 상황
1. 엄마: "저녁 뭐 먹을까?" (main 함수 실행)
2. 나: "치킨!" (함수 A 호출 - 새 스택 프레임 생성)
3. 엄마: "어디 치킨?" (함수 B 호출 - 또 다른 스택 프레임 생성)
4. 나: "BBQ!" (함수 B 종료 - 스택 프레임 삭제)
5. 엄마: "주문할게!" (함수 A 종료 - 스택 프레임 삭제)
6. 대화 끝 (main 함수 종료)
```

**코드 예시:**
```c
void cook_breakfast() {    // 🏠 새 방(스택 프레임) 생성!
    int eggs = 2;          // 🥚 eggs 변수가 이 방에 생김
    char toast[10];        // 🍞 toast 배열도 이 방에 생김
    
    make_eggs(eggs);       // 🏠 또 다른 새 방 생성!
}                          // 🧹 함수 끝! 방 자동 정리됨
```

✨ **스택의 마법 같은 특징들:**

**1. 🤖 완전 자동 관리**
- 함수 시작 → 방 생성 
- 함수 끝 → 방 자동 정리
- 개발자는 신경 쓸 필요 없음!

**2. ⚡ 번개처럼 빠름**
- 단순히 "쌓기/빼기"만 하면 됨
- 복잡한 검색이나 정리가 필요 없음

**3. ⚠️ 공간 제한**  
- 책상이 작아서 너무 많이 쌓으면 무너짐
- **스택 오버플로우** 에러 발생!

**4. 📍 지역성**
- 각 함수의 변수들은 그 함수 안에서만 유효

**예시 시나리오:** 클라이언트의 요청을 처리하는 간단한 함수
```c
// 클라이언트 연결을 처리하는 함수
void handle_client_connection() {
    // 'message_buffer'는 handle_client_connection 함수가
    // 실행되는 동안에만 스택 메모리에 존재합니다.
    char message_buffer[256]; 
    
    // 'client_fd' 역시 이 함수 안에서만 유효한 지역 변수입니다.
    int client_fd;             

    // ... 클라이언트와 통신하는 코드 ...

} // <--- 함수가 여기서 끝나면, 'message_buffer'와 'client_fd'는
  //      마치 마법처럼 자동으로 메모리에서 사라집니다!
```

**활용성**: 이처럼 스택은 크기가 작고 수명이 짧은 데이터를 임시로 다룰 때 매우 유용하며, 프로그램의 안정성과 속도를 높여줍니다.

### 🏗️ 힙 메모리: "자유로운 대형 창고"

힙을 **대형 창고 대여업체**에 비유해보겠습니다.

🏭 **힙 = 대형 창고 대여업체**
- 필요한 만큼 공간을 빌릴 수 있어요 (작은 상자방부터 거대한 창고까지!)
- 언제든지 빌리고 반납할 수 있어요
- 하지만 **직접 관리**해야 해요 (빌렸으면 반드시 반납!)

🤔 **힙은 언제 사용하나요?**

**📏 크기를 미리 모를 때:**
```c
// 사용자가 몇 명인지 모르겠어요...
int user_count = get_user_input(); // 100명? 1000명?
User* users = malloc(sizeof(User) * user_count); // 필요한 만큼만 빌려요!
```

**🔄 함수 끝나도 살아있어야 할 때:**
```c
char* create_message() {
    char* msg = malloc(100);          // 힙에서 빌림
    strcpy(msg, "Important Data");
    return msg;                       // 함수 끝나도 msg는 살아있음!
}   // 스택 변수라면 여기서 사라졌을 거예요
```

**🏋️ 너무 큰 데이터일 때:**
```c
// 1MB 배열 - 스택에서는 터질 수도!
int* big_array = malloc(1000000 * sizeof(int)); // 힙에서 안전하게 빌림
```

⚡ **힙의 특징들:**

**1. 🔧 수동 관리 (빌렸으면 반납!)**
```c
int* data = malloc(100);    // 🏗️ 빌리기
// ... 사용 ...
free(data);                 // 🚛 반납하기 (필수!)
```

**2. 📏 자유로운 크기**
- 1바이트부터 기가바이트까지 자유자재로!

**3. 🐌 상대적으로 느림**
- 창고 관리자가 빈 공간을 찾아야 해서 시간이 걸려요

**4. 🧩 단편화 문제**
- 빌리고 반납하다 보면 자투리 공간들이 생겨요
- 충분한 총 공간이 있어도 큰 덩어리를 못 빌릴 수 있어요

**🎯 실전 예제: 로그 시스템에서 힙 사용하기**

LogCaster에서 실제로 힙 메모리를 어떻게 사용하는지 봅시다:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 로그 엔트리 구조체
typedef struct LogEntry {
    char* message;     // 메시지 길이가 다양하므로 힙에 저장
    time_t timestamp;  // 시간 정보
    int level;         // 로그 레벨 (INFO=1, WARNING=2, ERROR=3)
} LogEntry;

// 동적으로 로그 엔트리 생성
LogEntry* create_log_entry(const char* msg, int level) {
    // 1단계: LogEntry 구조체용 메모리 할당
    LogEntry* entry = malloc(sizeof(LogEntry));
    if (entry == NULL) {
        printf("메모리 할당 실패!\n");
        return NULL;
    }
    
    // 2단계: 메시지용 메모리 할당 (메시지 길이만큼만!)
    entry->message = malloc(strlen(msg) + 1);  // +1은 null terminator용
    if (entry->message == NULL) {
        free(entry);  // 실패시 이미 할당한 메모리 해제
        printf("메시지 메모리 할당 실패!\n");
        return NULL;
    }
    
    // 3단계: 데이터 설정
    strcpy(entry->message, msg);
    entry->timestamp = time(NULL);
    entry->level = level;
    
    printf("✅ 로그 생성: '%s' (힙 메모리 사용)\n", msg);
    return entry;
}

// 메모리 해제 함수
void destroy_log_entry(LogEntry* entry) {
    if (entry != NULL) {
        if (entry->message != NULL) {
            free(entry->message);    // 1단계: 메시지 메모리 해제
            entry->message = NULL;   // 안전을 위해 NULL로 설정
        }
        free(entry);                 // 2단계: 구조체 메모리 해제
        printf("🗑️  로그 메모리 해제됨\n");
    }
}

// 사용 예시
int main() {
    // 동적으로 로그 생성 (크기가 다른 메시지들)
    LogEntry* log1 = create_log_entry("Server started", 1);
    LogEntry* log2 = create_log_entry("This is a very long error message that would waste space in a fixed-size buffer", 3);
    LogEntry* log3 = create_log_entry("OK", 1);
    
    // 로그 출력
    printf("\n📋 생성된 로그들:\n");
    if (log1) printf("Log 1: %s\n", log1->message);
    if (log2) printf("Log 2: %s\n", log2->message);
    if (log3) printf("Log 3: %s\n", log3->message);
    
    // 메모리 해제 (중요!)
    printf("\n🧹 메모리 정리:\n");
    destroy_log_entry(log1);
    destroy_log_entry(log2);
    destroy_log_entry(log3);
    
    return 0;
}
```

**🔍 위 예제에서 힙을 사용하는 이유:**
1. **다양한 크기**: 로그 메시지가 "OK" 2글자부터 매우 긴 에러 메시지까지 다양해요
2. **동적 생성**: 프로그램 실행 중에 필요에 따라 로그를 만들어야 해요
3. **장기 보관**: 로그를 다른 함수에 전달하거나 나중에 처리해야 할 수도 있어요

**💡 스택 vs 힙 비교 정리:**

| 특징 | 스택 (Stack) | 힙 (Heap) |
|------|-------------|-----------|
| **속도** | ⚡ 매우 빠름 | 🐌 상대적으로 느림 |
| **크기** | 🔒 제한적 (보통 1-8MB) | 📏 큰 공간 (RAM 허용범위) |
| **관리** | 🤖 자동 (컴파일러가 처리) | 🔧 수동 (개발자가 직접) |
| **사용법** | `int x = 10;` | `int* x = malloc(sizeof(int));` |
| **해제** | 🔄 자동 (함수 종료시) | 🧹 수동 (`free()` 필요) |
| **용도** | 임시 데이터, 지역 변수 | 큰 데이터, 동적 크기, 장기 보관 |

**🚨 힙 사용시 주의사항:**

**1. 메모리 누수 (Memory Leak) 방지:**
```c
// ❌ 나쁜 예: 메모리 누수!
void bad_function() {
    char* data = malloc(1000);
    // ... 작업 ...
    return;  // free()를 안 했어요! 메모리 누수 발생!
}

// ✅ 좋은 예: 항상 해제!
void good_function() {
    char* data = malloc(1000);
    if (data == NULL) return;  // 할당 실패 체크
    
    // ... 작업 ...
    
    free(data);  // 반드시 해제!
    data = NULL; // 안전을 위해 NULL로 설정
}
```

**2. 이중 해제 (Double Free) 방지:**
```c
// ❌ 위험한 예: 같은 메모리를 두 번 해제!
char* data = malloc(100);
free(data);
free(data);  // 💥 크래시 발생!

// ✅ 안전한 예: 해제 후 NULL로 설정
char* data = malloc(100);
free(data);
data = NULL;  // 이제 안전해요
if (data != NULL) {  // 이미 NULL이므로 실행 안됨
    free(data);
}
```

---

## 2. C의 동적 메모리 할당: 직접 관리하는 메모리
C 언어에서는 동적 메모리 할당을 위해 `malloc()`, `calloc()`, `realloc()`, `free()`와 같은 함수를 사용합니다. 이 함수들은 힙 메모리에서 필요한 크기만큼 메모리를 할당하고, 사용이 끝난 후에는 반드시 해제해야 합니다.

### malloc() 함수 패밀리
동적 메모리 할당을 위한 함수들입니다. 이 함수들은 힙 영역에서 메모리를 할당하고, 할당된 메모리의 시작 주소를 반환합니다. 만약 메모리 할당에 실패하면 `NULL`을 반환합니다.

**기본 함수들:**
```c
#include <stdlib.h>

// 메모리 할당
void* malloc(size_t size);              // 지정된 크기만큼 할당
void* calloc(size_t num, size_t size);  // 0으로 초기화된 메모리 할당
void* realloc(void* ptr, size_t size);  // 기존 메모리 크기 변경

// 메모리 해제
void free(void* ptr);                   // 할당된 메모리 해제
```

### LogCaster-C에서의 동적 메모리 사용

로그 메시지는 언제, 얼마나 길게 들어올지 예측할 수 없으므로 힙 메모리를 사용하는 것이 이상적입니다.

**1. 로그 데이터 동적 생성:**
```c
#include <stdio.h>
#include <stdlib.h> // malloc, free를 위해 필요
#include <string.h> // strcpy, strlen을 위해 필요
#include <time.h>   // time을 위해 필요

// 로그 하나를 표현하는 데이터 구조(struct)
typedef struct LogEntry {
    char* message;           // 로그 메시지 (주소를 저장할 포인터)
    // ... 기타 정보 ...
} LogEntry;

// 로그를 생성하고 힙 메모리에 할당하는 함수
LogEntry* create_log_entry(const char* message) {
    // 1. LogEntry 구조체를 담을 공간을 힙에서 빌린다.
    LogEntry* entry = malloc(sizeof(LogEntry));
    // ★ 중요: 혹시 메모리가 부족해 빌리지 못했다면 NULL을 반환한다.
    if (entry == NULL) {
        return NULL; // 항상 실패 가능성을 염두에 둬야 합니다.
    }
    
    // 2. 메시지 문자열을 담을 공간을 별도로 힙에서 빌린다.
    //    (+1은 문자열의 끝을 알리는 NULL 문자를 위함)
    size_t message_len = strlen(message) + 1;
    entry->message = malloc(message_len);
    if (entry->message == NULL) {
        // 메시지 공간 빌리기에 실패했다면,
        // 이전에 빌린 구조체 공간(entry)이라도 반납해야 한다!
        free(entry); 
        return NULL;
    }
    
    // 3. 빌린 공간에 실제 데이터를 복사한다.
    strcpy(entry->message, message);
    
    // 4. 완성된 로그 데이터의 주소를 반환한다.
    return entry;
}

// 힙에 생성된 로그 데이터를 반납(해제)하는 함수
void destroy_log_entry(LogEntry* entry) {
    if (entry != NULL) {
        // ★ 중요: 빌린 순서의 역순으로, 내부부터 반납한다.
        // 2번에서 빌렸던 '메시지 공간'을 먼저 반납
        free(entry->message);
        // 1번에서 빌렸던 '구조체 공간'을 나중에 반납
        free(entry);
    }
}
```

**2. 동적 버퍼 관리:**
```c
// 동적으로 크기가 변하는 버퍼
typedef struct DynamicBuffer {
    char* data;
    size_t size;
    size_t capacity;
} DynamicBuffer;

// 버퍼 초기화
DynamicBuffer* buffer_create(size_t initial_capacity) {
    DynamicBuffer* buffer = malloc(sizeof(DynamicBuffer));
    if (buffer == NULL) return NULL;
    
    buffer->data = malloc(initial_capacity);
    if (buffer->data == NULL) {
        free(buffer);
        return NULL;
    }
    
    buffer->size = 0;
    buffer->capacity = initial_capacity;
    return buffer;
}

// 버퍼 크기 확장
int buffer_resize(DynamicBuffer* buffer, size_t new_capacity) {
    if (new_capacity <= buffer->capacity) {
        return 1;  // 이미 충분한 크기
    }
    
    char* new_data = realloc(buffer->data, new_capacity);
    if (new_data == NULL) {
        return 0;  // 메모리 할당 실패
    }
    
    buffer->data = new_data;
    buffer->capacity = new_capacity;
    return 1;
}

// 버퍼에 데이터 추가
int buffer_append(DynamicBuffer* buffer, const char* data, size_t length) {
    // 필요시 버퍼 크기 확장
    if (buffer->size + length > buffer->capacity) {
        size_t new_capacity = buffer->capacity * 2;
        if (new_capacity < buffer->size + length) {
            new_capacity = buffer->size + length;
        }
        
        if (!buffer_resize(buffer, new_capacity)) {
            return 0;  // 확장 실패
        }
    }
    
    // 데이터 복사
    memcpy(buffer->data + buffer->size, data, length);
    buffer->size += length;
    return 1;
}

// 버퍼 해제
void buffer_destroy(DynamicBuffer* buffer) {
    if (buffer != NULL) {
        free(buffer->data);
        free(buffer);
    }
}
```

### 메모리 할당 실패 처리

**안전한 메모리 할당 패턴:**
```c
// 메모리 할당 매크로 (에러 처리 포함)
#define SAFE_MALLOC(ptr, size) do { \
    ptr = malloc(size); \
    if (ptr == NULL) { \
        fprintf(stderr, "Memory allocation failed at %s:%d\n", __FILE__, __LINE__); \
        exit(EXIT_FAILURE); \
    } \
} while(0)

// 사용 예시
void example_safe_allocation() {
    char* buffer;
    SAFE_MALLOC(buffer, 1024);
    
    // 버퍼 사용...
    
    free(buffer);
}

// 더 복잡한 에러 처리
LogEntry* create_log_entry_safe(const char* message) {
    LogEntry* entry = NULL;
    char* message_copy = NULL;
    
    // 구조체 할당
    entry = malloc(sizeof(LogEntry));
    if (entry == NULL) {
        goto error;
    }
    
    // 메시지 할당
    size_t message_len = strlen(message) + 1;
    message_copy = malloc(message_len);
    if (message_copy == NULL) {
        goto error;
    }
    
    // 초기화
    entry->message = message_copy;
    entry->message_length = message_len;
    strcpy(entry->message, message);
    entry->timestamp = time(NULL);
    entry->next = NULL;
    
    return entry;

error:
    if (entry != NULL) free(entry);
    if (message_copy != NULL) free(message_copy);
    return NULL;
}
```

---

## 3. C++의 동적 메모리 할당: 스마트 포인터와 RAII

### 😰 "C언어 메모리 관리가 너무 어려워요!"

맞아요! C언어의 수동 관리는 강력하지만 **실수하기 쉬워요**:

```c
// 🚨 이런 실수들이 자주 발생해요
int* data = malloc(100);
// ... 작업 중 ...
if (error_occurred) {
    return -1;  // 😱 free()를 깜빡했어요! (메모리 누수)
}
free(data);
free(data);     // 😱 또 반납하려고 해요! (이중 해제)
```

### 🦸‍♂️ "C++의 구세주: RAII와 스마트 포인터!"

C++는 이런 문제를 해결하기 위해 **"자동 관리 시스템"**을 만들었습니다!

### 🤖 RAII: "자동 메모리 관리 로봇"

**RAII**를 **자동 청소 로봇**에 비유해보겠습니다!

🏠 **RAII = 똑똑한 청소 로봇**
- **로봇 켜짐**(객체 생성) → 자동으로 방 청소 시작 (자원 획득)
- **로봇 꺼짐**(객체 소멸) → 자동으로 모든 정리 완료 (자원 해제)
- **중간에 정전**되어도 → 로봇이 알아서 정리하고 꺼짐!

🔍 **실제 예시: 자동 파일 관리자**
```cpp
class SmartFileHandler {
    FILE* file;
public:
    // 🔛 생성자: 로봇 켜짐 = 파일 열기
    SmartFileHandler(const string& filename) {
        file = fopen(filename.c_str(), "w");
        cout << "📂 파일이 열렸습니다!" << endl;
    }
    
    // 🔚 소멸자: 로봇 꺼짐 = 파일 닫기  
    ~SmartFileHandler() {
        if (file) fclose(file);
        cout << "📄 파일이 자동으로 닫혔습니다!" << endl;
    }
};

void write_log() {
    SmartFileHandler logger("log.txt");  // 🔛 파일 자동 열림
    // 파일에 뭔가 쓰기...
    
    if (error) return;  // 🚨 에러 발생해도 걱정 없어요!
}   // 🔚 여기서 logger가 사라지면서 파일 자동 닫힘!
```

**🎉 RAII의 마법:**
- 개발자가 `fclose()`를 깜빡해도 OK!
- 중간에 에러가 나도 OK!
- 함수가 어디서 끝나든 OK!
- **100% 자동 정리 보장!**

**RAII 패턴 예시:**
```cpp
class FileHandler {
private:
    FILE* file_;

public:
    // 생성자: 자원 획득
    FileHandler(const std::string& filename) {
        file_ = fopen(filename.c_str(), "w");
        if (file_ == nullptr) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
    }
    
    // 소멸자: 자원 해제
    ~FileHandler() {
        if (file_ != nullptr) {
            fclose(file_);
        }
    }
    
    // 복사 방지 (Rule of Three/Five)
    FileHandler(const FileHandler&) = delete;
    FileHandler& operator=(const FileHandler&) = delete;
    
    // 파일 쓰기
    void write(const std::string& data) {
        if (file_ != nullptr) {
            fprintf(file_, "%s", data.c_str());
        }
    }
};

// 사용 예시
void write_log_to_file() {
    FileHandler log_file("server.log");  // 생성자에서 파일 열기
    log_file.write("Server started\n");
    // 함수 종료 시 소멸자에서 자동으로 파일 닫기
}
```

이제 개발자는 파일을 직접 닫는 것을 신경 쓸 필요가 없습니다. 심지어 코드 중간에 에러가 발생해도, C++가 책임지고 상자를 없애주므로 파일은 안전하게 닫힙니다.

### 🧠 스마트 포인터: "메모리 관리 비서들"

스마트 포인터를 **똑똑한 비서들**에 비유해보겠습니다! 각각 다른 관리 스타일을 가지고 있어요.

🎯 **스마트 포인터 = 메모리 관리 전문 비서**
- 메모리를 빌리고 반납하는 일을 **대신** 해줍니다
- 실수할 가능성을 **0%**로 만들어줍니다  
- 각각 다른 **특기**를 가지고 있어요!

#### 🔐 1. `unique_ptr` - "독점 관리 비서"

**🏠 unique_ptr = 독점 관리자**
- **"이 메모리는 내가 혼자 관리할게!"**
- 마치 **자동차 키**를 한 사람만 가질 수 있는 것과 같아요

🔍 **특징들:**
- **❌ 복사 불가**: "내 키는 복사할 수 없어!"
- **✅ 이동 가능**: "키를 다른 사람에게 건네줄 수는 있어!"  
- **⚡ 매우 빠름**: 가장 가볍고 효율적
- **🎯 가장 많이 사용**: 90% 상황에서 이걸 써요!

```cpp
// 🔐 unique_ptr 사용 예시
void manage_memory() {
    // 🏗️ 메모리 할당 + 자동 관리 시작
    auto smart_data = std::make_unique<int>(42);
    
    // 사용
    cout << *smart_data << endl;  // 42 출력
    
    // 🚚 자동 해제! free() 필요 없음!
}   // 여기서 smart_data 소멸 → 메모리 자동 반납
```

**🎮 실제 LogCaster 예시:**
```cpp
class LogBuffer {
    std::vector<std::unique_ptr<LogEntry>> logs;  // 로그들을 독점 관리
public:
    void addLog(const string& message) {
        // 🆕 새 로그 생성 + 독점 소유권 획득
        auto newLog = std::make_unique<LogEntry>(message);
        logs.push_back(std::move(newLog));  // 🚚 소유권 이동
    }
    // 🧹 LogBuffer가 사라지면 모든 로그들도 자동 정리!
};
```

**활용 예시**: 로그 버퍼에 로그를 추가할 때

```cpp
#include <memory> // 스마트 포인터를 위해 필요
#include <vector>

// LogEntry 클래스는 C의 struct와 유사하지만, 기능(메서드)을 가질 수 있습니다.
class LogEntry { /* ... */ }; 

class LogBuffer {
private:
    // LogEntry 객체의 소유권을 갖는 unique_ptr들을 저장하는 벡터
    std::vector<std::unique_ptr<LogEntry>> entries_;

public:
    void addLog(const std::string& message) {
        // 1. 힙에 LogEntry 객체를 만들고, 그 소유권을 갖는
        //    unique_ptr 'entry'를 생성합니다.
        auto entry = std::make_unique<LogEntry>(message);
        
        // 2. 'entry'가 가진 소유권을 벡터의 원소로 '이동'시킵니다.
        //    이제 'entry'는 빈 껍데기가 되고, 벡터가 소유권을 갖습니다.
        entries_.push_back(std::move(entry));
    }
    
}; // LogBuffer 객체가 소멸될 때, 벡터가 파괴되고,
   // 벡터 안의 모든 unique_ptr들이 각자 맡은 LogEntry 메모리를 자동으로 해제합니다.
```
`
#### 🤝 2. `shared_ptr` - "공유 관리 비서"

**👥 shared_ptr = 공동 관리자들**
- **"우리가 함께 이 메모리를 관리할게!"**
- 마치 **도서관 책**을 여러 사람이 돌려보는 것과 같아요

🔍 **특징들:**
- **📊 참조 카운터**: "현재 몇 명이 이 메모리를 사용 중인지 센다!"
- **👥 복사 가능**: 새로운 관리자가 추가될 때마다 카운터 +1
- **👋 소멸시**: 관리자가 떠날 때마다 카운터 -1  
- **🗑️ 마지막 관리자**: 카운터가 0이 되면 메모리 자동 정리!

```cpp
// 🤝 shared_ptr 사용 예시  
void share_memory() {
    auto data = std::make_shared<int>(100);  // 👤 관리자 1명 (카운터: 1)
    
    {
        auto copy1 = data;                    // 👤👤 관리자 2명 (카운터: 2)
        auto copy2 = data;                    // 👤👤👤 관리자 3명 (카운터: 3)
        
        cout << data.use_count() << endl;     // "3" 출력
    }   // 👋👋 copy1, copy2 소멸 (카운터: 1)
    
    cout << data.use_count() << endl;         // "1" 출력
}   // 👋 마지막 data 소멸 → 메모리 자동 해제! (카운터: 0)
```

**🎮 실제 LogCaster 예시:**
```cpp
class Client {
    std::string ip;
public:
    Client(string ip) : ip(ip) {}
    void sendMessage(const string& msg) { /* ... */ }
};

class Server {
    vector<shared_ptr<Client>> clients;  // 🤝 클라이언트들을 공유 관리
public:
    void addClient(const string& ip) {
        auto client = make_shared<Client>(ip);  // 👤 클라이언트 생성
        clients.push_back(client);              // 👤👤 Server도 관리 시작
        
        // 다른 모듈들도 이 클라이언트를 참조할 수 있어요
        messageRouter.registerClient(client);   // 👤👤👤 MessageRouter도 관리
        logger.trackClient(client);             // 👤👤👤👤 Logger도 관리
        
        // 🎉 어떤 모듈이 먼저 끝나든 상관없어요! 
        // 모든 참조가 사라져야 클라이언트가 정리됩니다
    }
};
```

**활용 예시**: 하나의 클라이언트 객체를 서버와 다른 관리 모듈이 동시에 참조할 때

```cpp
// Client 클래스
class Client { /* ... */ };

// 서버 클래스
class Server {
private:
    // 여러 Client 객체를 공유 소유하는 shared_ptr들의 목록
    std::vector<std::shared_ptr<Client>> clients_;

public:
    void addNewClient() {
        // 1. 클라이언트를 생성하고 shared_ptr로 소유권을 갖는다. (참조 카운트: 1)
        auto new_client = std::make_shared<Client>(/*...*/>);
        
        // 2. 이 클라이언트를 서버 목록에 추가한다. (복사되면서 참조 카운트: 2)
        clients_.push_back(new_client);
        
        // 3. 다른 관리자 객체에도 이 클라이언트를 전달할 수 있다.
        //    (또 복사되면 참조 카운트: 3)
        // some_manager.monitor(new_client); 
    }
}; // Server가 사라져도, 다른 곳에서 Client를 참조하고 있다면 메모리는 해제되지 않습니다.
   // 모든 참조가 사라져야만 비로소 해제됩니다.
```

#### 3. `std::weak_ptr` - 관찰자

- **문제점**: `shared_ptr`는 매우 편리하지만, 두 객체가 서로를 `shared_ptr`로 가리키는 순환 참조 문제가 발생할 수 있습니다. A는 B가 끝나길 기다리고, B는 A가 끝나길 기다리면서 아무도 끝나지 못해 메모리 누수가 발생합니다.
- **개념**: `weak_ptr`는 `shared_ptr`가 관리하는 객체를 소유하지 않고 관찰(약한 참조)만 합니다. 참조 카운트에 영향을 주지 않으므로 순환 참조를 끊을 수 있습니다.
- **특징**:
    - 객체에 직접 접근할 수 없습니다.
    - 객체에 접근하려면, `lock()` 메서드를 통해 임시 `shared_ptr`를 얻어야 합니다. 만약 객체가 이미 사라졌다면 빈 포인터를 얻게 됩니다.

**활용 예시**: 클라이언트가 자신을 관리하는 서버를 가리켜야 할 때
```cpp
class Server; // 전방 선언: Server라는 클래스가 있다고 미리 알려줌

class Client {
private:
    // 내가 속한 서버를 가리키지만, 소유하지는 않는다.
    // 이것이 shared_ptr였다면 Server와 Client가 서로를 소유하는 순환 참조가 발생한다.
    std::weak_ptr<Server> server_; 
};

class Server {
private:
    std::vector<std::shared_ptr<Client>> clients_;
};
```

---

## 4. 흔한 메모리 실수와 해결책

메모리 관리는 강력한 만큼 위험도 따릅니다. 초보자가 흔히 저지르는 실수를 알아봅시다.

### 일반적인 메모리 오류들

#### 1. 메모리 누수 (Memory Leak)

- **현상**: 힙에 빌린 메모리를 반납(`free` 또는 스마트 포인터 관리)하는 것을 잊어버리는 것.
- **비유**: 호텔 방을 쓰고 체크아웃을 안 하는 것. 프로그램이 실행되는 동안 쓰레기 데이터가 계속 쌓여 결국 시스템을 마비시킵니다.
- **해결책**:
    - (C) `malloc`을 사용했으면, 코드의 모든 분기점에서 `free`가 호출되는지 반드시 확인합니다.
    - (C++) 가급적 스마트 포인터를 사용합니다. RAII가 대부분의 누수를 막아줍니다.

```c
// 잘못된 예시
void memory_leak_example() {
    char* buffer = malloc(1024);
    
    if (some_condition) {
        return;  // free()를 호출하지 않고 반환
    }
    
    free(buffer);
}

// 올바른 예시
void correct_memory_management() {
    char* buffer = malloc(1024);
    
    if (buffer == NULL) {
        return;
    }
    
    // 작업 수행...
    
    // 모든 경로에서 메모리 해제
    if (some_condition) {
        free(buffer);
        return;
    }
    
    free(buffer);
}
```

#### 2. 이중 해제 (Double Free)

- **현상**: 이미 반납한 메모리를 또 반납하려고 시도하는 것.
- **비유**: 이미 반납한 렌터카 키를 또 반납하려는 것. 예측 불가능한 오류나 프로그램 충돌을 일으킵니다.
- **해결책**: (C) `free`를 호출한 후에는 포인터를 `즉시` NULL로 설정하는 습관을 들입니다. `if (ptr != NULL) { free(ptr); }` 처럼 확인하는 로직을 추가합니다.

```c
// 잘못된 예시
void double_free_example() {
    char* buffer = malloc(1024);
    
    free(buffer);
    free(buffer);  // 오류: 이미 해제된 메모리를 다시 해제
}

// 올바른 예시
void safe_free_example() {
    char* buffer = malloc(1024);
    
    free(buffer);
    buffer = NULL;  // 포인터를 NULL로 설정
    
    // 안전한 해제 함수
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
}

// 안전한 해제 매크로
#define SAFE_FREE(ptr) do { \
    if (ptr != NULL) { \
        free(ptr); \
        ptr = NULL; \
    } \
} while(0)
```

#### 3. 댕글링 포인터 (Dangling Pointer)

- **현상**: 이미 반납된 메모리 공간을 계속 가리키고 있는 포인터.
- **비유**: 철거된 집의 주소를 계속 들고 있는 것. 그 주소로 찾아가면 무엇이 있을지(혹은 어떤 위험이 있을지) 알 수 없습니다.
- **해결책**: 이중 해제와 마찬가지로, `free` 후에 포인터를 `NULL`로 설정합니다. 특히 함수가 끝날 때 사라지는 지역 변수의 주소를 반환하지 않도록 주의해야 합니다.

```c
// 잘못된 예시
char* get_temporary_string() {
    char buffer[256] = "Hello World";
    return buffer;  // 지역 변수의 주소를 반환 (위험!)
}

// 올바른 예시
char* get_string_copy(const char* source) {
    size_t len = strlen(source) + 1;
    char* copy = malloc(len);
    if (copy != NULL) {
        strcpy(copy, source);
    }
    return copy;  // 호출자가 free() 해야 함
}
```

### 메모리 디버깅 도구

#### 1. Valgrind (Linux/Mac)

리눅스/Mac에서 사용하는 대표적인 메모리 분석 도구. 메모리 누수, 잘못된 접근 등을 귀신같이 찾아내 보고서를 작성해줍니다

```bash
# 메모리 누수 검사
valgrind --tool=memcheck --leak-check=full ./logcaster_c

# 출력 예시
==12345== Memcheck, a memory error detector
==12345== ERROR SUMMARY: 0 errors from 0 contexts
==12345== LEAK SUMMARY:
==12345==    definitely lost: 0 bytes in 0 blocks
==12345==    indirectly lost: 0 bytes in 0 blocks
==12345==    possibly lost: 0 bytes in 0 blocks
```

#### 2. AddressSanitizer (GCC/Clang)

주소 검사기(AddressSanitizer)는 C/C++ 프로그램에서 메모리 오류를 자동으로 찾아주는 도구입니다. 이는 현대 C/C++ 컴파일러(GCC, Clang)에 내장된 기능으로, 프로그램을 실행할 때 메모리 오류가 발생하는 즉시 알려줍니다.

```bash
# 컴파일 시 플래그 추가
gcc -fsanitize=address -g -o logcaster_c src/*.c

# 실행 시 자동으로 메모리 오류 검출
./logcaster_c
```

#### 3. 정적 분석 도구
```bash
# Clang Static Analyzer
clang --analyze src/*.c

# Cppcheck
cppcheck --enable=all src/
```

---

## 5.  고급 전략: 대용량 데이터를 위한 메모리 최적화

프로젝트에서 수천, 수만 개의 로그 데이터를 처리해야 할 때, 메모리 관리의 효율성을 높이는 것이 중요합니다. 수천, 수만 개의 작은 로그 데이터를 계속해서 생성하고 삭제해야 한다면, `malloc`/`free`를 계속 호출하는 것은 비효율적일 수 있습니다. '관리인'을 너무 자주 부르면 그만큼 시간이 걸리기 때문이죠.

이럴 때 사용하는 고급 기법이 **메모리 풀(Memory Pool)**입니다.

- **개념**: 처음에 거대한 메모리 덩어리(풀)를 한 번만 빌려온 뒤, 프로그램 안에서 자체적으로 작은 조각으로 나눠주고 반납받아 재활용하는 방식입니다.
- **비유**: 레고 블록 한 통을 통째로 사 와서, 필요할 때마다 블록을 가져다 쓰고 다 쓰면 다시 통에 넣어두는 것과 같습니다. 매번 가게에 가서 블록을 하나씩 사 오는 것보다 훨씬 빠릅니다.
- **C++의 구현**: C++에서는 **커스텀 할당자(Custom Allocator)**를 만들어 메모리 풀과 같은 개념을 표준 컨테이너(`std::vector`, `std::list` 등)와 세련되게 결합할 수 있습니다.

이러한 기법은 실시간 시스템이나 게임 엔진, 고성능 서버처럼 성능이 극도로 중요한 분야에서 핵심적인 역할을 합니다.

### C 버전: 메모리 풀 (Memory Pool)

대량의 로그 데이터를 효율적으로 처리하기 위한 메모리 풀 구현:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 메모리 풀 구조체
typedef struct MemoryPool {
    char* pool;           // 실제 메모리 영역
    size_t pool_size;     // 전체 풀 크기
    size_t current_pos;   // 현재 할당 위치
    size_t block_size;    // 블록 크기
    char** free_list;     // 해제된 블록 리스트
    size_t free_count;    // 해제된 블록 수
    size_t max_blocks;    // 최대 블록 수
} MemoryPool;

// 메모리 풀 생성
MemoryPool* pool_create(size_t block_size, size_t block_count) {
    MemoryPool* pool = malloc(sizeof(MemoryPool));
    if (pool == NULL) return NULL;
    
    pool->block_size = block_size;
    pool->max_blocks = block_count;
    pool->pool_size = block_size * block_count;
    pool->current_pos = 0;
    pool->free_count = 0;
    
    // 메모리 풀 할당
    pool->pool = malloc(pool->pool_size);
    if (pool->pool == NULL) {
        free(pool);
        return NULL;
    }
    
    // 해제 리스트 초기화
    pool->free_list = malloc(sizeof(char*) * block_count);
    if (pool->free_list == NULL) {
        free(pool->pool);
        free(pool);
        return NULL;
    }
    
    return pool;
}

// 메모리 블록 할당
void* pool_alloc(MemoryPool* pool) {
    // 해제된 블록이 있으면 재사용
    if (pool->free_count > 0) {
        pool->free_count--;
        return pool->free_list[pool->free_count];
    }
    
    // 새 블록 할당
    if (pool->current_pos + pool->block_size <= pool->pool_size) {
        void* block = pool->pool + pool->current_pos;
        pool->current_pos += pool->block_size;
        return block;
    }
    
    return NULL;  // 풀이 가득 참
}

// 메모리 블록 해제
void pool_free(MemoryPool* pool, void* ptr) {
    if (ptr >= (void*)pool->pool && 
        ptr < (void*)(pool->pool + pool->pool_size)) {
        
        if (pool->free_count < pool->max_blocks) {
            pool->free_list[pool->free_count] = (char*)ptr;
            pool->free_count++;
        }
    }
}

// 메모리 풀 해제
void pool_destroy(MemoryPool* pool) {
    if (pool != NULL) {
        free(pool->free_list);
        free(pool->pool);
        free(pool);
    }
}

// LogCaster에서 메모리 풀 사용
MemoryPool* log_pool = NULL;

int initialize_log_system() {
    // 256바이트 블록 1000개로 메모리 풀 생성
    log_pool = pool_create(256, 1000);
    return (log_pool != NULL) ? 1 : 0;
}

LogEntry* create_log_entry_from_pool(const char* message) {
    LogEntry* entry = (LogEntry*)pool_alloc(log_pool);
    if (entry == NULL) return NULL;
    
    // 메시지 길이 확인 (블록 크기 내에서)
    size_t msg_len = strlen(message);
    if (msg_len >= 200) {  // 구조체 크기 고려
        pool_free(log_pool, entry);
        return NULL;
    }
    
    // 데이터 초기화
    strcpy(entry->message, message);
    entry->message_length = msg_len + 1;
    entry->timestamp = time(NULL);
    entry->next = NULL;
    
    return entry;
}

void destroy_log_entry_to_pool(LogEntry* entry) {
    if (entry != NULL && log_pool != NULL) {
        pool_free(log_pool, entry);
    }
}
```

### C++ 버전: 커스텀 할당자

```cpp
#include <memory>
#include <vector>
#include <deque>

template<typename T>
class PoolAllocator {
private:
    struct Block {
        alignas(T) char data[sizeof(T)];
        Block* next;
    };
    
    Block* free_list_;
    std::vector<std::unique_ptr<Block[]>> pools_;
    size_t pool_size_;

public:
    using value_type = T;
    
    explicit PoolAllocator(size_t pool_size = 1000) 
        : free_list_(nullptr), pool_size_(pool_size) {
        allocate_new_pool();
    }
    
    template<typename U>
    PoolAllocator(const PoolAllocator<U>& other) noexcept 
        : free_list_(nullptr), pool_size_(other.pool_size_) {}
    
    T* allocate(size_t n) {
        if (n != 1) {
            throw std::bad_alloc();
        }
        
        if (free_list_ == nullptr) {
            allocate_new_pool();
        }
        
        if (free_list_ == nullptr) {
            throw std::bad_alloc();
        }
        
        Block* block = free_list_;
        free_list_ = free_list_->next;
        return reinterpret_cast<T*>(block);
    }
    
    void deallocate(T* ptr, size_t n) {
        if (ptr == nullptr || n != 1) return;
        
        Block* block = reinterpret_cast<Block*>(ptr);
        block->next = free_list_;
        free_list_ = block;
    }
    
private:
    void allocate_new_pool() {
        auto new_pool = std::make_unique<Block[]>(pool_size_);
        
        for (size_t i = 0; i < pool_size_ - 1; ++i) {
            new_pool[i].next = &new_pool[i + 1];
        }
        new_pool[pool_size_ - 1].next = free_list_;
        
        free_list_ = &new_pool[0];
        pools_.push_back(std::move(new_pool));
    }
};

// LogCaster에서 커스텀 할당자 사용
class LogEntry {
private:
    std::string message_;
    std::chrono::system_clock::time_point timestamp_;

public:
    LogEntry(const std::string& message) 
        : message_(message), timestamp_(std::chrono::system_clock::now()) {}
    
    const std::string& getMessage() const { return message_; }
};

// 커스텀 할당자를 사용하는 벡터
using LogVector = std::vector<LogEntry, PoolAllocator<LogEntry>>;

class LogBuffer {
private:
    LogVector entries_;

public:
    LogBuffer() : entries_(PoolAllocator<LogEntry>(1000)) {}
    
    void addLog(const std::string& message) {
        entries_.emplace_back(message);
    }
    
    size_t size() const { return entries_.size(); }
    
    const LogEntry& getLog(size_t index) const {
        return entries_[index];
    }
};
```

## 🗺️ 6. 메모리 매핑 파일 (Memory-Mapped Files)

### 6.1 mmap의 개념과 활용

메모리 매핑 파일은 파일을 메모리에 직접 매핑하여 파일 I/O를 메모리 접근처럼 수행할 수 있게 합니다.

#### 기본 사용법
```c
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

// 파일을 메모리에 매핑
void* map_file(const char* filename, size_t* file_size) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return NULL;
    }
    
    // 파일 크기 확인
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return NULL;
    }
    
    *file_size = st.st_size;
    
    // 메모리에 매핑
    void* mapped = mmap(NULL, st.st_size, PROT_READ, 
                       MAP_PRIVATE, fd, 0);
    
    close(fd);  // 매핑 후에는 fd 필요 없음
    
    if (mapped == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }
    
    return mapped;
}

// 매핑 해제
void unmap_file(void* addr, size_t size) {
    munmap(addr, size);
}
```

### 6.2 LogCaster에서의 활용
```c
// 대용량 로그 파일 읽기
typedef struct {
    char* data;
    size_t size;
    size_t current_offset;
} mmap_reader_t;

mmap_reader_t* create_mmap_reader(const char* filename) {
    mmap_reader_t* reader = malloc(sizeof(mmap_reader_t));
    if (!reader) return NULL;
    
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        free(reader);
        return NULL;
    }
    
    struct stat st;
    fstat(fd, &st);
    reader->size = st.st_size;
    
    reader->data = mmap(NULL, reader->size, PROT_READ, 
                       MAP_PRIVATE, fd, 0);
    close(fd);
    
    if (reader->data == MAP_FAILED) {
        free(reader);
        return NULL;
    }
    
    reader->current_offset = 0;
    return reader;
}

// 줄 단위로 읽기
char* read_line_mmap(mmap_reader_t* reader) {
    if (reader->current_offset >= reader->size) {
        return NULL;
    }
    
    char* start = reader->data + reader->current_offset;
    char* end = memchr(start, '\n', 
                      reader->size - reader->current_offset);
    
    if (!end) {
        end = reader->data + reader->size;
    }
    
    size_t line_len = end - start;
    char* line = malloc(line_len + 1);
    memcpy(line, start, line_len);
    line[line_len] = '\0';
    
    reader->current_offset = (end - reader->data) + 1;
    return line;
}
```

### 6.3 Shared Memory 구현
```cpp
// 프로세스 간 공유 메모리
class SharedMemory {
private:
    void* addr_;
    size_t size_;
    int fd_;
    std::string name_;
    
public:
    SharedMemory(const std::string& name, size_t size) 
        : name_(name), size_(size) {
        // 공유 메모리 생성
        fd_ = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
        if (fd_ < 0) {
            throw std::runtime_error("shm_open failed");
        }
        
        // 크기 설정
        if (ftruncate(fd_, size) < 0) {
            close(fd_);
            shm_unlink(name.c_str());
            throw std::runtime_error("ftruncate failed");
        }
        
        // 메모리 매핑
        addr_ = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                    MAP_SHARED, fd_, 0);
        if (addr_ == MAP_FAILED) {
            close(fd_);
            shm_unlink(name.c_str());
            throw std::runtime_error("mmap failed");
        }
    }
    
    ~SharedMemory() {
        munmap(addr_, size_);
        close(fd_);
        shm_unlink(name_.c_str());
    }
    
    void* get() { return addr_; }
    size_t size() const { return size_; }
};
```

## 🧪 7. 메모리 디버깅 도구

### 7.1 Valgrind 활용
```bash
# 메모리 누수 검사
valgrind --leak-check=full --show-leak-kinds=all ./logcaster

# 메모리 접근 오류 검사
valgrind --track-origins=yes ./logcaster

# 캐시 성능 분석
valgrind --tool=cachegrind ./logcaster
```

### 7.2 AddressSanitizer (ASan)
```bash
# 컴파일 시 ASan 활성화
gcc -fsanitize=address -g -O1 logcaster.c -o logcaster

# 실행
ASAN_OPTIONS=detect_leaks=1 ./logcaster
```

### 7.3 커스텀 메모리 디버거
```c
// 간단한 메모리 추적기
typedef struct allocation {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    struct allocation* next;
} allocation_t;

static allocation_t* allocations = NULL;
static pthread_mutex_t alloc_mutex = PTHREAD_MUTEX_INITIALIZER;

void* debug_malloc(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if (!ptr) return NULL;
    
    allocation_t* alloc = malloc(sizeof(allocation_t));
    alloc->ptr = ptr;
    alloc->size = size;
    alloc->file = file;
    alloc->line = line;
    
    pthread_mutex_lock(&alloc_mutex);
    alloc->next = allocations;
    allocations = alloc;
    pthread_mutex_unlock(&alloc_mutex);
    
    return ptr;
}

void print_memory_leaks() {
    pthread_mutex_lock(&alloc_mutex);
    allocation_t* current = allocations;
    size_t total_leaked = 0;
    
    while (current) {
        fprintf(stderr, "LEAK: %zu bytes at %p (allocated at %s:%d)\n",
                current->size, current->ptr, current->file, current->line);
        total_leaked += current->size;
        current = current->next;
    }
    
    if (total_leaked > 0) {
        fprintf(stderr, "Total leaked: %zu bytes\n", total_leaked);
    }
    pthread_mutex_unlock(&alloc_mutex);
}

#define MALLOC(size) debug_malloc(size, __FILE__, __LINE__)
```

### 7.4 Memory Profiling
```cpp
// 메모리 사용량 프로파일러
class MemoryProfiler {
private:
    struct AllocationInfo {
        size_t size;
        std::string stack_trace;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    std::unordered_map<void*, AllocationInfo> allocations_;
    std::mutex mutex_;
    size_t total_allocated_ = 0;
    size_t peak_allocated_ = 0;
    
public:
    void recordAllocation(void* ptr, size_t size) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        allocations_[ptr] = {
            size,
            getStackTrace(),
            std::chrono::steady_clock::now()
        };
        
        total_allocated_ += size;
        peak_allocated_ = std::max(peak_allocated_, total_allocated_);
    }
    
    void printReport() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::cout << "=== Memory Profile Report ===\n";
        std::cout << "Current allocated: " << total_allocated_ << " bytes\n";
        std::cout << "Peak allocated: " << peak_allocated_ << " bytes\n";
        std::cout << "Active allocations: " << allocations_.size() << "\n\n";
        
        // Top allocations by size
        std::vector<std::pair<void*, AllocationInfo>> sorted_allocs(
            allocations_.begin(), allocations_.end()
        );
        
        std::sort(sorted_allocs.begin(), sorted_allocs.end(),
            [](const auto& a, const auto& b) {
                return a.second.size > b.second.size;
            });
        
        std::cout << "Top 10 allocations:\n";
        for (size_t i = 0; i < std::min(size_t(10), sorted_allocs.size()); i++) {
            const auto& [ptr, info] = sorted_allocs[i];
            std::cout << i + 1 << ". " << info.size << " bytes at " 
                     << ptr << "\n";
        }
    }
};
```

---

## 8. 다음 단계 준비

이 문서를 완전히 이해했다면:

1. **스택과 힙의 차이점**을 명확히 설명할 수 있어야 합니다.
2. **C의 `malloc`/`free`**를 안전하게 사용할 수 있어야 합니다.
3. **C++의 스마트 포인터**를 적절히 선택하고 사용할 수 있어야 합니다.
4. **메모리 누수를 방지**하는 코딩 습관을 가져야 합니다.
5. **디버깅 도구**를 활용할 수 있어야 합니다.
6. **메모리 매핑 파일**을 사용하여 효율적인 파일 I/O를 구현할 수 있어야 합니다.
7. **커스텀 할당자**를 설계하고 구현할 수 있어야 합니다.

### 확인 문제

1. 다음 코드에서 메모리 누수가 발생하는 이유는 무엇인가요?
```c
char* create_buffer() {
    char* buf = malloc(1024);
    strcpy(buf, "Hello");
    if (strlen(buf) < 10) {
        return NULL;  // 문제!
    }
    return buf;
}
```

2. `std::unique_ptr`와 `std::shared_ptr` 중 언제 어떤 것을 사용해야 할까요?

3. LogCaster 프로젝트에서 대량의 로그 데이터를 메모리에 저장할 때 고려해야 할 사항은 무엇인가요?

## 🚨 메모리 관련 자주 하는 실수와 해결법

### 1. 메모리 누수 (Memory Leak)
**문제**: malloc/new 후 free/delete를 하지 않음
```c
// ❌ 잘못된 코드
void process_logs() {
    char* buffer = malloc(1024);
    // ... 처리 ...
    return;  // free(buffer) 잊음!
}

// ✅ 올바른 코드
void process_logs() {
    char* buffer = malloc(1024);
    if (buffer == NULL) return;
    
    // ... 처리 ...
    
    free(buffer);  // 반드시 해제!
}
```

### 2. 댕글링 포인터 (Dangling Pointer)
**문제**: 해제된 메모리를 가리키는 포인터 사용
```c
// ❌ 위험한 코드
char* ptr = malloc(100);
free(ptr);
strcpy(ptr, "위험!");  // 이미 해제된 메모리에 접근!

// ✅ 안전한 코드
char* ptr = malloc(100);
free(ptr);
ptr = NULL;  // 포인터를 NULL로 설정
```

### 3. 버퍼 오버플로우
**문제**: 할당된 크기보다 많은 데이터 쓰기
```c
// ❌ 위험한 코드
char buffer[10];
strcpy(buffer, "This is too long!");  // 10바이트 초과!

// ✅ 안전한 코드
char buffer[10];
strncpy(buffer, "This is too long!", sizeof(buffer) - 1);
buffer[sizeof(buffer) - 1] = '\0';  // 널 종료 보장
```

## 🛠️ 메모리 디버깅 도구

### Valgrind 사용법 (Linux/Mac)
```bash
# 컴파일 (디버그 정보 포함)
gcc -g -o logcaster main.c

# Valgrind로 메모리 누수 검사
valgrind --leak-check=full ./logcaster
```

### AddressSanitizer 사용법
```bash
# 컴파일 시 AddressSanitizer 활성화
gcc -fsanitize=address -g -o logcaster main.c

# 실행하면 자동으로 메모리 문제 감지
./logcaster
```

## 💡 실습 프로젝트

### 미니 프로젝트: 간단한 메모리 풀 구현
```c
typedef struct MemoryPool {
    void* pool;          // 전체 메모리 블록
    size_t block_size;   // 각 블록의 크기
    size_t num_blocks;   // 블록 개수
    void** free_list;    // 사용 가능한 블록 리스트
} MemoryPool;

// 구현해야 할 함수들
MemoryPool* pool_create(size_t block_size, size_t num_blocks);
void* pool_alloc(MemoryPool* pool);
void pool_free(MemoryPool* pool, void* block);
void pool_destroy(MemoryPool* pool);
```

## 📚 추가 학습 자료

### 책 추천
- "The C Programming Language" - K&R (메모리 관리 기초)
- "Effective Modern C++" - Scott Meyers (스마트 포인터)
- "Computer Systems: A Programmer's Perspective" - Bryant & O'Hallaron

### 온라인 자료
- [cppreference.com - Dynamic memory management](https://en.cppreference.com/w/cpp/memory)
- [Valgrind Quick Start Guide](http://valgrind.org/docs/manual/quick-start.html)
- [AddressSanitizer Wiki](https://github.com/google/sanitizers/wiki/AddressSanitizer)

## ✅ 학습 체크리스트

메모리 관리 마스터를 위한 체크리스트:

### 기초 (1-2주)
- [ ] 스택과 힙의 차이점 이해
- [ ] malloc/free 기본 사용법
- [ ] 메모리 할당 실패 처리
- [ ] 간단한 동적 배열 구현

### 중급 (3-4주)  
- [ ] 메모리 누수 디버깅
- [ ] 버퍼 오버플로우 방지
- [ ] 동적 버퍼 크기 조정
- [ ] 메모리 풀 기본 구현

### 고급 (5-6주)
- [ ] C++ 스마트 포인터 활용
- [ ] RAII 패턴 적용
- [ ] 커스텀 할당자 구현
- [ ] 메모리 매핑 파일 사용

### 전문가 (7-8주)
- [ ] 락프리 메모리 관리
- [ ] 메모리 프로파일링
- [ ] 대규모 시스템 최적화
- [ ] 프로덕션 디버깅

## 🔄 다음 학습 단계

이 문서를 완료했다면 다음 문서로 진행하세요:

1. **[02_DataStructures.md](02.%20DataStructures.md)** - 효율적인 자료구조 활용
   - 배열 vs 연결 리스트
   - 원형 버퍼 구현
   - 해시 테이블 활용

2. **[03. Networking.md](03.%20Networking.md)** - 네트워킹 기초
   - TCP/IP 이해
   - 소켓 프로그래밍
   - 비동기 I/O

3. **[05. Multithreading.md](05.%20Multithreading.md)** - 동시성 프로그래밍
   - 스레드 안전한 메모리 관리
   - 락프리 자료구조
   - 메모리 순서와 원자성

---

*💡 팁: 메모리 관리는 C/C++의 핵심입니다. 충분히 연습하고 디버깅 도구에 익숙해지세요!*