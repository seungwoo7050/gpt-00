// [SEQUENCE: CPP-MVP6-6]
#ifndef IRCCOMMANDHANDLER_H
#define IRCCOMMANDHANDLER_H

#include <memory>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include "IRCCommandParser.h"

class IRCServer;
class IRCClient;

class IRCCommandHandler {
public:
    using CommandHandler = std::function<void(std::shared_ptr<IRCClient>, 
                                              const IRCCommandParser::IRCCommand&)>;
    
    explicit IRCCommandHandler(IRCServer* server);
    ~IRCCommandHandler();
    
    void handleCommand(std::shared_ptr<IRCClient> client, 
                      const IRCCommandParser::IRCCommand& cmd);
    
private:
    void handleNICK(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handleUSER(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handleJOIN(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handlePART(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handlePRIVMSG(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handleQUIT(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handlePING(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handleLIST(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);
    void handleNAMES(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& cmd);

    void sendWelcome(std::shared_ptr<IRCClient> client);
    void checkAuthentication(std::shared_ptr<IRCClient> client);

    IRCServer* server_;
    std::map<std::string, CommandHandler> handlers_;
};

#endif // IRCCOMMANDHANDLER_H
