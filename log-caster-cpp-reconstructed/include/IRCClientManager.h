// [SEQUENCE: CPP-MVP6-4]
#ifndef IRCCLIENTMANAGER_H
#define IRCCLIENTMANAGER_H

#include <string>
#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <chrono>
#include <set>

class IRCClient;

class IRCClientManager {
public:
    IRCClientManager();
    ~IRCClientManager();
    
    std::shared_ptr<IRCClient> addClient(int fd, const std::string& address);
    void removeClient(int fd);
    bool clientExists(int fd) const;
    
    std::shared_ptr<IRCClient> getClientByFd(int fd) const;
    std::shared_ptr<IRCClient> getClientByNickname(const std::string& nickname) const;
    std::vector<std::shared_ptr<IRCClient>> getAllClients() const;
    
    bool isNicknameAvailable(const std::string& nickname) const;
    void registerNickname(int fd, const std::string& nickname);
    
    void updateClientActivity(int fd);
    
    size_t getClientCount() const;
    
private:
    std::unordered_map<int, std::shared_ptr<IRCClient>> clientsByFd_;
    std::unordered_map<std::string, std::shared_ptr<IRCClient>> clientsByNick_;
    
    mutable std::shared_mutex mutex_;
    
    std::string normalizeNickname(const std::string& nick) const;
};

#endif // IRCCLIENTMANAGER_H
