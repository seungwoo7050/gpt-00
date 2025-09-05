// [SEQUENCE: CPP-MVP6-10]
#include "IRCCommandParser.h"
#include <algorithm>
#include <cctype>

IRCCommandParser::IRCCommand IRCCommandParser::parse(const std::string& line) {
    IRCCommand cmd;
    
    if (line.empty()) {
        return cmd;
    }
    
    std::string workingLine = line;
    
    if (!workingLine.empty() && (workingLine.back() == '\r' || workingLine.back() == '\n')) {
        workingLine.pop_back();
    }
    
    cmd.prefix = extractPrefix(workingLine);
    cmd.command = extractCommand(workingLine);
    cmd.params = extractParams(workingLine);
    
    if (!cmd.params.empty() && !cmd.params.back().empty() && cmd.params.back()[0] == ':') {
        cmd.trailing = cmd.params.back().substr(1);
        cmd.params.pop_back();
    }
    
    return cmd;
}

std::string IRCCommandParser::extractPrefix(std::string& line) {
    if (line.empty() || line[0] != ':') {
        return "";
    }
    
    size_t spacePos = line.find(' ');
    if (spacePos == std::string::npos) {
        return "";
    }
    
    std::string prefix = line.substr(1, spacePos - 1);
    line = line.substr(spacePos + 1);
    return prefix;
}

std::string IRCCommandParser::extractCommand(std::string& line) {
    if (line.empty()) {
        return "";
    }
    
    size_t spacePos = line.find(' ');
    if (spacePos == std::string::npos) {
        std::string command = line;
        line.clear();
        return toUpper(command);
    }
    
    std::string command = line.substr(0, spacePos);
    line = line.substr(spacePos + 1);
    return toUpper(command);
}

std::vector<std::string> IRCCommandParser::extractParams(std::string& line) {
    std::vector<std::string> params;
    
    while (!line.empty()) {
        if (line[0] == ':') {
            params.push_back(line);
            break;
        }
        
        size_t spacePos = line.find(' ');
        if (spacePos == std::string::npos) {
            params.push_back(line);
            break;
        }
        
        params.push_back(line.substr(0, spacePos));
        line = line.substr(spacePos + 1);
    }
    
    return params;
}

std::string IRCCommandParser::formatReply(const std::string& serverName,
                                          const std::string& nick,
                                          int code,
                                          const std::string& params) {
    std::ostringstream oss;
    oss << ":" << serverName << " " << code << " " << nick << " " << params;
    return oss.str();
}

std::string IRCCommandParser::formatUserMessage(const std::string& nick,
                                                const std::string& user,
                                                const std::string& host,
                                                const std::string& command,
                                                const std::string& target,
                                                const std::string& message) {
    std::ostringstream oss;
    oss << ":" << nick << "!" << user << "@" << host 
        << " " << command << " " << target;
    
    if (!message.empty()) {
        oss << " :" << message;
    }
    
    return oss.str();
}

std::string IRCCommandParser::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::vector<std::string> IRCCommandParser::splitChannels(const std::string& channels) {
    std::vector<std::string> result;
    std::stringstream ss(channels);
    std::string channel;
    
    while (std::getline(ss, channel, ',')) {
        if (!channel.empty()) {
            result.push_back(channel);
        }
    }
    
    return result;
}