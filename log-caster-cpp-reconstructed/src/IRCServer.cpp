// [SEQUENCE: CPP-MVP6-7]
#include "IRCServer.h"
#include "IRCChannelManager.h"
#include "IRCClientManager.h"
#include "IRCCommandParser.h"
#include "IRCCommandHandler.h"
#include "IRCClient.h"
#include "ThreadPool.h"
#include "LogBuffer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>

IRCServer::IRCServer(int port, std::shared_ptr<LogBuffer> logBuffer)
    : port_(port)
    , listenFd_(-1)
    , running_(false)
    , logBuffer_(logBuffer) {
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    serverCreated_ = std::ctime(&time_t);
    serverCreated_.pop_back(); // Remove newline
    
    threadPool_ = std::make_unique<ThreadPool>(THREAD_POOL_SIZE);
    channelManager_ = std::make_shared<IRCChannelManager>();
    clientManager_ = std::make_shared<IRCClientManager>();
    commandParser_ = std::make_unique<IRCCommandParser>();
    commandHandler_ = std::make_unique<IRCCommandHandler>(this);
}

IRCServer::~IRCServer() {
    stop();
}

void IRCServer::start() {
    if (running_.load()) {
        return;
    }
    
    try {
        setupSocket();
        
        channelManager_->initializeLogChannels();
        
        if (logBuffer_) {
            logBuffer_->registerCallback("#logs-all", 
                [this](const LogEntry& entry) {
                    channelManager_->distributeLogEntry(entry);
                });
            logBuffer_->registerCallback("#logs-error", 
                [this](const LogEntry& entry) {
                    if (entry.level == "ERROR") {
                        channelManager_->distributeLogEntry(entry);
                    }
                });
        }
        
        running_ = true;
        
        acceptThread_ = std::thread(&IRCServer::acceptClients, this);
        
        std::cout << "IRC server started on port " << port_ << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to start IRC server: " << e.what() << std::endl;
        throw;
    }
}

void IRCServer::stop() {
    if (!running_.load()) {
        return;
    }
    
    running_ = false;
    
    if (listenFd_ != -1) {
        close(listenFd_);
        listenFd_ = -1;
    }
    
    if (acceptThread_.joinable()) {
        acceptThread_.join();
    }
    
    auto allClients = clientManager_->getAllClients();
    for (const auto& client : allClients) {
        channelManager_->partAllChannels(client, "Server shutting down");
        clientManager_->removeClient(client->getFd());
    }
    
    std::cout << "IRC server stopped" << std::endl;
}

void IRCServer::setLogBuffer(std::shared_ptr<LogBuffer> buffer) {
    std::lock_guard<std::mutex> lock(mutex_);
    logBuffer_ = buffer;
}

void IRCServer::setupSocket() {
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
    }
    
    int opt = 1;
    if (setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(listenFd_);
        throw std::runtime_error("Failed to set socket options: " + std::string(strerror(errno)));
    }
    
    int flags = fcntl(listenFd_, F_GETFL, 0);
    if (flags < 0 || fcntl(listenFd_, F_SETFL, flags | O_NONBLOCK) < 0) {
        close(listenFd_);
        throw std::runtime_error("Failed to set non-blocking mode: " + std::string(strerror(errno)));
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);
    
    if (bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(listenFd_);
        throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
    }
    
    if (listen(listenFd_, 128) < 0) {
        close(listenFd_);
        throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));
    }
}

void IRCServer::acceptClients() {
    while (running_.load()) {
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        
        int clientFd = accept(listenFd_, (struct sockaddr*)&clientAddr, &addrLen);
        
        if (clientFd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            
            if (running_.load()) {
                std::cerr << "Accept error: " << strerror(errno) << std::endl;
            }
            continue;
        }
        
        char addrStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, addrStr, sizeof(addrStr));
        std::string clientAddress = std::string(addrStr) + ":" + 
                                   std::to_string(ntohs(clientAddr.sin_port));
        
        if (clientManager_->getClientCount() >= MAX_CLIENTS) {
            std::string errorMsg = "ERROR :Server is full\r\n";
            send(clientFd, errorMsg.c_str(), errorMsg.length(), MSG_NOSIGNAL);
            close(clientFd);
            continue;
        }
        
        int flags = fcntl(clientFd, F_GETFL, 0);
        if (flags >= 0) {
            fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
        }
        
        auto client = clientManager_->addClient(clientFd, clientAddress);
        if (!client) {
            close(clientFd);
            continue;
        }
        
        std::cout << "New IRC client connected from " << clientAddress << std::endl;
        
        threadPool_->enqueue([this, clientFd, clientAddress]() {
            handleClient(clientFd, clientAddress);
        });
    }
}

void IRCServer::handleClient(int clientFd, const std::string& clientAddr) {
    char buffer[4096];
    std::string incomplete;
    
    while (running_.load()) {
        ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            
            std::string data = incomplete + std::string(buffer, bytesRead);
            incomplete.clear();
            
            size_t pos = 0;
            while ((pos = data.find("\r\n")) != std::string::npos || 
                   (pos = data.find("\n")) != std::string::npos) {
                
                std::string line = data.substr(0, pos);
                data.erase(0, pos + (data[pos] == '\r' ? 2 : 1));
                
                if (!line.empty()) {
                    processClientData(clientFd, line);
                }
            }
            
            if (!data.empty()) {
                incomplete = data;
            }
            
            clientManager_->updateClientActivity(clientFd);
            
        } else if (bytesRead == 0) {
            break;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                break;
            }
            
            if (!clientManager_->clientExists(clientFd)) {
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    auto client = clientManager_->getClientByFd(clientFd);
    if (client) {
        std::cout << "IRC client disconnected: " << client->getNickname() 
                  << " (" << clientAddr << ")" << std::endl;
        
        IRCCommandParser::IRCCommand quitCmd;
        quitCmd.command = "QUIT";
        quitCmd.trailing = "Connection closed";
        commandHandler_->handleCommand(client, quitCmd);
        
        clientManager_->removeClient(clientFd);
    }
    
    close(clientFd);
}

void IRCServer::processClientData(int clientFd, const std::string& data) {
    auto client = clientManager_->getClientByFd(clientFd);
    if (!client) {
        return;
    }
    
    auto cmd = IRCCommandParser::parse(data);
    
    if (!cmd.command.empty()) {
        commandHandler_->handleCommand(client, cmd);
    }
}
