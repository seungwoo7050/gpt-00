// [SEQUENCE: CPP-MVP6-5]
#ifndef IRCCOMMANDPARSER_H
#define IRCCOMMANDPARSER_H

#include <string>
#include <vector>
#include <sstream>
#include <map>

class IRCCommandParser {
public:
    struct IRCCommand {
        std::string prefix;
        std::string command;
        std::vector<std::string> params;
        std::string trailing;
        
        std::string getParam(size_t index) const {
            return (index < params.size()) ? params[index] : "";
        }
    };
    
    enum ReplyCode {
        RPL_WELCOME = 1,
        RPL_YOURHOST = 2,
        RPL_CREATED = 3,
        RPL_MYINFO = 4,
        RPL_NAMREPLY = 353,
        RPL_ENDOFNAMES = 366,
        ERR_NOSUCHNICK = 401,
        ERR_NOSUCHCHANNEL = 403,
        ERR_CANNOTSENDTOCHAN = 404,
        ERR_NORECIPIENT = 411,
        ERR_NOTEXTTOSEND = 412,
        ERR_UNKNOWNCOMMAND = 421,
        ERR_NONICKNAMEGIVEN = 431,
        ERR_NICKNAMEINUSE = 433,
        ERR_NOTONCHANNEL = 442,
        ERR_NOTREGISTERED = 451,
        ERR_NEEDMOREPARAMS = 461,
        ERR_ALREADYREGISTRED = 462,
    };
    
    static IRCCommand parse(const std::string& line);
    
    static std::string formatReply(const std::string& serverName, 
                                   const std::string& nick,
                                   int code, 
                                   const std::string& params);

    static std::string formatUserMessage(const std::string& nick,
                                         const std::string& user,
                                         const std::string& host,
                                         const std::string& command,
                                         const std::string& target,
                                         const std::string& message);

    static std::string toUpper(const std::string& str);
    static std::vector<std::string> splitChannels(const std::string& channels);

private:
    static std::string extractPrefix(std::string& line);
    static std::string extractCommand(std::string& line);
    static std::vector<std::string> extractParams(std::string& line);
};

#endif // IRCCOMMANDPARSER_H
