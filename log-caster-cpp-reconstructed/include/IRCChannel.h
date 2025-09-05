// [SEQUENCE: CPP-MVP6-12]
#ifndef IRCCHANNEL_H
#define IRCCHANNEL_H

#include <string>
#include <map>
#include <memory>
#include <functional>
#include <shared_mutex>
#include <chrono>
#include <set>
#include <vector>

class IRCClient;
struct LogEntry;

class IRCChannel {
public:
    enum class Type {
        NORMAL,
        LOG_STREAM,
    };
    
    struct ChannelModes {
        bool topicProtected = true;
    };
    
    explicit IRCChannel(const std::string& name, Type type = Type::NORMAL);
    ~IRCChannel();
    
    void addClient(std::shared_ptr<IRCClient> client);
    void removeClient(const std::string& nickname);
    bool hasClient(const std::string& nickname) const;
    
    void broadcast(const std::string& message);
    void broadcastExcept(const std::string& message, const std::string& exceptNick);
    
    void setTopic(const std::string& topic, const std::string& setBy);
    const std::string& getTopic() const { return topic_; }
    
    const ChannelModes& getModes() const { return modes_; }
    
    bool isOperator(const std::string& nickname) const;
    
    void setLogFilter(const std::function<bool(const LogEntry&)>& filter);
    void enableLogStreaming(bool enable);
    bool isLogStreamingEnabled() const { return streamingEnabled_; }
    void processLogEntry(const LogEntry& entry);
    
    const std::string& getName() const { return name_; }
    Type getType() const { return type_; }
    size_t getClientCount() const;
    std::vector<std::shared_ptr<IRCClient>> getClients() const;
    
    static std::function<bool(const LogEntry&)> createLevelFilter(const std::string& level);
    
private:
    std::string name_;
    Type type_;
    std::string topic_;
    std::string topicSetBy_;
    std::chrono::system_clock::time_point topicSetTime_;
    
    ChannelModes modes_;
    
    std::map<std::string, std::shared_ptr<IRCClient>> clients_;
    std::set<std::string> operators_;
    
    bool streamingEnabled_;
    std::function<bool(const LogEntry&)> logFilter_;
    
    mutable std::shared_mutex mutex_;
    
    std::string formatLogEntry(const LogEntry& entry) const;
};

#endif // IRCCHANNEL_H
