# 8. 객체 지향 프로그래밍 (C++)
## 레고 블록처럼 조립하는 프로그래밍 🧱

*LogCaster 프로젝트를 위한 사전지식 문서*

---

## 📋 문서 정보
- **난이도**: 🟡 중급 → 🔴 고급
- **예상 학습 시간**: 8-10시간
- **전제 조건**: 
  - C++ 기본 문법 (변수, 함수, 포인터)
  - 메모리 관리 개념
  - 컴파일과 링킹 과정 이해
- **실습 환경**: G++ 컴파일러 (C++11 이상), IDE (선택사항)

## 🎯 이 문서에서 배울 내용

**"객체 지향? 그게 대체 뭔가요?"**

좋은 질문이에요! 객체 지향 프로그래밍(OOP)을 **레고 놀이**에 비유해보겠습니다.

🧱 **OOP = 똑똑한 레고 놀이**
- **레고 블록** = 객체 (각각이 고유한 기능을 가진 부품)
- **설계도** = 클래스 (어떤 블록을 어떻게 만들지 정한 계획)
- **완성작** = 프로그램 (블록들을 조립해서 만든 결과물)

일반 레고와 다른 점? 이 블록들은 **살아있어요**! 스스로 생각하고, 서로 소통하고, 필요할 때 도움을 주고받습니다.

### 📚 왜 객체 지향 프로그래밍인가?

🔹 **직관적**: 현실 세계와 비슷하게 생각할 수 있어요
🔹 **재사용성**: 한 번 만든 코드를 여러 곳에서 사용
🔹 **유지보수**: 문제가 생기면 해당 부품만 고치면 됨
🔹 **확장성**: 새로운 기능을 쉽게 추가 가능

### 🎮 LogCaster에서 OOP 활용 예시

```
🏢 LogCaster 서버 (전체 프로그램)
├── 📝 LogEntry (로그 데이터를 담는 객체)
├── 💾 LogStorage (로그를 저장하는 객체)
├── 🔍 LogFilter (로그를 필터링하는 객체)
├── 📤 LogSender (로그를 전송하는 객체)
└── 🎛️ LogProcessor (로그를 처리하는 객체)
```

---

## 🏗️ 1. 클래스와 객체

### 🎨 클래스란 무엇인가?

클래스는 "붕어빵 틀"과 같습니다:
- **클래스** = 붕어빵 틀 (설계도)
- **객체** = 붕어빵 (실제 만들어진 것)

하나의 틀(클래스)로 여러 개의 붕어빵(객체)을 만들 수 있죠!

### 🌟 클래스를 만드는 이유: 현실 세계의 모델링

프로그래밍에서 클래스를 사용하는 이유를 **자동차 공장**에 비유해보겠습니다:

```
🏭 자동차 공장 (클래스)
├── 📋 설계도: 엔진, 바퀴 4개, 문 4개...
├── 🔧 제조 과정: 조립, 도색, 검사...
└── 📦 완성품: 실제 자동차 (객체)

하나의 설계도로:
🚗 빨간 소나타 (객체1)
🚙 파란 아반떼 (객체2)  
🚘 흰색 그랜저 (객체3)
```

**왜 이렇게 할까요?**
- ✅ **재사용성**: 설계도 하나로 여러 자동차 제작
- ✅ **일관성**: 모든 자동차가 동일한 기본 구조
- ✅ **관리 용이**: 설계 변경 시 한 곳만 수정
- ✅ **확장성**: 새로운 기능 쉽게 추가

### 🧱 클래스의 핵심 구성 요소

클래스는 **데이터(멤버 변수)**와 **기능(멤버 함수)**을 하나로 묶습니다:

```cpp
// 🏠 간단한 집 클래스로 이해해보기
class House {
private:  // 🔒 집 안에만 있는 것들
    int rooms_;           // 방 개수
    double size_;         // 집 크기
    bool has_garage_;     // 차고 유무
    
public:   // 🌍 외부에서 접근 가능한 것들
    // 🏗️ 생성자: 집을 지을 때 호출
    House(int rooms, double size, bool garage) 
        : rooms_(rooms), size_(size), has_garage_(garage) {
        std::cout << "새 집이 지어졌습니다!" << std::endl;
    }
    
    // 🚪 public 메서드: 집의 기능들
    void openDoor() {
        std::cout << "문을 열었습니다." << std::endl;
    }
    
    int getRooms() const { return rooms_; }  // 방 개수 확인
    double getSize() const { return size_; }  // 집 크기 확인
    
    void addRoom() {  // 방 추가 (리모델링)
        rooms_++;
        std::cout << "방이 하나 추가되었습니다. 현재 " << rooms_ << "개" << std::endl;
    }
};

// 🎯 사용 예시: 다양한 집 객체 만들기
void house_example() {
    // 객체 생성: 생성자가 자동 호출됨
    House myHouse(3, 120.5, true);    // 3방, 120.5㎡, 차고 있음
    House neighborHouse(2, 85.0, false); // 2방, 85㎡, 차고 없음
    
    // 각 집의 고유한 상태와 동작
    std::cout << "내 집 방 개수: " << myHouse.getRooms() << std::endl;
    std::cout << "이웃집 크기: " << neighborHouse.getSize() << "㎡" << std::endl;
    
    myHouse.openDoor();        // 내 집 문 열기
    myHouse.addRoom();         // 내 집에 방 추가
    
    neighborHouse.openDoor();  // 이웃집 문 열기
    // neighborHouse.rooms_ = 5;  // ❌ 컴파일 에러! private 멤버
}
```

### 🏠 클래스의 구조

클래스는 세 가지 주요 구성 요소를 가집니다:

```cpp
class 집 {
private:    // 🔒 비공개 (집 안에만 있는 것)
    int 금고;
    
protected:  // 🔐 보호 (가족만 접근 가능)
    int 가족사진;
    
public:     // 🌍 공개 (누구나 접근 가능)
    int 우편함;
};
```

### 클래스 기본 구조

```cpp
#include <string>
#include <chrono>
#include <iostream>

// 🎯 LogCaster의 기본 로그 엔트리 클래스
// 로그 하나하나를 표현하는 객체입니다
class LogEntry {
private:
    // 🔒 Private 멤버 변수 (외부에서 직접 접근 불가)
    std::string message_;      // 로그 메시지 내용
    std::string level_;        // 로그 레벨 (INFO, ERROR 등)
    std::string source_;       // 로그 발생 위치 (IP 주소 등)
    std::chrono::system_clock::time_point timestamp_;  // 로그 발생 시간
    static int total_entries_; // 🌍 모든 객체가 공유하는 변수 (총 로그 개수)

public:
    // 🏗️ 기본 생성자 (객체를 만들 때 자동 호출)
    // 초기화 리스트를 사용한 효율적인 초기화
    LogEntry() 
        : message_(""),        // 빈 메시지로 초기화
          level_("INFO"),      // 기본 레벨은 INFO
          source_("unknown"),  // 출처 불명
          timestamp_(std::chrono::system_clock::now()) {  // 현재 시간
        total_entries_++;      // 전체 로그 개수 증가
    }
    
    // 🎨 매개변수 생성자 (원하는 값으로 초기화)
    LogEntry(const std::string& message,    // 로그 메시지
             const std::string& level,      // 로그 레벨
             const std::string& source)      // 로그 출처
        : message_(message), 
          level_(level), 
          source_(source),
          timestamp_(std::chrono::system_clock::now()) {
        total_entries_++;
    }
    
    // 📋 복사 생성자 (다른 객체를 복사해서 새 객체 생성)
    // LogEntry log2(log1); 처럼 사용
    LogEntry(const LogEntry& other)
        : message_(other.message_),     // other의 메시지 복사
          level_(other.level_),         // other의 레벨 복사
          source_(other.source_),       // other의 출처 복사
          timestamp_(other.timestamp_) {// other의 시간 복사
        total_entries_++;  // 복사해도 새로운 객체이므로 카운트 증가
    }
    
    // 🚀 이동 생성자 (C++11) - 성능 최적화
    // 복사 대신 데이터를 "이동"시켜 더 빠름
    LogEntry(LogEntry&& other) noexcept  // noexcept: 예외를 던지지 않음
        : message_(std::move(other.message_)),   // 문자열 내용을 이동
          level_(std::move(other.level_)),       // 복사하지 않고 이동
          source_(std::move(other.source_)),     // 메모리 절약
          timestamp_(other.timestamp_) {         // 기본 타입은 그냥 복사
        total_entries_++;
    }
    
    // 💥 소멸자 (객체가 사라질 때 자동 호출)
    // 정리 작업 수행 (메모리 해제, 카운터 감소 등)
    ~LogEntry() {
        total_entries_--;  // 전체 로그 개수 감소
    }
    
    // 복사 할당 연산자
    LogEntry& operator=(const LogEntry& other) {
        if (this != &other) {
            message_ = other.message_;
            level_ = other.level_;
            source_ = other.source_;
            timestamp_ = other.timestamp_;
        }
        return *this;
    }
    
    // 이동 할당 연산자 (C++11)
    LogEntry& operator=(LogEntry&& other) noexcept {
        if (this != &other) {
            message_ = std::move(other.message_);
            level_ = std::move(other.level_);
            source_ = std::move(other.source_);
            timestamp_ = other.timestamp_;
        }
        return *this;
    }
    
    // 📖 Getter 메서드들 (값을 읽기만 하는 메서드)
    // const가 붙어서 객체의 상태를 변경하지 않음을 보장
    const std::string& getMessage() const { return message_; }
    const std::string& getLevel() const { return level_; }
    const std::string& getSource() const { return source_; }
    const std::chrono::system_clock::time_point& getTimestamp() const { 
        return timestamp_; 
    }
    
    // ✏️ Setter 메서드들 (값을 변경하는 메서드)
    // private 변수에 안전하게 접근하는 방법
    void setMessage(const std::string& message) { message_ = message; }
    void setLevel(const std::string& level) { level_ = level; }
    void setSource(const std::string& source) { source_ = source; }
    
    // 🛠️ 유틸리티 메서드 (도우미 함수)
    std::string getFormattedTime() const {
        // chrono 시간을 읽기 쉬운 문자열로 변환
        auto time_t = std::chrono::system_clock::to_time_t(timestamp_);
        std::string time_str = std::ctime(&time_t);
        
        // ctime은 끝에 '\n'을 붙이므로 제거
        if (!time_str.empty() && time_str.back() == '\n') {
            time_str.pop_back();
        }
        return time_str;
    }
    
    std::string toString() const {
        return "[" + level_ + "] " + getFormattedTime() + " - " + source_ + ": " + message_;
    }
    
    // 🎮 연산자 오버로딩 (연산자에 새로운 의미 부여)
    // log1 < log2 처럼 직관적으로 비교 가능
    bool operator<(const LogEntry& other) const {
        return timestamp_ < other.timestamp_;  // 시간순으로 비교
    }
    
    // log1 == log2 로 내용이 같은지 확인
    bool operator==(const LogEntry& other) const {
        return message_ == other.message_ && 
               level_ == other.level_ && 
               source_ == other.source_;
        // 시간은 비교하지 않음 (같은 내용이면 동일한 로그로 간주)
    }
    
    // 🌍 정적 메서드 (객체 없이도 호출 가능)
    // LogEntry::getTotalEntries() 처럼 클래스 이름으로 호출
    static int getTotalEntries() { return total_entries_; }
    
    // 👥 친구 함수 (private 멤버에 접근 가능한 특별한 함수)
    // cout << log 처럼 직접 출력 가능하게 만듦
    friend std::ostream& operator<<(std::ostream& os, const LogEntry& entry) {
        os << entry.toString();
        return os;
    }
};

// 🌍 정적 멤버 변수 초기화 (클래스 외부에서 반드시 해야 함)
// 모든 LogEntry 객체가 공유하는 하나의 변수
int LogEntry::total_entries_ = 0;

// 🎯 클래스 사용 예시
void basic_class_usage() {
    // 1️⃣ 다양한 방법으로 객체 생성
    LogEntry log1;  // 기본 생성자 호출
    LogEntry log2("Server started", "INFO", "192.168.1.100");  // 값 지정
    LogEntry log3(log2);  // log2를 복사해서 log3 생성
    
    // 메서드 호출
    std::cout << log2.toString() << std::endl;
    std::cout << "Total entries: " << LogEntry::getTotalEntries() << std::endl;
    
    // 연산자 사용
    if (log1 < log2) {
        std::cout << "log1 is older than log2" << std::endl;
    }
    
    std::cout << log2 << std::endl;  // 친구 함수 사용
}
```

### 🔐 접근 제어 지시자: 보안과 캡슐화의 핵심

접근 제어자는 클래스의 "보안 수준"을 정하는 키워드입니다. **은행**에 비유해보겠습니다:

| 접근 제어자 | 설명 | 은행 비유 | 실제 사용 |
|------------|------|-----------|----------|
| `private` | 클래스 내부에서만 접근 | 🏦 금고실 (직원만 출입) | 내부 데이터, 헬퍼 함수 |
| `protected` | 클래스와 상속받은 클래스에서 접근 | 👔 직원 휴게실 (직원과 관리자만) | 상속용 멤버 |
| `public` | 어디서나 접근 가능 | 🚪 로비 (누구나 이용) | 인터페이스 메서드 |

#### 🎯 왜 접근 제어가 중요한가?

**실생활 예시**: 스마트폰을 생각해보세요
- 📱 **Public**: 화면, 버튼 (사용자가 조작)
- 🔧 **Protected**: 내부 설정 (개발자/관리자만)
- 🔒 **Private**: CPU, 메모리 (직접 접근 불가)

만약 모든 부품이 public이라면?
- 😱 사용자가 실수로 CPU를 만질 수 있음
- 💥 시스템이 불안정해짐
- 🐛 예측 불가능한 오류 발생

#### 📋 접근 제어의 실제 적용 예시

```cpp
// 🏦 은행 계좌 클래스: 접근 제어의 완벽한 예시
class BankAccount {
private:  // 🔒 절대 외부에서 직접 접근하면 안 되는 것들
    double balance_;          // 잔액 (직접 수정하면 큰일!)
    std::string account_number_; // 계좌번호
    std::string pin_;         // 비밀번호
    
    // 🛡️ Private 헬퍼 메서드 (내부 검증용)
    bool validatePin(const std::string& pin) const {
        return pin_ == pin;
    }
    
    bool hasSufficientFunds(double amount) const {
        return balance_ >= amount;
    }
    
protected:  // 🔐 상속받은 클래스(예: PremiumAccount)에서 사용
    double daily_limit_;      // 일일 한도
    int transaction_count_;   // 거래 횟수
    
    void logTransaction(const std::string& type, double amount) {
        std::cout << "[LOG] " << type << ": $" << amount 
                  << ", Balance: $" << balance_ << std::endl;
    }
    
public:   // 🌍 고객이 사용할 수 있는 인터페이스
    // 생성자
    BankAccount(const std::string& account_num, const std::string& pin, double initial = 0.0)
        : account_number_(account_num), pin_(pin), balance_(initial), 
          daily_limit_(1000.0), transaction_count_(0) {
        std::cout << "계좌가 개설되었습니다: " << account_num << std::endl;
    }
    
    // 안전한 인터페이스 메서드들
    bool deposit(double amount, const std::string& pin) {
        if (!validatePin(pin)) {
            std::cout << "❌ 잘못된 PIN입니다." << std::endl;
            return false;
        }
        
        if (amount <= 0) {
            std::cout << "❌ 유효하지 않은 금액입니다." << std::endl;
            return false;
        }
        
        balance_ += amount;
        transaction_count_++;
        logTransaction("DEPOSIT", amount);
        return true;
    }
    
    bool withdraw(double amount, const std::string& pin) {
        if (!validatePin(pin)) {
            std::cout << "❌ 잘못된 PIN입니다." << std::endl;
            return false;
        }
        
        if (!hasSufficientFunds(amount)) {
            std::cout << "❌ 잔액이 부족합니다." << std::endl;
            return false;
        }
        
        if (amount > daily_limit_) {
            std::cout << "❌ 일일 한도를 초과했습니다." << std::endl;
            return false;
        }
        
        balance_ -= amount;
        transaction_count_++;
        logTransaction("WITHDRAW", amount);
        return true;
    }
    
    // 읽기 전용 정보 제공
    double getBalance(const std::string& pin) const {
        if (validatePin(pin)) {
            return balance_;
        }
        std::cout << "❌ 잘못된 PIN입니다." << std::endl;
        return -1.0;  // 오류 표시
    }
    
    std::string getAccountNumber() const {
        // 계좌번호는 뒤 4자리만 표시 (보안)
        std::string masked = account_number_;
        for (size_t i = 0; i < masked.length() - 4; ++i) {
            masked[i] = '*';
        }
        return masked;
    }
};

// 사용 예시: 안전한 은행 거래
void bank_example() {
    BankAccount myAccount("1234567890", "1234", 1000.0);
    
    // ✅ 올바른 사용법
    std::cout << "계좌번호: " << myAccount.getAccountNumber() << std::endl;
    
    if (myAccount.deposit(500.0, "1234")) {
        std::cout << "✅ 입금 성공" << std::endl;
    }
    
    std::cout << "잔액: $" << myAccount.getBalance("1234") << std::endl;
    
    if (myAccount.withdraw(200.0, "1234")) {
        std::cout << "✅ 출금 성공" << std::endl;
    }
    
    // ❌ 잘못된 시도들
    myAccount.withdraw(200.0, "9999");  // 잘못된 PIN
    myAccount.withdraw(5000.0, "1234"); // 한도 초과
    
    // myAccount.balance_ = 1000000;  // ❌ 컴파일 에러! private 접근 불가
    // myAccount.pin_ = "0000";       // ❌ 컴파일 에러! private 접근 불가
}
```

#### 🎓 접근 제어 설계 원칙

1. **최소 권한 원칙**: 꼭 필요한 만큼만 공개
2. **인터페이스 분리**: public은 사용자 인터페이스만
3. **구현 은닉**: private으로 내부 구현 숨기기
4. **상속 고려**: protected로 확장 가능성 열어두기

```cpp
// 🎯 좋은 접근 제어 설계 예시
class LogEntry {
private:  // 내부 데이터는 철저히 보호
    std::string message_;
    std::chrono::system_clock::time_point timestamp_;
    LogLevel level_;
    
protected:  // 상속받는 클래스에서 필요할 수 있는 것들
    void updateTimestamp() {
        timestamp_ = std::chrono::system_clock::now();
    }
    
public:  // 사용자가 실제로 필요한 기능만 공개
    LogEntry(const std::string& msg, LogLevel level);
    
    // 읽기 전용 접근자
    const std::string& getMessage() const { return message_; }
    LogLevel getLevel() const { return level_; }
    
    // 안전한 수정 메서드
    void setMessage(const std::string& msg) {
        message_ = msg;
        updateTimestamp();  // 메시지 변경 시 시간도 업데이트
    }
    
    std::string toString() const;
};
```

```cpp
class AccessExample {
private:
    int private_data_;        // 클래스 내부에서만 접근 가능
    
protected:
    int protected_data_;      // 클래스와 파생 클래스에서 접근 가능
    
public:
    int public_data_;         // 어디서나 접근 가능
    
    AccessExample() : private_data_(0), protected_data_(0), public_data_(0) {}
    
    // private 멤버에 접근하는 public 메서드
    void setPrivateData(int value) { private_data_ = value; }
    int getPrivateData() const { return private_data_; }
    
protected:
    void protectedMethod() {
        // 파생 클래스에서도 호출 가능
    }
    
private:
    void privateMethod() {
        // 클래스 내부에서만 호출 가능
    }
};
```

---

## 🧬 2. 상속 (Inheritance): 코드의 DNA 전수

### 🌳 상속이란? 가족의 유전자 전수 과정

상속은 "부모-자식" 관계를 코드로 표현하는 것입니다. **가족의 유전자 전수**에 비유해보겠습니다:

```
       👨 아버지 (기본 유전자)
       ├── 👀 눈 색깔: 갈색
       ├── 📏 키: 큰 편
       └── 🧬 혈액형: A형
            ↓ 유전자 전수
       👦 아들 (상속받은 특성 + 고유 특성)
       ├── 👀 눈 색깔: 갈색 (아버지에게서 상속)
       ├── 📏 키: 큰 편 (아버지에게서 상속)
       ├── 🧬 혈액형: A형 (아버지에게서 상속)
       └── 🎮 게임 실력: 뛰어남 (자신만의 특성)
```

**프로그래밍에서의 상속도 마찬가지입니다:**

```
       🚗 Vehicle (부모 클래스 - 모든 탈것의 공통 특성)
       ├── 🛞 wheels (바퀴 개수)
       ├── ⚡ engine (엔진)
       ├── 🚀 move() (이동 기능)
       └── 🛑 stop() (정지 기능)
            ↓ 상속
     🚙 Car (자식 클래스 - 자동차만의 특성 추가)
     ├── 🛞 wheels: 4 (부모에게서 상속)
     ├── ⚡ engine (부모에게서 상속)
     ├── 🚀 move() (부모에게서 상속)
     ├── 🛑 stop() (부모에게서 상속)
     └── 🚪 openTrunk() (자동차만의 고유 기능)
```

### 🎯 상속을 사용하는 이유: DRY (Don't Repeat Yourself)

상속 없이 코드를 작성한다면...

```cpp
// ❌ 상속 없는 코드: 중복이 많음
class Car {
    int wheels = 4;
    std::string engine_type;
    double speed;
    
    void start() { std::cout << "Engine started"; }
    void stop() { std::cout << "Engine stopped"; }
    void move() { std::cout << "Car is moving"; }
    void openTrunk() { std::cout << "Trunk opened"; }
};

class Truck {
    int wheels = 6;  // 중복!
    std::string engine_type;  // 중복!
    double speed;  // 중복!
    
    void start() { std::cout << "Engine started"; }  // 중복!
    void stop() { std::cout << "Engine stopped"; }   // 중복!
    void move() { std::cout << "Truck is moving"; }  // 거의 중복!
    void loadCargo() { std::cout << "Cargo loaded"; }
};

class Motorcycle {
    int wheels = 2;  // 중복!
    std::string engine_type;  // 중복!
    double speed;  // 중복!
    
    void start() { std::cout << "Engine started"; }  // 중복!
    void stop() { std::cout << "Engine stopped"; }   // 중복!
    void move() { std::cout << "Motorcycle is moving"; }  // 거의 중복!
    void doWheelie() { std::cout << "Wheelie!"; }
};
```

**문제점들:**
- 🔄 같은 코드를 계속 반복 작성
- 🐛 한 곳에서 버그 수정하면 모든 곳에서 수정해야 함
- 📈 코드량 증가, 유지보수 어려움
- 💭 논리적 관계 표현 불가

```cpp
// ✅ 상속을 사용한 깔끔한 코드
class Vehicle {  // 모든 탈것의 공통 특성
protected:  // 자식 클래스에서 접근 가능
    int wheels_;
    std::string engine_type_;
    double speed_;
    bool is_running_;
    
public:
    Vehicle(int wheels, const std::string& engine)
        : wheels_(wheels), engine_type_(engine), speed_(0), is_running_(false) {}
    
    virtual ~Vehicle() = default;
    
    // 모든 탈것의 공통 기능
    virtual void start() {
        is_running_ = true;
        std::cout << engine_type_ << " engine started" << std::endl;
    }
    
    virtual void stop() {
        is_running_ = false;
        speed_ = 0;
        std::cout << "Vehicle stopped" << std::endl;
    }
    
    virtual void move() {
        if (is_running_) {
            speed_ = 60;  // 기본 속도
            std::cout << "Vehicle moving at " << speed_ << "km/h" << std::endl;
        }
    }
    
    // 정보 조회
    int getWheels() const { return wheels_; }
    double getSpeed() const { return speed_; }
    bool isRunning() const { return is_running_; }
};

// 🚗 자동차: Vehicle의 특성 + 자동차만의 특성
class Car : public Vehicle {
private:
    bool trunk_open_;
    int passenger_capacity_;
    
public:
    Car(const std::string& engine = "Gasoline", int capacity = 5)
        : Vehicle(4, engine),  // 부모 생성자 호출
          trunk_open_(false), passenger_capacity_(capacity) {}
    
    // 부모 메서드 재정의 (오버라이드)
    void move() override {
        if (is_running_) {
            speed_ = 100;  // 자동차는 더 빠름
            std::cout << "Car cruising at " << speed_ << "km/h" << std::endl;
        } else {
            std::cout << "Please start the car first!" << std::endl;
        }
    }
    
    // 자동차만의 기능
    void openTrunk() {
        trunk_open_ = true;
        std::cout << "Trunk opened" << std::endl;
    }
    
    void closeTrunk() {
        trunk_open_ = false;
        std::cout << "Trunk closed" << std::endl;
    }
    
    void playMusic() {
        std::cout << "🎵 Playing music on car stereo" << std::endl;
    }
};

// 🚚 트럭: Vehicle의 특성 + 트럭만의 특성
class Truck : public Vehicle {
private:
    double cargo_capacity_;
    double current_load_;
    
public:
    Truck(double capacity = 1000.0) 
        : Vehicle(6, "Diesel"),  // 6개 바퀴, 디젤 엔진
          cargo_capacity_(capacity), current_load_(0) {}
    
    void move() override {
        if (is_running_) {
            speed_ = 80;  // 트럭은 상대적으로 느림
            if (current_load_ > cargo_capacity_ * 0.8) {
                speed_ = 60;  // 무거우면 더 느림
            }
            std::cout << "Truck moving at " << speed_ << "km/h with " 
                      << current_load_ << "kg cargo" << std::endl;
        }
    }
    
    void loadCargo(double weight) {
        if (current_load_ + weight <= cargo_capacity_) {
            current_load_ += weight;
            std::cout << "Loaded " << weight << "kg. Total: " 
                      << current_load_ << "/" << cargo_capacity_ << "kg" << std::endl;
        } else {
            std::cout << "Cannot load " << weight << "kg. Would exceed capacity!" << std::endl;
        }
    }
    
    void unloadCargo() {
        current_load_ = 0;
        std::cout << "All cargo unloaded" << std::endl;
    }
};

// 🏍️ 오토바이: Vehicle의 특성 + 오토바이만의 특성
class Motorcycle : public Vehicle {
private:
    bool helmet_required_;
    
public:
    Motorcycle(bool helmet_req = true)
        : Vehicle(2, "Gasoline"), helmet_required_(helmet_req) {}
    
    void start() override {
        if (helmet_required_) {
            std::cout << "⚠️ Please wear your helmet first!" << std::endl;
        }
        Vehicle::start();  // 부모의 start() 호출
        std::cout << "🏍️ Ready to ride!" << std::endl;
    }
    
    void move() override {
        if (is_running_) {
            speed_ = 120;  // 오토바이는 빠름
            std::cout << "Motorcycle zooming at " << speed_ << "km/h" << std::endl;
        }
    }
    
    void doWheelie() {
        if (is_running_ && speed_ > 30) {
            std::cout << "🤸 Performing wheelie!" << std::endl;
        } else {
            std::cout << "Need more speed for wheelie!" << std::endl;
        }
    }
};

// 🎯 상속의 진가: 다형성과 함께 사용
void vehicle_parade() {
    // 다양한 탈것들을 하나의 배열로 관리
    std::vector<std::unique_ptr<Vehicle>> vehicles;
    
    vehicles.push_back(std::make_unique<Car>("Hybrid", 7));
    vehicles.push_back(std::make_unique<Truck>(2000.0));
    vehicles.push_back(std::make_unique<Motorcycle>(true));
    
    std::cout << "🎪 Vehicle Parade Starting!" << std::endl;
    
    // 모든 탈것을 동일한 방식으로 제어
    for (auto& vehicle : vehicles) {
        std::cout << "\n--- Vehicle with " << vehicle->getWheels() << " wheels ---" << std::endl;
        vehicle->start();  // 각자의 start() 메서드 호출
        vehicle->move();   // 각자의 move() 메서드 호출
        vehicle->stop();   // 공통 stop() 메서드
    }
    
    std::cout << "\n🎉 Parade finished!" << std::endl;
}
```

### 📈 상속의 장점 정리

1. **코드 재사용**: 공통 기능을 한 번만 구현
2. **논리적 관계**: 현실 세계의 관계를 코드로 표현
3. **확장성**: 새로운 타입 쉽게 추가
4. **유지보수**: 공통 기능 수정 시 한 곳만 변경
5. **다형성**: 같은 인터페이스로 다른 동작 수행

### 🎨 기본 상속 구현

```cpp
// 🎯 기본 클래스 (부모 클래스)
// 모든 로거의 공통 기능을 정의
class Logger {
protected:  // 🔐 자식 클래스에서 접근 가능
    std::string name_;    // 로거 이름
    bool enabled_;        // 활성화 상태
    
public:
    // 생성자: 이름을 받아서 초기화
    Logger(const std::string& name) : name_(name), enabled_(true) {}
    
    // 🎭 가상 소멸자 (다형성을 위해 필수!)
    virtual ~Logger() = default;
    
    // 🎭 가상 함수 (자식 클래스에서 재정의 가능)
    // virtual 키워드가 핵심!
    virtual void log(const LogEntry& entry) {
        if (enabled_) {
            std::cout << "[" << name_ << "] " << entry.toString() << std::endl;
        }
    }
    
    void enable() { enabled_ = true; }
    void disable() { enabled_ = false; }
    bool isEnabled() const { return enabled_; }
    
    const std::string& getName() const { return name_; }
};

// 📝 파생 클래스 (자식 클래스) - 파일에 로그를 저장
// public Logger는 "Logger를 공개적으로 상속받는다"는 의미
class FileLogger : public Logger {
private:
    std::string filename_;
    std::ofstream file_;
    
public:
    FileLogger(const std::string& name, const std::string& filename)
        : Logger(name), filename_(filename) {
        file_.open(filename_, std::ios::app);
        if (!file_.is_open()) {
            throw std::runtime_error("Cannot open log file: " + filename_);
        }
    }
    
    ~FileLogger() {
        if (file_.is_open()) {
            file_.close();
        }
    }
    
    // 🎯 부모의 가상 함수를 재정의 (오버라이드)
    // override 키워드로 실수 방지
    void log(const LogEntry& entry) override {
        if (enabled_ && file_.is_open()) {
            file_ << entry.toString() << std::endl;
            file_.flush();  // 💾 버퍼를 비워서 즉시 저장
        }
    }
    
    // 추가 메서드
    void rotate() {
        if (file_.is_open()) {
            file_.close();
        }
        
        // 백업 파일명 생성
        std::string backup_name = filename_ + ".bak";
        std::rename(filename_.c_str(), backup_name.c_str());
        
        // 새 파일 열기
        file_.open(filename_, std::ios::app);
    }
    
    const std::string& getFilename() const { return filename_; }
};

// 🌐 네트워크 로거 - 원격 서버로 로그 전송
class NetworkLogger : public Logger {
private:
    std::string server_address_;
    int server_port_;
    int socket_fd_;
    
public:
    NetworkLogger(const std::string& name, const std::string& address, int port)
        : Logger(name), server_address_(address), server_port_(port), socket_fd_(-1) {
        connectToServer();
    }
    
    ~NetworkLogger() {
        if (socket_fd_ != -1) {
            close(socket_fd_);
        }
    }
    
    void log(const LogEntry& entry) override {
        if (enabled_ && socket_fd_ != -1) {
            std::string log_line = entry.toString() + "\n";
            send(socket_fd_, log_line.c_str(), log_line.length(), 0);
        }
    }
    
private:
    void connectToServer() {
        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd_ == -1) {
            throw std::runtime_error("Cannot create socket");
        }
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port_);
        inet_pton(AF_INET, server_address_.c_str(), &server_addr.sin_addr);
        
        if (connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            close(socket_fd_);
            socket_fd_ = -1;
            throw std::runtime_error("Cannot connect to log server");
        }
    }
};

// 🎭 복합 로거 - 여러 로거를 하나로 묶어서 사용
// 디자인 패턴 중 "Composite Pattern"의 예시
class CompositeLogger : public Logger {
private:
    std::vector<std::unique_ptr<Logger>> loggers_;
    
public:
    CompositeLogger(const std::string& name) : Logger(name) {}
    
    void addLogger(std::unique_ptr<Logger> logger) {
        loggers_.push_back(std::move(logger));
    }
    
    void log(const LogEntry& entry) override {
        if (enabled_) {
            for (auto& logger : loggers_) {
                logger->log(entry);
            }
        }
    }
    
    size_t getLoggerCount() const { return loggers_.size(); }
};
```

### 🌟 다중 상속

다중 상속은 여러 부모로부터 상속받는 것입니다:

```
   🎨 Drawable     🎵 Playable
         \           /
          \         /
           🎮 Game
     (그릴 수 있고 플레이할 수 있음)
```

⚠️ **주의**: 다중 상속은 복잡도를 높일 수 있으므로 신중하게 사용하세요!

```cpp
// 여러 인터페이스를 구현하는 클래스
class Configurable {
public:
    virtual ~Configurable() = default;
    virtual void configure(const std::string& config) = 0;
    virtual std::string getConfiguration() const = 0;
};

class Monitorable {
public:
    virtual ~Monitorable() = default;
    virtual void getStatistics(std::map<std::string, int>& stats) const = 0;
    virtual void resetStatistics() = 0;
};

// 다중 상속을 사용하는 고급 로거
class AdvancedLogger : public Logger, public Configurable, public Monitorable {
private:
    std::map<std::string, int> stats_;
    std::string config_;
    
public:
    AdvancedLogger(const std::string& name) : Logger(name) {
        stats_["total_logs"] = 0;
        stats_["error_logs"] = 0;
        stats_["warning_logs"] = 0;
    }
    
    void log(const LogEntry& entry) override {
        if (enabled_) {
            Logger::log(entry);  // 부모 클래스 메서드 호출
            
            // 통계 업데이트
            stats_["total_logs"]++;
            if (entry.getLevel() == "ERROR") {
                stats_["error_logs"]++;
            } else if (entry.getLevel() == "WARNING") {
                stats_["warning_logs"]++;
            }
        }
    }
    
    // Configurable 인터페이스 구현
    void configure(const std::string& config) override {
        config_ = config;
        // 설정 파싱 및 적용
    }
    
    std::string getConfiguration() const override {
        return config_;
    }
    
    // Monitorable 인터페이스 구현
    void getStatistics(std::map<std::string, int>& stats) const override {
        stats = stats_;
    }
    
    void resetStatistics() override {
        for (auto& pair : stats_) {
            pair.second = 0;
        }
    }
};
```

---

## 🎭 3. 다형성 (Polymorphism): 하나의 이름, 다양한 모습

### 🎨 다형성이란? 배우의 변신 과정

다형성은 "하나의 인터페이스, 여러 구현"을 의미합니다. **한 배우가 다양한 역할을 연기**하는 것과 같습니다:

```
🎭 배우 "김○○"
├── 🤴 사극에서: 왕 역할 → "과인이 명하노라!"
├── 💼 현대극에서: 회사원 역할 → "네, 알겠습니다!"
├── 🦸 액션에서: 히어로 역할 → "정의는 승리한다!"
└── 😂 코미디에서: 개그맨 역할 → "웃겨드릴게요!"

같은 배우, 다른 연기!
```

**프로그래밍에서도 마찬가지:**

```
🎵 음악재생기::play() (같은 메서드 이름)
├── 🎧 MP3Player::play() → "MP3 파일 재생 중..."
├── 📻 RadioPlayer::play() → "라디오 방송 수신 중..."
├── 🎵 StreamingPlayer::play() → "스트리밍 연결 중..."
└── 📀 CDPlayer::play() → "CD 읽는 중..."

같은 play() 호출, 다른 동작!
```

### 🔮 다형성의 마법: Virtual Functions (가상 함수)

#### 🌟 가상 함수 없이 VS 가상 함수와 함께

```cpp
// ❌ 가상 함수 없는 경우: 정적 바인딩
class Animal {
public:
    void makeSound() {  // virtual 키워드 없음
        std::cout << "Some animal sound" << std::endl;
    }
};

class Dog : public Animal {
public:
    void makeSound() {  // 부모 메서드 가림 (hiding)
        std::cout << "Woof!" << std::endl;
    }
};

void static_binding_problem() {
    Dog myDog;
    Animal* animalPtr = &myDog;  // Dog를 Animal 포인터로
    
    myDog.makeSound();        // "Woof!" - 직접 호출
    animalPtr->makeSound();   // "Some animal sound" - 😱 잘못된 호출!
    
    // 컴파일러가 포인터 타입(Animal*)만 보고 결정
    // 실제 객체(Dog)는 무시됨
}
```

```cpp
// ✅ 가상 함수 사용: 동적 바인딩
class Animal {
public:
    virtual ~Animal() = default;  // 가상 소멸자 필수!
    
    virtual void makeSound() {  // virtual 키워드가 핵심!
        std::cout << "Some animal sound" << std::endl;
    }
    
    virtual void move() {
        std::cout << "Animal moves" << std::endl;
    }
};

class Dog : public Animal {
public:
    void makeSound() override {  // override로 의도 명확히
        std::cout << "Woof! Woof!" << std::endl;
    }
    
    void move() override {
        std::cout << "Dog runs on 4 legs" << std::endl;
    }
    
    // Dog만의 고유 메서드
    void wagTail() {
        std::cout << "Tail wagging happily!" << std::endl;
    }
};

class Cat : public Animal {
public:
    void makeSound() override {
        std::cout << "Meow~ Meow~" << std::endl;
    }
    
    void move() override {
        std::cout << "Cat sneaks silently" << std::endl;
    }
    
    void purr() {
        std::cout << "Purrrrr..." << std::endl;
    }
};

class Bird : public Animal {
public:
    void makeSound() override {
        std::cout << "Tweet! Tweet!" << std::endl;
    }
    
    void move() override {
        std::cout << "Bird flies in the sky" << std::endl;
    }
    
    void fly() {
        std::cout << "Soaring high!" << std::endl;
    }
};

// 🎭 다형성의 진가: 동적 바인딩
void dynamic_binding_magic() {
    std::vector<std::unique_ptr<Animal>> zoo;
    
    // 다양한 동물들을 하나의 컨테이너에
    zoo.push_back(std::make_unique<Dog>());
    zoo.push_back(std::make_unique<Cat>());
    zoo.push_back(std::make_unique<Bird>());
    zoo.push_back(std::make_unique<Dog>());
    
    std::cout << "🎪 동물원 공연 시작!" << std::endl;
    
    // 모든 동물을 같은 방식으로 제어
    for (const auto& animal : zoo) {
        std::cout << "\n--- 다음 동물 ---" << std::endl;
        animal->makeSound();  // 🎭 각자의 소리!
        animal->move();       // 🎭 각자의 움직임!
        
        // 실행 시점에 실제 객체 타입을 확인하여
        // 올바른 메서드가 호출됨!
    }
    
    std::cout << "\n🎉 공연 끝!" << std::endl;
}
```

#### 🔍 Virtual Table (vtable): 다형성의 비밀

가상 함수가 어떻게 동작하는지 살펴보겠습니다:

```cpp
// 컴파일러가 내부적으로 만드는 가상 함수 테이블
/*
Animal vtable:
[0] -> Animal::makeSound()
[1] -> Animal::move()

Dog vtable:
[0] -> Dog::makeSound()     // 오버라이드
[1] -> Dog::move()          // 오버라이드

Cat vtable:
[0] -> Cat::makeSound()     // 오버라이드
[1] -> Cat::move()          // 오버라이드
*/

// 실행 과정:
void polymorphism_explanation() {
    Animal* ptr = new Dog();  // Dog 객체, Animal 포인터
    
    // ptr->makeSound() 호출 시:
    // 1. ptr이 가리키는 객체의 vtable 확인
    // 2. vtable[0] (makeSound 위치) 찾기
    // 3. Dog::makeSound() 주소 발견
    // 4. Dog::makeSound() 실행
    
    ptr->makeSound();  // "Woof! Woof!" 출력
    
    delete ptr;
}
```

### 🎯 실전 다형성: LogCaster의 플러그인 시스템

다형성을 활용한 실제적인 예시를 살펴보겠습니다:

```cpp
// 🎭 다양한 로그 출력 방식을 하나의 인터페이스로
class LogOutput {
public:
    virtual ~LogOutput() = default;
    
    // 순수 가상 함수: 반드시 구현해야 함
    virtual void write(const std::string& message) = 0;
    virtual std::string getOutputType() const = 0;
    
    // 공통 기능
    virtual bool isReady() const { return true; }
    
    virtual void flush() {
        // 기본적으로는 아무것도 하지 않음
    }
};

// 🖥️ 콘솔 출력
class ConsoleOutput : public LogOutput {
private:
    bool colored_;
    
public:
    ConsoleOutput(bool colored = true) : colored_(colored) {}
    
    void write(const std::string& message) override {
        if (colored_) {
            // ANSI 색상 코드 추가
            if (message.find("ERROR") != std::string::npos) {
                std::cout << "\033[31m" << message << "\033[0m" << std::endl;  // 빨간색
            } else if (message.find("WARNING") != std::string::npos) {
                std::cout << "\033[33m" << message << "\033[0m" << std::endl;  // 노란색
            } else {
                std::cout << message << std::endl;
            }
        } else {
            std::cout << message << std::endl;
        }
    }
    
    std::string getOutputType() const override {
        return "Console";
    }
};

// 📁 파일 출력
class FileOutput : public LogOutput {
private:
    std::ofstream file_;
    std::string filename_;
    
public:
    FileOutput(const std::string& filename) : filename_(filename) {
        file_.open(filename, std::ios::app);
    }
    
    ~FileOutput() {
        if (file_.is_open()) {
            file_.close();
        }
    }
    
    void write(const std::string& message) override {
        if (file_.is_open()) {
            file_ << message << std::endl;
        }
    }
    
    void flush() override {
        if (file_.is_open()) {
            file_.flush();
        }
    }
    
    bool isReady() const override {
        return file_.is_open();
    }
    
    std::string getOutputType() const override {
        return "File (" + filename_ + ")";
    }
};

// 🌐 네트워크 출력
class NetworkOutput : public LogOutput {
private:
    std::string server_address_;
    int port_;
    bool connected_;
    
public:
    NetworkOutput(const std::string& address, int port) 
        : server_address_(address), port_(port), connected_(false) {
        // 실제로는 소켓 연결
        connected_ = connectToServer();
    }
    
    void write(const std::string& message) override {
        if (connected_) {
            // 실제로는 네트워크로 전송
            std::cout << "[NET -> " << server_address_ << ":" << port_ 
                      << "] " << message << std::endl;
        }
    }
    
    bool isReady() const override {
        return connected_;
    }
    
    std::string getOutputType() const override {
        return "Network (" + server_address_ + ":" + std::to_string(port_) + ")";
    }
    
private:
    bool connectToServer() {
        // 실제 네트워크 연결 로직
        std::cout << "Connecting to " << server_address_ << ":" << port_ << "..." << std::endl;
        return true;  // 시뮬레이션
    }
};

// 📧 이메일 출력 (경고/에러만)
class EmailOutput : public LogOutput {
private:
    std::string recipient_;
    std::vector<std::string> critical_messages_;
    
public:
    EmailOutput(const std::string& recipient) : recipient_(recipient) {}
    
    void write(const std::string& message) override {
        // 중요한 메시지만 이메일로
        if (message.find("ERROR") != std::string::npos || 
            message.find("CRITICAL") != std::string::npos) {
            critical_messages_.push_back(message);
            
            // 실제로는 이메일 전송
            std::cout << "📧 Email to " << recipient_ << ": " << message << std::endl;
        }
    }
    
    std::string getOutputType() const override {
        return "Email (" + recipient_ + ")";
    }
    
    size_t getCriticalMessageCount() const {
        return critical_messages_.size();
    }
};

// 🎛️ 로그 매니저: 다형성의 진가 발휘
class LogManager {
private:
    std::vector<std::unique_ptr<LogOutput>> outputs_;
    
public:
    void addOutput(std::unique_ptr<LogOutput> output) {
        if (output && output->isReady()) {
            outputs_.push_back(std::move(output));
            std::cout << "✅ Added output: " << outputs_.back()->getOutputType() << std::endl;
        } else {
            std::cout << "❌ Failed to add output" << std::endl;
        }
    }
    
    void log(const std::string& message) {
        std::cout << "\n📝 Logging: " << message << std::endl;
        
        // 🎭 다형성의 마법: 모든 출력 방식에 동시에 로그
        for (auto& output : outputs_) {
            output->write(message);  // 각자의 방식으로 출력!
        }
        
        // 모든 출력을 플러시
        for (auto& output : outputs_) {
            output->flush();
        }
    }
    
    void listOutputs() const {
        std::cout << "\n📋 Active outputs:" << std::endl;
        for (size_t i = 0; i < outputs_.size(); ++i) {
            std::cout << "[" << i << "] " << outputs_[i]->getOutputType() << std::endl;
        }
    }
};

// 사용 예시: 플러그인처럼 다양한 출력 방식 조합
void polymorphism_in_action() {
    LogManager logger;
    
    // 다양한 출력 방식을 플러그인처럼 추가
    logger.addOutput(std::make_unique<ConsoleOutput>(true));
    logger.addOutput(std::make_unique<FileOutput>("application.log"));
    logger.addOutput(std::make_unique<NetworkOutput>("log-server.com", 5140));
    logger.addOutput(std::make_unique<EmailOutput>("admin@company.com"));
    
    logger.listOutputs();
    
    // 하나의 로그 호출로 모든 출력 방식에 동시 전달
    logger.log("[INFO] Application started successfully");
    logger.log("[WARNING] Low disk space detected");
    logger.log("[ERROR] Database connection failed");
    logger.log("[CRITICAL] System overheating!");
}
```

### 🎪 다형성의 핵심 개념 정리

#### 1️⃣ **정적 바인딩 vs 동적 바인딩**
- **정적 바인딩**: 컴파일 시점에 호출할 함수 결정
- **동적 바인딩**: 실행 시점에 실제 객체 타입으로 함수 결정

#### 2️⃣ **virtual 키워드의 중요성**
- virtual 없으면: 포인터/참조 타입으로 함수 선택
- virtual 있으면: 실제 객체 타입으로 함수 선택

#### 3️⃣ **override의 안전성**
- 컴파일러가 실수를 잡아줌
- 의도를 명확히 표현

#### 4️⃣ **가상 소멸자의 필수성**
- 다형성 사용 시 반드시 virtual destructor 필요
- 메모리 누수 방지

### 🔮 가상 함수와 동적 바인딩

```cpp
// 🎯 추상 기본 클래스 (인터페이스)
// 순수 가상 함수를 포함하여 직접 객체 생성 불가
class LogProcessor {
public:
    virtual ~LogProcessor() = default;
    
    // 🎭 순수 가상 함수 (= 0은 "구현이 없다"는 의미)
    // 자식 클래스에서 반드시 구현해야 함!
    virtual void process(const LogEntry& entry) = 0;
    virtual std::string getProcessorType() const = 0;
    
    // 🎨 일반 가상 함수 (기본 구현 제공)
    // 자식 클래스에서 필요시 재정의 가능
    virtual void preProcess() {
        std::cout << "Starting log processing..." << std::endl;
    }
    
    virtual void postProcess() {
        std::cout << "Log processing completed." << std::endl;
    }
    
    // 🏗️ 템플릿 메서드 패턴
    // 전체 흐름은 정의하되, 세부 구현은 자식에게 위임
    void execute(const LogEntry& entry) {
        preProcess();    // 전처리
        process(entry);  // 실제 처리 (자식이 구현)
        postProcess();   // 후처리
    }
};

// 🔍 필터 프로세서 - 키워드가 포함된 로그만 처리
class FilterProcessor : public LogProcessor {
private:
    std::string filter_keyword_;
    
public:
    FilterProcessor(const std::string& keyword) : filter_keyword_(keyword) {}
    
    void process(const LogEntry& entry) override {
        if (entry.getMessage().find(filter_keyword_) != std::string::npos) {
            std::cout << "Filtered: " << entry.toString() << std::endl;
        }
    }
    
    std::string getProcessorType() const override {
        return "FilterProcessor";
    }
};

class TransformProcessor : public LogProcessor {
public:
    void process(const LogEntry& entry) override {
        // 로그를 JSON 형태로 변환
        std::cout << "{\n";
        std::cout << "  \"level\": \"" << entry.getLevel() << "\",\n";
        std::cout << "  \"source\": \"" << entry.getSource() << "\",\n";
        std::cout << "  \"message\": \"" << entry.getMessage() << "\",\n";
        std::cout << "  \"timestamp\": \"" << entry.getFormattedTime() << "\"\n";
        std::cout << "}" << std::endl;
    }
    
    std::string getProcessorType() const override {
        return "TransformProcessor";
    }
};

class AlertProcessor : public LogProcessor {
private:
    std::vector<std::string> alert_levels_;
    
public:
    AlertProcessor(const std::vector<std::string>& levels) : alert_levels_(levels) {}
    
    void process(const LogEntry& entry) override {
        for (const auto& level : alert_levels_) {
            if (entry.getLevel() == level) {
                sendAlert(entry);
                break;
            }
        }
    }
    
    std::string getProcessorType() const override {
        return "AlertProcessor";
    }
    
private:
    void sendAlert(const LogEntry& entry) {
        std::cout << "🚨 ALERT: " << entry.toString() << std::endl;
        // 실제로는 이메일, SMS, 또는 다른 알림 시스템으로 전송
    }
};

// 🎛️ 프로세서 매니저 - 다형성의 진가를 보여주는 예시
// 다양한 프로세서를 하나의 인터페이스로 관리
class ProcessorManager {
private:
    std::vector<std::unique_ptr<LogProcessor>> processors_;
    
public:
    void addProcessor(std::unique_ptr<LogProcessor> processor) {
        processors_.push_back(std::move(processor));
    }
    
    void processLog(const LogEntry& entry) {
        // 🎭 다형성의 마법!
        // processor의 실제 타입이 무엇이든 올바른 메서드가 호출됨
        for (auto& processor : processors_) {
            processor->execute(entry);  // 동적 바인딩
        }
    }
    
    void listProcessors() const {
        std::cout << "Registered processors:" << std::endl;
        for (const auto& processor : processors_) {
            std::cout << "- " << processor->getProcessorType() << std::endl;
        }
    }
};

// 사용 예시
void polymorphism_example() {
    ProcessorManager manager;
    
    // 다양한 타입의 프로세서 추가
    manager.addProcessor(std::make_unique<FilterProcessor>("ERROR"));
    manager.addProcessor(std::make_unique<TransformProcessor>());
    manager.addProcessor(std::make_unique<AlertProcessor>(std::vector<std::string>{"ERROR", "CRITICAL"}));
    
    manager.listProcessors();
    
    // 로그 처리 - 각 프로세서가 자신의 방식으로 처리
    LogEntry errorLog("Database connection failed", "ERROR", "192.168.1.100");
    manager.processLog(errorLog);
}
```

### 🎮 연산자 오버로딩: 연산자에게 새로운 의미 부여하기

연산자 오버로딩은 +, -, ==, << 같은 연산자에 새로운 의미를 부여하는 것입니다. **언어의 새로운 표현법 만들기**와 같습니다:

#### 🌟 일상 언어에서의 다의어처럼

```
"밥" 이라는 단어:
🍚 한국어: 쌀로 만든 음식
👨 한국어: 사람 이름 (밥)
🍽️ 영어: Robert의 줄임말

문맥에 따라 다른 의미!
```

**프로그래밍에서도 마찬가지:**

```cpp
// + 연산자의 다양한 의미
int a = 3 + 5;              // 숫자 덧셈
string msg = "Hello" + " World";  // 문자열 연결
Vector3D pos = v1 + v2;     // 벡터 덧셈 (우리가 정의)
Matrix result = m1 + m2;    // 행렬 덧셈 (우리가 정의)
```

#### 🎯 왜 연산자 오버로딩을 사용할까?

**가독성과 직관성 향상:**

```cpp
// ❌ 연산자 오버로딩 없이
Complex c1(3, 4);  // 3 + 4i
Complex c2(1, 2);  // 1 + 2i
Complex result = c1.add(c2.multiply(Complex(2, 0))); // 복잡함!

// ✅ 연산자 오버로딩으로
Complex c1(3, 4);
Complex c2(1, 2);
Complex result = c1 + c2 * Complex(2, 0);  // 수학처럼 직관적!
```

#### 🔧 LogCaster에서의 연산자 오버로딩 실전 예시

```cpp
// 📊 로그 통계를 위한 LogStats 클래스
class LogStats {
private:
    std::map<std::string, int> level_counts_;  // 레벨별 개수
    std::map<std::string, int> source_counts_; // 소스별 개수
    int total_logs_;
    
public:
    LogStats() : total_logs_(0) {}
    
    // 생성자
    LogStats(const std::string& level, const std::string& source) : total_logs_(1) {
        level_counts_[level] = 1;
        source_counts_[source] = 1;
    }
    
    // 🔢 산술 연산자: 통계 합치기
    LogStats operator+(const LogStats& other) const {
        LogStats result = *this;
        result += other;  // += 연산자 활용
        return result;
    }
    
    LogStats& operator+=(const LogStats& other) {
        total_logs_ += other.total_logs_;
        
        // 레벨별 카운트 합치기
        for (const auto& [level, count] : other.level_counts_) {
            level_counts_[level] += count;
        }
        
        // 소스별 카운트 합치기
        for (const auto& [source, count] : other.source_counts_) {
            source_counts_[source] += count;
        }
        
        return *this;
    }
    
    // 📈 스케일링: 통계에 가중치 적용
    LogStats operator*(double multiplier) const {
        LogStats result = *this;
        result.total_logs_ = static_cast<int>(result.total_logs_ * multiplier);
        
        for (auto& [level, count] : result.level_counts_) {
            count = static_cast<int>(count * multiplier);
        }
        
        for (auto& [source, count] : result.source_counts_) {
            count = static_cast<int>(count * multiplier);
        }
        
        return result;
    }
    
    // 🆚 비교 연산자: 통계 비교
    bool operator>(const LogStats& other) const {
        return total_logs_ > other.total_logs_;
    }
    
    bool operator<(const LogStats& other) const {
        return total_logs_ < other.total_logs_;
    }
    
    bool operator==(const LogStats& other) const {
        return total_logs_ == other.total_logs_ &&
               level_counts_ == other.level_counts_ &&
               source_counts_ == other.source_counts_;
    }
    
    // 📊 인덱싱 연산자: 레벨별 개수 접근
    int operator[](const std::string& level) const {
        auto it = level_counts_.find(level);
        return (it != level_counts_.end()) ? it->second : 0;
    }
    
    // 📞 함수 호출 연산자: 통계 계산
    double operator()(const std::string& level) const {
        if (total_logs_ == 0) return 0.0;
        int count = (*this)[level];  // [] 연산자 활용
        return static_cast<double>(count) / total_logs_ * 100.0;  // 백분율
    }
    
    // 🎭 변환 연산자: bool로 자동 변환 (통계가 있는지 확인)
    explicit operator bool() const {
        return total_logs_ > 0;
    }
    
    // 📝 출력 연산자 (friend 함수)
    friend std::ostream& operator<<(std::ostream& os, const LogStats& stats) {
        os << "LogStats {\n";
        os << "  Total: " << stats.total_logs_ << " logs\n";
        os << "  By Level:\n";
        for (const auto& [level, count] : stats.level_counts_) {
            os << "    " << level << ": " << count 
               << " (" << std::fixed << std::setprecision(1) 
               << stats(level) << "%)\n";  // () 연산자 활용
        }
        os << "  By Source:\n";
        for (const auto& [source, count] : stats.source_counts_) {
            os << "    " << source << ": " << count << "\n";
        }
        os << "}";
        return os;
    }
    
    // 입력 연산자
    friend std::istream& operator>>(std::istream& is, LogStats& stats) {
        std::string level, source;
        is >> level >> source;
        if (is) {
            stats.level_counts_[level]++;
            stats.source_counts_[source]++;
            stats.total_logs_++;
        }
        return is;
    }
    
    // Getter 메서드들
    int getTotalLogs() const { return total_logs_; }
    
    const std::map<std::string, int>& getLevelCounts() const {
        return level_counts_;
    }
};

// 🌟 연산자 오버로딩의 진가: 직관적인 통계 처리
void operator_overloading_showcase() {
    // 다양한 로그 통계 생성
    LogStats webServerStats;
    webServerStats += LogStats("INFO", "WebServer");
    webServerStats += LogStats("ERROR", "WebServer");
    webServerStats += LogStats("WARNING", "WebServer");
    webServerStats += LogStats("INFO", "WebServer");
    
    LogStats databaseStats;
    databaseStats += LogStats("INFO", "Database");
    databaseStats += LogStats("ERROR", "Database");
    databaseStats += LogStats("CRITICAL", "Database");
    
    LogStats apiStats;
    apiStats += LogStats("INFO", "API");
    apiStats += LogStats("WARNING", "API");
    apiStats += LogStats("ERROR", "API");
    apiStats += LogStats("INFO", "API");
    apiStats += LogStats("INFO", "API");
    
    std::cout << "=== 개별 통계 ===" << std::endl;
    std::cout << "Web Server:\n" << webServerStats << std::endl;
    std::cout << "\nDatabase:\n" << databaseStats << std::endl;
    std::cout << "\nAPI:\n" << apiStats << std::endl;
    
    // ✨ 연산자 오버로딩의 마법: 수학처럼 직관적!
    LogStats totalStats = webServerStats + databaseStats + apiStats;
    std::cout << "\n=== 전체 통계 ===" << std::endl;
    std::cout << totalStats << std::endl;
    
    // 비교 연산
    std::cout << "\n=== 통계 비교 ===" << std::endl;
    if (webServerStats > databaseStats) {
        std::cout << "WebServer가 Database보다 로그가 많습니다." << std::endl;
    }
    
    if (apiStats > webServerStats) {
        std::cout << "API가 WebServer보다 로그가 많습니다." << std::endl;
    }
    
    // 인덱싱과 함수 호출 연산자
    std::cout << "\n=== 세부 분석 ===" << std::endl;
    std::cout << "전체 ERROR 로그: " << totalStats["ERROR"] << "개" << std::endl;
    std::cout << "ERROR 비율: " << totalStats("ERROR") << "%" << std::endl;
    std::cout << "WARNING 비율: " << totalStats("WARNING") << "%" << std::endl;
    
    // bool 변환 연산자
    if (totalStats) {
        std::cout << "통계 데이터가 존재합니다." << std::endl;
    }
    
    // 스케일링 (가중치 적용)
    LogStats projectedStats = totalStats * 1.5;  // 50% 증가 예상
    std::cout << "\n=== 50% 증가 예상치 ===" << std::endl;
    std::cout << "예상 총 로그: " << projectedStats.getTotalLogs() << "개" << std::endl;
}
```

#### ⚠️ 연산자 오버로딩 주의사항

1. **직관성 유지**: 연산자의 일반적인 의미와 비슷하게
2. **일관성**: 관련 연산자들을 함께 구현 (==와 !=)
3. **성능 고려**: 불필요한 복사 방지
4. **안전성**: 자기 자신과의 연산 처리

```cpp
// ✅ 좋은 연산자 오버로딩
class Vector3D {
    double x, y, z;
public:
    // 직관적: 벡터 덧셈
    Vector3D operator+(const Vector3D& other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }
    
    // 일관성: += 도 함께 구현
    Vector3D& operator+=(const Vector3D& other) {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }
    
    // 안전성: 자기 할당 체크
    Vector3D& operator=(const Vector3D& other) {
        if (this != &other) {  // 자기 할당 체크
            x = other.x; y = other.y; z = other.z;
        }
        return *this;
    }
};

// ❌ 나쁜 연산자 오버로딩
class BadExample {
public:
    // 직관성 위배: + 연산자로 출력?
    void operator+(const std::string& msg) {
        std::cout << msg << std::endl;  // 혼란스러움
    }
    
    // 위험성: 예상과 다른 동작
    bool operator<(const BadExample& other) {
        std::cout << "비교 중..." << std::endl;  // 부작용!
        return true;
    }
};
```

```cpp
// 일반적인 사용
int a = 1 + 2;           // 숫자 덧셈
string s = "Hello" + " World";  // 문자열 연결

// 우리가 정의한 사용
LogFilter filter1 + filter2;  // 필터 결합
if (filter(log)) { }         // 함수처럼 호출
```

```cpp
class LogFilter {
private:
    std::set<std::string> allowed_levels_;
    std::set<std::string> allowed_sources_;
    
public:
    LogFilter() = default;
    
    // 레벨 추가 연산자
    LogFilter& operator+=(const std::string& level) {
        allowed_levels_.insert(level);
        return *this;
    }
    
    // 레벨 제거 연산자
    LogFilter& operator-=(const std::string& level) {
        allowed_levels_.erase(level);
        return *this;
    }
    
    // 필터 조합 연산자
    LogFilter operator+(const LogFilter& other) const {
        LogFilter result = *this;
        result.allowed_levels_.insert(other.allowed_levels_.begin(), other.allowed_levels_.end());
        result.allowed_sources_.insert(other.allowed_sources_.begin(), other.allowed_sources_.end());
        return result;
    }
    
    // 로그 엔트리 필터링 연산자
    bool operator()(const LogEntry& entry) const {
        bool level_ok = allowed_levels_.empty() || 
                       allowed_levels_.find(entry.getLevel()) != allowed_levels_.end();
        bool source_ok = allowed_sources_.empty() || 
                        allowed_sources_.find(entry.getSource()) != allowed_sources_.end();
        return level_ok && source_ok;
    }
    
    // 비교 연산자
    bool operator==(const LogFilter& other) const {
        return allowed_levels_ == other.allowed_levels_ && 
               allowed_sources_ == other.allowed_sources_;
    }
    
    // 출력 연산자 (친구 함수)
    friend std::ostream& operator<<(std::ostream& os, const LogFilter& filter) {
        os << "LogFilter(levels: ";
        for (const auto& level : filter.allowed_levels_) {
            os << level << " ";
        }
        os << ", sources: ";
        for (const auto& source : filter.allowed_sources_) {
            os << source << " ";
        }
        os << ")";
        return os;
    }
    
    void addSource(const std::string& source) { allowed_sources_.insert(source); }
    void removeSource(const std::string& source) { allowed_sources_.erase(source); }
};

// 사용 예시
void operator_overloading_example() {
    LogFilter filter;
    filter += "ERROR";
    filter += "WARNING";
    filter.addSource("192.168.1.100");
    
    LogEntry log1("Error occurred", "ERROR", "192.168.1.100");
    LogEntry log2("Info message", "INFO", "192.168.1.101");
    
    if (filter(log1)) {
        std::cout << "Log1 passed filter" << std::endl;
    }
    
    if (!filter(log2)) {
        std::cout << "Log2 blocked by filter" << std::endl;
    }
    
    std::cout << filter << std::endl;
}
```

---

## 💊 4. 캡슐화 (Encapsulation)

### 🏰 캡슐화란?

캡슐화는 약의 캡슐처럼 내부를 감싸는 것입니다:

```
💊 캡슐 (클래스)
├── 🔒 내부 성분 (private 데이터)
└── 🚪 복용 방법 (public 메서드)
```

**장점**:
- ✅ 내부 구현을 숨김
- ✅ 잘못된 사용 방지
- ✅ 인터페이스만 공개
- ✅ 유지보수 용이

### 🔐 정보 은닉과 인터페이스 설계

```cpp
// 🗄️ 스레드 안전한 로그 저장소
class LogStorage {
private:  // 🔒 내부 구현은 완전히 숨김
    std::vector<LogEntry> logs_;         // 실제 로그 데이터
    mutable std::shared_mutex mutex_;    // 🔐 동시 접근 제어
    size_t max_size_;                    // 최대 저장 개수
    
    // 🛠️ Private 헬퍼 메서드 (내부에서만 사용)
    void enforceMaxSize() {
        // 최대 크기를 초과하면 오래된 로그 삭제
        if (logs_.size() > max_size_) {
            size_t to_remove = logs_.size() - max_size_;
            logs_.erase(logs_.begin(), logs_.begin() + to_remove);
        }
    }
    
    std::vector<LogEntry>::iterator findByTimestamp(
        const std::chrono::system_clock::time_point& timestamp) {
        return std::find_if(logs_.begin(), logs_.end(),
            [&timestamp](const LogEntry& entry) {
                return entry.getTimestamp() == timestamp;
            });
    }
    
public:
    explicit LogStorage(size_t max_size = 10000) : max_size_(max_size) {
        logs_.reserve(max_size_);
    }
    
    // 🚫 복사 방지 (뮤텍스는 복사할 수 없으므로)
    LogStorage(const LogStorage&) = delete;            // 복사 생성자 삭제
    LogStorage& operator=(const LogStorage&) = delete; // 복사 할당 삭제
    
    // ✅ 이동은 허용 (성능을 위해)
    LogStorage(LogStorage&&) = default;            // 이동 생성자
    LogStorage& operator=(LogStorage&&) = default; // 이동 할당
    
    // 스레드 안전한 로그 추가
    void addLog(const LogEntry& entry) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        logs_.push_back(entry);
        enforceMaxSize();
    }
    
    void addLog(LogEntry&& entry) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        logs_.push_back(std::move(entry));
        enforceMaxSize();
    }
    
    // 스레드 안전한 로그 조회
    std::vector<LogEntry> getLogs() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return logs_;  // 복사본 반환
    }
    
    std::vector<LogEntry> getLogsByLevel(const std::string& level) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        std::vector<LogEntry> result;
        
        std::copy_if(logs_.begin(), logs_.end(), std::back_inserter(result),
            [&level](const LogEntry& entry) {
                return entry.getLevel() == level;
            });
        
        return result;
    }
    
    std::vector<LogEntry> getLogsInRange(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) const {
        
        std::shared_lock<std::shared_mutex> lock(mutex_);
        std::vector<LogEntry> result;
        
        std::copy_if(logs_.begin(), logs_.end(), std::back_inserter(result),
            [&start, &end](const LogEntry& entry) {
                auto timestamp = entry.getTimestamp();
                return timestamp >= start && timestamp <= end;
            });
        
        return result;
    }
    
    // 통계 정보 (읽기 전용)
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return logs_.size();
    }
    
    size_t capacity() const { return max_size_; }
    
    bool empty() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return logs_.empty();
    }
    
    // 설정 변경
    void setMaxSize(size_t new_max_size) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        max_size_ = new_max_size;
        enforceMaxSize();
    }
    
    void clear() {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        logs_.clear();
    }
    
    // 반복자 지원 (읽기 전용)
    class const_iterator {
    private:
        std::vector<LogEntry>::const_iterator it_;
        std::shared_lock<std::shared_mutex> lock_;
        
    public:
        const_iterator(std::vector<LogEntry>::const_iterator it, 
                      std::shared_mutex& mutex)
            : it_(it), lock_(mutex) {}
        
        const LogEntry& operator*() const { return *it_; }
        const LogEntry* operator->() const { return &(*it_); }
        
        const_iterator& operator++() { ++it_; return *this; }
        const_iterator operator++(int) { auto temp = *this; ++it_; return temp; }
        
        bool operator==(const const_iterator& other) const { return it_ == other.it_; }
        bool operator!=(const const_iterator& other) const { return it_ != other.it_; }
    };
    
    const_iterator begin() const {
        return const_iterator(logs_.begin(), mutex_);
    }
    
    const_iterator end() const {
        return const_iterator(logs_.end(), mutex_);
    }
};
```

### 🎭 PIMPL (Pointer to Implementation) 패턴

PIMPL은 "구현을 가리키는 포인터"라는 뜻으로, 구현 세부사항을 완전히 숨기는 기법입니다:

```
📦 헤더 파일 (공개)
└── 🎭 인터페이스만 노출

📁 구현 파일 (비공개)
└── 🔧 실제 구현 숨김
```

**장점**:
- ✅ 컴파일 시간 단축
- ✅ ABI 호환성 유지
- ✅ 구현 변경 시 재컴파일 최소화

```cpp
// 헤더 파일 (LogServer.h)
class LogServer {
public:
    LogServer(int port);
    ~LogServer();
    
    // 복사/이동 제어
    LogServer(const LogServer&) = delete;
    LogServer& operator=(const LogServer&) = delete;
    LogServer(LogServer&&) noexcept;
    LogServer& operator=(LogServer&&) noexcept;
    
    void start();
    void stop();
    bool isRunning() const;
    
    void setMaxClients(int max_clients);
    int getActiveClients() const;
    
    void addLogProcessor(std::unique_ptr<LogProcessor> processor);
    
private:
    class Impl;  // 전방 선언
    std::unique_ptr<Impl> pImpl_;  // PIMPL 포인터
};

// 구현 파일 (LogServer.cpp)
class LogServer::Impl {
public:
    int port_;
    int server_fd_;
    int epoll_fd_;
    bool running_;
    int max_clients_;
    std::atomic<int> active_clients_;
    
    std::unique_ptr<LogStorage> log_storage_;
    std::vector<std::unique_ptr<LogProcessor>> processors_;
    std::thread server_thread_;
    
    Impl(int port) 
        : port_(port), server_fd_(-1), epoll_fd_(-1), 
          running_(false), max_clients_(100), active_clients_(0) {
        log_storage_ = std::make_unique<LogStorage>(10000);
    }
    
    ~Impl() {
        if (running_) {
            stop();
        }
    }
    
    void start() {
        if (running_) return;
        
        server_fd_ = createServerSocket();
        epoll_fd_ = epoll_create1(0);
        
        // epoll 설정...
        
        running_ = true;
        server_thread_ = std::thread(&Impl::serverLoop, this);
    }
    
    void stop() {
        running_ = false;
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
        
        if (epoll_fd_ != -1) {
            close(epoll_fd_);
            epoll_fd_ = -1;
        }
        
        if (server_fd_ != -1) {
            close(server_fd_);
            server_fd_ = -1;
        }
    }
    
private:
    int createServerSocket() {
        // 소켓 생성 및 설정 로직
        return -1;  // 구현 생략
    }
    
    void serverLoop() {
        // 메인 서버 루프 로직
    }
};

// LogServer 메서드 구현
LogServer::LogServer(int port) : pImpl_(std::make_unique<Impl>(port)) {}

LogServer::~LogServer() = default;

LogServer::LogServer(LogServer&&) noexcept = default;
LogServer& LogServer::operator=(LogServer&&) noexcept = default;

void LogServer::start() { pImpl_->start(); }
void LogServer::stop() { pImpl_->stop(); }
bool LogServer::isRunning() const { return pImpl_->running_; }

void LogServer::setMaxClients(int max_clients) { 
    pImpl_->max_clients_ = max_clients; 
}

int LogServer::getActiveClients() const { 
    return pImpl_->active_clients_.load(); 
}

void LogServer::addLogProcessor(std::unique_ptr<LogProcessor> processor) {
    pImpl_->processors_.push_back(std::move(processor));
}
```

---

## 🔧 5. 템플릿과 제네릭 프로그래밍

### 🎁 템플릿이란?

템플릿은 "타입에 대한 설계도"입니다. 마치 쿠키 틀처럼, 하나의 템플릿으로 다양한 타입의 코드를 생성할 수 있습니다:

```cpp
// 🍪 쿠키 틀 (템플릿)
template<typename 재료>
class 쿠키 {
    재료 content;
};

// 🍫 초콜릿 쿠키
쿠키<초콜릿> chocoCookie;

// 🍓 딸기 쿠키  
쿠키<딸기> strawberryCookie;
```

### 📦 클래스 템플릿

```cpp
// 🎯 제네릭 Thread-Safe 컨테이너
// 어떤 타입이든 저장할 수 있는 스레드 안전 컨테이너
template<typename T>  // T는 어떤 타입이든 될 수 있음
class ThreadSafeContainer {
private:
    std::vector<T> data_;
    mutable std::shared_mutex mutex_;
    size_t max_size_;
    
public:
    explicit ThreadSafeContainer(size_t max_size = 1000) : max_size_(max_size) {}
    
    void push_back(const T& item) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        data_.push_back(item);
        enforceMaxSize();
    }
    
    void push_back(T&& item) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        data_.push_back(std::move(item));
        enforceMaxSize();
    }
    
    // 🚀 Perfect Forwarding을 사용한 직접 생성
    // 복사 없이 컨테이너 내부에서 객체를 직접 생성
    template<typename... Args>  // 가변 인자 템플릿
    void emplace_back(Args&&... args) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        data_.emplace_back(std::forward<Args>(args)...);  // 완벽한 전달
        enforceMaxSize();
    }
    
    std::vector<T> getAll() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return data_;
    }
    
    template<typename Predicate>
    std::vector<T> filter(Predicate pred) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        std::vector<T> result;
        std::copy_if(data_.begin(), data_.end(), std::back_inserter(result), pred);
        return result;
    }
    
    template<typename Function>
    void forEach(Function func) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        std::for_each(data_.begin(), data_.end(), func);
    }
    
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return data_.size();
    }
    
private:
    void enforceMaxSize() {
        if (data_.size() > max_size_) {
            data_.erase(data_.begin(), data_.begin() + (data_.size() - max_size_));
        }
    }
};

// 특수화된 LogContainer
using LogContainer = ThreadSafeContainer<LogEntry>;

// 사용 예시
void template_example() {
    LogContainer logs(1000);
    
    // 다양한 방식으로 로그 추가
    logs.push_back(LogEntry("Test message", "INFO", "localhost"));
    logs.emplace_back("Another message", "ERROR", "192.168.1.100");
    
    // 필터링 사용
    auto errorLogs = logs.filter([](const LogEntry& entry) {
        return entry.getLevel() == "ERROR";
    });
    
    // 함수 적용
    logs.forEach([](const LogEntry& entry) {
        std::cout << entry.toString() << std::endl;
    });
}
```

### 🎨 함수 템플릿과 SFINAE

SFINAE는 "Substitution Failure Is Not An Error"의 약자로, 템플릿 특수화 실패가 에러가 아니라는 원칙입니다:

```cpp
// 🤔 컴파일러의 생각 과정
// "이 타입으로 치환해볼까? 안 되네... 에러? 아니야, 다른 걸 찾아보자!"
```

```cpp
#include <type_traits>

// 로그 가능한 타입인지 확인하는 타입 특성
template<typename T>
struct is_loggable {
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().toString(), std::true_type{});
    
    template<typename>
    static std::false_type test(...);
    
    static constexpr bool value = decltype(test<T>(0))::value;
};

// 로그 함수 템플릿 (toString() 메서드가 있는 타입용)
template<typename T>
std::enable_if_t<is_loggable<T>::value, void>
logObject(const T& obj, const std::string& level = "INFO") {
    LogEntry entry(obj.toString(), level, "system");
    std::cout << entry.toString() << std::endl;
}

// 로그 함수 템플릿 (일반 타입용)
template<typename T>
std::enable_if_t<!is_loggable<T>::value, void>
logObject(const T& obj, const std::string& level = "INFO") {
    std::ostringstream oss;
    oss << obj;
    LogEntry entry(oss.str(), level, "system");
    std::cout << entry.toString() << std::endl;
}

// 가변 인자 템플릿을 사용한 로그 포매팅
template<typename... Args>
void logFormatted(const std::string& level, const std::string& format, Args&&... args) {
    std::ostringstream oss;
    logFormattedImpl(oss, format, std::forward<Args>(args)...);
    LogEntry entry(oss.str(), level, "formatted");
    std::cout << entry.toString() << std::endl;
}

template<typename T>
void logFormattedImpl(std::ostringstream& oss, const std::string& format, T&& value) {
    size_t pos = format.find("{}");
    if (pos != std::string::npos) {
        oss << format.substr(0, pos) << value << format.substr(pos + 2);
    } else {
        oss << format;
    }
}

template<typename T, typename... Args>
void logFormattedImpl(std::ostringstream& oss, const std::string& format, 
                     T&& value, Args&&... args) {
    size_t pos = format.find("{}");
    if (pos != std::string::npos) {
        oss << format.substr(0, pos) << value;
        logFormattedImpl(oss, format.substr(pos + 2), std::forward<Args>(args)...);
    } else {
        oss << format;
    }
}

// 사용 예시
void template_functions_example() {
    // 다양한 타입 로깅
    logObject(42);
    logObject(std::string("Hello World"));
    
    // 포매팅된 로그
    logFormatted("INFO", "User {} logged in from IP {}", "john_doe", "192.168.1.100");
    logFormatted("ERROR", "Failed to connect to database after {} attempts", 3);
}
```

---

## 🏛️ 6. 현대적 C++ 기능들 (C++11/14/17)

### 🧠 스마트 포인터와 RAII

> 📝 **참고**: 메모리 관리와 RAII의 자세한 설명은 [2. Memory.md](./2.%20Memory.md) 문서를 참조하세요.

스마트 포인터와 RAII는 C++의 핵심 메모리 관리 기법입니다. **자동 정리 시스템**에 비유할 수 있습니다:

```
🏠 전통적인 집 (Raw Pointer)
├── 🧹 직접 청소: 매번 쓰레기 버려야 함
├── 💡 직접 전등: 나갈 때 꺼야 함
└── 🔒 직접 잠금: 나갈 때 문 잠가야 함
   ❌ 깜빡하면 문제 발생!

🏠 스마트홈 (Smart Pointer)
├── 🤖 자동 청소기: 알아서 청소
├── 💡 자동 조명: 사람 없으면 자동 OFF
└── 🔐 자동 잠금: 나가면 자동 잠금
   ✅ 깜빡해도 안전!
```

#### 🔧 LogCaster에서의 스마트 포인터 실전 활용

```cpp
// 🎯 리소스 관리의 진화 과정

// ❌ 전통적인 방식: 수동 메모리 관리
class OldStyleLogManager {
private:
    LogStorage* storage_;        // Raw pointer
    LogProcessor** processors_; // 배열도 Raw pointer
    int processor_count_;
    
public:
    OldStyleLogManager() {
        storage_ = new LogStorage(1000);  // 수동 할당
        processors_ = new LogProcessor*[10];  // 배열 할당
        processor_count_ = 0;
        
        // 각 프로세서도 수동 할당
        for (int i = 0; i < 10; ++i) {
            processors_[i] = nullptr;
        }
    }
    
    ~OldStyleLogManager() {
        // 😰 복잡한 수동 정리
        delete storage_;  // 스토리지 해제
        
        // 각 프로세서 해제
        for (int i = 0; i < processor_count_; ++i) {
            delete processors_[i];
        }
        delete[] processors_;  // 배열 해제
        
        // 하나라도 빼먹으면 메모리 누수!
    }
    
    void addProcessor(LogProcessor* proc) {
        if (processor_count_ < 10) {
            processors_[processor_count_++] = proc;
        }
        // 예외 상황에서 proc가 누수될 수 있음!
    }
};

// ✅ 현대적인 방식: 자동 메모리 관리
class ModernLogManager {
private:
    std::unique_ptr<LogStorage> storage_;                    // 독점 소유
    std::vector<std::unique_ptr<LogProcessor>> processors_;  // 동적 배열
    std::shared_ptr<ConfigManager> config_;                 // 공유 소유
    std::weak_ptr<LogServer> server_;                       // 약한 참조
    
public:
    ModernLogManager() 
        : storage_(std::make_unique<LogStorage>(1000)),     // 안전한 생성
          config_(std::make_shared<ConfigManager>()) {      // 공유 생성
        
        std::cout << "LogManager 생성 완료" << std::endl;
    }
    
    // 소멸자는 간단! 자동으로 모든 것이 정리됨
    ~ModernLogManager() {
        std::cout << "LogManager 소멸: 모든 리소스 자동 정리" << std::endl;
        // 컴파일러가 자동으로:
        // 1. processors_ 벡터의 모든 unique_ptr 소멸
        // 2. storage_ unique_ptr 소멸
        // 3. config_ shared_ptr 참조 카운트 감소
        // 4. server_ weak_ptr 자동 해제
    }
    
    // 이동 생성자와 할당 연산자 자동 생성됨
    ModernLogManager(ModernLogManager&&) = default;
    ModernLogManager& operator=(ModernLogManager&&) = default;
    
    // 복사는 금지 (unique_ptr 때문에)
    ModernLogManager(const ModernLogManager&) = delete;
    ModernLogManager& operator=(const ModernLogManager&) = delete;
    
    // 프로세서 추가: 예외 안전
    void addProcessor(std::unique_ptr<LogProcessor> proc) {
        if (proc) {
            processors_.push_back(std::move(proc));
            std::cout << "프로세서 추가됨. 총 " << processors_.size() << "개" << std::endl;
        }
        // 예외가 발생해도 proc은 자동으로 정리됨
    }
    
    // 팩토리 메서드로 프로세서 생성
    template<typename ProcessorType, typename... Args>
    void createProcessor(Args&&... args) {
        auto processor = std::make_unique<ProcessorType>(std::forward<Args>(args)...);
        addProcessor(std::move(processor));
    }
    
    // 서버 설정: 약한 참조 사용
    void setServer(std::shared_ptr<LogServer> server) {
        server_ = server;  // weak_ptr에 할당
    }
    
    // 로그 처리: 자동 수명 관리
    void processLog(const LogEntry& entry) {
        // 서버가 아직 살아있는지 확인
        if (auto server = server_.lock()) {  // weak_ptr을 shared_ptr로 승격
            server->validateEntry(entry);
        }
        
        // 모든 프로세서에서 처리
        for (auto& processor : processors_) {
            processor->process(entry);  // unique_ptr 자동 역참조
        }
        
        // 스토리지에 저장
        storage_->addLog(entry);
    }
    
    // 통계 정보
    void printStats() const {
        std::cout << "=== LogManager 통계 ===" << std::endl;
        std::cout << "프로세서 개수: " << processors_.size() << std::endl;
        std::cout << "총 로그 개수: " << storage_->size() << std::endl;
        
        if (auto server = server_.lock()) {
            std::cout << "서버 연결됨: " << server->getAddress() << std::endl;
        } else {
            std::cout << "서버 연결 없음" << std::endl;
        }
        
        std::cout << "Config 참조 카운트: " << config_.use_count() << std::endl;
    }
};

// 🎭 RAII의 진가: 자동 리소스 관리
class FileLogProcessor : public LogProcessor {
private:
    std::unique_ptr<std::ofstream> file_;  // 파일도 스마트 포인터로
    std::string filename_;
    
public:
    FileLogProcessor(const std::string& filename) : filename_(filename) {
        file_ = std::make_unique<std::ofstream>(filename, std::ios::app);
        
        if (!file_->is_open()) {
            throw std::runtime_error("파일 열기 실패: " + filename);
        }
        
        std::cout << "파일 로그 프로세서 생성: " << filename << std::endl;
    }
    
    ~FileLogProcessor() {
        std::cout << "파일 로그 프로세서 소멸: " << filename_ << std::endl;
        // file_ unique_ptr이 자동으로 ofstream 소멸
        // ofstream 소멸자가 자동으로 파일 닫기
    }
    
    void process(const LogEntry& entry) override {
        if (file_ && file_->is_open()) {
            *file_ << entry.toString() << std::endl;
            file_->flush();  // 즉시 디스크에 쓰기
        }
    }
    
    // 파일 교체 (리소스 안전하게 교체)
    void rotateFile(const std::string& new_filename) {
        auto new_file = std::make_unique<std::ofstream>(new_filename, std::ios::app);
        
        if (new_file->is_open()) {
            file_ = std::move(new_file);  // 원자적 교체
            filename_ = new_filename;
            std::cout << "파일 교체됨: " << filename_ << std::endl;
        } else {
            throw std::runtime_error("새 파일 열기 실패: " + new_filename);
        }
    }
};

// 사용 예시: 스마트 포인터의 완전 자동화
void smart_pointer_showcase() {
    std::cout << "\n=== 스마트 포인터 시연 ===" << std::endl;
    
    {
        // 스코프 시작
        auto manager = std::make_unique<ModernLogManager>();
        auto server = std::make_shared<LogServer>("localhost", 8080);
        
        // 서버 설정 (공유 참조)
        manager->setServer(server);
        
        // 다양한 프로세서 추가
        manager->createProcessor<FileLogProcessor>("app.log");
        manager->createProcessor<ConsoleOutput>(true);
        manager->createProcessor<NetworkOutput>("log.server.com", 514);
        
        // 로그 처리
        LogEntry entry("테스트 메시지", "INFO", "main");
        manager->processLog(entry);
        
        manager->printStats();
        
        std::cout << "서버 참조 카운트: " << server.use_count() << std::endl;
        
    }  // 스코프 끝 - 모든 것이 자동으로 정리됨!
    
    std::cout << "\n=== 모든 리소스가 자동으로 정리되었습니다 ===" << std::endl;
    // manager의 unique_ptr이 소멸하면서:
    // 1. 모든 프로세서들의 unique_ptr 소멸
    // 2. 각 프로세서의 소멸자 호출 (파일 자동 닫기 등)
    // 3. storage_의 unique_ptr 소멸
    // 4. server_의 weak_ptr 자동 해제
    // 5. config_의 shared_ptr 참조 카운트 감소
    
    // 메모리 누수, 파일 핸들 누수, 네트워크 연결 누수 등이 전혀 없음!
}
```

#### 🎯 스마트 포인터 선택 가이드

| 상황 | 사용할 스마트 포인터 | 이유 |
|------|---------------------|------|
| 독점 소유 | `unique_ptr` | 하나의 객체만 소유, 빠름 |
| 공유 소유 | `shared_ptr` | 여러 객체가 공유, 참조 카운팅 |
| 순환 참조 방지 | `weak_ptr` | shared_ptr과 함께 사용 |
| 배열 관리 | `unique_ptr<T[]>` | 동적 배열 자동 관리 |

LogCaster 프로젝트에서의 실제 사용 예시를 중심으로 간단히 소개합니다:

```cpp
// 현대적 LogCaster 서버 클래스
class ModernLogServer {
private:
    std::unique_ptr<LogStorage> storage_;
    std::vector<std::unique_ptr<LogProcessor>> processors_;
    std::shared_ptr<NetworkManager> network_manager_;
    std::weak_ptr<ConfigManager> config_manager_;  // 순환 참조 방지
    
    std::thread server_thread_;
    std::atomic<bool> running_{false};
    
public:
    ModernLogServer() 
        : storage_(std::make_unique<LogStorage>(10000)),
          network_manager_(std::make_shared<NetworkManager>()) {}
    
    // 팩토리 메서드
    static std::unique_ptr<ModernLogServer> create() {
        return std::make_unique<ModernLogServer>();
    }
    
    void addProcessor(std::unique_ptr<LogProcessor> processor) {
        processors_.push_back(std::move(processor));
    }
    
    void setConfigManager(std::shared_ptr<ConfigManager> config_manager) {
        config_manager_ = config_manager;
    }
    
    void start() {
        if (running_.exchange(true)) {
            return;  // 이미 실행 중
        }
        
        server_thread_ = std::thread([this] {
            serverLoop();
        });
    }
    
    void stop() {
        running_ = false;
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }
    
    ~ModernLogServer() {
        stop();
    }
    
private:
    void serverLoop() {
        while (running_) {
            // 서버 로직
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};

// RAII 리소스 가드
template<typename T>
class ResourceGuard {
private:
    T resource_;
    std::function<void(T&)> cleanup_;
    
public:
    template<typename CleanupFunc>
    ResourceGuard(T&& resource, CleanupFunc cleanup)
        : resource_(std::move(resource)), cleanup_(cleanup) {}
    
    ~ResourceGuard() {
        cleanup_(resource_);
    }
    
    T& get() { return resource_; }
    const T& get() const { return resource_; }
    
    // 이동만 허용
    ResourceGuard(ResourceGuard&&) = default;
    ResourceGuard& operator=(ResourceGuard&&) = default;
    
    // 복사 금지
    ResourceGuard(const ResourceGuard&) = delete;
    ResourceGuard& operator=(const ResourceGuard&) = delete;
};

// 사용 예시
void modern_cpp_example() {
    // 파일 리소스 가드
    auto file_guard = ResourceGuard(
        std::ofstream("server.log"),
        [](std::ofstream& file) {
            if (file.is_open()) {
                file.close();
            }
        }
    );
    
    file_guard.get() << "Server started\n";
    
    // 스마트 포인터 사용
    auto server = ModernLogServer::create();
    server->addProcessor(std::make_unique<FilterProcessor>("ERROR"));
    server->start();
    
    // 자동으로 정리됨
}
```

### 🎯 Modern C++ 핵심 기능들

#### 🔍 auto 키워드
타입을 자동으로 추론합니다:
```cpp
auto x = 42;          // int
auto name = "Hello"s; // std::string
auto vec = std::vector<int>{1,2,3}; // std::vector<int>
```

#### 🎭 Lambda 표현식
이름 없는 함수를 즉석에서 만듭니다:
```cpp
// [캡처](매개변수) -> 반환타입 { 본문 }
auto add = [](int a, int b) { return a + b; };
```

#### 🔄 범위 기반 for문
컨테이너를 쉽게 순회합니다:
```cpp
for (const auto& item : container) {
    // item 사용
}
```

```cpp
class ModernLogAnalyzer {
private:
    LogContainer logs_;
    
public:
    // auto를 사용한 타입 추론
    auto getLogsByTimeRange(const std::chrono::system_clock::time_point& start,
                           const std::chrono::system_clock::time_point& end) const {
        return logs_.filter([start, end](const auto& entry) {
            const auto& timestamp = entry.getTimestamp();
            return timestamp >= start && timestamp <= end;
        });
    }
    
    // 람다를 사용한 고급 분석
    void analyzeErrorPatterns() {
        auto error_logs = logs_.filter([](const auto& entry) {
            return entry.getLevel() == "ERROR";
        });
        
        // 에러 메시지별 그룹화
        std::map<std::string, int> error_counts;
        
        // 범위 기반 for문 사용
        for (const auto& log : error_logs) {
            error_counts[log.getMessage()]++;
        }
        
        // 정렬된 결과 출력
        std::vector<std::pair<std::string, int>> sorted_errors(
            error_counts.begin(), error_counts.end());
        
        std::sort(sorted_errors.begin(), sorted_errors.end(),
                 [](const auto& a, const auto& b) {
                     return a.second > b.second;  // 내림차순
                 });
        
        std::cout << "Top error messages:" << std::endl;
        for (const auto& [message, count] : sorted_errors) {  // 구조화된 바인딩 (C++17)
            std::cout << count << "x: " << message << std::endl;
        }
    }
    
    // 제네릭 람다 사용
    template<typename Predicate>
    auto countLogs(Predicate pred) const {
        return std::count_if(logs_.getAll().begin(), logs_.getAll().end(), pred);
    }
    
    // 고차 함수 패턴
    auto createLevelFilter(const std::string& level) const {
        return [level](const auto& entry) {
            return entry.getLevel() == level;
        };
    }
    
    void performComplexAnalysis() {
        // 함수 조합
        auto error_filter = createLevelFilter("ERROR");
        auto warning_filter = createLevelFilter("WARNING");
        
        auto critical_logs = logs_.filter([&](const auto& entry) {
            return error_filter(entry) || warning_filter(entry);
        });
        
        // 시간별 분석
        using TimePoint = std::chrono::system_clock::time_point;
        std::map<std::chrono::hours, int> hourly_counts;
        
        for (const auto& log : critical_logs) {
            auto time = log.getTimestamp();
            auto time_t = std::chrono::system_clock::to_time_t(time);
            auto* tm = std::localtime(&time_t);
            auto hour = std::chrono::hours(tm->tm_hour);
            hourly_counts[hour]++;
        }
        
        // 결과 출력
        std::cout << "Critical logs by hour:" << std::endl;
        for (const auto& [hour, count] : hourly_counts) {
            std::cout << hour.count() << ":00 - " << count << " logs" << std::endl;
        }
    }
};
```

### ⚡ constexpr와 컴파일 타임 계산

constexpr는 "컴파일 시간에 계산 가능"을 의미합니다:

```cpp
// 🏃 런타임 계산
int runtime_factorial(int n);

// ⚡ 컴파일 타임 계산
constexpr int compile_factorial(int n);

// 결과는 같지만 compile_factorial은 컴파일 중에 계산됨!
```

**장점**:
- ✅ 실행 시간 0
- ✅ 최적화 향상
- ✅ 타입 안전성

```cpp
// 컴파일 타임 로그 레벨 정의
enum class LogLevel : int {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

constexpr const char* levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

constexpr LogLevel stringToLevel(const char* str) {
    if (std::string_view(str) == "DEBUG") return LogLevel::DEBUG;
    if (std::string_view(str) == "INFO") return LogLevel::INFO;
    if (std::string_view(str) == "WARNING") return LogLevel::WARNING;
    if (std::string_view(str) == "ERROR") return LogLevel::ERROR;
    if (std::string_view(str) == "CRITICAL") return LogLevel::CRITICAL;
    return LogLevel::INFO;  // 기본값
}

// 컴파일 타임 설정
template<LogLevel MinLevel = LogLevel::INFO>
class CompileTimeLogger {
public:
    template<LogLevel Level>
    static void log(const std::string& message) {
        if constexpr (Level >= MinLevel) {
            std::cout << "[" << levelToString(Level) << "] " << message << std::endl;
        }
        // Level < MinLevel인 경우 코드가 컴파일되지 않음 (최적화)
    }
};

// 사용 예시
void constexpr_example() {
    CompileTimeLogger<LogLevel::WARNING> logger;
    
    logger.log<LogLevel::DEBUG>("Debug message");     // 컴파일 시 제거됨
    logger.log<LogLevel::INFO>("Info message");       // 컴파일 시 제거됨
    logger.log<LogLevel::WARNING>("Warning message"); // 출력됨
    logger.log<LogLevel::ERROR>("Error message");     // 출력됨
}
```

---

## ✅ 7. 실전 연습과 다음 단계

### 🎯 핵심 체크리스트

이 문서를 완전히 이해했다면:

- [ ] 클래스를 "설계도", 객체를 "실체"로 이해
- [ ] public, private, protected의 차이 설명 가능
- [ ] 상속을 통한 코드 재사용 구현 가능
- [ ] virtual 함수와 override의 동작 원리 이해
- [ ] 스마트 포인터로 메모리 안전하게 관리
- [ ] 템플릿으로 제네릭 코드 작성 가능

### 🧪 실전 연습 문제

#### 1️⃣ 기초 문제
```cpp
// TODO: Animal 추상 클래스를 만들고
// Dog, Cat 클래스로 상속받아 구현하세요
class Animal {
    // 힌트: makeSound()를 순수 가상 함수로
};
```

#### 2️⃣ 중급 문제
```cpp
// TODO: 스마트 포인터를 사용하여
// 메모리 안전한 LogManager 클래스 구현
class LogManager {
    // 힌트: unique_ptr<Logger> 사용
};
```

#### 3️⃣ 고급 문제
```cpp
// TODO: 템플릿을 사용하여 어떤 타입이든
// 저장할 수 있는 ThreadSafeQueue 구현
template<typename T>
class ThreadSafeQueue {
    // 힌트: mutex와 condition_variable 활용
};
```

### 🚀 LogCaster에서의 실제 활용

```cpp
// LogCaster의 핵심 구조
class LogCaster {
    unique_ptr<LogStorage> storage_;      // 로그 저장소
    vector<unique_ptr<Logger>> loggers_;  // 다양한 로거들
    shared_ptr<Config> config_;           // 설정 공유
    
public:
    // 다형성 활용: 어떤 Logger든 추가 가능
    void addLogger(unique_ptr<Logger> logger) {
        loggers_.push_back(std::move(logger));
    }
    
    // 템플릿 활용: 다양한 필터 조건
    template<typename Predicate>
    auto filterLogs(Predicate pred) { /* ... */ }
};
```

## 📚 추가 학습 자료

### 📖 추천 도서
- "Effective C++" - Scott Meyers
- "Modern Effective C++" - Scott Meyers
- "Design Patterns" - Gang of Four

### 🔗 온라인 자료
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [cppreference.com](https://en.cppreference.com/)
- [Learn C++](https://www.learncpp.com/)

### 🎮 실습 프로젝트
1. 간단한 도형 클래스 계층 구조 만들기
2. 스마트 포인터로 연결 리스트 구현
3. 템플릿 기반 컨테이너 라이브러리 제작

## 🚨 자주 하는 실수와 해결법

### 1. 가상 소멸자 누락
**문제**: 다형성 사용 시 메모리 누수 발생
```cpp
// ❌ 잘못된 코드
class Base {
public:
    ~Base() { }  // 가상이 아님!
};

class Derived : public Base {
private:
    int* data_;
public:
    Derived() : data_(new int[100]) {}
    ~Derived() { delete[] data_; }
};

// 문제 발생
Base* ptr = new Derived();
delete ptr;  // Derived의 소멸자가 호출되지 않음!

// ✅ 올바른 코드
class Base {
public:
    virtual ~Base() = default;  // 가상 소멸자
};
```

### 2. 객체 슬라이싱 (Object Slicing)
**문제**: 파생 클래스를 기본 클래스로 복사할 때 정보 손실
```cpp
// ❌ 위험한 코드
class Animal {
protected:
    std::string name_;
};

class Dog : public Animal {
private:
    std::string breed_;  // 이 정보가 사라짐!
};

void processAnimal(Animal a) {  // 값으로 전달
    // Dog의 breed_ 정보는 없음
}

Dog myDog;
processAnimal(myDog);  // 슬라이싱 발생!

// ✅ 올바른 코드
void processAnimal(const Animal& a) {  // 참조로 전달
    // 다형성 유지됨
}
```

### 3. 순환 참조 (Circular Reference)
**문제**: shared_ptr 사용 시 메모리 누수
```cpp
// ❌ 순환 참조 문제
class Node {
public:
    std::shared_ptr<Node> next;
    std::shared_ptr<Node> prev;  // 서로를 참조
};

// ✅ 해결책
class Node {
public:
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;  // weak_ptr 사용
};
```

### 4. 복사 생성자/할당 연산자 미구현
**문제**: 얕은 복사로 인한 중복 해제
```cpp
// ❌ Rule of Three 위반
class Buffer {
private:
    char* data_;
    size_t size_;
public:
    Buffer(size_t size) : size_(size) {
        data_ = new char[size];
    }
    ~Buffer() { delete[] data_; }
    // 복사 생성자, 할당 연산자 없음!
};

// ✅ Rule of Five 준수
class Buffer {
private:
    char* data_;
    size_t size_;
public:
    // 생성자
    Buffer(size_t size) : size_(size), data_(new char[size]) {}
    
    // 소멸자
    ~Buffer() { delete[] data_; }
    
    // 복사 생성자
    Buffer(const Buffer& other) : size_(other.size_) {
        data_ = new char[size_];
        std::copy(other.data_, other.data_ + size_, data_);
    }
    
    // 복사 할당 연산자
    Buffer& operator=(const Buffer& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = new char[size_];
            std::copy(other.data_, other.data_ + size_, data_);
        }
        return *this;
    }
    
    // 이동 생성자
    Buffer(Buffer&& other) noexcept 
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }
    
    // 이동 할당 연산자
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
};
```

## 💡 실습 프로젝트

### 프로젝트 1: 도형 클래스 계층구조
```cpp
// 다음 요구사항을 만족하는 클래스들을 구현하세요:
// 1. Shape 추상 클래스 (area(), perimeter() 순수 가상 함수)
// 2. Circle, Rectangle, Triangle 구체 클래스
// 3. ShapeContainer 클래스 (다형성 활용)

class Shape {
    // 구현하세요
};

// 테스트 코드
void testShapes() {
    ShapeContainer container;
    container.add(std::make_unique<Circle>(5.0));
    container.add(std::make_unique<Rectangle>(4.0, 6.0));
    container.add(std::make_unique<Triangle>(3.0, 4.0, 5.0));
    
    std::cout << "Total area: " << container.totalArea() << std::endl;
}
```

### 프로젝트 2: 이벤트 시스템 구현
```cpp
// Observer 패턴을 사용한 이벤트 시스템
// 1. EventListener 인터페이스
// 2. EventEmitter 클래스
// 3. 다양한 이벤트 타입 지원

template<typename EventType>
class EventEmitter {
    // 구현하세요
};

// 사용 예시
struct LogEvent {
    std::string message;
    std::string level;
};

class LogListener {
public:
    void onLogEvent(const LogEvent& event);
};
```

### 프로젝트 3: 스마트 포인터 구현
```cpp
// std::unique_ptr의 간단한 버전 구현
template<typename T>
class MyUniquePtr {
    // 구현 요구사항:
    // 1. RAII 원칙 준수
    // 2. 이동 의미론 지원
    // 3. 복사 금지
    // 4. operator*, operator-> 구현
};
```

## 📚 추가 학습 자료

### 책 추천
- "Effective C++" - Scott Meyers (필독서)
- "More Effective C++" - Scott Meyers
- "C++ Primer" - Stanley Lippman (기초부터 심화까지)
- "Design Patterns" - Gang of Four (디자인 패턴)

### 온라인 자료
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [cppreference.com](https://en.cppreference.com/)
- [Learn C++](https://www.learncpp.com/)
- [C++ FAQ](https://isocpp.org/faq)

### 동영상 강의
- CppCon YouTube 채널
- Back to Basics 시리즈
- Modern C++ 강의

## ✅ 학습 체크리스트

### 기초 (1-2주)
- [ ] 클래스와 객체의 차이 이해
- [ ] 생성자와 소멸자 작성
- [ ] 접근 제어자 활용
- [ ] 간단한 클래스 설계

### 중급 (3-4주)
- [ ] 상속과 다형성 구현
- [ ] 가상 함수 동작 원리 이해
- [ ] 연산자 오버로딩
- [ ] Rule of Three/Five 적용

### 고급 (5-6주)
- [ ] 템플릿 프로그래밍
- [ ] SFINAE 이해
- [ ] 스마트 포인터 활용
- [ ] RAII 패턴 적용

### 전문가 (7-8주)
- [ ] 디자인 패턴 적용
- [ ] 템플릿 메타프로그래밍
- [ ] Perfect Forwarding
- [ ] CRTP 패턴

## 🔄 다음 학습 단계

이 문서를 완료했다면 다음 문서로 진행하세요:

1. **[04. Project_Setup_Guide.md](04.%20Project_Setup_Guide.md)** - 프로젝트 설정
   - 빌드 시스템
   - 디렉토리 구조
   - 개발 환경 설정

2. **[11. Performance_Guide.md](11.%20Performance_Guide.md)** - 성능 최적화
   - 프로파일링
   - 최적화 기법
   - 벤치마킹

3. **[12. DesignPatterns.md](12.%20DesignPatterns.md)** - 디자인 패턴
   - GoF 패턴
   - 모던 C++ 패턴
   - 안티 패턴

---

*💡 팁: OOP는 "현실을 코드로" 옮기는 것입니다. 복잡한 이론보다 실제 코드를 작성하며 익히세요! 작은 프로젝트부터 시작해서 점차 복잡도를 높여가는 것이 좋습니다.*