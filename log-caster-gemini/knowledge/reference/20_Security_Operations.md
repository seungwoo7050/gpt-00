# 16. 보안 및 운영 🔒
## 안전한 서버 운영을 위한 필수 가이드

---

## 📋 문서 정보

- **난이도**: 고급 (Advanced)
- **예상 학습 시간**: 20-25시간
- **필요 선수 지식**: 
  - C/C++ 중급 이상
  - 네트워크 보안 기초
  - 암호화 개념
  - Docker/Kubernetes 기초
- **실습 환경**: Linux 프로덕션 환경, OpenSSL, Docker

---

## 🎯 이 문서에서 배울 내용

**"프로덕션 서버를 안전하게 지키려면 어떻게 해야 할까요?"** 🏰

정말 중요한 질문이에요! 프로덕션 서버 운영은 마치 **은행의 금고**를 관리하는 것과 같아요. 고객의 소중한 정보(로그 데이터)를 보호하면서도, 필요한 사람들이 언제든 접근할 수 있도록 해야 하죠!

마치 **최첨단 보안 시설**처럼, 여러 겹의 보안 장치 - 암호화, 접근 제어, 모니터링 - 가 모두 함께 작동해야 해요. 🛡️

이 문서는 LogCaster를 보안이 강화된 프로덕션 환경에서 운영하기 위한 핵심 보안 기술들을 다룹니다. 로그 데이터 암호화, 민감정보 마스킹, 접근 제어, 감사 로그, 컨테이너화 및 모니터링 등 엔터프라이즈급 보안 요구사항을 충족하는 방법을 설명합니다.

---

## 1. 로그 데이터 암호화

**"로그에 중요한 정보가 들어있다면?"** 🔐

좋은 관찰이에요! 로그는 마치 **일기장**과 같아서, 때로는 비밀스러운 내용도 들어있어요. 그래서 **암호로 잠가두는** 것이 필요하죠!

로그에는 민감한 정보가 포함될 수 있으므로 저장 시 암호화(encryption at rest)와 전송 시 암호화(encryption in transit)가 필요합니다.

### AES 암호화를 사용한 로그 저장 - 디지털 금고 💎

```cpp
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <cstring>

class EncryptedLogWriter {
private:
    std::string key_file_path_;
    std::string log_file_path_;
    std::vector<unsigned char> encryption_key_;
    
public:
    EncryptedLogWriter(const std::string& key_file, const std::string& log_file)
        : key_file_path_(key_file), log_file_path_(log_file) {
        
        if (!loadOrGenerateKey()) {
            throw std::runtime_error("Failed to initialize encryption key");
        }
    }
    
    bool writeEncryptedLog(const std::string& log_message) {
        try {
            // 로그 메시지 암호화
            std::vector<unsigned char> encrypted_data;
            std::vector<unsigned char> iv(AES_BLOCK_SIZE);
            
            if (!encryptMessage(log_message, encrypted_data, iv)) {
                std::cerr << "Failed to encrypt log message" << std::endl;
                return false;
            }
            
            // 암호화된 데이터를 파일에 저장
            return writeEncryptedDataToFile(encrypted_data, iv);
            
        } catch (const std::exception& e) {
            std::cerr << "Encryption error: " << e.what() << std::endl;
            return false;
        }
    }
    
    std::string readAndDecryptLog(size_t entry_index) {
        try {
            std::vector<unsigned char> encrypted_data;
            std::vector<unsigned char> iv;
            
            if (!readEncryptedDataFromFile(entry_index, encrypted_data, iv)) {
                return "";
            }
            
            return decryptMessage(encrypted_data, iv);
            
        } catch (const std::exception& e) {
            std::cerr << "Decryption error: " << e.what() << std::endl;
            return "";
        }
    }

private:
    bool loadOrGenerateKey() {
        std::ifstream key_file(key_file_path_, std::ios::binary);
        
        if (key_file.is_open()) {
            // 기존 키 로드
            encryption_key_.resize(32); // AES-256
            key_file.read(reinterpret_cast<char*>(encryption_key_.data()), 32);
            key_file.close();
            return true;
        } else {
            // 새 키 생성
            return generateNewKey();
        }
    }
    
    bool generateNewKey() {
        encryption_key_.resize(32); // AES-256 키 크기
        
        if (RAND_bytes(encryption_key_.data(), 32) != 1) {
            std::cerr << "Failed to generate random key" << std::endl;
            return false;
        }
        
        // 키를 파일에 저장 (실제 환경에서는 HSM이나 키 관리 서비스 사용)
        std::ofstream key_file(key_file_path_, std::ios::binary);
        if (!key_file.is_open()) {
            std::cerr << "Failed to create key file" << std::endl;
            return false;
        }
        
        key_file.write(reinterpret_cast<const char*>(encryption_key_.data()), 32);
        key_file.close();
        
        // 키 파일 권한 설정 (Unix 계열에서)
        #ifndef _WIN32
        chmod(key_file_path_.c_str(), 0600); // 소유자만 읽기/쓰기
        #endif
        
        return true;
    }
    
    bool encryptMessage(const std::string& plaintext, 
                       std::vector<unsigned char>& encrypted_data,
                       std::vector<unsigned char>& iv) {
        
        // IV 생성
        if (RAND_bytes(iv.data(), AES_BLOCK_SIZE) != 1) {
            return false;
        }
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) return false;
        
        // 암호화 초기화
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, 
                              encryption_key_.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        // 출력 버퍼 크기 계산
        int max_output_len = plaintext.length() + AES_BLOCK_SIZE;
        encrypted_data.resize(max_output_len);
        
        int len;
        int ciphertext_len = 0;
        
        // 암호화 수행
        if (EVP_EncryptUpdate(ctx, encrypted_data.data(), &len,
                             reinterpret_cast<const unsigned char*>(plaintext.c_str()),
                             plaintext.length()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        ciphertext_len = len;
        
        // 최종 블록 처리
        if (EVP_EncryptFinal_ex(ctx, encrypted_data.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        ciphertext_len += len;
        
        encrypted_data.resize(ciphertext_len);
        EVP_CIPHER_CTX_free(ctx);
        
        return true;
    }
    
    std::string decryptMessage(const std::vector<unsigned char>& encrypted_data,
                              const std::vector<unsigned char>& iv) {
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) return "";
        
        // 복호화 초기화
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                              encryption_key_.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return "";
        }
        
        std::vector<unsigned char> plaintext(encrypted_data.size() + AES_BLOCK_SIZE);
        int len;
        int plaintext_len = 0;
        
        // 복호화 수행
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                             encrypted_data.data(), encrypted_data.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return "";
        }
        plaintext_len = len;
        
        // 최종 블록 처리
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return "";
        }
        plaintext_len += len;
        
        EVP_CIPHER_CTX_free(ctx);
        
        return std::string(reinterpret_cast<char*>(plaintext.data()), plaintext_len);
    }
    
    bool writeEncryptedDataToFile(const std::vector<unsigned char>& encrypted_data,
                                 const std::vector<unsigned char>& iv) {
        
        std::ofstream file(log_file_path_, std::ios::binary | std::ios::app);
        if (!file.is_open()) return false;
        
        // 헤더: IV 크기 + 데이터 크기
        uint32_t iv_size = iv.size();
        uint32_t data_size = encrypted_data.size();
        
        file.write(reinterpret_cast<const char*>(&iv_size), sizeof(iv_size));
        file.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));
        
        // IV와 암호화된 데이터 저장
        file.write(reinterpret_cast<const char*>(iv.data()), iv.size());
        file.write(reinterpret_cast<const char*>(encrypted_data.data()), encrypted_data.size());
        
        file.close();
        return true;
    }
    
    bool readEncryptedDataFromFile(size_t entry_index,
                                  std::vector<unsigned char>& encrypted_data,
                                  std::vector<unsigned char>& iv) {
        
        std::ifstream file(log_file_path_, std::ios::binary);
        if (!file.is_open()) return false;
        
        // 특정 엔트리까지 건너뛰기
        for (size_t i = 0; i < entry_index; i++) {
            uint32_t iv_size, data_size;
            
            file.read(reinterpret_cast<char*>(&iv_size), sizeof(iv_size));
            file.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
            
            if (file.eof()) return false;
            
            file.seekg(iv_size + data_size, std::ios::cur);
        }
        
        // 대상 엔트리 읽기
        uint32_t iv_size, data_size;
        file.read(reinterpret_cast<char*>(&iv_size), sizeof(iv_size));
        file.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
        
        if (file.eof()) return false;
        
        iv.resize(iv_size);
        encrypted_data.resize(data_size);
        
        file.read(reinterpret_cast<char*>(iv.data()), iv_size);
        file.read(reinterpret_cast<char*>(encrypted_data.data()), data_size);
        
        file.close();
        return true;
    }
};
```

### TLS를 사용한 네트워크 로그 전송

```cpp
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class SecureLogTransmitter {
private:
    SSL_CTX* ssl_ctx_;
    SSL* ssl_;
    int socket_fd_;
    bool connected_;

public:
    SecureLogTransmitter() : ssl_ctx_(nullptr), ssl_(nullptr), socket_fd_(-1), connected_(false) {
        SSL_load_error_strings();
        SSL_library_init();
        OpenSSL_add_all_algorithms();
    }
    
    ~SecureLogTransmitter() {
        disconnect();
        if (ssl_ctx_) {
            SSL_CTX_free(ssl_ctx_);
        }
    }
    
    bool connect(const std::string& hostname, int port, 
                const std::string& cert_file = "", 
                const std::string& key_file = "") {
        
        // SSL 컨텍스트 생성
        ssl_ctx_ = SSL_CTX_new(TLS_client_method());
        if (!ssl_ctx_) {
            std::cerr << "Failed to create SSL context" << std::endl;
            return false;
        }
        
        // 클라이언트 인증서 설정 (선택적)
        if (!cert_file.empty() && !key_file.empty()) {
            if (SSL_CTX_use_certificate_file(ssl_ctx_, cert_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
                std::cerr << "Failed to load certificate file" << std::endl;
                return false;
            }
            
            if (SSL_CTX_use_PrivateKey_file(ssl_ctx_, key_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
                std::cerr << "Failed to load private key file" << std::endl;
                return false;
            }
        }
        
        // 소켓 생성
        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd_ < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
        
        // 서버에 연결
        struct sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, hostname.c_str(), &server_addr.sin_addr);
        
        if (::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Failed to connect to server" << std::endl;
            close(socket_fd_);
            return false;
        }
        
        // SSL 연결 설정
        ssl_ = SSL_new(ssl_ctx_);
        SSL_set_fd(ssl_, socket_fd_);
        
        if (SSL_connect(ssl_) <= 0) {
            std::cerr << "SSL connection failed" << std::endl;
            ERR_print_errors_fp(stderr);
            return false;
        }
        
        connected_ = true;
        std::cout << "Secure connection established with " << hostname << ":" << port << std::endl;
        
        // 서버 인증서 검증
        if (!verifyCertificate(hostname)) {
            std::cerr << "Certificate verification failed" << std::endl;
            disconnect();
            return false;
        }
        
        return true;
    }
    
    bool sendLog(const std::string& log_message) {
        if (!connected_ || !ssl_) {
            std::cerr << "Not connected to server" << std::endl;
            return false;
        }
        
        // 메시지 길이 + 메시지 전송
        uint32_t message_length = log_message.length();
        
        if (SSL_write(ssl_, &message_length, sizeof(message_length)) <= 0) {
            std::cerr << "Failed to send message length" << std::endl;
            return false;
        }
        
        if (SSL_write(ssl_, log_message.c_str(), message_length) <= 0) {
            std::cerr << "Failed to send log message" << std::endl;
            return false;
        }
        
        return true;
    }
    
    void disconnect() {
        if (ssl_) {
            SSL_shutdown(ssl_);
            SSL_free(ssl_);
            ssl_ = nullptr;
        }
        
        if (socket_fd_ >= 0) {
            close(socket_fd_);
            socket_fd_ = -1;
        }
        
        connected_ = false;
    }

private:
    bool verifyCertificate(const std::string& expected_hostname) {
        X509* cert = SSL_get_peer_certificate(ssl_);
        if (!cert) {
            std::cerr << "No peer certificate" << std::endl;
            return false;
        }
        
        // 인증서 검증 (간소화된 예제)
        long verify_result = SSL_get_verify_result(ssl_);
        if (verify_result != X509_V_OK) {
            std::cerr << "Certificate verification failed: " << verify_result << std::endl;
            X509_free(cert);
            return false;
        }
        
        // 호스트명 검증 (실제로는 더 정교한 검증 필요)
        char* subject_name = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        std::cout << "Server certificate subject: " << subject_name << std::endl;
        
        OPENSSL_free(subject_name);
        X509_free(cert);
        
        return true;
    }
};
```

---

## 2. 민감정보 마스킹과 데이터 보호

로그에서 개인정보, 패스워드, API 키 등의 민감한 정보를 자동으로 마스킹해야 합니다.

### 정규식 기반 민감정보 마스킹

```cpp
#include <regex>
#include <unordered_map>
#include <string>

class SensitiveDataMasker {
private:
    struct MaskingRule {
        std::regex pattern;
        std::string replacement;
        std::string description;
        
        MaskingRule(const std::string& regex_pattern, 
                   const std::string& repl, 
                   const std::string& desc)
            : pattern(regex_pattern), replacement(repl), description(desc) {}
    };
    
    std::vector<MaskingRule> masking_rules_;

public:
    SensitiveDataMasker() {
        initializeDefaultRules();
    }
    
    std::string maskSensitiveData(const std::string& log_message) {
        std::string masked_message = log_message;
        
        for (const auto& rule : masking_rules_) {
            try {
                masked_message = std::regex_replace(masked_message, rule.pattern, rule.replacement);
            } catch (const std::regex_error& e) {
                std::cerr << "Regex error in rule '" << rule.description << "': " << e.what() << std::endl;
            }
        }
        
        return masked_message;
    }
    
    void addCustomRule(const std::string& pattern, 
                      const std::string& replacement,
                      const std::string& description) {
        try {
            masking_rules_.emplace_back(pattern, replacement, description);
            std::cout << "Added masking rule: " << description << std::endl;
        } catch (const std::regex_error& e) {
            std::cerr << "Invalid regex pattern: " << e.what() << std::endl;
        }
    }
    
    void listRules() {
        std::cout << "\n=== Active Masking Rules ===" << std::endl;
        for (size_t i = 0; i < masking_rules_.size(); i++) {
            std::cout << i + 1 << ". " << masking_rules_[i].description << std::endl;
        }
    }

private:
    void initializeDefaultRules() {
        // 이메일 주소 마스킹
        addCustomRule(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})",
                     "***@***.***", "Email addresses");
        
        // 전화번호 마스킹 (다양한 형식)
        addCustomRule(R"(\b\d{3}-\d{3,4}-\d{4}\b)",
                     "***-***-****", "Phone numbers (xxx-xxx-xxxx)");
        
        addCustomRule(R"(\b\d{3}\.\d{3,4}\.\d{4}\b)",
                     "***.***.****", "Phone numbers (xxx.xxx.xxxx)");
        
        // 신용카드 번호 마스킹
        addCustomRule(R"(\b\d{4}[\s-]?\d{4}[\s-]?\d{4}[\s-]?\d{4}\b)",
                     "****-****-****-****", "Credit card numbers");
        
        // 주민등록번호 마스킹 (한국)
        addCustomRule(R"(\b\d{6}-\d{7}\b)",
                     "******-*******", "Korean SSN");
        
        // IP 주소 부분 마스킹
        addCustomRule(R"(\b(\d{1,3}\.\d{1,3}\.\d{1,3}\.)\d{1,3}\b)",
                     "$1***", "IP addresses");
        
        // 패스워드 필드 마스킹
        addCustomRule(R"((password|pwd|pass|secret|key)\s*[:=]\s*[\"']?([^\"'\s,}]+)[\"']?)",
                     "$1=***", "Password fields", std::regex_constants::icase);
        
        // API 키 패턴 마스킹
        addCustomRule(R"((api[_-]?key|access[_-]?token|bearer)\s*[:=]\s*[\"']?([A-Za-z0-9+/=]{20,})[\"']?)",
                     "$1=***", "API keys and tokens", std::regex_constants::icase);
        
        // URL의 쿼리 파라미터에서 민감정보 마스킹
        addCustomRule(R"(([?&](password|token|key|secret)=)[^&\s]+)",
                     "$1***", "URL query parameters");
        
        // JSON 필드에서 민감정보 마스킹
        addCustomRule(R"(("(?:password|secret|token|key|credential)"\s*:\s*")[^"]+(")",
                     "$1***$2", "JSON sensitive fields", std::regex_constants::icase);
    }
    
    void addCustomRule(const std::string& pattern, 
                      const std::string& replacement,
                      const std::string& description,
                      std::regex_constants::syntax_option_type flags = std::regex_constants::ECMAScript) {
        try {
            masking_rules_.emplace_back(std::regex(pattern, flags), replacement, description);
        } catch (const std::regex_error& e) {
            std::cerr << "Failed to add rule '" << description << "': " << e.what() << std::endl;
        }
    }
};

// 컨텍스트 인식 마스킹
class ContextAwareMasker {
private:
    SensitiveDataMasker base_masker_;
    std::unordered_map<std::string, std::vector<std::string>> context_patterns_;

public:
    ContextAwareMasker() {
        initializeContextPatterns();
    }
    
    std::string maskWithContext(const std::string& log_message, const std::string& context = "") {
        std::string masked = base_masker_.maskSensitiveData(log_message);
        
        // 컨텍스트별 추가 마스킹
        if (!context.empty() && context_patterns_.count(context)) {
            for (const auto& pattern : context_patterns_[context]) {
                try {
                    std::regex context_regex(pattern);
                    masked = std::regex_replace(masked, context_regex, "***");
                } catch (const std::regex_error& e) {
                    std::cerr << "Context regex error: " << e.what() << std::endl;
                }
            }
        }
        
        return masked;
    }
    
    void addContextPattern(const std::string& context, const std::string& pattern) {
        context_patterns_[context].push_back(pattern);
    }

private:
    void initializeContextPatterns() {
        // 데이터베이스 컨텍스트
        context_patterns_["database"] = {
            R"(INSERT INTO \w+ \([^)]+\) VALUES \([^)]+\))", // INSERT 쿼리의 값들
            R"(UPDATE \w+ SET [^W]+ WHERE)", // UPDATE 쿼리의 SET 절
        };
        
        // HTTP 컨텍스트
        context_patterns_["http"] = {
            R"(Authorization: [^\r\n]+)", // Authorization 헤더
            R"(Cookie: [^\r\n]+)", // Cookie 헤더
        };
        
        // 파일 시스템 컨텍스트
        context_patterns_["filesystem"] = {
            R"(/home/[^/\s]+)", // 사용자 홈 디렉터리 경로
            R"(C:\\Users\\[^\\s]+)", // Windows 사용자 디렉터리
        };
    }
};

// 사용 예제
int main() {
    ContextAwareMasker masker;
    
    // 테스트 로그 메시지들
    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"User login failed for email: john.doe@example.com", ""},
        {"Database query: INSERT INTO users (email, password) VALUES ('user@test.com', 'secret123')", "database"},
        {"HTTP request: Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9", "http"},
        {"Credit card payment: 4532-1234-5678-9012 charged $99.99", ""},
        {"API call with key: api_key=abc123def456ghi789", ""},
        {"User data: {\"email\":\"test@example.com\",\"password\":\"mypassword\"}", ""},
        {"Phone verification: 010-1234-5678", ""},
        {"Access from IP: 192.168.1.100", ""}
    };
    
    std::cout << "=== Sensitive Data Masking Test ===" << std::endl;
    
    for (const auto& test_case : test_cases) {
        std::cout << "\nOriginal: " << test_case.first << std::endl;
        std::cout << "Masked:   " << masker.maskWithContext(test_case.first, test_case.second) << std::endl;
    }
    
    return 0;
}
```

---

## 3. 접근 제어와 인증

로그 시스템에 대한 접근을 제어하고 사용자를 인증해야 합니다.

### 역할 기반 접근 제어 (RBAC)

```cpp
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <chrono>

enum class Permission {
    READ_LOGS,
    WRITE_LOGS,
    DELETE_LOGS,
    CONFIGURE_SYSTEM,
    MANAGE_USERS,
    VIEW_METRICS,
    EXPORT_LOGS
};

enum class LogLevel {
    DEBUG, INFO, WARN, ERROR, FATAL
};

class Role {
private:
    std::string name_;
    std::unordered_set<Permission> permissions_;
    std::unordered_set<std::string> allowed_log_categories_;
    LogLevel min_log_level_;

public:
    Role(const std::string& name, LogLevel min_level = LogLevel::DEBUG)
        : name_(name), min_log_level_(min_level) {}
    
    void addPermission(Permission perm) {
        permissions_.insert(perm);
    }
    
    void addLogCategory(const std::string& category) {
        allowed_log_categories_.insert(category);
    }
    
    bool hasPermission(Permission perm) const {
        return permissions_.count(perm) > 0;
    }
    
    bool canAccessLogCategory(const std::string& category) const {
        return allowed_log_categories_.empty() || allowed_log_categories_.count(category) > 0;
    }
    
    bool canAccessLogLevel(LogLevel level) const {
        return static_cast<int>(level) >= static_cast<int>(min_log_level_);
    }
    
    const std::string& getName() const { return name_; }
};

class User {
private:
    std::string username_;
    std::string password_hash_;
    std::vector<Role> roles_;
    std::chrono::system_clock::time_point last_login_;
    bool active_;

public:
    User(const std::string& username, const std::string& password_hash)
        : username_(username), password_hash_(password_hash), active_(true) {}
    
    void addRole(const Role& role) {
        roles_.push_back(role);
    }
    
    bool hasPermission(Permission perm) const {
        if (!active_) return false;
        
        for (const auto& role : roles_) {
            if (role.hasPermission(perm)) {
                return true;
            }
        }
        return false;
    }
    
    bool canAccessLog(const std::string& category, LogLevel level) const {
        if (!active_) return false;
        
        for (const auto& role : roles_) {
            if (role.canAccessLogCategory(category) && role.canAccessLogLevel(level)) {
                return true;
            }
        }
        return false;
    }
    
    void updateLastLogin() {
        last_login_ = std::chrono::system_clock::now();
    }
    
    const std::string& getUsername() const { return username_; }
    bool isActive() const { return active_; }
    void setActive(bool active) { active_ = active; }
    
    // 간단한 패스워드 검증 (실제로는 bcrypt 등 사용)
    bool verifyPassword(const std::string& password) const {
        return password_hash_ == std::to_string(std::hash<std::string>{}(password));
    }
};

class AccessControlManager {
private:
    std::unordered_map<std::string, User> users_;
    std::unordered_map<std::string, Role> roles_;
    std::unordered_map<std::string, std::chrono::system_clock::time_point> active_sessions_;

public:
    AccessControlManager() {
        initializeDefaultRoles();
        initializeDefaultUsers();
    }
    
    // 사용자 인증
    std::string authenticate(const std::string& username, const std::string& password) {
        auto user_it = users_.find(username);
        if (user_it == users_.end() || !user_it->second.isActive()) {
            logSecurityEvent("Failed login attempt for user: " + username);
            return "";
        }
        
        if (!user_it->second.verifyPassword(password)) {
            logSecurityEvent("Invalid password for user: " + username);
            return "";
        }
        
        // 세션 토큰 생성 (간소화)
        std::string session_token = generateSessionToken(username);
        active_sessions_[session_token] = std::chrono::system_clock::now();
        
        user_it->second.updateLastLogin();
        logSecurityEvent("Successful login for user: " + username);
        
        return session_token;
    }
    
    // 권한 확인
    bool authorize(const std::string& session_token, Permission permission) {
        User* user = getUserFromSession(session_token);
        if (!user) return false;
        
        bool authorized = user->hasPermission(permission);
        
        if (!authorized) {
            logSecurityEvent("Unauthorized access attempt by user: " + user->getUsername() +
                           " for permission: " + std::to_string(static_cast<int>(permission)));
        }
        
        return authorized;
    }
    
    // 로그 접근 권한 확인
    bool authorizeLogAccess(const std::string& session_token, 
                           const std::string& log_category, 
                           LogLevel log_level) {
        User* user = getUserFromSession(session_token);
        if (!user) return false;
        
        bool authorized = user->canAccessLog(log_category, log_level);
        
        if (!authorized) {
            logSecurityEvent("Unauthorized log access attempt by user: " + user->getUsername() +
                           " for category: " + log_category + 
                           " level: " + std::to_string(static_cast<int>(log_level)));
        }
        
        return authorized;
    }
    
    // 세션 무효화
    void logout(const std::string& session_token) {
        auto session_it = active_sessions_.find(session_token);
        if (session_it != active_sessions_.end()) {
            User* user = getUserFromSession(session_token);
            if (user) {
                logSecurityEvent("User logged out: " + user->getUsername());
            }
            active_sessions_.erase(session_it);
        }
    }
    
    // 만료된 세션 정리
    void cleanupExpiredSessions() {
        auto now = std::chrono::system_clock::now();
        auto session_timeout = std::chrono::hours(24);
        
        for (auto it = active_sessions_.begin(); it != active_sessions_.end();) {
            if (now - it->second > session_timeout) {
                logSecurityEvent("Session expired: " + it->first);
                it = active_sessions_.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    void initializeDefaultRoles() {
        // 관리자 역할
        Role admin_role("admin");
        admin_role.addPermission(Permission::READ_LOGS);
        admin_role.addPermission(Permission::WRITE_LOGS);
        admin_role.addPermission(Permission::DELETE_LOGS);
        admin_role.addPermission(Permission::CONFIGURE_SYSTEM);
        admin_role.addPermission(Permission::MANAGE_USERS);
        admin_role.addPermission(Permission::VIEW_METRICS);
        admin_role.addPermission(Permission::EXPORT_LOGS);
        roles_["admin"] = admin_role;
        
        // 개발자 역할
        Role developer_role("developer", LogLevel::DEBUG);
        developer_role.addPermission(Permission::READ_LOGS);
        developer_role.addPermission(Permission::WRITE_LOGS);
        developer_role.addPermission(Permission::VIEW_METRICS);
        developer_role.addLogCategory("APPLICATION");
        developer_role.addLogCategory("DEBUG");
        roles_["developer"] = developer_role;
        
        // 운영자 역할
        Role operator_role("operator", LogLevel::INFO);
        operator_role.addPermission(Permission::READ_LOGS);
        operator_role.addPermission(Permission::VIEW_METRICS);
        operator_role.addLogCategory("SYSTEM");
        operator_role.addLogCategory("SECURITY");
        roles_["operator"] = operator_role;
        
        // 감사자 역할
        Role auditor_role("auditor", LogLevel::WARN);
        auditor_role.addPermission(Permission::READ_LOGS);
        auditor_role.addPermission(Permission::EXPORT_LOGS);
        auditor_role.addLogCategory("SECURITY");
        auditor_role.addLogCategory("AUDIT");
        roles_["auditor"] = auditor_role;
    }
    
    void initializeDefaultUsers() {
        // 기본 관리자 계정 (실제 환경에서는 초기 설정 후 변경)
        User admin("admin", std::to_string(std::hash<std::string>{}("admin123")));
        admin.addRole(roles_["admin"]);
        users_["admin"] = admin;
        
        // 개발자 계정
        User developer("developer", std::to_string(std::hash<std::string>{}("dev123")));
        developer.addRole(roles_["developer"]);
        users_["developer"] = developer;
        
        // 운영자 계정
        User operator_user("operator", std::to_string(std::hash<std::string>{}("ops123")));
        operator_user.addRole(roles_["operator"]);
        users_["operator"] = operator_user;
    }
    
    std::string generateSessionToken(const std::string& username) {
        // 실제로는 JWT나 더 안전한 토큰 생성 방식 사용
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        return username + "_" + std::to_string(now);
    }
    
    User* getUserFromSession(const std::string& session_token) {
        if (active_sessions_.find(session_token) == active_sessions_.end()) {
            return nullptr;
        }
        
        // 세션 토큰에서 사용자명 추출 (간소화)
        size_t pos = session_token.find('_');
        if (pos == std::string::npos) return nullptr;
        
        std::string username = session_token.substr(0, pos);
        auto user_it = users_.find(username);
        
        return (user_it != users_.end()) ? &user_it->second : nullptr;
    }
    
    void logSecurityEvent(const std::string& event) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::cout << "[SECURITY] " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
                  << " " << event << std::endl;
    }
};
```

---

## 4. 감사 로그와 추적성

시스템의 모든 중요한 활동을 추적하고 감사할 수 있도록 별도의 감사 로그를 구현해야 합니다.

### 감사 로그 시스템

```cpp
#include <nlohmann/json.hpp>
#include <fstream>
#include <mutex>
#include <thread>
#include <queue>

using json = nlohmann::json;

enum class AuditEventType {
    USER_LOGIN,
    USER_LOGOUT,
    LOG_ACCESS,
    LOG_EXPORT,
    CONFIGURATION_CHANGE,
    SYSTEM_START,
    SYSTEM_STOP,
    SECURITY_VIOLATION,
    DATA_MODIFICATION,
    PRIVILEGED_ACTION
};

struct AuditEvent {
    AuditEventType type;
    std::string user_id;
    std::string session_id;
    std::string resource;
    std::string action;
    std::string details;
    std::chrono::system_clock::time_point timestamp;
    std::string source_ip;
    std::string user_agent;
    bool success;
    
    AuditEvent(AuditEventType t, const std::string& user, const std::string& session)
        : type(t), user_id(user), session_id(session), 
          timestamp(std::chrono::system_clock::now()), success(true) {}
};

class AuditLogger {
private:
    std::queue<AuditEvent> audit_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::thread processing_thread_;
    std::atomic<bool> running_{true};
    
    std::string audit_log_file_;
    EncryptedLogWriter encrypted_writer_;
    
    // 성능 통계
    std::atomic<int> events_processed_{0};
    std::atomic<int> events_failed_{0};

public:
    AuditLogger(const std::string& audit_file, const std::string& key_file)
        : audit_log_file_(audit_file)
        , encrypted_writer_(key_file, audit_file) {
        
        processing_thread_ = std::thread(&AuditLogger::processAuditEvents, this);
        
        // 시스템 시작 이벤트 기록
        logSystemEvent(AuditEventType::SYSTEM_START, "system", "", "LogCaster audit system started");
    }
    
    ~AuditLogger() {
        logSystemEvent(AuditEventType::SYSTEM_STOP, "system", "", "LogCaster audit system stopping");
        
        running_ = false;
        queue_cv_.notify_all();
        
        if (processing_thread_.joinable()) {
            processing_thread_.join();
        }
    }
    
    // 사용자 활동 감사
    void logUserActivity(AuditEventType type, const std::string& user_id, 
                        const std::string& session_id, const std::string& details,
                        bool success = true, const std::string& source_ip = "") {
        
        AuditEvent event(type, user_id, session_id);
        event.details = details;
        event.success = success;
        event.source_ip = source_ip;
        
        enqueueAuditEvent(event);
    }
    
    // 로그 접근 감사
    void logLogAccess(const std::string& user_id, const std::string& session_id,
                     const std::string& log_category, const std::string& query_details,
                     int records_accessed, bool success = true) {
        
        AuditEvent event(AuditEventType::LOG_ACCESS, user_id, session_id);
        event.resource = log_category;
        event.action = "READ";
        event.success = success;
        
        json details = {
            {"query", query_details},
            {"records_accessed", records_accessed},
            {"category", log_category}
        };
        event.details = details.dump();
        
        enqueueAuditEvent(event);
    }
    
    // 설정 변경 감사
    void logConfigurationChange(const std::string& user_id, const std::string& session_id,
                               const std::string& config_key, const std::string& old_value,
                               const std::string& new_value) {
        
        AuditEvent event(AuditEventType::CONFIGURATION_CHANGE, user_id, session_id);
        event.resource = config_key;
        event.action = "MODIFY";
        
        json details = {
            {"config_key", config_key},
            {"old_value", old_value},
            {"new_value", new_value}
        };
        event.details = details.dump();
        
        enqueueAuditEvent(event);
    }
    
    // 보안 위반 감사
    void logSecurityViolation(const std::string& user_id, const std::string& session_id,
                             const std::string& violation_type, const std::string& details,
                             const std::string& source_ip = "") {
        
        AuditEvent event(AuditEventType::SECURITY_VIOLATION, user_id, session_id);
        event.action = violation_type;
        event.details = details;
        event.success = false;
        event.source_ip = source_ip;
        
        enqueueAuditEvent(event);
    }
    
    // 데이터 수정 감사
    void logDataModification(const std::string& user_id, const std::string& session_id,
                            const std::string& table_or_resource, const std::string& operation,
                            const std::string& record_id, const json& changes) {
        
        AuditEvent event(AuditEventType::DATA_MODIFICATION, user_id, session_id);
        event.resource = table_or_resource;
        event.action = operation;
        
        json details = {
            {"operation", operation},
            {"record_id", record_id},
            {"changes", changes}
        };
        event.details = details.dump();
        
        enqueueAuditEvent(event);
    }
    
    // 감사 로그 통계
    void printAuditStatistics() {
        std::cout << "\n=== Audit Log Statistics ===" << std::endl;
        std::cout << "Events processed: " << events_processed_ << std::endl;
        std::cout << "Events failed: " << events_failed_ << std::endl;
        std::cout << "Queue size: " << getQueueSize() << std::endl;
    }

private:
    void enqueueAuditEvent(const AuditEvent& event) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        audit_queue_.push(event);
        queue_cv_.notify_one();
    }
    
    void processAuditEvents() {
        while (running_) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            queue_cv_.wait(lock, [this] { 
                return !audit_queue_.empty() || !running_; 
            });
            
            while (!audit_queue_.empty()) {
                AuditEvent event = audit_queue_.front();
                audit_queue_.pop();
                lock.unlock();
                
                if (writeAuditEvent(event)) {
                    events_processed_++;
                } else {
                    events_failed_++;
                }
                
                lock.lock();
            }
        }
    }
    
    bool writeAuditEvent(const AuditEvent& event) {
        try {
            json audit_record = createAuditRecord(event);
            std::string audit_line = audit_record.dump();
            
            return encrypted_writer_.writeEncryptedLog(audit_line);
            
        } catch (const std::exception& e) {
            std::cerr << "Failed to write audit event: " << e.what() << std::endl;
            return false;
        }
    }
    
    json createAuditRecord(const AuditEvent& event) {
        auto time_t = std::chrono::system_clock::to_time_t(event.timestamp);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            event.timestamp.time_since_epoch()) % 1000;
        
        std::stringstream timestamp_ss;
        timestamp_ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
        timestamp_ss << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z";
        
        json audit_record = {
            {"timestamp", timestamp_ss.str()},
            {"event_type", eventTypeToString(event.type)},
            {"user_id", event.user_id},
            {"session_id", event.session_id},
            {"resource", event.resource},
            {"action", event.action},
            {"success", event.success},
            {"source_ip", event.source_ip},
            {"user_agent", event.user_agent},
            {"details", event.details.empty() ? json::object() : json::parse(event.details)},
            {"audit_id", generateAuditId()},
            {"system_info", {
                {"hostname", getHostname()},
                {"process_id", getCurrentPid()}
            }}
        };
        
        return audit_record;
    }
    
    std::string eventTypeToString(AuditEventType type) {
        switch (type) {
            case AuditEventType::USER_LOGIN: return "USER_LOGIN";
            case AuditEventType::USER_LOGOUT: return "USER_LOGOUT";
            case AuditEventType::LOG_ACCESS: return "LOG_ACCESS";
            case AuditEventType::LOG_EXPORT: return "LOG_EXPORT";
            case AuditEventType::CONFIGURATION_CHANGE: return "CONFIGURATION_CHANGE";
            case AuditEventType::SYSTEM_START: return "SYSTEM_START";
            case AuditEventType::SYSTEM_STOP: return "SYSTEM_STOP";
            case AuditEventType::SECURITY_VIOLATION: return "SECURITY_VIOLATION";
            case AuditEventType::DATA_MODIFICATION: return "DATA_MODIFICATION";
            case AuditEventType::PRIVILEGED_ACTION: return "PRIVILEGED_ACTION";
            default: return "UNKNOWN";
        }
    }
    
    std::string generateAuditId() {
        // UUID 생성 (간소화)
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        return "audit_" + std::to_string(now);
    }
    
    void logSystemEvent(AuditEventType type, const std::string& user_id, 
                       const std::string& session_id, const std::string& details) {
        AuditEvent event(type, user_id, session_id);
        event.details = details;
        enqueueAuditEvent(event);
    }
    
    size_t getQueueSize() {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return audit_queue_.size();
    }
    
    std::string getHostname() {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            return std::string(hostname);
        }
        return "unknown";
    }
    
    int getCurrentPid() {
        #ifdef _WIN32
            return GetCurrentProcessId();
        #else
            return getpid();
        #endif
    }
};
```

---

## 5. 컨테이너화와 운영

### Docker를 사용한 컨테이너화

```dockerfile
# Dockerfile
FROM ubuntu:22.04

# 필요한 패키지 설치
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libcurl4-openssl-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# 애플리케이션 디렉토리 생성
WORKDIR /app

# 소스 코드 복사
COPY src/ ./src/
COPY include/ ./include/
COPY CMakeLists.txt .

# 빌드
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# 로그 디렉토리 생성
RUN mkdir -p /var/log/logcaster

# 보안을 위한 사용자 생성
RUN useradd -r -s /bin/false logcaster && \
    chown -R logcaster:logcaster /app /var/log/logcaster

# 설정 파일 복사
COPY config/ ./config/

# 헬스체크 스크립트 추가
COPY scripts/healthcheck.sh /healthcheck.sh
RUN chmod +x /healthcheck.sh

# 포트 노출
EXPOSE 8080 8443

# 헬스체크 설정
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD /healthcheck.sh

# 사용자 변경
USER logcaster

# 시작 명령어
CMD ["./build/logcaster", "--config", "./config/production.conf"]
```

```bash
#!/bin/bash
# scripts/healthcheck.sh
# LogCaster 헬스체크 스크립트

# HTTP 헬스체크 엔드포인트 확인
HTTP_RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/health)

if [ "$HTTP_RESPONSE" != "200" ]; then
    echo "HTTP health check failed: $HTTP_RESPONSE"
    exit 1
fi

# 로그 파일 쓰기 권한 확인
if [ ! -w /var/log/logcaster ]; then
    echo "Log directory not writable"
    exit 1
fi

# 프로세스 확인
if ! pgrep -x "logcaster" > /dev/null; then
    echo "LogCaster process not running"
    exit 1
fi

echo "Health check passed"
exit 0
```

### Docker Compose 설정

```yaml
# docker-compose.yml
version: '3.8'

services:
  logcaster:
    build: .
    container_name: logcaster-server
    restart: unless-stopped
    ports:
      - "8080:8080"
      - "8443:8443"
    volumes:
      - logcaster-data:/var/log/logcaster
      - logcaster-config:/app/config
      - ./ssl:/app/ssl:ro
    environment:
      - LOG_LEVEL=INFO
      - MAX_CONNECTIONS=1000
      - ENCRYPTION_ENABLED=true
    networks:
      - logcaster-network
    depends_on:
      - redis
      - elasticsearch
    security_opt:
      - no-new-privileges:true
    cap_drop:
      - ALL
    cap_add:
      - NET_BIND_SERVICE
    user: "1000:1000"

  redis:
    image: redis:7-alpine
    container_name: logcaster-redis
    restart: unless-stopped
    volumes:
      - redis-data:/data
    networks:
      - logcaster-network
    command: redis-server --appendonly yes --requirepass ${REDIS_PASSWORD}

  elasticsearch:
    image: elasticsearch:8.11.0
    container_name: logcaster-elasticsearch
    restart: unless-stopped
    environment:
      - discovery.type=single-node
      - xpack.security.enabled=true
      - ELASTIC_PASSWORD=${ELASTIC_PASSWORD}
    volumes:
      - elasticsearch-data:/usr/share/elasticsearch/data
    networks:
      - logcaster-network
    ulimits:
      memlock:
        soft: -1
        hard: -1

  prometheus:
    image: prom/prometheus:latest
    container_name: logcaster-prometheus
    restart: unless-stopped
    ports:
      - "9090:9090"
    volumes:
      - ./monitoring/prometheus.yml:/etc/prometheus/prometheus.yml:ro
      - prometheus-data:/prometheus
    networks:
      - logcaster-network
    command:
      - '--config.file=/etc/prometheus/prometheus.yml'
      - '--storage.tsdb.path=/prometheus'
      - '--web.console.libraries=/etc/prometheus/console_libraries'
      - '--web.console.templates=/etc/prometheus/consoles'

  grafana:
    image: grafana/grafana:latest
    container_name: logcaster-grafana
    restart: unless-stopped
    ports:
      - "3000:3000"
    volumes:
      - grafana-data:/var/lib/grafana
      - ./monitoring/grafana/dashboards:/var/lib/grafana/dashboards
      - ./monitoring/grafana/provisioning:/etc/grafana/provisioning
    environment:
      - GF_SECURITY_ADMIN_PASSWORD=${GRAFANA_PASSWORD}
    networks:
      - logcaster-network

volumes:
  logcaster-data:
  logcaster-config:
  redis-data:
  elasticsearch-data:
  prometheus-data:
  grafana-data:

networks:
  logcaster-network:
    driver: bridge
```

### 모니터링 설정

```yaml
# monitoring/prometheus.yml
global:
  scrape_interval: 15s
  evaluation_interval: 15s

rule_files:
  - "rules/*.yml"

scrape_configs:
  - job_name: 'logcaster'
    static_configs:
      - targets: ['logcaster:8080']
    metrics_path: '/metrics'
    scrape_interval: 5s

  - job_name: 'redis'
    static_configs:
      - targets: ['redis:6379']

  - job_name: 'elasticsearch'
    static_configs:
      - targets: ['elasticsearch:9200']

alerting:
  alertmanagers:
    - static_configs:
        - targets:
          - alertmanager:9093
```

---

## 🔥 6. 흔한 실수와 해결방법

### 6.1 암호화 관련 실수

#### [SEQUENCE: 45] 키 관리 소홀
```cpp
// ❌ 잘못된 예: 하드코딩된 키
const char* ENCRYPTION_KEY = "mysecretkey123";

// ✅ 올바른 예: 환경변수 또는 키 관리 시스템
const char* key = getenv("LOGCRAFTER_ENCRYPTION_KEY");
if (!key) {
    // HSM 또는 키 관리 서비스에서 가져오기
    key = loadKeyFromHSM();
}
```

### 6.2 접근 제어 실수

#### [SEQUENCE: 46] 권한 상승 위험
```cpp
// ❌ 잘못된 예: 세션 검증만으로 권한 부여
bool checkPermission(const std::string& session) {
    return isValidSession(session);  // 권한 확인 없음
}

// ✅ 올바른 예: 세션 + 권한 검증
bool checkPermission(const std::string& session, Permission perm) {
    if (!isValidSession(session)) return false;
    User* user = getUserFromSession(session);
    return user && user->hasPermission(perm);
}
```

### 6.3 감사 로그 실수

#### [SEQUENCE: 47] 감사 로그 누락
```cpp
// ❌ 잘못된 예: 예외 시 감사 로그 누락
void performSensitiveOperation() {
    try {
        // 작업 수행
        doOperation();
        logAudit("Operation completed");
    } catch (...) {
        // 감사 로그 없이 예외 처리
        throw;
    }
}

// ✅ 올바른 예: 항상 감사 로그 기록
void performSensitiveOperation() {
    bool success = false;
    try {
        doOperation();
        success = true;
    } catch (const std::exception& e) {
        logAudit("Operation failed", false, e.what());
        throw;
    }
    if (success) {
        logAudit("Operation completed", true);
    }
}
```

---

## 7. 실습 프로젝트

### 프로젝트 1: 기본 보안 로거 (기초)
**목표**: 암호화와 기본 인증을 갖춘 로거

**구현 사항**:
- AES-256 로그 암호화
- 기본 사용자 인증
- 민감 데이터 마스킹
- 간단한 감사 로그

### 프로젝트 2: RBAC 로그 시스템 (중급)
**목표**: 역할 기반 접근 제어를 갖춘 보안 로거

**구현 사항**:
- 다중 역할 지원
- 세밀한 권한 관리
- 시간 기반 접근 제어
- 상세 감사 추적

### 프로젝트 3: 엔터프라이즈 보안 로그 플랫폼 (고급)
**목표**: 대규모 조직용 완전한 보안 로그 시스템

**구프 사항**:
- PKI 기반 인증
- 엔드투엔드 암호화
- 컴플라이언스 리포팅
- 실시간 위협 탐지

---

## 8. 학습 체크리스트

### 기초 레벨 ✅
- [ ] 대칭키 암호화 이해
- [ ] 기본 인증/인가 구현
- [ ] 로그 마스킹 기법
- [ ] Docker 기초

### 중급 레벨 📚
- [ ] TLS/SSL 통신 구현
- [ ] RBAC 설계 및 구현
- [ ] 감사 로그 시스템
- [ ] 컨테이너 보안

### 고급 레벨 🚀
- [ ] 키 관리 시스템 (HSM)
- [ ] Zero Trust 아키텍처
- [ ] 컴플라이언스 자동화
- [ ] 침입 탐지 시스템

### 전문가 레벨 🏆
- [ ] 포렌식 분석 도구
- [ ] 보안 운영 자동화
- [ ] 대규모 보안 아키텍처
- [ ] 쾌리런톰처 종합

---

## 9. 추가 학습 자료

### 추천 도서
- "Applied Cryptography" - Bruce Schneier
- "Security Engineering" - Ross Anderson
- "Zero Trust Networks" - Evan Gilman & Doug Barth

### 온라인 리소스
- [OWASP Security Practices](https://owasp.org/)
- [NIST Cybersecurity Framework](https://www.nist.gov/cyberframework)
- [CIS Controls](https://www.cisecurity.org/controls/)

### 실습 도구
- HashiCorp Vault (키 관리)
- ELK Stack + Security 플러그인
- Falco (컨테이너 보안)
- Open Policy Agent (OPA)

---

## 6. 다음 단계 준비

이 보안 및 운영 가이드를 완료했다면 LogCaster를 엔터프라이즈급 환경에서 안전하게 운영할 수 있습니다:

1. **데이터 보안**: 암호화와 민감정보 마스킹으로 데이터 보호
2. **접근 제어**: RBAC를 통한 세밀한 권한 관리
3. **감사 추적**: 모든 활동에 대한 완전한 감사 로그
4. **운영 효율성**: 컨테이너화와 모니터링으로 안정적 운영

### 확인 문제

1. 로그 데이터를 암호화할 때 키 관리는 어떻게 해야 할까요?
2. RBAC에서 최소 권한 원칙을 어떻게 적용해야 할까요?
3. 감사 로그 자체의 무결성은 어떻게 보장할까요?

---

*💡 팁: 보안은 한 번 설정하고 끝나는 것이 아닙니다. 지속적인 모니터링과 업데이트가 필요합니다!*