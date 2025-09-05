// [SEQUENCE: CPP-MVP6-3]
#ifndef IRCCHANNELMANAGER_H
#define IRCCHANNELMANAGER_H

#include <string>
#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <functional>

#include "IRCChannel.h"
class IRCClient;
struct LogEntry;

class IRCChannelManager {
public:
    IRCChannelManager();
    ~IRCChannelManager();
    
    std::shared_ptr<IRCChannel> createChannel(const std::string& name, IRCChannel::Type type = IRCChannel::Type::NORMAL);
    void removeChannel(const std::string& name);
    bool channelExists(const std::string& name) const;
    std::shared_ptr<IRCChannel> getChannel(const std::string& name) const;
    
    bool joinChannel(std::shared_ptr<IRCClient> client, const std::string& channelName, const std::string& key = "");
    bool partChannel(std::shared_ptr<IRCClient> client, const std::string& channelName, const std::string& reason = "");
    void partAllChannels(std::shared_ptr<IRCClient> client, const std::string& reason = "");
    
    std::vector<std::string> getChannelList() const;
    
    void initializeLogChannels();
    void distributeLogEntry(const LogEntry& entry);
    
private:
    std::unordered_map<std::string, std::shared_ptr<IRCChannel>> channels_;
    
    mutable std::shared_mutex mutex_;
    
    bool isValidChannelName(const std::string& name) const;
    std::string normalizeChannelName(const std::string& name) const;
    void sendJoinMessages(std::shared_ptr<IRCChannel> channel, std::shared_ptr<IRCClient> client);
    void sendPartMessages(std::shared_ptr<IRCChannel> channel, std::shared_ptr<IRCClient> client, const std::string& reason);
    
    struct LogChannelConfig {
        std::string name;
        std::string level;
        std::string description;
    };
    static const std::vector<LogChannelConfig> defaultLogChannels_;
};

#endif // IRCCHANNELMANAGER_H
