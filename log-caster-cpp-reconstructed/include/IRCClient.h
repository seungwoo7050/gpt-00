// [SEQUENCE: CPP-MVP6-14]
#ifndef IRCCLIENT_H
#define IRCCLIENT_H

#include <string>
#include <set>
#include <mutex>
#include <chrono>
#include <memory>

class IRCClient {
public:
    enum class State {
        CONNECTED,
        AUTHENTICATED,
        DISCONNECTED
    };
    
    IRCClient(int fd, const std::string& address);
    ~IRCClient();
    
    void setNickname(const std::string& nick);
    void setUsername(const std::string& user);
    void setRealname(const std::string& realname);
    void setHostname(const std::string& hostname);
    
    void joinChannel(const std::string& channel);
    void partChannel(const std::string& channel);
    bool isInChannel(const std::string& channel) const;
    
    void sendMessage(const std::string& message);
    void sendNumericReply(int code, const std::string& params);
    void sendErrorReply(int code, const std::string& params);
    
    void setState(State state);
    State getState() const;
    bool isAuthenticated() const;
    void updateLastActivity();
    
    int getFd() const { return fd_; }
    const std::string& getAddress() const { return address_; }
    const std::string& getNickname() const { return nickname_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getRealname() const { return realname_; }
    const std::string& getHostname() const { return hostname_; }
    const std::set<std::string>& getChannels() const { return channels_; }
    
    std::string getFullIdentifier() const;
    
private:
    int fd_;
    std::string address_;
    std::string nickname_;
    std::string username_;
    std::string realname_;
    std::string hostname_;
    State state_;
    
    std::set<std::string> channels_;
    
    std::chrono::steady_clock::time_point lastActivity_;
    
    mutable std::mutex mutex_;
    
    void sendRawMessage(const std::string& message);
};

#endif // IRCCLIENT_H
