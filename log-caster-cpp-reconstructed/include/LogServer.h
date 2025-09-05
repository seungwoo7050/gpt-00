// [SEQUENCE: CPP-MVP1-9]
#ifndef LOGSERVER_H
#define LOGSERVER_H

#include <memory>
#include <atomic>
#include <string>
// [SEQUENCE: CPP-MVP2-30]
#include "Logger.h"
#include "ThreadPool.h"
#include "LogBuffer.h"
#include "QueryHandler.h"
#include "Persistence.h"

// [SEQUENCE: CPP-MVP1-9]
class LogServer {
public:
    explicit LogServer(int port = 9999, int queryPort = 9998);
    ~LogServer();

    LogServer(const LogServer&) = delete;
    LogServer& operator=(const LogServer&) = delete;

    // [SEQUENCE: CPP-MVP2-30]
    void start();
    void stop();

    // [SEQUENCE: CPP-MVP4-14]
    void setPersistenceManager(std::unique_ptr<PersistenceManager> persistence);

    // [SEQUENCE: CPP-MVP6-9]
    std::shared_ptr<LogBuffer> getLogBuffer() const { return logBuffer_; }

private:
    // [SEQUENCE: CPP-MVP2-30]
    void initialize();
    void runEventLoop();
    void handleNewConnection(int listener_fd, bool is_query_port);
    void handleClientTask(int client_fd);
    void handleQueryTask(int client_fd);

    int port_;
    // [SEQUENCE: CPP-MVP2-30]
    int queryPort_;
    int listenFd_;
    int queryFd_;
    fd_set masterSet_;
    std::atomic<bool> running_;
    
    std::unique_ptr<Logger> logger_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::shared_ptr<LogBuffer> logBuffer_;
    std::unique_ptr<QueryHandler> queryHandler_;

    // [SEQUENCE: CPP-MVP4-15]
    std::unique_ptr<PersistenceManager> persistence_;

    // [SEQUENCE: CPP-MVP5-1]
    std::atomic<int> client_count_{0};
};

#endif // LOGSERVER_H