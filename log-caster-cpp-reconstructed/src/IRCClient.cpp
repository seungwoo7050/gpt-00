// [SEQUENCE: CPP-MVP6-15]
#include "IRCClient.h"
#include <sys/socket.h>
#include "IRCCommandParser.h"
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>

IRCClient::IRCClient(int fd, const std::string& address)
    : fd_(fd)
    , address_(address)
    , state_(State::CONNECTED)
    , lastActivity_(std::chrono::steady_clock::now()) {
    
    size_t colonPos = address.find(':');
    if (colonPos != std::string::npos) {
        hostname_ = address.substr(0, colonPos);
    } else {
        hostname_ = address;
    }
}

IRCClient::~IRCClient() {
    if (fd_ != -1) {
        close(fd_);
    }
}

void IRCClient::setNickname(const std::string& nick) {
    std::lock_guard<std::mutex> lock(mutex_);
    nickname_ = nick;
    updateLastActivity();
}

void IRCClient::setUsername(const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    username_ = user;
    updateLastActivity();
}

void IRCClient::setRealname(const std::string& realname) {
    std::lock_guard<std::mutex> lock(mutex_);
    realname_ = realname;
    updateLastActivity();
}

void IRCClient::setHostname(const std::string& hostname) {
    std::lock_guard<std::mutex> lock(mutex_);
    hostname_ = hostname;
    updateLastActivity();
}

void IRCClient::joinChannel(const std::string& channel) {
    std::lock_guard<std::mutex> lock(mutex_);
    channels_.insert(channel);
    updateLastActivity();
}

void IRCClient::partChannel(const std::string& channel) {
    std::lock_guard<std::mutex> lock(mutex_);
    channels_.erase(channel);
    updateLastActivity();
}

bool IRCClient::isInChannel(const std::string& channel) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return channels_.find(channel) != channels_.end();
}

void IRCClient::sendMessage(const std::string& message) {
    if (fd_ == -1) {
        return;
    }
    
    std::string fullMessage = message;
    if (fullMessage.size() < 2 || 
        fullMessage.substr(fullMessage.size() - 2) != "\r\n") {
        fullMessage += "\r\n";
    }
    
    sendRawMessage(fullMessage);
}

void IRCClient::sendNumericReply(int code, const std::string& params) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string serverName = "logcaster-irc";
    std::string nick = nickname_.empty() ? "*" : nickname_;
    
    std::string reply = IRCCommandParser::formatReply(serverName, nick, code, params);
    sendMessage(reply);
}

void IRCClient::sendErrorReply(int code, const std::string& params) {
    sendNumericReply(code, params);
}

void IRCClient::setState(State state) {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = state;
    updateLastActivity();
}

IRCClient::State IRCClient::getState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

bool IRCClient::isAuthenticated() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == State::AUTHENTICATED;
}

void IRCClient::updateLastActivity() {
    lastActivity_ = std::chrono::steady_clock::now();
}

std::string IRCClient::getFullIdentifier() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (nickname_.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    oss << nickname_ << "!" << username_ << "@" << hostname_;
    return oss.str();
}

void IRCClient::sendRawMessage(const std::string& message) {
    if (fd_ == -1) {
        return;
    }
    
    ssize_t totalSent = 0;
    ssize_t messageLen = message.length();
    
    while (totalSent < messageLen) {
        ssize_t sent = send(fd_, message.c_str() + totalSent, 
                           messageLen - totalSent, MSG_NOSIGNAL);
        
        if (sent <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            std::cerr << "Error sending to client " << nickname_ 
                     << ": " << strerror(errno) << std::endl;
            break;
        }
        
        totalSent += sent;
    }
}
