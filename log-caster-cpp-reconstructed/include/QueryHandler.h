// [SEQUENCE: MVP2-23]
#ifndef QUERYHANDLER_H
#define QUERYHANDLER_H

#include <string>
#include <memory>
#include "LogBuffer.h"

class QueryHandler {
public:
    explicit QueryHandler(std::shared_ptr<LogBuffer> buffer);
    std::string processQuery(const std::string& query);

private:
    std::string handleSearch(const std::string& query);
    std::string handleStats();
    std::string handleCount();
    std::string handleHelp();

    std::shared_ptr<LogBuffer> buffer_;
};

#endif // QUERYHANDLER_H
