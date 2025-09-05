// [SEQUENCE: CPP-MVP1-10]
#include "LogServer.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <signal.h>

// [SEQUENCE: CPP-MVP1-10]
// 생성자: 리소스 획득 (소켓 생성, 바인딩, 리스닝)
// [SEQUENCE: CPP-MVP2-32]
// 생성자: 모든 멤버 변수 초기화
LogServer::LogServer(int port, int queryPort)
    : port_(port), queryPort_(queryPort), listenFd_(-1), queryFd_(-1), running_(false) {
    logger_ = std::make_unique<ConsoleLogger>();
    threadPool_ = std::make_unique<ThreadPool>();
    logBuffer_ = std::make_shared<LogBuffer>();
    queryHandler_ = std::make_unique<QueryHandler>(logBuffer_);
    FD_ZERO(&masterSet_);
}

// [SEQUENCE: CPP-MVP1-11]
// 소멸자: 리소스 해제 (스레드 정리, 소켓 닫기)
// [SEQUENCE: CPP-MVP2-33]
// 소멸자: 서버 중지
LogServer::~LogServer() {
    stop();
}

// [SEQUENCE: CPP-MVP2-34]
// 서버 시작
void LogServer::start() {
    if (running_) return;
    initialize();
    running_ = true;
    logger_->log("Server started.");
    runEventLoop();
}

// [SEQUENCE: CPP-MVP2-35]
// 서버 중지
void LogServer::stop() {
    if (!running_.exchange(false)) return;

    if (listenFd_ != -1) {
        shutdown(listenFd_, SHUT_RDWR);
        close(listenFd_);
    }
    if (queryFd_ != -1) {
        shutdown(queryFd_, SHUT_RDWR);
        close(queryFd_);
    }
    logger_->log("Server stopped.");
}

// [SEQUENCE: CPP-MVP2-36]
// 리스너 소켓 생성 및 초기화
void LogServer::initialize() {
    auto create_listener = [&](int port) -> int {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) throw std::runtime_error("Socket creation failed");
        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) throw std::runtime_error("Bind failed");
        if (listen(fd, 128) < 0) throw std::runtime_error("Listen failed");
        return fd;
    };
    listenFd_ = create_listener(port_);
    queryFd_ = create_listener(queryPort_);
    FD_SET(listenFd_, &masterSet_);
    FD_SET(queryFd_, &masterSet_);
}

// [SEQUENCE: CPP-MVP1-12]
// [SEQUENCE: CPP-MVP2-37]
// 메인 이벤트 루프
void LogServer::runEventLoop() {
    while (running_) {
        fd_set read_fds = masterSet_;
        int max_fd = std::max(listenFd_, queryFd_);
        int result = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
        if (result < 0 && errno != EINTR) {
            logger_->log("Select error");
            break;
        }

        if (FD_ISSET(listenFd_, &read_fds)) {
            handleNewConnection(listenFd_, false);
        }
        if (FD_ISSET(queryFd_, &read_fds)) {
            handleNewConnection(queryFd_, true);
        }
    }
}

// [SEQUENCE: CPP-MVP2-38]
// 새 연결 처리
void LogServer::handleNewConnection(int listener_fd, bool is_query_port) {
    int client_fd = accept(listener_fd, nullptr, nullptr);
    if (client_fd < 0) return;

    // [SEQUENCE: CPP-MVP5-2]
    // 클라이언트 수 제한 확인
    if (client_count_ >= 1024) { // MAX_CLIENTS
        close(client_fd);
        return;
    }

    if (is_query_port) {
        threadPool_->enqueue(&LogServer::handleQueryTask, this, client_fd);
    } else {
        threadPool_->enqueue(&LogServer::handleClientTask, this, client_fd);
    }
}

// [SEQUENCE: CPP-MVP1-13]
// [SEQUENCE: CPP-MVP2-39]
// [SEQUENCE: CPP-MVP4-17]
// 클라이언트 작업 핸들러 (MVP4 버전)
void LogServer::handleClientTask(int client_fd) {
    client_count_++;
    char buffer[4096];
    while (true) {
        ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (nbytes <= 0) break;
        buffer[nbytes] = '\0';
        
        // [SEQUENCE: CPP-MVP5-3]
        // 로그 메시지 크기 제한
        const int SAFE_LOG_LENGTH = 1024;
        std::string log_message(buffer, (nbytes > SAFE_LOG_LENGTH) ? SAFE_LOG_LENGTH : nbytes);
        if (nbytes > SAFE_LOG_LENGTH) {
            log_message += "...";
        }

        // 1. 인메모리 버퍼에 저장
        logBuffer_->push(log_message, "info", "unknown"); // MVP6: Add level and source

        // [SEQUENCE: CPP-MVP4-18]
        // 2. 영속성 관리자에게 쓰기 요청 (활성화된 경우)
        if (persistence_) {
            persistence_->write(log_message);
        }
    }
    close(client_fd);
    client_count_--;
}

// [SEQUENCE: CPP-MVP4-19]
// PersistenceManager 설정 메소드 구현
void LogServer::setPersistenceManager(std::unique_ptr<PersistenceManager> persistence) {
    persistence_ = std::move(persistence);
}

// [SEQUENCE: CPP-MVP2-40]
// 쿼리 클라이언트 작업
void LogServer::handleQueryTask(int client_fd) {
    char buffer[4096];
    ssize_t nbytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (nbytes > 0) {
        buffer[nbytes] = '\0';
        std::string query(buffer);
        // Remove trailing newline
        query.erase(query.find_last_not_of("\r\n") + 1);
        std::string response = queryHandler_->processQuery(query);
        send(client_fd, response.c_str(), response.length(), 0);
    }
    close(client_fd);
}