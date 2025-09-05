// [SEQUENCE: CPP-MVP6-2]
#ifndef IRCSERVER_H
#define IRCSERVER_H

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>

class LogBuffer;
class ThreadPool;
class IRCChannelManager;
class IRCClientManager;
class IRCCommandParser;
class IRCCommandHandler;

class IRCServer {
public:
    static constexpr int DEFAULT_IRC_PORT = 6667;
    static constexpr int MAX_CLIENTS = 1000;
    static constexpr int THREAD_POOL_SIZE = 8;
    
    explicit IRCServer(int port = DEFAULT_IRC_PORT, std::shared_ptr<LogBuffer> logBuffer = nullptr);
    ~IRCServer();
    
    void start();
    void stop();
    bool isRunning() const { return running_.load(); }
    
    void setLogBuffer(std::shared_ptr<LogBuffer> buffer);
    std::shared_ptr<LogBuffer> getLogBuffer() const { return logBuffer_; }
    
    std::shared_ptr<IRCChannelManager> getChannelManager() const { return channelManager_; }
    std::shared_ptr<IRCClientManager> getClientManager() const { return clientManager_; }
    
    std::string getServerName() const { return "logcaster-irc"; }
    std::string getServerVersion() const { return "1.0"; }
    std::string getServerCreated() const { return serverCreated_; }
    
private:
    void setupSocket();
    void acceptClients();
    void handleClient(int clientFd, const std::string& clientAddr);
    void processClientData(int clientFd, const std::string& data);
    
    int port_;
    int listenFd_;
    std::atomic<bool> running_;
    std::thread acceptThread_;
    std::string serverCreated_;
    
    std::shared_ptr<LogBuffer> logBuffer_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::shared_ptr<IRCChannelManager> channelManager_;
    std::shared_ptr<IRCClientManager> clientManager_;
    std::unique_ptr<IRCCommandParser> commandParser_;
    std::unique_ptr<IRCCommandHandler> commandHandler_;
    
    mutable std::mutex mutex_;
};

#endif // IRCSERVER_H
