// [SEQUENCE: CPP-MVP6-8]
#include "IRCChannelManager.h"
#include "IRCChannel.h"
#include "IRCClient.h"
#include "LogBuffer.h"
#include <algorithm>
#include <iostream>

const std::vector<IRCChannelManager::LogChannelConfig> IRCChannelManager::defaultLogChannels_ = {
    {"#logs-all", "*", "All log messages"},
    {"#logs-error", "ERROR", "Error level logs only"},
};

IRCChannelManager::IRCChannelManager() {}

IRCChannelManager::~IRCChannelManager() = default;

std::shared_ptr<IRCChannel> IRCChannelManager::createChannel(const std::string& name, 
                                                             IRCChannel::Type type) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    std::string normalizedName = normalizeChannelName(name);
    
    if (channels_.find(normalizedName) != channels_.end()) {
        return channels_[normalizedName];
    }
    
    auto channel = std::make_shared<IRCChannel>(normalizedName, type);
    channels_[normalizedName] = channel;
    
    return channel;
}

void IRCChannelManager::removeChannel(const std::string& name) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    std::string normalizedName = normalizeChannelName(name);
    
    if (normalizedName.find("#logs-") == 0) {
        return;
    }
    
    channels_.erase(normalizedName);
}

bool IRCChannelManager::channelExists(const std::string& name) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::string normalizedName = normalizeChannelName(name);
    return channels_.find(normalizedName) != channels_.end();
}

std::shared_ptr<IRCChannel> IRCChannelManager::getChannel(const std::string& name) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::string normalizedName = normalizeChannelName(name);
    auto it = channels_.find(normalizedName);
    
    if (it != channels_.end()) {
        return it->second;
    }
    
    return nullptr;
}

bool IRCChannelManager::joinChannel(std::shared_ptr<IRCClient> client, 
                                   const std::string& channelName, 
                                   const std::string& key) {
    if (!client || !client->isAuthenticated()) {
        return false;
    }
    
    std::shared_ptr<IRCChannel> channel;
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        std::string normalizedName = normalizeChannelName(channelName);
        auto it = channels_.find(normalizedName);
        
        if (it == channels_.end()) {
            if (normalizedName.find("#logs-") != 0) {
                channel = std::make_shared<IRCChannel>(normalizedName, IRCChannel::Type::NORMAL);
                channels_[normalizedName] = channel;
            } else {
                return false; 
            }
        } else {
            channel = it->second;
        }
    }
    
    channel->addClient(client);
    client->joinChannel(channelName);
    
    sendJoinMessages(channel, client);
    
    return true;
}

bool IRCChannelManager::partChannel(std::shared_ptr<IRCClient> client, 
                                   const std::string& channelName, 
                                   const std::string& reason) {
    if (!client) {
        return false;
    }
    
    auto channel = getChannel(channelName);
    if (!channel || !channel->hasClient(client->getNickname())) {
        return false;
    }
    
    sendPartMessages(channel, client, reason);
    
    channel->removeClient(client->getNickname());
    client->partChannel(channelName);
    
    if (channel->getClientCount() == 0 && channelName.find("#logs-") != 0) {
        removeChannel(channelName);
    }
    
    return true;
}

void IRCChannelManager::partAllChannels(std::shared_ptr<IRCClient> client, 
                                       const std::string& reason) {
    if (!client) {
        return;
    }
    
    auto clientChannels = client->getChannels();
    for (const auto& channelName : clientChannels) {
        partChannel(client, channelName, reason);
    }
}

std::vector<std::string> IRCChannelManager::getChannelList() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<std::string> channelNames;
    channelNames.reserve(channels_.size());
    
    for (const auto& [name, channel] : channels_) {
        channelNames.push_back(name);
    }
    
    return channelNames;
}

void IRCChannelManager::initializeLogChannels() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& config : defaultLogChannels_) {
        auto channel = std::make_shared<IRCChannel>(config.name, IRCChannel::Type::LOG_STREAM);
        channel->setTopic(config.description, "LogCaster");
        channel->enableLogStreaming(true);
        
        if (config.level != "*") {
            channel->setLogFilter(IRCChannel::createLevelFilter(config.level));
        }
        
        channels_[config.name] = channel;
    }
}

void IRCChannelManager::distributeLogEntry(const LogEntry& entry) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& [name, channel] : channels_) {
        if (channel->getType() == IRCChannel::Type::LOG_STREAM && 
            channel->isLogStreamingEnabled()) {
            channel->processLogEntry(entry);
        }
    }
}

bool IRCChannelManager::isValidChannelName(const std::string& name) const {
    if (name.empty() || name.length() > 50) {
        return false;
    }
    
    if (name[0] != '#' && name[0] != '&') {
        return false;
    }
    
    for (char c : name) {
        if (c == ' ' || c == ',' || c == '\x07' || c < 32) {
            return false;
        }
    }
    
    return true;
}

std::string IRCChannelManager::normalizeChannelName(const std::string& name) const {
    return name;
}

void IRCChannelManager::sendJoinMessages(std::shared_ptr<IRCChannel> channel, 
                                         std::shared_ptr<IRCClient> client) {
    std::string joinMsg = ":" + client->getFullIdentifier() + " JOIN :" + channel->getName();
    channel->broadcast(joinMsg);
}

void IRCChannelManager::sendPartMessages(std::shared_ptr<IRCChannel> channel, 
                                        std::shared_ptr<IRCClient> client, 
                                        const std::string& reason) {
    std::string partMsg = ":" + client->getFullIdentifier() + " PART " + channel->getName();
    if (!reason.empty()) {
        partMsg += " :" + reason;
    }
    channel->broadcast(partMsg);
}
