// [SEQUENCE: CPP-MVP6-9]
#include "IRCClientManager.h"
#include "IRCClient.h"
#include <algorithm>
#include <iostream>

IRCClientManager::IRCClientManager() {}

IRCClientManager::~IRCClientManager() = default;

std::shared_ptr<IRCClient> IRCClientManager::addClient(int fd, const std::string& address) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto client = std::make_shared<IRCClient>(fd, address);
    clientsByFd_[fd] = client;
    
    return client;
}

void IRCClientManager::removeClient(int fd) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto it = clientsByFd_.find(fd);
    if (it == clientsByFd_.end()) {
        return;
    }
    
    auto client = it->second;
    std::string nick = client->getNickname();
    if (!nick.empty()) {
        clientsByNick_.erase(normalizeNickname(nick));
    }

    clientsByFd_.erase(it);
}

bool IRCClientManager::clientExists(int fd) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return clientsByFd_.find(fd) != clientsByFd_.end();
}

std::shared_ptr<IRCClient> IRCClientManager::getClientByFd(int fd) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = clientsByFd_.find(fd);
    if (it != clientsByFd_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<IRCClient> IRCClientManager::getClientByNickname(const std::string& nickname) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = clientsByNick_.find(normalizeNickname(nickname));
    if (it != clientsByNick_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<IRCClient>> IRCClientManager::getAllClients() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<std::shared_ptr<IRCClient>> clients;
    clients.reserve(clientsByFd_.size());
    
    for (const auto& [fd, client] : clientsByFd_) {
        clients.push_back(client);
    }
    
    return clients;
}

bool IRCClientManager::isNicknameAvailable(const std::string& nickname) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return clientsByNick_.find(normalizeNickname(nickname)) == clientsByNick_.end();
}

void IRCClientManager::registerNickname(int fd, const std::string& nickname) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto it = clientsByFd_.find(fd);
    if (it == clientsByFd_.end()) {
        return;
    }
    
    std::string normalized = normalizeNickname(nickname);
    clientsByNick_[normalized] = it->second;
}

void IRCClientManager::updateClientActivity(int fd) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = clientsByFd_.find(fd);
    if (it != clientsByFd_.end()) {
        it->second->updateLastActivity();
    }
}

size_t IRCClientManager::getClientCount() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return clientsByFd_.size();
}

std::string IRCClientManager::normalizeNickname(const std::string& nick) const {
    std::string normalized = nick;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    return normalized;
}
