// [SEQUENCE: CPP-MVP6-13]
#include "IRCChannel.h"
#include "IRCClient.h"
#include "IRCCommandParser.h"
#include "LogBuffer.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

IRCChannel::IRCChannel(const std::string& name, Type type)
    : name_(name)
    , type_(type)
    , topic_("")
    , topicSetBy_("")
    , streamingEnabled_(false) {
}

IRCChannel::~IRCChannel() = default;

void IRCChannel::addClient(std::shared_ptr<IRCClient> client) {
    if (!client) return;
    
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    std::string nickname = client->getNickname();
    clients_[nickname] = client;
    
    if (clients_.size() == 1) {
        operators_.insert(nickname);
    }
}

void IRCChannel::removeClient(const std::string& nickname) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    clients_.erase(nickname);
    operators_.erase(nickname);
}

bool IRCChannel::hasClient(const std::string& nickname) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return clients_.find(nickname) != clients_.end();
}

void IRCChannel::broadcast(const std::string& message) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& [nick, client] : clients_) {
        client->sendMessage(message);
    }
}

void IRCChannel::broadcastExcept(const std::string& message, const std::string& exceptNick) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& [nick, client] : clients_) {
        if (nick != exceptNick) {
            client->sendMessage(message);
        }
    }
}

void IRCChannel::setTopic(const std::string& topic, const std::string& setBy) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    topic_ = topic;
    topicSetBy_ = setBy;
    topicSetTime_ = std::chrono::system_clock::now();
}

bool IRCChannel::isOperator(const std::string& nickname) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return operators_.find(nickname) != operators_.end();
}

void IRCChannel::setLogFilter(const std::function<bool(const LogEntry&)>& filter) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    logFilter_ = filter;
}

void IRCChannel::enableLogStreaming(bool enable) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    streamingEnabled_ = enable;
}

void IRCChannel::processLogEntry(const LogEntry& entry) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    if (!streamingEnabled_ || clients_.empty()) {
        return;
    }
    
    if (logFilter_ && !logFilter_(entry)) {
        return;
    }
    
    std::string formattedLog = formatLogEntry(entry);
    std::string message = IRCCommandParser::formatUserMessage(
        "LogBot", "log", "system", "PRIVMSG", name_, formattedLog
    );
    
    lock.unlock();
    broadcast(message);
}

size_t IRCChannel::getClientCount() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return clients_.size();
}

std::vector<std::shared_ptr<IRCClient>> IRCChannel::getClients() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::vector<std::shared_ptr<IRCClient>> clients;
    clients.reserve(clients_.size());
    for (const auto& [nick, client] : clients_) {
        clients.push_back(client);
    }
    return clients;
}

std::function<bool(const LogEntry&)> IRCChannel::createLevelFilter(const std::string& level) {
    return [level](const LogEntry& entry) {
        return entry.level == level;
    };
}

std::string IRCChannel::formatLogEntry(const LogEntry& entry) const {
    std::ostringstream oss;
    
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    struct tm* tm = localtime(&time_t);
    oss << "[" << std::put_time(tm, "%Y-%m-%d %H:%M:%S") << "] ";
    
    if (!entry.level.empty()) {
        oss << entry.level << ": ";
    }
    
    if (!entry.source.empty()) {
        oss << "[" << entry.source << "] ";
    }
    
    oss << entry.message;
    
    return oss.str();
}
