// [SEQUENCE: CPP-MVP6-11]
#include "IRCCommandHandler.h"
#include "IRCServer.h"
#include "IRCClient.h"
#include "IRCChannel.h"
#include "IRCChannelManager.h"
#include "IRCClientManager.h"
#include "LogBuffer.h"
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>

IRCCommandHandler::IRCCommandHandler(IRCServer* server) : server_(server) {
    handlers_["NICK"] = [this](auto client, auto cmd) { handleNICK(client, cmd); };
    handlers_["USER"] = [this](auto client, auto cmd) { handleUSER(client, cmd); };
    handlers_["JOIN"] = [this](auto client, auto cmd) { handleJOIN(client, cmd); };
    handlers_["PART"] = [this](auto client, auto cmd) { handlePART(client, cmd); };
    handlers_["PRIVMSG"] = [this](auto client, auto cmd) { handlePRIVMSG(client, cmd); };
    handlers_["QUIT"] = [this](auto client, auto cmd) { handleQUIT(client, cmd); };
    handlers_["PING"] = [this](auto client, auto cmd) { handlePING(client, cmd); };
    handlers_["LIST"] = [this](auto client, auto cmd) { handleLIST(client, cmd); };
    handlers_["NAMES"] = [this](auto client, auto cmd) { handleNAMES(client, cmd); };
}

IRCCommandHandler::~IRCCommandHandler() = default;

void IRCCommandHandler::handleCommand(std::shared_ptr<IRCClient> client, 
                                     const IRCCommandParser::IRCCommand& cmd) {
    if (!client || cmd.command.empty()) {
        return;
    }
    
    if (!client->isAuthenticated() && 
        cmd.command != "NICK" && cmd.command != "USER" && 
        cmd.command != "QUIT") {
        client->sendErrorReply(IRCCommandParser::ERR_NOTREGISTERED, 
                              ":You have not registered");
        return;
    }
    
    auto it = handlers_.find(cmd.command);
    if (it != handlers_.end()) {
        it->second(client, cmd);
    } else {
        client->sendErrorReply(IRCCommandParser::ERR_UNKNOWNCOMMAND,
                              cmd.command + " :Unknown command");
    }
}

void IRCCommandHandler::handleNICK(std::shared_ptr<IRCClient> client, 
                                   const IRCCommandParser::IRCCommand& cmd) {
    if (cmd.params.empty()) {
        client->sendErrorReply(IRCCommandParser::ERR_NONICKNAMEGIVEN,
                              ":No nickname given");
        return;
    }
    
    std::string newNick = cmd.getParam(0);
    
    if (!server_->getClientManager()->isNicknameAvailable(newNick)) {
        client->sendErrorReply(IRCCommandParser::ERR_NICKNAMEINUSE,
                              newNick + " :Nickname is already in use");
        return;
    }
    
    std::string oldNick = client->getNickname();
    auto clientManager = server_->getClientManager();
    
    if (!oldNick.empty()) {
        clientManager->removeClient(client->getFd());
    }

    client->setNickname(newNick);
    clientManager->registerNickname(client->getFd(), newNick);
    checkAuthentication(client);
}

void IRCCommandHandler::handleUSER(std::shared_ptr<IRCClient> client,
                                   const IRCCommandParser::IRCCommand& cmd) {
    if (client->isAuthenticated()) {
        client->sendErrorReply(IRCCommandParser::ERR_ALREADYREGISTRED,
                              ":You may not reregister");
        return;
    }
    
    if (cmd.params.size() < 3) {
        client->sendErrorReply(IRCCommandParser::ERR_NEEDMOREPARAMS,
                              "USER :Not enough parameters");
        return;
    }
    
    client->setUsername(cmd.getParam(0));
    client->setHostname(cmd.getParam(1));
    client->setRealname(cmd.trailing.empty() ? cmd.getParam(3) : cmd.trailing);
    
    checkAuthentication(client);
}

void IRCCommandHandler::handleJOIN(std::shared_ptr<IRCClient> client,
                                   const IRCCommandParser::IRCCommand& cmd) {
    if (cmd.params.empty()) {
        client->sendErrorReply(IRCCommandParser::ERR_NEEDMOREPARAMS,
                              "JOIN :Not enough parameters");
        return;
    }
    
    std::vector<std::string> channels = IRCCommandParser::splitChannels(cmd.getParam(0));
    
    auto channelManager = server_->getChannelManager();
    
    for (const auto& channelName : channels) {
        if (channelName.find("#logs-") == 0 && !channelManager->channelExists(channelName)) {
            client->sendErrorReply(IRCCommandParser::ERR_NOSUCHCHANNEL,
                                  channelName + " :Log channel does not exist");
            continue;
        }
        
        channelManager->joinChannel(client, channelName);
    }
}

void IRCCommandHandler::handlePART(std::shared_ptr<IRCClient> client,
                                   const IRCCommandParser::IRCCommand& cmd) {
    if (cmd.params.empty()) {
        client->sendErrorReply(IRCCommandParser::ERR_NEEDMOREPARAMS,
                              "PART :Not enough parameters");
        return;
    }
    
    std::vector<std::string> channels = IRCCommandParser::splitChannels(cmd.getParam(0));
    std::string reason = cmd.trailing.empty() ? client->getNickname() : cmd.trailing;
    
    auto channelManager = server_->getChannelManager();
    
    for (const auto& channelName : channels) {
        if (!channelManager->channelExists(channelName)) {
            client->sendErrorReply(IRCCommandParser::ERR_NOSUCHCHANNEL,
                                  channelName + " :No such channel");
            continue;
        }
        
        if (!client->isInChannel(channelName)) {
            client->sendErrorReply(IRCCommandParser::ERR_NOTONCHANNEL,
                                  channelName + " :You're not on that channel");
            continue;
        }
        
        channelManager->partChannel(client, channelName, reason);
    }
}

void IRCCommandHandler::handlePRIVMSG(std::shared_ptr<IRCClient> client,
                                      const IRCCommandParser::IRCCommand& cmd) {
    if (cmd.params.empty()) {
        client->sendErrorReply(IRCCommandParser::ERR_NORECIPIENT,
                              ":No recipient given (PRIVMSG)");
        return;
    }
    
    if (cmd.trailing.empty() && cmd.params.size() < 2) {
        client->sendErrorReply(IRCCommandParser::ERR_NOTEXTTOSEND,
                              ":No text to send");
        return;
    }
    
    std::string target = cmd.getParam(0);
    std::string message = cmd.trailing.empty() ? cmd.getParam(1) : cmd.trailing;
    
    if (target[0] == '#' || target[0] == '&') {
        auto channel = server_->getChannelManager()->getChannel(target);
        if (!channel) {
            client->sendErrorReply(IRCCommandParser::ERR_NOSUCHCHANNEL,
                                  target + " :No such channel");
            return;
        }
        
        if (!channel->hasClient(client->getNickname())) {
            client->sendErrorReply(IRCCommandParser::ERR_CANNOTSENDTOCHAN,
                                  target + " :Cannot send to channel");
            return;
        }
        
        std::string privmsg = IRCCommandParser::formatUserMessage(
            client->getNickname(), client->getUsername(), client->getHostname(),
            "PRIVMSG", target, message
        );
        channel->broadcastExcept(privmsg, client->getNickname());
    } else {
        auto targetClient = server_->getClientManager()->getClientByNickname(target);
        if (!targetClient) {
            client->sendErrorReply(IRCCommandParser::ERR_NOSUCHNICK,
                                  target + " :No such nick/channel");
            return;
        }
        
        std::string privmsg = IRCCommandParser::formatUserMessage(
            client->getNickname(), client->getUsername(), client->getHostname(),
            "PRIVMSG", target, message
        );
        targetClient->sendMessage(privmsg);
    }
}

void IRCCommandHandler::handleQUIT(std::shared_ptr<IRCClient> client,
                                   const IRCCommandParser::IRCCommand& cmd) {
    std::string quitMessage = cmd.trailing.empty() ? "Client Quit" : cmd.trailing;
    
    std::string quitNotice = ":" + client->getFullIdentifier() + " QUIT :" + quitMessage;
    for (const auto& channelName : client->getChannels()) {
        auto channel = server_->getChannelManager()->getChannel(channelName);
        if (channel) {
            channel->broadcastExcept(quitNotice, client->getNickname());
        }
    }
    
    server_->getChannelManager()->partAllChannels(client, quitMessage);
    client->setState(IRCClient::State::DISCONNECTED);
}

void IRCCommandHandler::handlePING(std::shared_ptr<IRCClient> client,
                                   const IRCCommandParser::IRCCommand& cmd) {
    std::string param = cmd.params.empty() ? server_->getServerName() : cmd.getParam(0);
    client->sendMessage(":" + server_->getServerName() + " PONG " + 
                       server_->getServerName() + " :" + param);
}

void IRCCommandHandler::handleLIST(std::shared_ptr<IRCClient> client,
                                   const IRCCommandParser::IRCCommand& cmd) {
    auto channelManager = server_->getChannelManager();
    auto channels = channelManager->getChannelList();

    for(const auto& channelName : channels) {
        auto channel = channelManager->getChannel(channelName);
        if(channel) {
            client->sendMessage(channel->getName() + " " + std::to_string(channel->getClientCount()) + " :" + channel->getTopic());
        }
    }
}

void IRCCommandHandler::handleNAMES(std::shared_ptr<IRCClient> client,
                                    const IRCCommandParser::IRCCommand& cmd) {
    if (cmd.params.empty()) {
        // Send names for all channels
        auto channelManager = server_->getChannelManager();
        auto channels = channelManager->getChannelList();
        for(const auto& channelName : channels) {
            auto channel = channelManager->getChannel(channelName);
            if(channel) {
                std::string names;
                for(const auto& member : channel->getClients()) {
                    names += member->getNickname() + " ";
                }
                client->sendNumericReply(IRCCommandParser::RPL_NAMREPLY, "= " + channelName + " :" + names);
                client->sendNumericReply(IRCCommandParser::RPL_ENDOFNAMES, channelName + " :End of /NAMES list.");
            }
        }
        return;
    }

    std::vector<std::string> channels = IRCCommandParser::splitChannels(cmd.getParam(0));
    auto channelManager = server_->getChannelManager();

    for (const auto& channelName : channels) {
        auto channel = channelManager->getChannel(channelName);
        if (channel) {
            std::string names;
            for(const auto& member : channel->getClients()) {
                names += member->getNickname() + " ";
            }
            client->sendNumericReply(IRCCommandParser::RPL_NAMREPLY, "= " + channelName + " :" + names);
            client->sendNumericReply(IRCCommandParser::RPL_ENDOFNAMES, channelName + " :End of /NAMES list.");
        }
    }
}

void IRCCommandHandler::sendWelcome(std::shared_ptr<IRCClient> client) {
    std::string nick = client->getNickname();
    std::string serverName = server_->getServerName();
    
    client->sendNumericReply(IRCCommandParser::RPL_WELCOME,
        ":Welcome to the LogCaster IRC Network " + client->getFullIdentifier());
    
    client->sendNumericReply(IRCCommandParser::RPL_YOURHOST,
        ":Your host is " + serverName + ", running version " + server_->getServerVersion());
    
    client->sendNumericReply(IRCCommandParser::RPL_CREATED,
        ":This server was created " + server_->getServerCreated());
    
    client->sendNumericReply(IRCCommandParser::RPL_MYINFO,
        serverName + " " + server_->getServerVersion() + " o o");
}

void IRCCommandHandler::checkAuthentication(std::shared_ptr<IRCClient> client) {
    if (!client->getNickname().empty() && !client->getUsername().empty()) {
        client->setState(IRCClient::State::AUTHENTICATED);
        sendWelcome(client);
    }
}
