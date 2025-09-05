# 20. 채팅 서버 보안 🛡️
## 안전한 채팅 서비스를 위한 보안 완벽 가이드

---

## 📋 문서 정보

- **난이도**: 고급 (Advanced)
- **예상 학습 시간**: 25-30시간
- **필요 선수 지식**: 
  - 네트워크 보안 기초
  - 암호화와 인증
  - IRC 프로토콜
  - 시스템 프로그래밍
- **실습 환경**: OpenSSL, 보안 테스트 도구, 침투 테스트 환경

---

## Overview

**"인터넷에서 안전하게 채팅하려면 어떻게 해야 할까요?"** 🛡️

정말 중요한 질문이에요! 채팅 서버를 운영하는 것은 마치 **큰 파티를 여는 것**과 같아요. 많은 사람들이 와서 대화하지만, 때로는 **불청객**들도 끼어들 수 있어요. 해커들은 플러딩, 스푸핑, 인젝션 공격 등으로 우리의 파티를 망치려고 해요!

하지만 걱정하지 마세요! 마치 **경비원과 보안 시스템**을 갖춘 안전한 건물처럼, 우리도 여러 겹의 보안 장치를 만들 수 있어요. 🏰

IRC 서버를 LogCaster에 통합하면 새로운 공격 벡터가 생깁니다. 채팅 서버는 플러딩, 스푸핑, 인젝션 공격 등 다양한 보안 위협에 노출됩니다. 이 가이드는 LogCaster-IRC를 안전하게 운영하기 위한 보안 기술을 다룹니다.

## 1. 네트워크 레벨 보안

**"무수히 많은 연결 요청이 몰려든다면?"** 🌊

좋은 질문이에요! 마치 **클럽 입구의 도어맨**처럼, 우리도 누가 들어올 수 있는지 철저히 관리해야 해요!

### 1.1 연결 제한과 플러드 방어 - 디지털 도어맨 🚪
```cpp
class ConnectionManager {
private:
    struct ConnectionInfo {
        std::chrono::steady_clock::time_point connectTime;
        std::atomic<int> connectionCount{0};
        std::atomic<int> recentAttempts{0};
        std::chrono::steady_clock::time_point lastAttempt;
        bool banned = false;
        std::chrono::steady_clock::time_point banExpiry;
    };
    
    std::unordered_map<std::string, ConnectionInfo> ipInfo_;
    std::shared_mutex ipMutex_;
    
    // 설정 가능한 제한값
    static constexpr int MAX_CONNECTIONS_PER_IP = 5;
    static constexpr int MAX_CONNECTION_RATE = 10;  // per minute
    static constexpr auto BAN_DURATION = std::chrono::minutes(30);
    
public:
    bool acceptConnection(int sockfd) {
        // 클라이언트 IP 추출
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        getpeername(sockfd, (struct sockaddr*)&addr, &len);
        std::string ip = inet_ntoa(addr.sin_addr);
        
        auto now = std::chrono::steady_clock::now();
        
        {
            std::unique_lock lock(ipMutex_);
            auto& info = ipInfo_[ip];
            
            // 차단 확인
            if (info.banned) {
                if (now < info.banExpiry) {
                    logWarning("Rejected connection from banned IP: {}", ip);
                    return false;
                } else {
                    // 차단 해제
                    info.banned = false;
                    info.recentAttempts = 0;
                }
            }
            
            // 연결 속도 제한
            if (now - info.lastAttempt < std::chrono::seconds(6)) {
                info.recentAttempts++;
                
                if (info.recentAttempts > MAX_CONNECTION_RATE) {
                    // 일시 차단
                    info.banned = true;
                    info.banExpiry = now + BAN_DURATION;
                    logWarning("Banned IP {} for connection flooding", ip);
                    return false;
                }
            } else {
                info.recentAttempts = 1;
            }
            
            info.lastAttempt = now;
            
            // 동시 연결 수 제한
            if (info.connectionCount >= MAX_CONNECTIONS_PER_IP) {
                logWarning("Too many connections from IP: {}", ip);
                return false;
            }
            
            info.connectionCount++;
            info.connectTime = now;
        }
        
        // 연결 종료 시 카운트 감소 콜백 설정
        setDisconnectCallback(sockfd, [this, ip]() {
            std::unique_lock lock(ipMutex_);
            if (auto it = ipInfo_.find(ip); it != ipInfo_.end()) {
                it->second.connectionCount--;
            }
        });
        
        return true;
    }
    
    // IP 기반 필터링
    class IPFilter {
    private:
        std::vector<std::pair<uint32_t, uint32_t>> whitelist_;  // IP 범위
        std::vector<std::pair<uint32_t, uint32_t>> blacklist_;
        std::shared_mutex filterMutex_;
        
    public:
        void addWhitelist(const std::string& cidr) {
            auto range = parseCIDR(cidr);
            std::unique_lock lock(filterMutex_);
            whitelist_.push_back(range);
        }
        
        void addBlacklist(const std::string& cidr) {
            auto range = parseCIDR(cidr);
            std::unique_lock lock(filterMutex_);
            blacklist_.push_back(range);
        }
        
        bool isAllowed(const std::string& ip) {
            uint32_t ipNum = ipToUint32(ip);
            std::shared_lock lock(filterMutex_);
            
            // 화이트리스트 우선
            if (!whitelist_.empty()) {
                for (const auto& [start, end] : whitelist_) {
                    if (ipNum >= start && ipNum <= end) {
                        return true;
                    }
                }
                return false;  // 화이트리스트에 없으면 거부
            }
            
            // 블랙리스트 확인
            for (const auto& [start, end] : blacklist_) {
                if (ipNum >= start && ipNum <= end) {
                    return false;
                }
            }
            
            return true;
        }
        
    private:
        std::pair<uint32_t, uint32_t> parseCIDR(const std::string& cidr) {
            size_t slashPos = cidr.find('/');
            std::string ip = cidr.substr(0, slashPos);
            int prefixLen = std::stoi(cidr.substr(slashPos + 1));
            
            uint32_t ipNum = ipToUint32(ip);
            uint32_t mask = ~((1 << (32 - prefixLen)) - 1);
            
            uint32_t start = ipNum & mask;
            uint32_t end = start | ~mask;
            
            return {start, end};
        }
        
        uint32_t ipToUint32(const std::string& ip) {
            struct in_addr addr;
            inet_pton(AF_INET, ip.c_str(), &addr);
            return ntohl(addr.s_addr);
        }
    };
};
```

### 1.2 SSL/TLS 통합 - 비밀스러운 대화를 위한 암호화 🔒

**"다른 사람이 우리 대화를 엿듣고 있다면?"**

무서운 생각이에요! 하지만 SSL/TLS는 마치 **암호 편지**를 주고받는 것 같아요. 중간에 누가 가로채도 암호문만 보이죠!
```cpp
class TLSServer {
private:
    SSL_CTX* ctx_;
    std::string certFile_;
    std::string keyFile_;
    
public:
    TLSServer(const std::string& cert, const std::string& key) 
        : certFile_(cert), keyFile_(key) {
        
        // OpenSSL 초기화
        SSL_library_init();
        SSL_load_error_strings();
        
        // TLS 1.2 이상만 허용
        ctx_ = SSL_CTX_new(TLS_server_method());
        SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION);
        
        // 강력한 암호 스위트만 사용
        SSL_CTX_set_cipher_list(ctx_, 
            "ECDHE-ECDSA-AES256-GCM-SHA384:"
            "ECDHE-RSA-AES256-GCM-SHA384:"
            "ECDHE-ECDSA-AES128-GCM-SHA256:"
            "ECDHE-RSA-AES128-GCM-SHA256");
        
        // 인증서 로드
        if (SSL_CTX_use_certificate_file(ctx_, certFile_.c_str(), 
                                        SSL_FILETYPE_PEM) <= 0) {
            throw std::runtime_error("Failed to load certificate");
        }
        
        if (SSL_CTX_use_PrivateKey_file(ctx_, keyFile_.c_str(), 
                                        SSL_FILETYPE_PEM) <= 0) {
            throw std::runtime_error("Failed to load private key");
        }
        
        // DH 파라미터 설정 (Perfect Forward Secrecy)
        setupDHParams();
    }
    
    std::unique_ptr<TLSConnection> acceptTLS(int clientFd) {
        SSL* ssl = SSL_new(ctx_);
        SSL_set_fd(ssl, clientFd);
        
        if (SSL_accept(ssl) <= 0) {
            SSL_free(ssl);
            throw std::runtime_error("SSL handshake failed");
        }
        
        // 클라이언트 인증서 검증 (선택적)
        X509* clientCert = SSL_get_peer_certificate(ssl);
        if (clientCert) {
            if (SSL_get_verify_result(ssl) != X509_V_OK) {
                logWarning("Client certificate verification failed");
            }
            X509_free(clientCert);
        }
        
        return std::make_unique<TLSConnection>(ssl);
    }
    
private:
    void setupDHParams() {
        DH* dh = DH_new();
        BIGNUM* p = BN_new();
        BIGNUM* g = BN_new();
        
        // 2048비트 DH 파라미터
        BN_hex2bn(&p, "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1"
                      "29024E088A67CC74020BBEA63B139B22514A08798E3404DD"
                      "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245"
                      "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED"
                      "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D"
                      "C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F"
                      "83655D23DCA3AD961C62F356208552BB9ED529077096966D"
                      "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B"
                      "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9"
                      "DE2BCBF6955817183995497CEA956AE515D2261898FA0510"
                      "15728E5A8AACAA68FFFFFFFFFFFFFFFF");
        BN_set_word(g, 2);
        
        DH_set0_pqg(dh, p, NULL, g);
        SSL_CTX_set_tmp_dh(ctx_, dh);
        DH_free(dh);
    }
};

// TLS 연결 래퍼
class TLSConnection {
private:
    SSL* ssl_;
    std::mutex writeMutex_;
    
public:
    explicit TLSConnection(SSL* ssl) : ssl_(ssl) {}
    
    ~TLSConnection() {
        if (ssl_) {
            SSL_shutdown(ssl_);
            SSL_free(ssl_);
        }
    }
    
    ssize_t read(char* buffer, size_t size) {
        return SSL_read(ssl_, buffer, size);
    }
    
    ssize_t write(const char* data, size_t size) {
        std::lock_guard lock(writeMutex_);
        return SSL_write(ssl_, data, size);
    }
    
    std::string getCipherName() {
        return SSL_get_cipher_name(ssl_);
    }
};
```

## 2. 애플리케이션 레벨 보안

### 2.1 입력 검증과 살균
```cpp
class InputValidator {
private:
    // IRC 프로토콜 제약
    static constexpr size_t MAX_NICK_LENGTH = 30;
    static constexpr size_t MAX_CHANNEL_LENGTH = 50;
    static constexpr size_t MAX_MESSAGE_LENGTH = 512;
    
    // 금지된 문자 패턴
    std::regex ctrlCharPattern_{R"([\x00-\x1F\x7F])"};  // 제어 문자
    std::regex nickPattern_{R"(^[a-zA-Z\[\]\\`_^{|}][a-zA-Z0-9\[\]\\`_^{|}-]*$)"};
    std::regex channelPattern_{R"(^[#&][^\x00\x07\x0A\x0D ,]+$)"};
    
public:
    struct ValidationResult {
        bool valid;
        std::string error;
        std::string sanitized;
    };
    
    ValidationResult validateNickname(const std::string& nick) {
        if (nick.empty()) {
            return {false, "Nickname cannot be empty", ""};
        }
        
        if (nick.length() > MAX_NICK_LENGTH) {
            return {false, "Nickname too long", ""};
        }
        
        if (!std::regex_match(nick, nickPattern_)) {
            return {false, "Invalid characters in nickname", ""};
        }
        
        // 예약된 닉네임 확인
        if (isReservedNick(nick)) {
            return {false, "Reserved nickname", ""};
        }
        
        return {true, "", nick};
    }
    
    ValidationResult validateChannel(const std::string& channel) {
        if (channel.length() < 2 || channel.length() > MAX_CHANNEL_LENGTH) {
            return {false, "Invalid channel name length", ""};
        }
        
        if (!std::regex_match(channel, channelPattern_)) {
            return {false, "Invalid channel name format", ""};
        }
        
        return {true, "", channel};
    }
    
    ValidationResult validateMessage(const std::string& message) {
        if (message.length() > MAX_MESSAGE_LENGTH) {
            // 메시지 자르기
            std::string truncated = message.substr(0, MAX_MESSAGE_LENGTH);
            return {true, "Message truncated", truncated};
        }
        
        // 제어 문자 제거
        std::string sanitized = std::regex_replace(message, ctrlCharPattern_, "");
        
        // CTCP 공격 방지
        if (sanitized.find('\x01') != std::string::npos) {
            sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '\x01'), 
                           sanitized.end());
        }
        
        return {true, "", sanitized};
    }
    
    // SQL 인젝션 방지 (로그 쿼리용)
    std::string sanitizeQueryParam(const std::string& param) {
        std::string sanitized = param;
        
        // 위험한 문자 이스케이프
        std::vector<std::pair<std::string, std::string>> replacements = {
            {"'", "''"},
            {"\"", "\\\""},
            {";", "\\;"},
            {"--", ""},
            {"/*", ""},
            {"*/", ""},
            {"xp_", ""},
            {"sp_", ""}
        };
        
        for (const auto& [from, to] : replacements) {
            size_t pos = 0;
            while ((pos = sanitized.find(from, pos)) != std::string::npos) {
                sanitized.replace(pos, from.length(), to);
                pos += to.length();
            }
        }
        
        return sanitized;
    }
    
private:
    bool isReservedNick(const std::string& nick) {
        static const std::set<std::string> reserved = {
            "server", "admin", "root", "operator", "chanserv", 
            "nickserv", "hostserv", "botserv", "memoserv",
            "logbot", "logcaster"
        };
        
        std::string lower = nick;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        
        return reserved.count(lower) > 0;
    }
};
```

### 2.2 명령어 인젝션 방지
```cpp
class CommandSanitizer {
private:
    struct CommandContext {
        std::string command;
        std::vector<std::string> allowedParams;
        std::function<bool(const std::string&)> validator;
    };
    
    std::unordered_map<std::string, CommandContext> commandWhitelist_;
    
public:
    CommandSanitizer() {
        // 허용된 명령어와 파라미터 정의
        commandWhitelist_["LOGQUERY"] = {
            "LOGQUERY",
            {"COUNT", "SEARCH", "FILTER", "RECENT"},
            [](const std::string& param) {
                // 추가 검증 로직
                return param.length() < 1000;
            }
        };
        
        commandWhitelist_["LOGFILTER"] = {
            "LOGFILTER",
            {"ADD", "REMOVE", "LIST"},
            [](const std::string& param) {
                // 정규식 검증
                try {
                    std::regex test(param);
                    return true;
                } catch (...) {
                    return false;
                }
            }
        };
    }
    
    bool isCommandAllowed(const std::string& command, 
                         const std::vector<std::string>& params) {
        auto it = commandWhitelist_.find(command);
        if (it == commandWhitelist_.end()) {
            return false;  // 화이트리스트에 없는 명령어
        }
        
        const auto& context = it->second;
        
        // 파라미터 검증
        if (!params.empty()) {
            const auto& firstParam = params[0];
            
            // 허용된 파라미터인지 확인
            if (std::find(context.allowedParams.begin(), 
                         context.allowedParams.end(), 
                         firstParam) == context.allowedParams.end()) {
                return false;
            }
            
            // 추가 검증
            if (context.validator && params.size() > 1) {
                for (size_t i = 1; i < params.size(); ++i) {
                    if (!context.validator(params[i])) {
                        return false;
                    }
                }
            }
        }
        
        return true;
    }
    
    // 셸 명령어 실행 방지
    std::string sanitizeForShell(const std::string& input) {
        static const std::regex dangerousChars(R"([;&|`$(){}[\]<>!#~*?'"\\\n\r])");
        
        // 위험한 문자를 제거하거나 이스케이프
        return std::regex_replace(input, dangerousChars, "");
    }
};
```

## 3. 인증과 권한 관리

### 3.1 사용자 인증 시스템
```cpp
class AuthenticationManager {
private:
    struct UserAccount {
        std::string username;
        std::string passwordHash;  // bcrypt/scrypt 해시
        std::vector<std::string> privileges;
        std::chrono::system_clock::time_point lastLogin;
        int failedAttempts = 0;
        bool locked = false;
    };
    
    std::unordered_map<std::string, UserAccount> accounts_;
    std::shared_mutex accountMutex_;
    
    // 세션 관리
    struct Session {
        std::string username;
        std::string token;
        std::chrono::steady_clock::time_point expiry;
        std::set<std::string> permissions;
    };
    
    std::unordered_map<std::string, Session> sessions_;
    std::shared_mutex sessionMutex_;
    
public:
    // 비밀번호 기반 인증
    std::optional<std::string> authenticate(const std::string& username, 
                                           const std::string& password) {
        std::unique_lock lock(accountMutex_);
        
        auto it = accounts_.find(username);
        if (it == accounts_.end()) {
            // 타이밍 공격 방지를 위해 가짜 해시 검증
            bcrypt_checkpw(password.c_str(), "$2b$10$fake.hash.to.prevent.timing");
            return std::nullopt;
        }
        
        auto& account = it->second;
        
        // 계정 잠금 확인
        if (account.locked) {
            logWarning("Login attempt for locked account: {}", username);
            return std::nullopt;
        }
        
        // 비밀번호 검증
        if (!verifyPassword(password, account.passwordHash)) {
            account.failedAttempts++;
            
            if (account.failedAttempts >= 5) {
                account.locked = true;
                logWarning("Account locked due to failed attempts: {}", username);
            }
            
            return std::nullopt;
        }
        
        // 성공적인 로그인
        account.failedAttempts = 0;
        account.lastLogin = std::chrono::system_clock::now();
        
        // 세션 토큰 생성
        std::string token = generateSecureToken();
        
        {
            std::unique_lock sessionLock(sessionMutex_);
            sessions_[token] = {
                username,
                token,
                std::chrono::steady_clock::now() + std::chrono::hours(24),
                std::set<std::string>(account.privileges.begin(), 
                                     account.privileges.end())
            };
        }
        
        return token;
    }
    
    // 토큰 기반 인증
    bool validateToken(const std::string& token) {
        std::shared_lock lock(sessionMutex_);
        
        auto it = sessions_.find(token);
        if (it == sessions_.end()) {
            return false;
        }
        
        // 만료 확인
        if (std::chrono::steady_clock::now() > it->second.expiry) {
            return false;
        }
        
        return true;
    }
    
    // 권한 확인
    bool hasPermission(const std::string& token, const std::string& permission) {
        std::shared_lock lock(sessionMutex_);
        
        auto it = sessions_.find(token);
        if (it == sessions_.end()) {
            return false;
        }
        
        return it->second.permissions.count(permission) > 0 ||
               it->second.permissions.count("admin") > 0;
    }
    
private:
    bool verifyPassword(const std::string& password, const std::string& hash) {
        return bcrypt_checkpw(password.c_str(), hash.c_str()) == 0;
    }
    
    std::string generateSecureToken() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;
        
        std::stringstream ss;
        for (int i = 0; i < 4; ++i) {
            ss << std::hex << dis(gen);
        }
        
        return ss.str();
    }
};
```

### 3.2 OAuth2/SASL 통합
```cpp
class SASLAuthenticator {
private:
    enum class Mechanism {
        PLAIN,
        SCRAM_SHA_256,
        EXTERNAL  // 클라이언트 인증서
    };
    
public:
    // SASL 협상 시작
    void startAuthentication(IRCClient& client, const std::string& mechanism) {
        if (mechanism == "PLAIN") {
            client.send("AUTHENTICATE +");
            client.setState(ClientState::AUTHENTICATING_PLAIN);
        } else if (mechanism == "SCRAM-SHA-256") {
            startSCRAM(client);
        } else if (mechanism == "EXTERNAL") {
            if (client.hasTLSCert()) {
                authenticateExternal(client);
            } else {
                client.send("904 * :SASL authentication failed");
            }
        } else {
            client.send("908 * " + mechanism + " :Unknown SASL mechanism");
        }
    }
    
    // PLAIN 메커니즘 처리
    bool handlePlainAuth(IRCClient& client, const std::string& data) {
        // base64 디코드
        std::string decoded = base64_decode(data);
        
        // PLAIN 형식: authzid\0authcid\0passwd
        std::vector<std::string> parts;
        size_t pos = 0;
        
        while (pos < decoded.length()) {
            size_t nullPos = decoded.find('\0', pos);
            if (nullPos == std::string::npos) {
                parts.push_back(decoded.substr(pos));
                break;
            }
            parts.push_back(decoded.substr(pos, nullPos - pos));
            pos = nullPos + 1;
        }
        
        if (parts.size() != 3) {
            return false;
        }
        
        const std::string& authzid = parts[0];  // 권한 ID (보통 비움)
        const std::string& authcid = parts[1];  // 인증 ID
        const std::string& password = parts[2];
        
        return authenticateUser(authcid, password);
    }
    
    // SCRAM-SHA-256 구현
    void startSCRAM(IRCClient& client) {
        // 클라이언트 첫 메시지 생성
        std::string clientNonce = generateNonce();
        std::string clientFirstMessage = "n,,n=" + client.getUsername() + 
                                        ",r=" + clientNonce;
        
        client.setSASLState("client-nonce", clientNonce);
        client.send("AUTHENTICATE " + base64_encode(clientFirstMessage));
    }
    
private:
    std::string generateNonce() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        std::string nonce;
        for (int i = 0; i < 32; ++i) {
            nonce += static_cast<char>(dis(gen));
        }
        
        return base64_encode(nonce);
    }
};
```

## 4. 채널 보안

### 4.1 채널 권한 관리
```cpp
class ChannelSecurity {
private:
    struct ChannelPermissions {
        std::set<std::string> operators;      // @ 권한
        std::set<std::string> voiced;         // + 권한
        std::set<std::string> banned;         // 차단된 사용자
        std::map<std::string, std::chrono::steady_clock::time_point> invites;
        
        // 채널 모드
        bool moderated = false;      // +m: voiced만 발언 가능
        bool inviteOnly = false;     // +i: 초대만 가능
        bool topicLocked = false;    // +t: op만 토픽 변경
        bool noExternal = false;     // +n: 채널 멤버만 메시지
        
        std::string key;             // +k: 채널 비밀번호
        int userLimit = 0;           // +l: 사용자 수 제한
    };
    
    std::unordered_map<std::string, ChannelPermissions> channels_;
    std::shared_mutex channelMutex_;
    
public:
    // 채널 참여 권한 확인
    bool canJoinChannel(const std::string& channel, const IRCClient& client,
                       const std::string& key = "") {
        std::shared_lock lock(channelMutex_);
        
        auto it = channels_.find(channel);
        if (it == channels_.end()) {
            return true;  // 새 채널
        }
        
        const auto& perms = it->second;
        
        // 차단 확인
        if (perms.banned.count(client.getNickname()) > 0) {
            client.sendNumeric(474, channel + " :Cannot join channel (+b)");
            return false;
        }
        
        // 초대 전용
        if (perms.inviteOnly) {
            auto inviteIt = perms.invites.find(client.getNickname());
            if (inviteIt == perms.invites.end() || 
                std::chrono::steady_clock::now() > inviteIt->second) {
                client.sendNumeric(473, channel + " :Cannot join channel (+i)");
                return false;
            }
        }
        
        // 비밀번호 확인
        if (!perms.key.empty() && perms.key != key) {
            client.sendNumeric(475, channel + " :Cannot join channel (+k)");
            return false;
        }
        
        // 사용자 수 제한
        if (perms.userLimit > 0) {
            size_t currentUsers = getChannelUserCount(channel);
            if (currentUsers >= perms.userLimit) {
                client.sendNumeric(471, channel + " :Cannot join channel (+l)");
                return false;
            }
        }
        
        return true;
    }
    
    // 메시지 전송 권한
    bool canSendToChannel(const std::string& channel, const IRCClient& client) {
        std::shared_lock lock(channelMutex_);
        
        auto it = channels_.find(channel);
        if (it == channels_.end()) {
            return false;
        }
        
        const auto& perms = it->second;
        const std::string& nick = client.getNickname();
        
        // 운영자는 항상 가능
        if (perms.operators.count(nick) > 0) {
            return true;
        }
        
        // 음소거 모드
        if (perms.moderated && perms.voiced.count(nick) == 0) {
            client.sendNumeric(404, channel + " :Cannot send to channel (+m)");
            return false;
        }
        
        // 외부 메시지 금지
        if (perms.noExternal && !client.isInChannel(channel)) {
            client.sendNumeric(404, channel + " :Cannot send to channel (+n)");
            return false;
        }
        
        return true;
    }
    
    // 권한 변경
    bool setChannelMode(const std::string& channel, const IRCClient& client,
                       char mode, bool add, const std::string& param = "") {
        if (!isOperator(channel, client.getNickname())) {
            client.sendNumeric(482, channel + " :You're not channel operator");
            return false;
        }
        
        std::unique_lock lock(channelMutex_);
        auto& perms = channels_[channel];
        
        switch (mode) {
        case 'o':  // 운영자 권한
            if (add) {
                perms.operators.insert(param);
            } else {
                perms.operators.erase(param);
            }
            break;
            
        case 'v':  // 발언권
            if (add) {
                perms.voiced.insert(param);
            } else {
                perms.voiced.erase(param);
            }
            break;
            
        case 'b':  // 차단
            if (add) {
                perms.banned.insert(param);
                // 차단된 사용자 강제 퇴장
                kickBannedUser(channel, param);
            } else {
                perms.banned.erase(param);
            }
            break;
            
        case 'm':  // 음소거 모드
            perms.moderated = add;
            break;
            
        case 'i':  // 초대 전용
            perms.inviteOnly = add;
            break;
            
        case 'k':  // 채널 키
            if (add) {
                perms.key = param;
            } else {
                perms.key.clear();
            }
            break;
            
        case 'l':  // 사용자 제한
            if (add) {
                perms.userLimit = std::stoi(param);
            } else {
                perms.userLimit = 0;
            }
            break;
            
        default:
            return false;
        }
        
        return true;
    }
};
```

## 5. 로그 보안

### 5.1 로그 접근 제어
```cpp
class LogAccessControl {
private:
    struct LogPermission {
        std::set<std::string> allowedUsers;
        std::set<std::string> allowedChannels;
        std::function<bool(const LogEntry&)> filter;
        std::chrono::system_clock::time_point expiry;
    };
    
    std::unordered_map<std::string, LogPermission> permissions_;
    std::shared_mutex permMutex_;
    
public:
    // 로그 쿼리 권한 확인
    bool canQueryLogs(const IRCClient& client, const QueryRequest& request) {
        std::shared_lock lock(permMutex_);
        
        // 관리자는 모든 권한
        if (client.hasRole("admin")) {
            return true;
        }
        
        // 일반 사용자 권한 확인
        auto it = permissions_.find(client.getNickname());
        if (it == permissions_.end()) {
            return false;
        }
        
        const auto& perm = it->second;
        
        // 만료 확인
        if (std::chrono::system_clock::now() > perm.expiry) {
            return false;
        }
        
        // 채널 권한 확인
        if (request.hasChannelFilter()) {
            const std::string& channel = request.getChannel();
            if (perm.allowedChannels.count(channel) == 0 &&
                perm.allowedChannels.count("*") == 0) {
                return false;
            }
        }
        
        return true;
    }
    
    // 로그 필터링
    std::vector<LogEntry> filterLogs(const std::vector<LogEntry>& logs,
                                    const IRCClient& client) {
        std::shared_lock lock(permMutex_);
        
        auto it = permissions_.find(client.getNickname());
        if (it == permissions_.end()) {
            return {};  // 권한 없음
        }
        
        const auto& filter = it->second.filter;
        if (!filter) {
            return logs;  // 필터 없음
        }
        
        std::vector<LogEntry> filtered;
        std::copy_if(logs.begin(), logs.end(), std::back_inserter(filtered),
                    filter);
        
        return filtered;
    }
    
    // 민감한 정보 마스킹
    LogEntry sanitizeLog(const LogEntry& log, const IRCClient& client) {
        LogEntry sanitized = log;
        
        // IP 주소 마스킹
        static const std::regex ipPattern(
            R"(\b(?:\d{1,3}\.){3}\d{1,3}\b)");
        sanitized.message = std::regex_replace(
            sanitized.message, ipPattern, "xxx.xxx.xxx.xxx");
        
        // 이메일 마스킹
        static const std::regex emailPattern(
            R"(\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Z|a-z]{2,}\b)");
        sanitized.message = std::regex_replace(
            sanitized.message, emailPattern, "****@****.***");
        
        // 신용카드 번호 마스킹
        static const std::regex ccPattern(
            R"(\b(?:\d[ -]*?){13,16}\b)");
        sanitized.message = std::regex_replace(
            sanitized.message, ccPattern, "****-****-****-****");
        
        // 사용자 정의 패턴
        if (client.hasSensitivePatterns()) {
            for (const auto& pattern : client.getSensitivePatterns()) {
                sanitized.message = std::regex_replace(
                    sanitized.message, pattern, "[REDACTED]");
            }
        }
        
        return sanitized;
    }
};
```

## 6. 모니터링과 감사

### 6.1 보안 이벤트 로깅
```cpp
class SecurityAuditor {
private:
    struct SecurityEvent {
        std::chrono::system_clock::time_point timestamp;
        std::string eventType;
        std::string clientInfo;
        std::string details;
        SeverityLevel severity;
    };
    
    std::deque<SecurityEvent> events_;
    std::mutex eventMutex_;
    std::ofstream auditLog_;
    
    // 이상 탐지
    struct AnomalyDetector {
        std::unordered_map<std::string, int> failedLogins;
        std::unordered_map<std::string, int> suspiciousQueries;
        std::unordered_map<std::string, std::chrono::steady_clock::time_point> lastSeen;
    } anomalyDetector_;
    
public:
    void logSecurityEvent(const std::string& type, const IRCClient& client,
                         const std::string& details, SeverityLevel severity) {
        SecurityEvent event{
            std::chrono::system_clock::now(),
            type,
            formatClientInfo(client),
            details,
            severity
        };
        
        {
            std::lock_guard lock(eventMutex_);
            events_.push_back(event);
            
            // 오래된 이벤트 제거
            while (events_.size() > 10000) {
                events_.pop_front();
            }
        }
        
        // 파일에 기록
        writeToAuditLog(event);
        
        // 심각한 이벤트는 즉시 알림
        if (severity >= SeverityLevel::HIGH) {
            alertAdmins(event);
        }
        
        // 이상 탐지
        detectAnomalies(type, client);
    }
    
private:
    void detectAnomalies(const std::string& type, const IRCClient& client) {
        const std::string& clientId = client.getAddress();
        
        if (type == "FAILED_LOGIN") {
            anomalyDetector_.failedLogins[clientId]++;
            
            if (anomalyDetector_.failedLogins[clientId] > 10) {
                logSecurityEvent("BRUTE_FORCE_DETECTED", client,
                               "Multiple failed login attempts",
                               SeverityLevel::CRITICAL);
            }
        } else if (type == "SUSPICIOUS_QUERY") {
            anomalyDetector_.suspiciousQueries[clientId]++;
            
            if (anomalyDetector_.suspiciousQueries[clientId] > 50) {
                logSecurityEvent("QUERY_FLOOD_DETECTED", client,
                               "Excessive suspicious queries",
                               SeverityLevel::HIGH);
            }
        }
        
        // 시간 기반 이상 탐지
        auto now = std::chrono::steady_clock::now();
        auto lastSeen = anomalyDetector_.lastSeen[clientId];
        
        if (lastSeen != std::chrono::steady_clock::time_point{}) {
            auto gap = std::chrono::duration_cast<std::chrono::hours>(
                now - lastSeen).count();
            
            if (gap > 24 * 30) {  // 30일 이상 비활성 후 접속
                logSecurityEvent("DORMANT_ACCOUNT_ACCESS", client,
                               "Access after long inactivity",
                               SeverityLevel::MEDIUM);
            }
        }
        
        anomalyDetector_.lastSeen[clientId] = now;
    }
    
    std::string formatClientInfo(const IRCClient& client) {
        return fmt::format("{}@{} [{}]", 
                          client.getNickname(),
                          client.getHostname(),
                          client.getAddress());
    }
};
```

## 🔥 흔한 실수와 해결방법

### 1. 취약한 인증

#### [SEQUENCE: 126] 평문 비밀번호 저장
```cpp
// ❌ 잘못된 예: 평문 비밀번호
struct User {
    std::string username;
    std::string password;  // 평문!
};

// ✅ 올바른 예: 해시된 비밀번호
struct User {
    std::string username;
    std::string passwordHash;  // bcrypt/scrypt/argon2
    
    bool verifyPassword(const std::string& password) {
        return bcrypt_checkpw(password.c_str(), passwordHash.c_str()) == 0;
    }
};
```

### 2. 불충분한 입력 검증

#### [SEQUENCE: 127] 포맷 스트링 취약점
```cpp
// ❌ 잘못된 예: 사용자 입력을 직접 포맷
void logMessage(const std::string& userInput) {
    fprintf(logFile, userInput.c_str());  // %s, %n 공격 가능
}

// ✅ 올바른 예: 안전한 포맷
void logMessage(const std::string& userInput) {
    fprintf(logFile, "%s", userInput.c_str());
    // 또는 C++ 스트림 사용
    logStream << userInput;
}
```

### 3. 타이밍 공격

#### [SEQUENCE: 128] 조기 반환으로 인한 정보 누출
```cpp
// ❌ 잘못된 예: 타이밍 차이로 사용자 존재 여부 노출
bool authenticate(const std::string& user, const std::string& pass) {
    auto it = users.find(user);
    if (it == users.end()) {
        return false;  // 빠른 반환
    }
    return verifyPassword(pass, it->second.hash);  // 느린 해시 검증
}

// ✅ 올바른 예: 일정한 시간 소요
bool authenticate(const std::string& user, const std::string& pass) {
    auto it = users.find(user);
    std::string hash = (it != users.end()) ? it->second.hash : dummyHash;
    bool valid = verifyPassword(pass, hash);
    return (it != users.end()) && valid;
}
```

## 마무리

채팅 서버 보안은 다층 방어 전략이 필요합니다:

1. **네트워크 레벨**: 연결 제한, TLS 암호화
2. **애플리케이션 레벨**: 입력 검증, 명령어 필터링
3. **인증/권한**: 강력한 인증, 세밀한 권한 관리
4. **채널 보안**: 접근 제어, 메시지 필터링
5. **로그 보안**: 민감 정보 보호, 접근 제어
6. **모니터링**: 실시간 감사, 이상 탐지

이러한 보안 조치들은 LogCaster-IRC를 안전하고 신뢰할 수 있는 시스템으로 만들어줍니다.

## 실습 프로젝트

### 프로젝트 1: 기본 보안 IRC 서버 (기초)
**목표**: 기본적인 보안 기능을 갖춘 IRC 서버

**구현 사항**:
- 연결 수 제한
- 기본 인증 시스템
- 입력 검증
- 로깅 시스템

### 프로젝트 2: TLS 지원 채팅 서버 (중급)
**목표**: 암호화된 통신을 지원하는 서버

**구현 사항**:
- SSL/TLS 통합
- 인증서 관리
- SASL 인증
- 채널 권한 시스템

### 프로젝트 3: 엔터프라이즈 보안 시스템 (고급)
**목표**: 기업용 보안 요구사항을 충족하는 시스템

**구현 사항**:
- 다중 인증 (MFA)
- 감사 로깅
- 이상 탐지
- 자동 대응 시스템

## 학습 체크리스트

### 기초 레벨 ✅
- [ ] 기본 네트워크 보안 이해
- [ ] 입력 검증 구현
- [ ] 비밀번호 해싱
- [ ] 기본 로깅 시스템

### 중급 레벨 📚
- [ ] TLS/SSL 구현
- [ ] 세션 관리
- [ ] 권한 시스템 설계
- [ ] Rate limiting 구현

### 고급 레벨 🚀
- [ ] 고급 인증 메커니즘
- [ ] 실시간 위협 탐지
- [ ] 자동화된 대응
- [ ] 포렌식 로깅

### 전문가 레벨 🏆
- [ ] 제로 트러스트 아키텍처
- [ ] 침투 테스트 수행
- [ ] 보안 감사 프로세스
- [ ] 인시던트 대응 계획

## 추가 학습 자료

### 추천 도서
- "Network Security Essentials" - William Stallings
- "Applied Cryptography" - Bruce Schneier
- "The Web Application Hacker's Handbook" - Stuttard & Pinto
- "Practical Cryptography" - Ferguson & Schneier

### 온라인 리소스
- [OWASP Top 10](https://owasp.org/www-project-top-ten/)
- [NIST Cybersecurity Framework](https://www.nist.gov/cyberframework)
- [SANS Security Resources](https://www.sans.org/security-resources/)
- [CVE Database](https://cve.mitre.org/)

### 실습 도구
- Wireshark - 네트워크 분석
- Metasploit - 침투 테스트
- OWASP ZAP - 웹 보안 테스트
- Burp Suite - 보안 테스팅

---

*💡 팁: 보안은 끝이 없는 과정입니다. 항상 최신 위협과 대응 방법을 학습하고, 정기적으로 시스템을 감사하세요.*