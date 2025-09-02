# 12. 디자인 패턴 🎨
## C/C++로 배우는 소프트웨어 설계 패턴

---

## 📋 문서 정보

- **난이도**: 중급-고급 (Intermediate-Advanced)
- **예상 학습 시간**: 12-15시간
- **필요 선수 지식**: 
  - C/C++ 중급 이상
  - 객체지향 프로그래밍 개념
  - 포인터와 메모리 관리
  - 기본 자료구조
- **실습 환경**: C++11 이상 컴파일러

---

## Overview
디자인 패턴은 반복적으로 발생하는 프로그래밍 문제들에 대한 검증된 해결책입니다. 이 문서에서는 LogCaster 프로젝트에 직접적으로 관련된 핵심 패턴들을 중점적으로 다룹니다.

## 1. RAII (Resource Acquisition Is Initialization) - C++ Only

### 개념
- 리소스 획득과 초기화를 결합
- 소멸자에서 자동으로 리소스 해제
- 예외 안전성 보장

### 구현 예제
```cpp
// File wrapper with RAII
class FileWrapper {
private:
    FILE* file_;
    
public:
    explicit FileWrapper(const char* filename, const char* mode) {
        file_ = fopen(filename, mode);
        if (!file_) {
            throw std::runtime_error("Failed to open file");
        }
    }
    
    ~FileWrapper() {
        if (file_) {
            fclose(file_);
        }
    }
    
    // Delete copy operations
    FileWrapper(const FileWrapper&) = delete;
    FileWrapper& operator=(const FileWrapper&) = delete;
    
    // Move operations
    FileWrapper(FileWrapper&& other) noexcept : file_(other.file_) {
        other.file_ = nullptr;
    }
    
    FILE* get() { return file_; }
};

// Usage
void processFile() {
    FileWrapper file("data.txt", "r");
    // File automatically closed when leaving scope
    // Even if exception is thrown
}
```

### Smart Pointers (RAII의 표준 구현)
```cpp
// unique_ptr example
std::unique_ptr<int[]> buffer(new int[1024]);
// Memory automatically freed

// shared_ptr with custom deleter
auto fileDeleter = [](FILE* f) { if (f) fclose(f); };
std::shared_ptr<FILE> file(fopen("test.txt", "r"), fileDeleter);
```

## 2. Factory Pattern

### C++ 구현
```cpp
// Abstract product
class Logger {
public:
    virtual ~Logger() = default;
    virtual void log(const std::string& message) = 0;
};

// Concrete products
class FileLogger : public Logger {
public:
    void log(const std::string& message) override {
        // Write to file
    }
};

class ConsoleLogger : public Logger {
public:
    void log(const std::string& message) override {
        std::cout << message << std::endl;
    }
};

// Factory
class LoggerFactory {
public:
    enum class LoggerType { FILE, CONSOLE };
    
    static std::unique_ptr<Logger> createLogger(LoggerType type) {
        switch (type) {
            case LoggerType::FILE:
                return std::make_unique<FileLogger>();
            case LoggerType::CONSOLE:
                return std::make_unique<ConsoleLogger>();
            default:
                throw std::invalid_argument("Unknown logger type");
        }
    }
};
```

### C 구현 (함수 포인터 활용)
```c
// Logger interface using function pointers
typedef struct {
    void (*log)(const char* message, void* context);
    void (*destroy)(void* context);
    void* context;
} logger_t;

// File logger implementation
void file_logger_log(const char* message, void* context) {
    FILE* file = (FILE*)context;
    fprintf(file, "%s\n", message);
}

void file_logger_destroy(void* context) {
    FILE* file = (FILE*)context;
    if (file) fclose(file);
}

// Factory function
logger_t* create_logger(const char* type) {
    logger_t* logger = malloc(sizeof(logger_t));
    if (!logger) return NULL;
    
    if (strcmp(type, "file") == 0) {
        FILE* file = fopen("log.txt", "a");
        logger->log = file_logger_log;
        logger->destroy = file_logger_destroy;
        logger->context = file;
    } else if (strcmp(type, "console") == 0) {
        logger->log = console_logger_log;
        logger->destroy = NULL;
        logger->context = NULL;
    }
    
    return logger;
}
```

## 3. Singleton Pattern

### Thread-Safe C++ Implementation
```cpp
class ConfigManager {
private:
    ConfigManager() = default;
    static std::once_flag init_flag_;
    static std::unique_ptr<ConfigManager> instance_;
    
public:
    // Delete copy/move operations
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    static ConfigManager& getInstance() {
        std::call_once(init_flag_, []() {
            instance_.reset(new ConfigManager());
        });
        return *instance_;
    }
    
    void loadConfig(const std::string& filename) {
        // Load configuration
    }
};

// Static member definitions
std::once_flag ConfigManager::init_flag_;
std::unique_ptr<ConfigManager> ConfigManager::instance_;
```

### C Implementation
```c
typedef struct {
    char config_file[256];
    int port;
    int max_connections;
} config_manager_t;

static config_manager_t* instance = NULL;
static pthread_mutex_t singleton_mutex = PTHREAD_MUTEX_INITIALIZER;

config_manager_t* get_config_manager() {
    if (instance == NULL) {
        pthread_mutex_lock(&singleton_mutex);
        if (instance == NULL) {  // Double-checked locking
            instance = malloc(sizeof(config_manager_t));
            // Initialize instance
        }
        pthread_mutex_unlock(&singleton_mutex);
    }
    return instance;
}
```

## 4. Observer Pattern

### C++ Implementation
```cpp
// Observer interface
class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void update(const std::string& message) = 0;
};

// Subject
class EventManager {
private:
    std::vector<std::weak_ptr<IObserver>> observers_;
    std::mutex mutex_;
    
public:
    void subscribe(std::shared_ptr<IObserver> observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.push_back(observer);
    }
    
    void notify(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Remove expired observers
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [](const std::weak_ptr<IObserver>& o) { return o.expired(); }),
            observers_.end()
        );
        
        // Notify all valid observers
        for (auto& weak_obs : observers_) {
            if (auto observer = weak_obs.lock()) {
                observer->update(message);
            }
        }
    }
};

// Concrete observer
class LogObserver : public IObserver {
private:
    std::string name_;
    
public:
    explicit LogObserver(const std::string& name) : name_(name) {}
    
    void update(const std::string& message) override {
        std::cout << name_ << " received: " << message << std::endl;
    }
};
```

### C Implementation (Callback-based)
```c
typedef void (*event_callback_t)(const char* message, void* context);

typedef struct callback_node {
    event_callback_t callback;
    void* context;
    struct callback_node* next;
} callback_node_t;

typedef struct {
    callback_node_t* head;
    pthread_mutex_t mutex;
} event_manager_t;

void event_manager_subscribe(event_manager_t* manager, 
                           event_callback_t callback, 
                           void* context) {
    callback_node_t* node = malloc(sizeof(callback_node_t));
    node->callback = callback;
    node->context = context;
    
    pthread_mutex_lock(&manager->mutex);
    node->next = manager->head;
    manager->head = node;
    pthread_mutex_unlock(&manager->mutex);
}

void event_manager_notify(event_manager_t* manager, const char* message) {
    pthread_mutex_lock(&manager->mutex);
    callback_node_t* current = manager->head;
    while (current) {
        current->callback(message, current->context);
        current = current->next;
    }
    pthread_mutex_unlock(&manager->mutex);
}
```

## 5. Strategy Pattern

### C++ Implementation
```cpp
// Strategy interface
class CompressionStrategy {
public:
    virtual ~CompressionStrategy() = default;
    virtual std::vector<uint8_t> compress(const std::vector<uint8_t>& data) = 0;
};

// Concrete strategies
class ZipCompression : public CompressionStrategy {
public:
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data) override {
        // ZIP compression implementation
        return data;  // Placeholder
    }
};

class LZ4Compression : public CompressionStrategy {
public:
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data) override {
        // LZ4 compression implementation
        return data;  // Placeholder
    }
};

// Context
class FileCompressor {
private:
    std::unique_ptr<CompressionStrategy> strategy_;
    
public:
    void setStrategy(std::unique_ptr<CompressionStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    void compressFile(const std::string& filename) {
        if (!strategy_) {
            throw std::runtime_error("No compression strategy set");
        }
        
        // Read file
        std::vector<uint8_t> data = readFile(filename);
        
        // Compress using strategy
        auto compressed = strategy_->compress(data);
        
        // Write compressed data
        writeFile(filename + ".compressed", compressed);
    }
};
```

### C Implementation
```c
typedef struct {
    size_t (*compress)(const uint8_t* input, size_t input_size, 
                      uint8_t* output, size_t output_size);
    const char* name;
} compression_strategy_t;

// Concrete strategies
size_t zip_compress(const uint8_t* input, size_t input_size,
                   uint8_t* output, size_t output_size) {
    // ZIP compression
    return 0;  // Placeholder
}

size_t lz4_compress(const uint8_t* input, size_t input_size,
                   uint8_t* output, size_t output_size) {
    // LZ4 compression
    return 0;  // Placeholder
}

static compression_strategy_t zip_strategy = {
    .compress = zip_compress,
    .name = "ZIP"
};

static compression_strategy_t lz4_strategy = {
    .compress = lz4_compress,
    .name = "LZ4"
};

// Context
typedef struct {
    compression_strategy_t* strategy;
} file_compressor_t;

void compress_file(file_compressor_t* compressor, const char* filename) {
    if (!compressor->strategy) {
        fprintf(stderr, "No compression strategy set\n");
        return;
    }
    
    // Read file and compress
    // ...
}
```

## 6. Command Pattern

### C++ Implementation
```cpp
// Command interface
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

// Receiver
class TextEditor {
private:
    std::string content_;
    
public:
    void insert(size_t pos, const std::string& text) {
        content_.insert(pos, text);
    }
    
    void erase(size_t pos, size_t len) {
        content_.erase(pos, len);
    }
    
    const std::string& getContent() const { return content_; }
};

// Concrete command
class InsertCommand : public Command {
private:
    TextEditor& editor_;
    size_t position_;
    std::string text_;
    
public:
    InsertCommand(TextEditor& editor, size_t pos, const std::string& text)
        : editor_(editor), position_(pos), text_(text) {}
    
    void execute() override {
        editor_.insert(position_, text_);
    }
    
    void undo() override {
        editor_.erase(position_, text_.length());
    }
};

// Command manager with undo/redo
class CommandManager {
private:
    std::stack<std::unique_ptr<Command>> undoStack_;
    std::stack<std::unique_ptr<Command>> redoStack_;
    
public:
    void executeCommand(std::unique_ptr<Command> cmd) {
        cmd->execute();
        undoStack_.push(std::move(cmd));
        // Clear redo stack
        while (!redoStack_.empty()) {
            redoStack_.pop();
        }
    }
    
    void undo() {
        if (!undoStack_.empty()) {
            auto cmd = std::move(undoStack_.top());
            undoStack_.pop();
            cmd->undo();
            redoStack_.push(std::move(cmd));
        }
    }
    
    void redo() {
        if (!redoStack_.empty()) {
            auto cmd = std::move(redoStack_.top());
            redoStack_.pop();
            cmd->execute();
            undoStack_.push(std::move(cmd));
        }
    }
};
```

## 7. Template Method Pattern

### C++ Implementation
```cpp
// Abstract class with template method
class DataProcessor {
public:
    // Template method
    void process(const std::string& filename) {
        auto data = loadData(filename);
        auto processed = processData(data);
        saveResults(processed);
        cleanup();
    }
    
    virtual ~DataProcessor() = default;
    
protected:
    // Steps to be implemented by subclasses
    virtual std::vector<uint8_t> loadData(const std::string& filename) = 0;
    virtual std::vector<uint8_t> processData(const std::vector<uint8_t>& data) = 0;
    
    // Optional hook
    virtual void cleanup() {}
    
private:
    // Common implementation
    void saveResults(const std::vector<uint8_t>& data) {
        std::ofstream file("output.dat", std::ios::binary);
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
    }
};

// Concrete implementation
class ImageProcessor : public DataProcessor {
protected:
    std::vector<uint8_t> loadData(const std::string& filename) override {
        // Load image data
        return {};
    }
    
    std::vector<uint8_t> processData(const std::vector<uint8_t>& data) override {
        // Apply image processing
        return data;
    }
    
    void cleanup() override {
        // Clean up temporary files
    }
};
```

## 8. Object Pool Pattern

### C++ Implementation
```cpp
template<typename T>
class ObjectPool {
private:
    std::queue<std::unique_ptr<T>> available_;
    std::unordered_set<T*> inUse_;
    std::mutex mutex_;
    size_t maxSize_;
    
    std::function<std::unique_ptr<T>()> factory_;
    std::function<void(T*)> reset_;
    
public:
    ObjectPool(size_t maxSize, 
               std::function<std::unique_ptr<T>()> factory,
               std::function<void(T*)> reset = nullptr)
        : maxSize_(maxSize), factory_(factory), reset_(reset) {}
    
    std::unique_ptr<T, std::function<void(T*)>> acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (available_.empty() && inUse_.size() < maxSize_) {
            auto obj = factory_();
            T* ptr = obj.release();
            inUse_.insert(ptr);
            
            return std::unique_ptr<T, std::function<void(T*)>>(
                ptr,
                [this](T* p) { this->release(p); }
            );
        }
        
        if (!available_.empty()) {
            auto obj = std::move(available_.front());
            available_.pop();
            T* ptr = obj.release();
            inUse_.insert(ptr);
            
            return std::unique_ptr<T, std::function<void(T*)>>(
                ptr,
                [this](T* p) { this->release(p); }
            );
        }
        
        return nullptr;  // Pool exhausted
    }
    
private:
    void release(T* obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = inUse_.find(obj);
        if (it != inUse_.end()) {
            inUse_.erase(it);
            
            if (reset_) {
                reset_(obj);
            }
            
            available_.push(std::unique_ptr<T>(obj));
        }
    }
};

// Usage example
class Connection {
public:
    void reset() { /* Reset connection state */ }
    void send(const std::string& data) { /* Send data */ }
};

ObjectPool<Connection> connectionPool(
    10,  // Max 10 connections
    []() { return std::make_unique<Connection>(); },
    [](Connection* c) { c->reset(); }
);
```

### C Implementation
```c
typedef struct {
    void* objects;
    size_t object_size;
    size_t pool_size;
    size_t* free_list;
    size_t free_count;
    pthread_mutex_t mutex;
    void (*reset_func)(void*);
} object_pool_t;

object_pool_t* object_pool_create(size_t object_size, size_t pool_size,
                                 void (*reset_func)(void*)) {
    object_pool_t* pool = malloc(sizeof(object_pool_t));
    pool->object_size = object_size;
    pool->pool_size = pool_size;
    pool->objects = calloc(pool_size, object_size);
    pool->free_list = malloc(pool_size * sizeof(size_t));
    pool->reset_func = reset_func;
    
    // Initialize free list
    for (size_t i = 0; i < pool_size; i++) {
        pool->free_list[i] = i;
    }
    pool->free_count = pool_size;
    
    pthread_mutex_init(&pool->mutex, NULL);
    return pool;
}

void* object_pool_acquire(object_pool_t* pool) {
    pthread_mutex_lock(&pool->mutex);
    
    if (pool->free_count == 0) {
        pthread_mutex_unlock(&pool->mutex);
        return NULL;  // Pool exhausted
    }
    
    size_t index = pool->free_list[--pool->free_count];
    void* obj = (char*)pool->objects + (index * pool->object_size);
    
    pthread_mutex_unlock(&pool->mutex);
    return obj;
}

void object_pool_release(object_pool_t* pool, void* obj) {
    pthread_mutex_lock(&pool->mutex);
    
    // Calculate object index
    size_t index = ((char*)obj - (char*)pool->objects) / pool->object_size;
    
    // Reset object if function provided
    if (pool->reset_func) {
        pool->reset_func(obj);
    }
    
    // Add back to free list
    pool->free_list[pool->free_count++] = index;
    
    pthread_mutex_unlock(&pool->mutex);
}
```

## 9. Builder Pattern

### C++ Implementation
```cpp
// Product
class ServerConfig {
private:
    std::string host_;
    int port_;
    int maxConnections_;
    bool enableSSL_;
    std::string certPath_;
    std::chrono::seconds timeout_;
    
public:
    // Builder class
    class Builder {
    private:
        ServerConfig config_;
        
    public:
        Builder& withHost(const std::string& host) {
            config_.host_ = host;
            return *this;
        }
        
        Builder& withPort(int port) {
            config_.port_ = port;
            return *this;
        }
        
        Builder& withMaxConnections(int max) {
            config_.maxConnections_ = max;
            return *this;
        }
        
        Builder& withSSL(const std::string& certPath) {
            config_.enableSSL_ = true;
            config_.certPath_ = certPath;
            return *this;
        }
        
        Builder& withTimeout(std::chrono::seconds timeout) {
            config_.timeout_ = timeout;
            return *this;
        }
        
        ServerConfig build() {
            // Validation
            if (config_.host_.empty()) {
                throw std::runtime_error("Host is required");
            }
            if (config_.port_ <= 0 || config_.port_ > 65535) {
                throw std::runtime_error("Invalid port");
            }
            return std::move(config_);
        }
    };
    
    // Getters
    const std::string& getHost() const { return host_; }
    int getPort() const { return port_; }
    // ... other getters
};

// Usage
auto config = ServerConfig::Builder()
    .withHost("localhost")
    .withPort(8080)
    .withMaxConnections(100)
    .withSSL("/path/to/cert.pem")
    .withTimeout(std::chrono::seconds(30))
    .build();
```

## 10. Decorator Pattern

### C++ Implementation
```cpp
// Component interface
class Stream {
public:
    virtual ~Stream() = default;
    virtual void write(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> read(size_t size) = 0;
};

// Concrete component
class FileStream : public Stream {
private:
    std::fstream file_;
    
public:
    explicit FileStream(const std::string& filename) 
        : file_(filename, std::ios::binary | std::ios::in | std::ios::out) {}
    
    void write(const std::vector<uint8_t>& data) override {
        file_.write(reinterpret_cast<const char*>(data.data()), data.size());
    }
    
    std::vector<uint8_t> read(size_t size) override {
        std::vector<uint8_t> buffer(size);
        file_.read(reinterpret_cast<char*>(buffer.data()), size);
        buffer.resize(file_.gcount());
        return buffer;
    }
};

// Decorator base class
class StreamDecorator : public Stream {
protected:
    std::unique_ptr<Stream> stream_;
    
public:
    explicit StreamDecorator(std::unique_ptr<Stream> stream)
        : stream_(std::move(stream)) {}
};

// Concrete decorators
class CompressedStream : public StreamDecorator {
public:
    using StreamDecorator::StreamDecorator;
    
    void write(const std::vector<uint8_t>& data) override {
        auto compressed = compress(data);
        stream_->write(compressed);
    }
    
    std::vector<uint8_t> read(size_t size) override {
        auto compressed = stream_->read(size * 2);  // Read more for compressed data
        return decompress(compressed);
    }
    
private:
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data) {
        // Compression logic
        return data;  // Placeholder
    }
    
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data) {
        // Decompression logic
        return data;  // Placeholder
    }
};

class EncryptedStream : public StreamDecorator {
private:
    std::string key_;
    
public:
    EncryptedStream(std::unique_ptr<Stream> stream, const std::string& key)
        : StreamDecorator(std::move(stream)), key_(key) {}
    
    void write(const std::vector<uint8_t>& data) override {
        auto encrypted = encrypt(data);
        stream_->write(encrypted);
    }
    
    std::vector<uint8_t> read(size_t size) override {
        auto encrypted = stream_->read(size);
        return decrypt(encrypted);
    }
    
private:
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) {
        // Encryption logic
        return data;  // Placeholder
    }
    
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) {
        // Decryption logic
        return data;  // Placeholder
    }
};

// Usage: Compressed and encrypted file stream
auto stream = std::make_unique<EncryptedStream>(
    std::make_unique<CompressedStream>(
        std::make_unique<FileStream>("data.bin")
    ),
    "secret_key"
);
```

## 🔥 흔한 실수와 해결방법

### 1. 패턴 오남용

#### [SEQUENCE: 109] 과도한 추상화
```cpp
// ❌ 잘못된 예: 단순한 로거에 과도한 패턴 적용
class LoggerFactoryFactoryProvider {
    // 너무 많은 추상화 레이어
};

// ✅ 올바른 예: 필요한 만큼만 추상화
class Logger {
    // 직접적이고 간단한 인터페이스
};
```

### 2. 메모리 누수

#### [SEQUENCE: 110] Singleton 소멸자 문제
```cpp
// ❌ 잘못된 예: 정적 소멸 순서 문제
class BadSingleton {
    static BadSingleton* instance;
public:
    ~BadSingleton() {
        // 다른 정적 객체 접근 시 크래시 가능
    }
};

// ✅ 올바른 예: Meyers Singleton
class GoodSingleton {
public:
    static GoodSingleton& getInstance() {
        static GoodSingleton instance;
        return instance;
    }
};
```

### 3. 스레드 안전성

#### [SEQUENCE: 111] Observer 패턴의 동시성 문제
```cpp
// ❌ 잘못된 예: 순회 중 컬렉션 수정
void notify() {
    for (auto& observer : observers_) {
        observer->update();  // update()에서 unsubscribe() 호출 시 크래시
    }
}

// ✅ 올바른 예: 복사본으로 순회
void notify() {
    auto observers_copy = observers_;  // 스냅샷
    for (auto& observer : observers_copy) {
        if (observers_.find(observer) != observers_.end()) {
            observer->update();
        }
    }
}
```

## Best Practices

### 1. Pattern Selection
- 패턴을 과도하게 사용하지 마세요 (Over-engineering 주의)
- 문제에 적합한 패턴을 선택하세요
- 단순한 해결책이 있다면 패턴보다 우선하세요

### 2. C vs C++ 고려사항
- **C++**: RAII, 템플릿, 상속 활용
- **C**: 함수 포인터, 구조체, 명시적 메모리 관리

### 3. 성능 고려사항
- 가상 함수는 간접 호출 오버헤드 발생
- 템플릿은 컴파일 시간 증가 but 런타임 성능 향상
- 동적 할당 최소화 (Object Pool 패턴 활용)

### 4. 메모리 관리
- C++: 스마트 포인터 사용
- C: 명확한 소유권과 해제 규칙 정의

### 5. 스레드 안전성
- 패턴 구현 시 동시성 고려
- 적절한 동기화 메커니즘 사용

## 실습 프로젝트

### 프로젝트 1: 플러그인 로거 시스템 (기초)
**목표**: Factory와 Strategy 패턴을 사용한 확장 가능한 로거

**구현 사항**:
- 다양한 출력 대상 (파일, 콘솔, 네트워크)
- 런타임 플러그인 로딩
- 설정 기반 로거 생성
- 체인 가능한 필터링

### 프로젝트 2: 이벤트 기반 로그 시스템 (중급)
**목표**: Observer와 Command 패턴을 활용한 비동기 로깅

**구현 사항**:
- 이벤트 버스 구현
- 로그 명령 큐잉
- Undo/Redo 가능한 로그 작업
- 멀티스레드 안전성

### 프로젝트 3: 고성능 로그 파이프라인 (고급)
**목표**: 여러 패턴을 조합한 프로덕션급 로거

**구현 사항**:
- Object Pool로 메모리 최적화
- Decorator로 로그 변환 체인
- Builder로 복잡한 설정 관리
- Template Method로 로그 처리 커스터마이징

## 학습 체크리스트

### 기초 레벨 ✅
- [ ] RAII 원칙 이해 및 적용
- [ ] Factory 패턴 구현
- [ ] Singleton 패턴의 장단점 이해
- [ ] 간단한 Observer 패턴 구현

### 중급 레벨 📚
- [ ] Strategy 패턴으로 알고리즘 교체
- [ ] Command 패턴으로 작업 큐 구현
- [ ] Object Pool로 성능 최적화
- [ ] 스레드 안전한 패턴 구현

### 고급 레벨 🚀
- [ ] Template Method와 상속 계층 설계
- [ ] Decorator로 기능 동적 추가
- [ ] Builder로 DSL 구현
- [ ] 여러 패턴 조합 설계

### 전문가 레벨 🏆
- [ ] 패턴 간 트레이드오프 분석
- [ ] 도메인 특화 패턴 설계
- [ ] 성능 최적화된 패턴 구현
- [ ] 패턴 리팩토링 전략

## 추가 학습 자료

### 추천 도서
- "Design Patterns: Elements of Reusable Object-Oriented Software" - Gang of Four
- "Modern C++ Design" - Andrei Alexandrescu
- "Head First Design Patterns" - Freeman & Freeman
- "Game Programming Patterns" - Robert Nystrom

### 온라인 리소스
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Refactoring Guru - Design Patterns](https://refactoring.guru/design-patterns)
- [Source Making - Design Patterns](https://sourcemaking.com/design_patterns)

### 실습 도구
- UML 다이어그램 도구 (PlantUML, draw.io)
- 정적 분석 도구 (cppcheck, clang-tidy)
- 디자인 패턴 템플릿 생성기

---

# 부록: LogCaster에 덜 관련된 패턴들

다음 패턴들은 일반적인 C/C++ 개발에는 유용하지만 LogCaster 프로젝트에는 직접적으로 필요하지 않습니다. 참고용으로 보관합니다.

## Template Method Pattern

Template Method 패턴은 알고리즘의 골격을 정의하고 일부 단계를 서브클래스에서 구현하도록 하는 패턴입니다. LogCaster의 단순한 구조에서는 과도한 추상화가 될 수 있습니다.

## Decorator Pattern

Decorator 패턴은 객체에 동적으로 새로운 책임을 추가하는 패턴입니다. LogCaster의 로그 처리는 단순하여 이러한 복잡한 구조가 필요하지 않습니다.

## Builder Pattern

Builder 패턴은 복잡한 객체의 생성 과정을 캡슐화하는 패턴입니다. LogCaster의 설정은 간단하여 이 패턴이 과도할 수 있습니다.

---

*💡 팁: 디자인 패턴은 도구입니다. 모든 문제를 패턴으로 해결하려 하지 마세요. 단순함이 최고의 디자인입니다!*