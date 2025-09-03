# 6단계: 데이터 구조
*LogCaster 프로젝트를 위한 사전지식 문서*

---

## 📋 문서 정보
- **난이도**: 🟡 중급
- **예상 학습 시간**: 6-8시간
- **전제 조건**: 
  - C 기본 문법 (포인터, 구조체)
  - 메모리 관리 기초
  - C++ 클래스 기본 개념
- **실습 환경**: GCC/G++ 컴파일러, C++11 이상

## 🎯 이 문서에서 배울 내용

**"데이터 구조? 그게 뭔가요? 왜 중요한가요?"**

좋은 질문이에요! 데이터 구조를 **정리 방법**에 비유해보겠습니다:

📚 **데이터 구조 = 정리의 기술**
- **책장 정리**: 장르별로? 저자별로? 크기별로?
- **옷장 정리**: 색깔별로? 종류별로? 계절별로?
- **로그 정리**: 시간별로? 중요도별로? 출처별로?

각각의 정리 방법마다 **찾는 속도, 추가하는 속도, 관리의 편의성**이 다릅니다!

🎓 **이 문서에서 배울 것들:**
- C언어로 직접 만드는 기본 데이터 구조 (연결 리스트, 스택, 큐) 🔧
- C++가 제공하는 편리한 컨테이너들 (vector, map, queue 등) 📦  
- LogCaster에 특화된 고성능 데이터 구조 설계 🚀
- "언제 어떤 구조를 써야 하는지" 판단하는 기준 🎯

💡 **친근한 학습 방식:**
- 모든 개념을 일상생활 예시로 설명합니다
- 각 구조의 장단점을 솔직하게 알려드립니다
- 실제 성능 차이를 체감할 수 있는 예시를 제공합니다
- LogCaster 프로젝트에서 실제로 어떻게 쓰이는지 보여드립니다

---

## 📋 1. 연결 리스트 구현 (C)

### 🚂 "연결 리스트 = 기차"

연결 리스트를 **기차**에 비유해보겠습니다!

🚂 **기차의 특징:**
- 각 칸(노드)마다 **승객(데이터)**이 타고 있어요
- 각 칸은 **다음 칸의 위치**를 알고 있어요 (포인터)
- 중간에 **새 칸을 끼워넣기** 쉬워요 (삽입)
- 필요 없는 칸을 **분리하기** 쉬워요 (삭제)  
- 하지만 5번째 칸에 가려면 1→2→3→4→5 **순서대로** 가야 해요

### 🔍 단일 연결 리스트: "단방향 기차"

**특징:**
- **동적 크기**: 필요한 만큼 칸을 늘리거나 줄일 수 있어요
- **빠른 삽입/삭제**: 중간에 끼워넣거나 빼는 게 쉬워요  
- **순차 접근**: 원하는 칸에 가려면 처음부터 차례대로 가야 해요

**🤔 언제 사용할까요?**

**✅ 연결 리스트가 좋은 경우:**
- 데이터 개수를 미리 모를 때 (동적 크기)
- 자주 삽입/삭제가 일어날 때 
- 메모리를 아껴야 할 때 (필요한 만큼만 사용)
- 순서대로 데이터를 처리할 때

**❌ 연결 리스트가 나쁜 경우:**
- 특정 위치의 데이터에 빠르게 접근해야 할 때
- 메모리가 연속적이어야 할 때 (캐시 성능)
- 작은 데이터에서는 포인터 오버헤드가 부담스러울 때

**🎯 LogCaster에서의 활용 예시:**
```
실시간 로그 스트림: [새 로그] → [이전 로그] → [더 이전 로그] → ...
                     ↑ 새로 들어오는 로그를 맨 앞에 추가하기 쉬움!
                     
검색 결과 리스트: [결과1] → [결과2] → [결과3] → ...
                  ↑ 검색 조건에 맞는 로그들을 하나씩 연결
```

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 로그 엔트리를 위한 노드 구조체
typedef struct LogNode {
    char message[256];           // 로그 메시지
    time_t timestamp;            // 타임스탬프
    char level[16];              // 로그 레벨 (INFO, WARNING, ERROR)
    char source[64];             // 로그 소스 (클라이언트 IP)
    struct LogNode* next;        // 다음 노드 포인터
} LogNode;

// 연결 리스트 구조체
// 기차의 시작과 끝을 추적하는 개념
typedef struct LogList {
    LogNode* head;               // 🚂 첫 번째 노드 (기관차)
    LogNode* tail;               // 🚃 마지막 노드 (꼬리칸)
    int count;                   // 🔢 노드 개수 (총 칸수)
} LogList;

// 리스트 초기화
LogList* create_log_list() {
    LogList* list = malloc(sizeof(LogList));
    if (list == NULL) {
        return NULL;
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    
    return list;
}

// 노드 생성
LogNode* create_log_node(const char* message, const char* level, const char* source) {
    LogNode* node = malloc(sizeof(LogNode));
    if (node == NULL) {
        return NULL;
    }
    
    // 데이터 복사
    strncpy(node->message, message, sizeof(node->message) - 1);
    node->message[sizeof(node->message) - 1] = '\0';
    
    strncpy(node->level, level, sizeof(node->level) - 1);
    node->level[sizeof(node->level) - 1] = '\0';
    
    strncpy(node->source, source, sizeof(node->source) - 1);
    node->source[sizeof(node->source) - 1] = '\0';
    
    node->timestamp = time(NULL);
    node->next = NULL;
    
    return node;
}

// 리스트 앞에 삽입 (최신 로그가 앞에 오도록)
// 🏃 O(1) - 항상 빠름!
int log_list_prepend(LogList* list, const char* message, const char* level, const char* source) {
    LogNode* new_node = create_log_node(message, level, source);
    if (new_node == NULL) {
        return -1;
    }
    
    // 새 노드를 맨 앞에 삽입
    // [🆕][📦] → [🆕][📦][📦]
    new_node->next = list->head;
    list->head = new_node;
    
    // 첫 번째 노드라면 tail도 설정
    if (list->tail == NULL) {
        list->tail = new_node;
    }
    
    list->count++;
    return 0;
}

// 리스트 뒤에 삽입 (시간 순서대로)
int log_list_append(LogList* list, const char* message, const char* level, const char* source) {
    LogNode* new_node = create_log_node(message, level, source);
    if (new_node == NULL) {
        return -1;
    }
    
    if (list->tail == NULL) {
        // 첫 번째 노드
        list->head = new_node;
        list->tail = new_node;
    } else {
        list->tail->next = new_node;
        list->tail = new_node;
    }
    
    list->count++;
    return 0;
}

// 특정 위치에 삽입
int log_list_insert_at(LogList* list, int index, const char* message, 
                      const char* level, const char* source) {
    if (index < 0 || index > list->count) {
        return -1;  // 잘못된 인덱스
    }
    
    if (index == 0) {
        return log_list_prepend(list, message, level, source);
    }
    
    if (index == list->count) {
        return log_list_append(list, message, level, source);
    }
    
    LogNode* new_node = create_log_node(message, level, source);
    if (new_node == NULL) {
        return -1;
    }
    
    // 삽입 위치 이전 노드 찾기
    LogNode* current = list->head;
    for (int i = 0; i < index - 1; i++) {
        current = current->next;
    }
    
    new_node->next = current->next;
    current->next = new_node;
    
    list->count++;
    return 0;
}

// 첫 번째 노드 제거
LogNode* log_list_remove_first(LogList* list) {
    if (list->head == NULL) {
        return NULL;
    }
    
    LogNode* removed = list->head;
    list->head = removed->next;
    
    if (list->head == NULL) {
        list->tail = NULL;  // 리스트가 비었음
    }
    
    removed->next = NULL;  // 연결 끊기
    list->count--;
    
    return removed;
}

// 마지막 노드 제거
LogNode* log_list_remove_last(LogList* list) {
    if (list->head == NULL) {
        return NULL;
    }
    
    if (list->head == list->tail) {
        // 노드가 하나뿐
        LogNode* removed = list->head;
        list->head = NULL;
        list->tail = NULL;
        list->count--;
        return removed;
    }
    
    // 마지막 이전 노드 찾기
    LogNode* current = list->head;
    while (current->next != list->tail) {
        current = current->next;
    }
    
    LogNode* removed = list->tail;
    current->next = NULL;
    list->tail = current;
    
    list->count--;
    return removed;
}

// 특정 조건으로 검색
LogNode* log_list_find(LogList* list, const char* keyword) {
    LogNode* current = list->head;
    
    while (current != NULL) {
        if (strstr(current->message, keyword) != NULL ||
            strstr(current->level, keyword) != NULL ||
            strstr(current->source, keyword) != NULL) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// 모든 매칭 결과 찾기
int log_list_find_all(LogList* list, const char* keyword, LogNode** results, int max_results) {
    LogNode* current = list->head;
    int found_count = 0;
    
    while (current != NULL && found_count < max_results) {
        if (strstr(current->message, keyword) != NULL ||
            strstr(current->level, keyword) != NULL ||
            strstr(current->source, keyword) != NULL) {
            
            // 결과 복사 (원본 보호)
            results[found_count] = malloc(sizeof(LogNode));
            if (results[found_count] != NULL) {
                memcpy(results[found_count], current, sizeof(LogNode));
                results[found_count]->next = NULL;
                found_count++;
            }
        }
        current = current->next;
    }
    
    return found_count;
}

// 시간 범위로 검색
int log_list_find_by_time_range(LogList* list, time_t start_time, time_t end_time,
                                LogNode** results, int max_results) {
    LogNode* current = list->head;
    int found_count = 0;
    
    while (current != NULL && found_count < max_results) {
        if (current->timestamp >= start_time && current->timestamp <= end_time) {
            results[found_count] = malloc(sizeof(LogNode));
            if (results[found_count] != NULL) {
                memcpy(results[found_count], current, sizeof(LogNode));
                results[found_count]->next = NULL;
                found_count++;
            }
        }
        current = current->next;
    }
    
    return found_count;
}

// 리스트 출력
void log_list_print(LogList* list) {
    printf("=== Log List (%d entries) ===\n", list->count);
    
    LogNode* current = list->head;
    int index = 0;
    
    while (current != NULL) {
        printf("[%d] %s | %s | %s | %s", 
               index++,
               current->level,
               current->source,
               ctime(&current->timestamp),  // 자동으로 \n 포함
               current->message);
        current = current->next;
    }
}

// 메모리 정리
void destroy_log_list(LogList* list) {
    if (list == NULL) return;
    
    LogNode* current = list->head;
    while (current != NULL) {
        LogNode* next = current->next;
        free(current);
        current = next;
    }
    
    free(list);
}

// 노드 메모리 해제
void destroy_log_node(LogNode* node) {
    if (node != NULL) {
        free(node);
    }
}
```

### 이중 연결 리스트 (Doubly Linked List)

양방향 탐색이 가능한 연결 리스트입니다.

```c
// 이중 연결 리스트 노드
typedef struct DoubleLogNode {
    char message[256];
    time_t timestamp;
    char level[16];
    char source[64];
    struct DoubleLogNode* next;    // 다음 노드
    struct DoubleLogNode* prev;    // 이전 노드
} DoubleLogNode;

typedef struct DoubleLogList {
    DoubleLogNode* head;
    DoubleLogNode* tail;
    int count;
} DoubleLogList;

// 이중 연결 리스트 초기화
DoubleLogList* create_double_log_list() {
    DoubleLogList* list = malloc(sizeof(DoubleLogList));
    if (list == NULL) return NULL;
    
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    
    return list;
}

// 노드 생성
DoubleLogNode* create_double_log_node(const char* message, const char* level, const char* source) {
    DoubleLogNode* node = malloc(sizeof(DoubleLogNode));
    if (node == NULL) return NULL;
    
    strncpy(node->message, message, sizeof(node->message) - 1);
    node->message[sizeof(node->message) - 1] = '\0';
    
    strncpy(node->level, level, sizeof(node->level) - 1);
    node->level[sizeof(node->level) - 1] = '\0';
    
    strncpy(node->source, source, sizeof(node->source) - 1);
    node->source[sizeof(node->source) - 1] = '\0';
    
    node->timestamp = time(NULL);
    node->next = NULL;
    node->prev = NULL;
    
    return node;
}

// 앞에 삽입
int double_list_prepend(DoubleLogList* list, const char* message, 
                       const char* level, const char* source) {
    DoubleLogNode* new_node = create_double_log_node(message, level, source);
    if (new_node == NULL) return -1;
    
    if (list->head == NULL) {
        // 첫 번째 노드
        list->head = new_node;
        list->tail = new_node;
    } else {
        new_node->next = list->head;
        list->head->prev = new_node;
        list->head = new_node;
    }
    
    list->count++;
    return 0;
}

// 뒤에 삽입
int double_list_append(DoubleLogList* list, const char* message,
                      const char* level, const char* source) {
    DoubleLogNode* new_node = create_double_log_node(message, level, source);
    if (new_node == NULL) return -1;
    
    if (list->tail == NULL) {
        // 첫 번째 노드
        list->head = new_node;
        list->tail = new_node;
    } else {
        list->tail->next = new_node;
        new_node->prev = list->tail;
        list->tail = new_node;
    }
    
    list->count++;
    return 0;
}

// 특정 노드 제거
int double_list_remove_node(DoubleLogList* list, DoubleLogNode* node) {
    if (node == NULL) return -1;
    
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    }
    
    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;
    }
    
    free(node);
    list->count--;
    
    return 0;
}

// 역방향 출력
void double_list_print_reverse(DoubleLogList* list) {
    printf("=== Log List (Reverse Order) ===\n");
    
    DoubleLogNode* current = list->tail;
    int index = list->count - 1;
    
    while (current != NULL) {
        printf("[%d] %s | %s | %s", 
               index--,
               current->level,
               current->source,
               current->message);
        current = current->prev;
    }
}
```

---

## 🗂️ 2. STL 컨테이너 (C++)

### 🎁 "STL = 프로그래머를 위한 선물상자"

**STL(Standard Template Library)**을 **완벽한 도구상자**에 비유해보겠습니다!

🧰 **STL = 만능 도구상자**
- 이미 **잘 만들어진 도구들**이 들어있어요
- **테스트도 완료**되어서 안전해요
- **성능도 최적화**되어 있어요
- 바퀴를 다시 발명할 필요가 없어요!

**🤔 "C로 직접 만들기 vs STL 사용하기"**

| 측면 | C 직접 구현 | C++ STL |
|------|------------|---------|
| **개발 시간** | ⏳ 오래 걸림 (며칠~주) | ⚡ 즉시 사용 (몇 분) |
| **버그 위험** | 🐛 높음 (메모리 누수, 로직 오류) | 🛡️ 낮음 (철저히 테스트됨) |
| **성능** | 🎯 특화 최적화 가능 | 🏃 일반적으로 최적화됨 |
| **유지보수** | 🔧 직접 관리해야 함 | 🤖 자동 관리 |
| **학습 효과** | 📚 내부 구조 깊이 이해 | 🚀 빠른 개발 집중 |
| **코드 길이** | 📜 길고 복잡 | 📝 짧고 명확 |

**💡 실무에서의 선택 기준:**
- **학습 목적**: C로 직접 구현해보기 (이해도 증진)
- **실제 프로젝트**: STL 사용하기 (생산성과 안정성)
- **특수한 요구사항**: 하이브리드 (STL 기반 + 커스텀 최적화)

### 📊 `std::vector`: "마법의 신축 가방"

`vector`를 **해리포터의 신축 가방**에 비유해보겠습니다!

🎒 **vector = 신축 가방**
- **자동으로 크기 조절**: 물건이 많아지면 가방이 커져요!
- **빠른 접근**: "5번째 물건 주세요!" → 즉시!
- **연속된 공간**: 물건들이 나란히 정렬되어 있어요
- **뿌림 끝에서 추가/제거**: 가방 끝에서 넣고 빼는 게 제일 빨라요

🔍 **언제 vector를 사용하나요?**
- **빠른 랜덤 접근**이 필요할 때: "10번째 로그를 바로 보여줘!"
- **순서대로 저장**하고 싶을 때: 시간 순서대로 로그 저장
- **자주 조회**하지만 **가끔 추가**하는 데이터: 설정값, 사용자 목록 등

**🚀 성능 비교 실험:**

```cpp
#include <vector>
#include <list>
#include <chrono>
#include <iostream>

void performance_comparison() {
    const int SIZE = 1000000;
    
    // 1. 순차 접근 성능 비교
    std::vector<int> vec(SIZE, 1);
    std::list<int> lst(SIZE, 1);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Vector: 인덱스 접근
    long long sum1 = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum1 += vec[i];  // O(1) 접근
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    
    // List: 반복자 접근
    long long sum2 = 0;
    for (auto it = lst.begin(); it != lst.end(); ++it) {
        sum2 += *it;  // O(1) 접근 (하지만 캐시 미스)
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    
    auto vec_time = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start).count();
    auto list_time = std::chrono::duration_cast<std::chrono::microseconds>(end2 - end1).count();
    
    std::cout << "Vector 순회: " << vec_time << " 마이크로초\n";
    std::cout << "List 순회: " << list_time << " 마이크로초\n";
    std::cout << "Vector이 " << (double)list_time / vec_time << "배 빠름!\n";
}
```

**📊 일반적인 결과:**
```
Vector 순회: 750 마이크로초
List 순회: 3200 마이크로초
Vector이 4.3배 빠름!
```

**🤔 왜 vector가 더 빠를까요?**
1. **메모리 지역성**: 데이터가 연속적으로 배치되어 CPU 캐시 효율 증가
2. **컴파일러 최적화**: 벡터화(SIMD) 같은 최적화 가능
3. **간단한 주소 계산**: `시작주소 + (인덱스 × 크기)`로 바로 계산

```cpp
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <iostream>

class LogEntry {
private:
    std::string message_;
    std::chrono::system_clock::time_point timestamp_;
    std::string level_;
    std::string source_;

public:
    LogEntry(const std::string& message, const std::string& level, const std::string& source)
        : message_(message), level_(level), source_(source),
          timestamp_(std::chrono::system_clock::now()) {}
    
    // Getter 메서드들
    const std::string& getMessage() const { return message_; }
    const std::string& getLevel() const { return level_; }
    const std::string& getSource() const { return source_; }
    const std::chrono::system_clock::time_point& getTimestamp() const { return timestamp_; }
    
    // 출력용
    std::string toString() const {
        auto time_t = std::chrono::system_clock::to_time_t(timestamp_);
        return "[" + level_ + "] " + source_ + " - " + message_ + " (" + std::ctime(&time_t) + ")";
    }
    
    // 비교 연산자 (정렬용)
    bool operator<(const LogEntry& other) const {
        return timestamp_ < other.timestamp_;
    }
};

class VectorLogStorage {
private:
    std::vector<LogEntry> logs_;

public:
    // 로그 추가
    void addLog(const std::string& message, const std::string& level, const std::string& source) {
        logs_.emplace_back(message, level, source);
    }
    
    // 크기 조회
    size_t size() const {
        return logs_.size();
    }
    
    // 인덱스로 접근
    const LogEntry& operator[](size_t index) const {
        return logs_[index];
    }
    
    LogEntry& operator[](size_t index) {
        return logs_[index];
    }
    
    // 안전한 접근
    const LogEntry& at(size_t index) const {
        return logs_.at(index);
    }
    
    // 범위 기반 for문 지원
    auto begin() const { return logs_.begin(); }
    auto end() const { return logs_.end(); }
    auto begin() { return logs_.begin(); }
    auto end() { return logs_.end(); }
    
    // 최근 N개 로그 가져오기
    std::vector<LogEntry> getRecentLogs(size_t count) const {
        if (count >= logs_.size()) {
            return logs_;
        }
        
        return std::vector<LogEntry>(logs_.end() - count, logs_.end());
    }
    
    // 키워드로 검색
    std::vector<LogEntry> searchByKeyword(const std::string& keyword) const {
        std::vector<LogEntry> results;
        
        for (const auto& log : logs_) {
            if (log.getMessage().find(keyword) != std::string::npos ||
                log.getLevel().find(keyword) != std::string::npos ||
                log.getSource().find(keyword) != std::string::npos) {
                results.push_back(log);
            }
        }
        
        return results;
    }
    
    // 레벨별 필터링
    std::vector<LogEntry> filterByLevel(const std::string& level) const {
        std::vector<LogEntry> results;
        
        std::copy_if(logs_.begin(), logs_.end(), std::back_inserter(results),
                     [&level](const LogEntry& log) {
                         return log.getLevel() == level;
                     });
        
        return results;
    }
    
    // 시간 범위로 필터링
    std::vector<LogEntry> filterByTimeRange(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) const {
        
        std::vector<LogEntry> results;
        
        std::copy_if(logs_.begin(), logs_.end(), std::back_inserter(results),
                     [&start, &end](const LogEntry& log) {
                         auto timestamp = log.getTimestamp();
                         return timestamp >= start && timestamp <= end;
                     });
        
        return results;
    }
    
    // 정렬
    void sortByTimestamp() {
        std::sort(logs_.begin(), logs_.end());
    }
    
    // 메모리 최적화
    void shrinkToFit() {
        logs_.shrink_to_fit();
    }
    
    // 용량 예약
    void reserve(size_t capacity) {
        logs_.reserve(capacity);
    }
    
    // 로그 제한 (오래된 로그 삭제)
    void limitSize(size_t maxSize) {
        if (logs_.size() > maxSize) {
            logs_.erase(logs_.begin(), logs_.begin() + (logs_.size() - maxSize));
        }
    }
    
    // 통계 정보
    struct Statistics {
        size_t totalLogs = 0;
        size_t errorCount = 0;
        size_t warningCount = 0;
        size_t infoCount = 0;
        std::string mostActiveSource;
    };
    
    Statistics getStatistics() const {
        Statistics stats;
        stats.totalLogs = logs_.size();
        
        std::map<std::string, size_t> sourceCounts;
        
        for (const auto& log : logs_) {
            const std::string& level = log.getLevel();
            if (level == "ERROR") stats.errorCount++;
            else if (level == "WARNING") stats.warningCount++;
            else if (level == "INFO") stats.infoCount++;
            
            sourceCounts[log.getSource()]++;
        }
        
        // 가장 활발한 소스 찾기
        auto maxSource = std::max_element(sourceCounts.begin(), sourceCounts.end(),
                                        [](const auto& a, const auto& b) {
                                            return a.second < b.second;
                                        });
        
        if (maxSource != sourceCounts.end()) {
            stats.mostActiveSource = maxSource->first;
        }
        
        return stats;
    }
    
    // 모든 로그 출력
    void printAll() const {
        std::cout << "=== All Logs (" << logs_.size() << " entries) ===" << std::endl;
        for (size_t i = 0; i < logs_.size(); ++i) {
            std::cout << "[" << i << "] " << logs_[i].toString();
        }
    }
};
```

### std::deque - 양단 큐

양쪽 끝에서 효율적인 삽입/삭제가 가능한 컨테이너입니다.

```cpp
#include <deque>
#include <mutex>

class CircularLogBuffer {
private:
    std::deque<LogEntry> buffer_;
    size_t maxSize_;
    mutable std::mutex mutex_;

public:
    CircularLogBuffer(size_t maxSize = 1000) : maxSize_(maxSize) {}
    
    // 로그 추가 (최대 크기 초과 시 오래된 로그 제거)
    void addLog(const std::string& message, const std::string& level, const std::string& source) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        buffer_.emplace_back(message, level, source);
        
        if (buffer_.size() > maxSize_) {
            buffer_.pop_front();  // 가장 오래된 로그 제거
        }
    }
    
    // 최신 로그 가져오기
    LogEntry getLatestLog() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (buffer_.empty()) {
            throw std::runtime_error("Buffer is empty");
        }
        
        return buffer_.back();
    }
    
    // 가장 오래된 로그 가져오기
    LogEntry getOldestLog() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (buffer_.empty()) {
            throw std::runtime_error("Buffer is empty");
        }
        
        return buffer_.front();
    }
    
    // 최근 N개 로그
    std::vector<LogEntry> getRecentLogs(size_t count) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<LogEntry> result;
        
        size_t startIndex = (count >= buffer_.size()) ? 0 : buffer_.size() - count;
        
        for (size_t i = startIndex; i < buffer_.size(); ++i) {
            result.push_back(buffer_[i]);
        }
        
        return result;
    }
    
    // 버퍼 상태 정보
    struct BufferInfo {
        size_t currentSize;
        size_t maxSize;
        bool isFull;
        double utilization;
    };
    
    BufferInfo getBufferInfo() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        BufferInfo info;
        info.currentSize = buffer_.size();
        info.maxSize = maxSize_;
        info.isFull = (buffer_.size() == maxSize_);
        info.utilization = static_cast<double>(buffer_.size()) / maxSize_;
        
        return info;
    }
    
    // 크기 조정
    void resize(size_t newMaxSize) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        maxSize_ = newMaxSize;
        
        while (buffer_.size() > maxSize_) {
            buffer_.pop_front();
        }
    }
    
    // 버퍼 비우기
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_.clear();
    }
};
```

### 🗺️ `std::map`: "똑똑한 전화번호부"

`map`을 **전화번호부**에 비유해보겠습니다!

📞 **map = 전화번호부**
- **키(이름) → 값(전화번호)**: "김철수" → "010-1234-5678"
- **자동 정렬**: 이름이 가나다순으로 자동 정렬돼요
- **빠른 검색**: 이름만 알면 전화번호를 빠르게 찾아요
- **중복 불가**: 같은 이름은 하나만 등록할 수 있어요

🔍 **언제 map을 사용하나요?**
- **키로 빠르게 찾기**: "ERROR 로그만 보여줘!"
- **분류별 저장**: IP주소별 접속 횟수, 사용자별 로그 등
- **자동 정렬**: 키가 자동으로 정렬되어 있으면 좋을 때

```cpp
// 🗺️ map 사용 예시
std::map<std::string, int> errorCount;  // 에러 종류별 개수

// 추가 (자동으로 정렬됨)
errorCount["NetworkError"] = 5;      // 🌐 네트워크 에러 5개
errorCount["DatabaseError"] = 3;     // 💾 데이터베이스 에러 3개  
errorCount["AuthError"] = 8;         // 🔐 인증 에러 8개

// 검색 (빠름!)
cout << "Auth 에러: " << errorCount["AuthError"] << "개" << endl;

// 순회 (자동으로 알파벳 순서로!)
for (const auto& pair : errorCount) {
    cout << pair.first << ": " << pair.second << "개" << endl;
}
// 출력: AuthError: 8개, DatabaseError: 3개, NetworkError: 5개
```

```cpp
#include <map>
#include <unordered_map>

class LogIndex {
private:
    std::vector<LogEntry> logs_;
    std::map<std::string, std::vector<size_t>> levelIndex_;     // 레벨별 인덱스
    std::map<std::string, std::vector<size_t>> sourceIndex_;    // 소스별 인덱스
    std::unordered_map<std::string, std::vector<size_t>> keywordIndex_;  // 키워드별 인덱스

public:
    // 로그 추가 및 인덱스 업데이트
    void addLog(const std::string& message, const std::string& level, const std::string& source) {
        size_t index = logs_.size();
        logs_.emplace_back(message, level, source);
        
        // 인덱스 업데이트
        levelIndex_[level].push_back(index);
        sourceIndex_[source].push_back(index);
        
        // 메시지에서 키워드 추출 및 인덱스 업데이트
        updateKeywordIndex(message, index);
    }
    
private:
    void updateKeywordIndex(const std::string& message, size_t index) {
        // 간단한 키워드 추출 (공백으로 분리)
        std::istringstream iss(message);
        std::string word;
        
        while (iss >> word) {
            // 특수 문자 제거 및 소문자 변환
            std::string cleanWord;
            for (char c : word) {
                if (std::isalnum(c)) {
                    cleanWord += std::tolower(c);
                }
            }
            
            if (!cleanWord.empty() && cleanWord.length() > 2) {  // 3글자 이상만
                keywordIndex_[cleanWord].push_back(index);
            }
        }
    }

public:
    // 레벨별 검색
    std::vector<LogEntry> getLogsByLevel(const std::string& level) const {
        std::vector<LogEntry> results;
        
        auto it = levelIndex_.find(level);
        if (it != levelIndex_.end()) {
            for (size_t index : it->second) {
                results.push_back(logs_[index]);
            }
        }
        
        return results;
    }
    
    // 소스별 검색
    std::vector<LogEntry> getLogsBySource(const std::string& source) const {
        std::vector<LogEntry> results;
        
        auto it = sourceIndex_.find(source);
        if (it != sourceIndex_.end()) {
            for (size_t index : it->second) {
                results.push_back(logs_[index]);
            }
        }
        
        return results;
    }
    
    // 키워드별 검색 (빠른 검색)
    std::vector<LogEntry> getLogsByKeyword(const std::string& keyword) const {
        std::vector<LogEntry> results;
        
        std::string cleanKeyword;
        for (char c : keyword) {
            if (std::isalnum(c)) {
                cleanKeyword += std::tolower(c);
            }
        }
        
        auto it = keywordIndex_.find(cleanKeyword);
        if (it != keywordIndex_.end()) {
            for (size_t index : it->second) {
                results.push_back(logs_[index]);
            }
        }
        
        return results;
    }
    
    // 복합 검색 (AND 조건)
    std::vector<LogEntry> searchLogs(const std::string& level = "", 
                                   const std::string& source = "",
                                   const std::string& keyword = "") const {
        std::set<size_t> resultIndices;
        bool firstCondition = true;
        
        // 레벨 조건
        if (!level.empty()) {
            auto it = levelIndex_.find(level);
            if (it != levelIndex_.end()) {
                if (firstCondition) {
                    resultIndices.insert(it->second.begin(), it->second.end());
                    firstCondition = false;
                } else {
                    std::set<size_t> levelSet(it->second.begin(), it->second.end());
                    std::set<size_t> intersection;
                    std::set_intersection(resultIndices.begin(), resultIndices.end(),
                                        levelSet.begin(), levelSet.end(),
                                        std::inserter(intersection, intersection.begin()));
                    resultIndices = intersection;
                }
            } else {
                return {};  // 조건에 맞는 로그 없음
            }
        }
        
        // 소스 조건
        if (!source.empty()) {
            auto it = sourceIndex_.find(source);
            if (it != sourceIndex_.end()) {
                if (firstCondition) {
                    resultIndices.insert(it->second.begin(), it->second.end());
                    firstCondition = false;
                } else {
                    std::set<size_t> sourceSet(it->second.begin(), it->second.end());
                    std::set<size_t> intersection;
                    std::set_intersection(resultIndices.begin(), resultIndices.end(),
                                        sourceSet.begin(), sourceSet.end(),
                                        std::inserter(intersection, intersection.begin()));
                    resultIndices = intersection;
                }
            } else {
                return {};
            }
        }
        
        // 키워드 조건
        if (!keyword.empty()) {
            std::string cleanKeyword;
            for (char c : keyword) {
                if (std::isalnum(c)) {
                    cleanKeyword += std::tolower(c);
                }
            }
            
            auto it = keywordIndex_.find(cleanKeyword);
            if (it != keywordIndex_.end()) {
                if (firstCondition) {
                    resultIndices.insert(it->second.begin(), it->second.end());
                } else {
                    std::set<size_t> keywordSet(it->second.begin(), it->second.end());
                    std::set<size_t> intersection;
                    std::set_intersection(resultIndices.begin(), resultIndices.end(),
                                        keywordSet.begin(), keywordSet.end(),
                                        std::inserter(intersection, intersection.begin()));
                    resultIndices = intersection;
                }
            } else {
                return {};
            }
        }
        
        // 결과 구성
        std::vector<LogEntry> results;
        for (size_t index : resultIndices) {
            results.push_back(logs_[index]);
        }
        
        return results;
    }
    
    // 통계 정보
    void printIndexStatistics() const {
        std::cout << "=== Index Statistics ===" << std::endl;
        std::cout << "Total logs: " << logs_.size() << std::endl;
        std::cout << "Indexed levels: " << levelIndex_.size() << std::endl;
        std::cout << "Indexed sources: " << sourceIndex_.size() << std::endl;
        std::cout << "Indexed keywords: " << keywordIndex_.size() << std::endl;
        
        std::cout << "\nLevel distribution:" << std::endl;
        for (const auto& pair : levelIndex_) {
            std::cout << "  " << pair.first << ": " << pair.second.size() << " logs" << std::endl;
        }
        
        std::cout << "\nSource distribution:" << std::endl;
        for (const auto& pair : sourceIndex_) {
            std::cout << "  " << pair.first << ": " << pair.second.size() << " logs" << std::endl;
        }
    }
};
```

---

## 📦 3. 버퍼와 큐의 개념

### 원형 버퍼 (Circular Buffer)

고정 크기의 효율적인 버퍼 구조입니다.

```cpp
template<typename T>
class CircularBuffer {
private:
    std::vector<T> buffer_;
    size_t head_;
    size_t tail_;
    size_t size_;
    size_t capacity_;
    bool full_;

public:
    explicit CircularBuffer(size_t capacity) 
        : buffer_(capacity), head_(0), tail_(0), size_(0), capacity_(capacity), full_(false) {}
    
    // 요소 추가
    void push(const T& item) {
        buffer_[tail_] = item;
        
        if (full_) {
            head_ = (head_ + 1) % capacity_;
        } else {
            size_++;
        }
        
        tail_ = (tail_ + 1) % capacity_;
        full_ = (tail_ == head_);
    }
    
    // 요소 제거
    T pop() {
        if (empty()) {
            throw std::runtime_error("Buffer is empty");
        }
        
        T item = buffer_[head_];
        head_ = (head_ + 1) % capacity_;
        full_ = false;
        size_--;
        
        return item;
    }
    
    // 상태 확인
    bool empty() const { return !full_ && (head_ == tail_); }
    bool isFull() const { return full_; }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    
    // 첫 번째/마지막 요소 확인 (제거하지 않음)
    const T& front() const {
        if (empty()) throw std::runtime_error("Buffer is empty");
        return buffer_[head_];
    }
    
    const T& back() const {
        if (empty()) throw std::runtime_error("Buffer is empty");
        size_t lastIndex = (tail_ == 0) ? capacity_ - 1 : tail_ - 1;
        return buffer_[lastIndex];
    }
    
    // 모든 요소에 접근 (인덱스 기반)
    const T& at(size_t index) const {
        if (index >= size_) throw std::out_of_range("Index out of range");
        size_t realIndex = (head_ + index) % capacity_;
        return buffer_[realIndex];
    }
    
    // 반복자 지원을 위한 클래스
    class iterator {
    private:
        const CircularBuffer* buffer_;
        size_t index_;
        
    public:
        iterator(const CircularBuffer* buffer, size_t index) 
            : buffer_(buffer), index_(index) {}
        
        const T& operator*() const {
            return buffer_->at(index_);
        }
        
        iterator& operator++() {
            ++index_;
            return *this;
        }
        
        bool operator!=(const iterator& other) const {
            return index_ != other.index_;
        }
    };
    
    iterator begin() const { return iterator(this, 0); }
    iterator end() const { return iterator(this, size_); }
};
```

### 우선순위 큐 (Priority Queue)

로그의 중요도에 따라 처리 순서를 결정하는 큐입니다.

```cpp
#include <queue>
#include <functional>

class PriorityLogEntry {
public:
    LogEntry logEntry;
    int priority;  // 높을수록 우선순위 높음
    
    PriorityLogEntry(const LogEntry& entry, int prio) 
        : logEntry(entry), priority(prio) {}
    
    // 우선순위 큐를 위한 비교 연산자 (낮은 우선순위가 top이 되도록)
    bool operator<(const PriorityLogEntry& other) const {
        return priority < other.priority;
    }
};

class PriorityLogQueue {
private:
    std::priority_queue<PriorityLogEntry> queue_;
    
    // 로그 레벨에 따른 우선순위 결정
    int getPriorityByLevel(const std::string& level) {
        if (level == "CRITICAL") return 5;
        if (level == "ERROR") return 4;
        if (level == "WARNING") return 3;
        if (level == "INFO") return 2;
        if (level == "DEBUG") return 1;
        return 0;
    }
    
public:
    // 로그 추가
    void addLog(const LogEntry& logEntry) {
        int priority = getPriorityByLevel(logEntry.getLevel());
        queue_.emplace(logEntry, priority);
    }
    
    // 커스텀 우선순위로 로그 추가
    void addLog(const LogEntry& logEntry, int customPriority) {
        queue_.emplace(logEntry, customPriority);
    }
    
    // 가장 높은 우선순위 로그 가져오기
    LogEntry getHighestPriorityLog() {
        if (queue_.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        
        PriorityLogEntry topEntry = queue_.top();
        queue_.pop();
        return topEntry.logEntry;
    }
    
    // 큐 상태 확인
    bool empty() const { return queue_.empty(); }
    size_t size() const { return queue_.size(); }
    
    // 최고 우선순위 로그 확인 (제거하지 않음)
    const LogEntry& top() const {
        if (queue_.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return queue_.top().logEntry;
    }
    
    // 모든 로그 처리
    void processAllLogs() {
        while (!queue_.empty()) {
            LogEntry log = getHighestPriorityLog();
            
            // 로그 처리 (예: 파일에 쓰기, 네트워크 전송 등)
            std::cout << "Processing: " << log.toString() << std::endl;
        }
    }
};
```

---

## 🏗️ 4. LogCaster 특화 데이터 구조

### 시간 기반 로그 파티션

시간대별로 로그를 분할하여 효율적인 검색을 지원합니다.

```cpp
#include <chrono>
#include <map>

class TimePartitionedLogStorage {
private:
    // 시간 파티션 (1시간 단위)
    std::map<std::chrono::hours, std::vector<LogEntry>> partitions_;
    std::chrono::hours partitionSize_;
    
    // 파티션 키 계산
    std::chrono::hours getPartitionKey(const std::chrono::system_clock::time_point& timestamp) const {
        auto duration = timestamp.time_since_epoch();
        auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
        return std::chrono::hours(hours.count() / partitionSize_.count() * partitionSize_.count());
    }

public:
    TimePartitionedLogStorage(std::chrono::hours partitionSize = std::chrono::hours(1))
        : partitionSize_(partitionSize) {}
    
    // 로그 추가
    void addLog(const LogEntry& logEntry) {
        auto partitionKey = getPartitionKey(logEntry.getTimestamp());
        partitions_[partitionKey].push_back(logEntry);
    }
    
    // 시간 범위로 검색
    std::vector<LogEntry> getLogsByTimeRange(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) const {
        
        std::vector<LogEntry> results;
        
        auto startPartition = getPartitionKey(start);
        auto endPartition = getPartitionKey(end);
        
        // 관련 파티션들을 순회
        for (auto it = partitions_.lower_bound(startPartition);
             it != partitions_.end() && it->first <= endPartition; ++it) {
            
            for (const auto& log : it->second) {
                auto timestamp = log.getTimestamp();
                if (timestamp >= start && timestamp <= end) {
                    results.push_back(log);
                }
            }
        }
        
        return results;
    }
    
    // 특정 시간대의 로그들
    std::vector<LogEntry> getLogsInHour(const std::chrono::system_clock::time_point& time) const {
        auto partitionKey = getPartitionKey(time);
        
        auto it = partitions_.find(partitionKey);
        if (it != partitions_.end()) {
            return it->second;
        }
        
        return {};
    }
    
    // 파티션 통계
    struct PartitionStats {
        size_t totalPartitions;
        size_t totalLogs;
        size_t avgLogsPerPartition;
        std::chrono::hours oldestPartition;
        std::chrono::hours newestPartition;
    };
    
    PartitionStats getPartitionStats() const {
        PartitionStats stats{};
        stats.totalPartitions = partitions_.size();
        
        if (!partitions_.empty()) {
            stats.oldestPartition = partitions_.begin()->first;
            stats.newestPartition = partitions_.rbegin()->first;
            
            for (const auto& partition : partitions_) {
                stats.totalLogs += partition.second.size();
            }
            
            stats.avgLogsPerPartition = stats.totalLogs / stats.totalPartitions;
        }
        
        return stats;
    }
    
    // 오래된 파티션 정리
    void cleanupOldPartitions(const std::chrono::system_clock::time_point& cutoffTime) {
        auto cutoffPartition = getPartitionKey(cutoffTime);
        
        auto it = partitions_.begin();
        while (it != partitions_.end()) {
            if (it->first < cutoffPartition) {
                it = partitions_.erase(it);
            } else {
                break;
            }
        }
    }
};
```

### 멀티레벨 캐시

자주 검색되는 로그를 여러 단계의 캐시에 저장합니다.

```cpp
template<typename K, typename V>
class LRUCache {
private:
    struct Node {
        K key;
        V value;
        Node* prev;
        Node* next;
        
        Node(const K& k, const V& v) : key(k), value(v), prev(nullptr), next(nullptr) {}
    };
    
    size_t capacity_;
    std::unordered_map<K, Node*> cache_;
    Node* head_;
    Node* tail_;
    
    void addToHead(Node* node) {
        node->prev = head_;
        node->next = head_->next;
        head_->next->prev = node;
        head_->next = node;
    }
    
    void removeNode(Node* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    
    void moveToHead(Node* node) {
        removeNode(node);
        addToHead(node);
    }
    
    Node* removeTail() {
        Node* lastNode = tail_->prev;
        removeNode(lastNode);
        return lastNode;
    }

public:
    explicit LRUCache(size_t capacity) : capacity_(capacity) {
        head_ = new Node(K{}, V{});
        tail_ = new Node(K{}, V{});
        head_->next = tail_;
        tail_->prev = head_;
    }
    
    ~LRUCache() {
        auto current = head_;
        while (current) {
            auto next = current->next;
            delete current;
            current = next;
        }
    }
    
    std::optional<V> get(const K& key) {
        auto it = cache_.find(key);
        if (it == cache_.end()) {
            return std::nullopt;
        }
        
        Node* node = it->second;
        moveToHead(node);
        return node->value;
    }
    
    void put(const K& key, const V& value) {
        auto it = cache_.find(key);
        
        if (it == cache_.end()) {
            Node* newNode = new Node(key, value);
            
            if (cache_.size() >= capacity_) {
                Node* tail = removeTail();
                cache_.erase(tail->key);
                delete tail;
            }
            
            cache_[key] = newNode;
            addToHead(newNode);
        } else {
            Node* node = it->second;
            node->value = value;
            moveToHead(node);
        }
    }
};

class MultiLevelLogCache {
private:
    LRUCache<std::string, std::vector<LogEntry>> searchCache_;   // 검색 결과 캐시
    LRUCache<std::string, LogEntry> singleLogCache_;            // 단일 로그 캐시
    LRUCache<std::string, size_t> countCache_;                  // 개수 캐시
    
public:
    MultiLevelLogCache() 
        : searchCache_(100), singleLogCache_(1000), countCache_(50) {}
    
    // 검색 결과 캐싱
    std::optional<std::vector<LogEntry>> getCachedSearch(const std::string& query) {
        return searchCache_.get(query);
    }
    
    void cacheSearchResult(const std::string& query, const std::vector<LogEntry>& results) {
        searchCache_.put(query, results);
    }
    
    // 단일 로그 캐싱
    std::optional<LogEntry> getCachedLog(const std::string& logId) {
        return singleLogCache_.get(logId);
    }
    
    void cacheLog(const std::string& logId, const LogEntry& log) {
        singleLogCache_.put(logId, log);
    }
    
    // 개수 캐싱
    std::optional<size_t> getCachedCount(const std::string& query) {
        return countCache_.get(query);
    }
    
    void cacheCount(const std::string& query, size_t count) {
        countCache_.put(query, count);
    }
};
```

---

## ✅ 5. 다음 단계 준비

이 문서를 완전히 이해했다면:

1. **C의 연결 리스트 구현**을 할 수 있어야 합니다
2. **C++의 STL 컨테이너**를 적절히 선택하고 사용할 수 있어야 합니다
3. **버퍼와 큐의 개념**을 이해하고 구현할 수 있어야 합니다
4. **LogCaster에 특화된 데이터 구조**를 설계할 수 있어야 합니다
5. **성능과 메모리 사용량**을 고려한 데이터 구조를 선택할 수 있어야 합니다

### 🎯 확인 문제

1. 연결 리스트와 배열의 장단점은 무엇이며, LogCaster에서 언제 어떤 것을 사용해야 할까요?

2. std::vector와 std::deque의 차이점은 무엇이고, 어떤 상황에서 어떤 것을 선택해야 할까요?

3. 대용량 로그 데이터를 효율적으로 검색하기 위한 인덱싱 전략은 무엇인가요?

다음 문서에서는 **I/O 멀티플렉싱**에 대해 자세히 알아보겠습니다!

## 🚨 자주 하는 실수와 해결법

### 1. 연결 리스트에서 메모리 누수
**문제**: 노드를 삭제할 때 메모리 해제를 잊음
```c
// ❌ 잘못된 코드
void remove_node(LogList* list, LogNode* node) {
    // 리스트에서 노드만 제거하고 메모리는 해제하지 않음
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    // free(node) 누락!
}

// ✅ 올바른 코드
void remove_node(LogList* list, LogNode* node) {
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    free(node);  // 반드시 메모리 해제
    list->count--;
}
```

### 2. STL 컨테이너 잘못된 반복자 사용
**문제**: 컨테이너 수정 중 반복자 무효화
```cpp
// ❌ 위험한 코드
std::vector<LogEntry> logs;
for (auto it = logs.begin(); it != logs.end(); ++it) {
    if (it->getLevel() == "DEBUG") {
        logs.erase(it);  // 반복자 무효화!
    }
}

// ✅ 안전한 코드
std::vector<LogEntry> logs;
for (auto it = logs.begin(); it != logs.end(); ) {
    if (it->getLevel() == "DEBUG") {
        it = logs.erase(it);  // erase는 다음 유효한 반복자 반환
    } else {
        ++it;
    }
}

// ✅ 더 나은 방법 (C++11)
logs.erase(std::remove_if(logs.begin(), logs.end(),
    [](const LogEntry& log) { return log.getLevel() == "DEBUG"; }),
    logs.end());
```

### 3. 원형 버퍼 오버플로우
**문제**: 버퍼가 가득 찬 상태 처리 실패
```c
// ❌ 위험한 코드
void circular_buffer_push(CircularBuffer* buf, void* data) {
    memcpy(buf->data[buf->tail], data, buf->item_size);
    buf->tail = (buf->tail + 1) % buf->capacity;
    // 버퍼가 가득 찼을 때 head 업데이트 누락!
}

// ✅ 올바른 코드
void circular_buffer_push(CircularBuffer* buf, void* data) {
    memcpy(buf->data[buf->tail], data, buf->item_size);
    
    if (buf->is_full) {
        buf->head = (buf->head + 1) % buf->capacity;
    }
    
    buf->tail = (buf->tail + 1) % buf->capacity;
    buf->is_full = (buf->tail == buf->head);
}
```

### 4. map/unordered_map 키 존재 확인
**문제**: 없는 키에 접근하여 의도치 않은 삽입
```cpp
// ❌ 잘못된 코드
std::map<std::string, int> logCounts;
if (logCounts["ERROR"] > 0) {  // "ERROR" 키가 없으면 0으로 생성됨
    // ...
}

// ✅ 올바른 코드
std::map<std::string, int> logCounts;
if (logCounts.find("ERROR") != logCounts.end() && logCounts["ERROR"] > 0) {
    // ...
}

// ✅ C++20의 contains 사용
if (logCounts.contains("ERROR") && logCounts["ERROR"] > 0) {
    // ...
}
```

## 💡 실습 프로젝트

### 프로젝트 1: 로그 분석기 구현
```c
// 다음 기능을 구현하세요:
// 1. 연결 리스트 기반 로그 저장소
// 2. 해시 테이블 기반 빠른 검색
// 3. 우선순위 큐를 이용한 중요 로그 처리

typedef struct LogAnalyzer {
    LogList* allLogs;           // 모든 로그 저장
    HashTable* indexByKeyword;  // 키워드 인덱스
    PriorityQueue* criticalLogs; // 중요 로그 큐
} LogAnalyzer;

// 구현해야 할 함수들
LogAnalyzer* create_analyzer();
void analyze_log(LogAnalyzer* analyzer, const char* logLine);
LogEntry** search_logs(LogAnalyzer* analyzer, const char* keyword);
LogEntry* get_next_critical_log(LogAnalyzer* analyzer);
void print_statistics(LogAnalyzer* analyzer);
```

### 프로젝트 2: 실시간 로그 모니터 (C++)
```cpp
// 다음 클래스를 완성하세요:
class RealtimeLogMonitor {
private:
    CircularBuffer<LogEntry> recentLogs;      // 최근 로그
    std::map<std::string, size_t> errorCounts; // 에러 통계
    TimePartitionedStorage partitions;         // 시간별 파티션
    
public:
    // 구현해야 할 메서드들
    void processLog(const std::string& logLine);
    std::vector<LogEntry> getRecentErrors(size_t count);
    void generateHourlyReport();
    void alertOnThreshold(const std::string& errorType, size_t threshold);
};
```

### 프로젝트 3: 로그 캐시 시스템
```cpp
// LRU 캐시를 이용한 로그 캐싱 시스템
template<typename K, typename V>
class LogCacheSystem {
    // 구현 내용:
    // 1. LRU 캐시 구현
    // 2. 캐시 히트율 측정
    // 3. 캐시 워밍업 전략
    // 4. 멀티스레드 안전성
};
```

## 📚 추가 학습 자료

### 책 추천
- "Introduction to Algorithms" - CLRS (자료구조 이론)
- "The C++ Standard Library" - Nicolai Josuttis (STL 깊이 있는 학습)
- "Algorithms in C" - Robert Sedgewick (C 구현)

### 온라인 자료
- [cppreference.com - Containers](https://en.cppreference.com/w/cpp/container)
- [GeeksforGeeks - Data Structures](https://www.geeksforgeeks.org/data-structures/)
- [Visualgo - 자료구조 시각화](https://visualgo.net/en)

## ✅ 학습 체크리스트

### 기초 (1-2주)
- [ ] 연결 리스트 직접 구현 (C)
- [ ] 동적 배열 구현
- [ ] 스택과 큐 구현
- [ ] 시간 복잡도 이해

### 중급 (3-4주)
- [ ] STL vector, list, deque 활용
- [ ] map과 unordered_map 차이 이해
- [ ] 원형 버퍼 구현
- [ ] 우선순위 큐 활용

### 고급 (5-6주)
- [ ] 커스텀 allocator 작성
- [ ] 해시 테이블 직접 구현
- [ ] B-트리 이해
- [ ] 락프리 자료구조 기초

### 전문가 (7-8주)
- [ ] 시간/공간 복잡도 최적화
- [ ] 캐시 친화적 자료구조
- [ ] 분산 자료구조
- [ ] 대용량 데이터 처리

## 🔄 다음 학습 단계

이 문서를 완료했다면 다음 문서로 진행하세요:

1. **[03. OOP_CPP_Guide.md](03.%20OOP_CPP_Guide.md)** - C++ 객체지향 프로그래밍
   - 클래스와 객체
   - 상속과 다형성
   - 템플릿 프로그래밍

2. **[07. Networking.md](07.%20Networking.md)** - 네트워크 프로그래밍
   - 소켓 프로그래밍
   - 비동기 I/O
   - 프로토콜 설계

3. **[09. IOMultiplexing.md](09.%20IOMultiplexing.md)** - I/O 멀티플렉싱
   - select/poll/epoll
   - 이벤트 기반 프로그래밍
   - 고성능 서버 설계

---

*💡 팁: 데이터 구조 선택은 성능에 큰 영향을 미칩니다. 항상 용도에 맞는 적절한 구조를 선택하세요!*