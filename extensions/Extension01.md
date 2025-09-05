# Extension Guide 01: Advanced Real-time Log Filtering

## 1. Objective

This guide explains how to implement a dynamic, real-time log filtering system. The current implementation only provides basic filtering by log level into hardcoded IRC channels (`#logs-all`, `#logs-error`). The goal is to empower users to create their own filters based on keywords or regular expressions and direct matching logs to specific channels on-the-fly.

This will be achieved by:
1.  Creating a new `IRCFilterManager` to manage dynamic filter rules.
2.  Introducing new IRC commands for users to add, remove, and list filters (e.g., `!FILTER ADD <channel> <type> <pattern>`)
3.  Modifying the `LogBuffer` callback mechanism to pass all logs through the new filter manager.

## 2. Current Limitation (Legacy Code)

The current filtering logic is static and not user-configurable. In `IRCServer.cpp`, two separate callbacks are registered with the `LogBuffer`. Each callback contains a hardcoded rule that sends logs to a specific channel based on a simple check.

**Legacy Code (`src/IRCServer.cpp`):**
```cpp
// [SEQUENCE: CPP-MVP6-7]
void IRCServer::start() {
    // ...
    try {
        // ...
        if (logBuffer_) {
            // Callback 1: Sends ALL logs to #logs-all
            logBuffer_->registerCallback("#logs-all", 
                [this](const LogEntry& entry) {
                    channelManager_->distributeLogEntry(entry);
                });
            
            // Callback 2: Sends only ERROR logs to #logs-error
            logBuffer_->registerCallback("#logs-error", 
                [this](const LogEntry& entry) {
                    if (entry.level == "ERROR") {
                        channelManager_->distributeLogEntry(entry);
                    }
                });
        }
        // ...
    } catch (const std::exception& e) {
        // ...
    }
}
```
This approach has several drawbacks:
- **Inflexible:** Users cannot create their own filters or target channels.
- **Inefficient:** The `LogBuffer` holds a map of callbacks, which is not ideal for a large number of dynamic rules.
- **Hardcoded:** The logic is compiled in and cannot be changed at runtime.

## 3. Proposed Solution: A Dynamic Filter Manager

We will introduce two new classes, `IRCFilter` and `IRCFilterManager`.

-   **`IRCFilter`**: A simple data structure to hold the definition of a single filter rule: an ID, target channel, filter type (keyword or regex), and the filter pattern.
-   **`IRCFilterManager`**: A class responsible for storing a list of all active `IRCFilter` objects. It will expose methods to `add`, `remove`, and `list` filters. Its core method, `processLogEntry`, will check each incoming log against all filters and distribute it to the appropriate channels.

`IRCServer` will be modified to use a single, generic callback that forwards every log entry to the `IRCFilterManager`. The `IRCCommandHandler` will be updated to parse the new `!FILTER` command and interact with the `IRCFilterManager`.

## 4. Implementation Guidance

#### **Step 1: Define `IRCFilter.h`**

Create a new header file for the `IRCFilter` data structure.

**New File (`include/IRCFilter.h`):**
```cpp
#pragma once

#include <string>
#include <atomic>

enum class FilterType {
    KEYWORD,
    REGEX
};

struct IRCFilter {
    int id;
    std::string channel;
    FilterType type;
    std::string pattern;
    std::regex compiled_regex; // For REGEX type

    static std::atomic<int> next_id;

    IRCFilter(const std::string& ch, FilterType t, const std::string& p) 
        : channel(ch), type(t), pattern(p) {
        id = next_id++;
        if (type == FilterType::REGEX) {
            try {
                compiled_regex.assign(pattern, std::regex_constants::icase);
            } catch (const std::regex_error& e) {
                throw std::runtime_error("Invalid regex pattern: " + std::string(e.what()));
            }
        }
    }
};

// In a corresponding .cpp file, you must define the static member:
// std::atomic<int> IRCFilter::next_id(1);
```

#### **Step 2: Create `IRCFilterManager.h` and `IRCFilterManager.cpp`**

**New File (`include/IRCFilterManager.h`):**
```cpp
#pragma once

#include "IRCFilter.h"
#include "LogEntry.h"
#include "IRCChannelManager.h"
#include <vector>
#include <string>
#include <mutex>
#include <memory>

class IRCFilterManager {
public:
    IRCFilterManager(std::shared_ptr<IRCChannelManager> channelManager);

    std::string addFilter(const std::string& channel, FilterType type, const std::string& pattern);
    std::string removeFilter(int id);
    std::string listFilters();

    void processLogEntry(const LogEntry& entry);

private:
    std::vector<IRCFilter> filters_;
    std::mutex mutex_;
    std::shared_ptr<IRCChannelManager> channelManager_;
};
```

**New File (`src/IRCFilterManager.cpp`):**
```cpp
#include "IRCFilterManager.h"
#include <sstream>

// Don't forget to define the static ID counter for IRCFilter
std::atomic<int> IRCFilter::next_id(1);

IRCFilterManager::IRCFilterManager(std::shared_ptr<IRCChannelManager> channelManager)
    : channelManager_(channelManager) {}

std::string IRCFilterManager::addFilter(const std::string& channel, FilterType type, const std::string& pattern) {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        filters_.emplace_back(channel, type, pattern);
        return "Successfully added filter with ID " + std::to_string(filters_.back().id) + ".";
    } catch (const std::exception& e) {
        return "Error: " + std::string(e.what());
    }
}

std::string IRCFilterManager::removeFilter(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::remove_if(filters_.begin(), filters_.end(), 
                             [id](const IRCFilter& f) { return f.id == id; });
    if (it != filters_.end()) {
        filters_.erase(it, filters_.end());
        return "Successfully removed filter ID " + std::to_string(id) + ".";
    }
    return "Error: Filter ID not found.";
}

std::string IRCFilterManager::listFilters() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (filters_.empty()) {
        return "No active filters.";
    }
    std::stringstream ss;
    ss << "Active filters:\n";
    for (const auto& f : filters_) {
        ss << "ID: " << f.id << ", Channel: " << f.channel 
           << ", Type: " << (f.type == FilterType::KEYWORD ? "KEYWORD" : "REGEX")
           << ", Pattern: " << f.pattern << "\n";
    }
    return ss.str();
}

void IRCFilterManager::processLogEntry(const LogEntry& entry) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string formatted_log = entry.level + ": " + entry.message;

    for (const auto& filter : filters_) {
        bool match = false;
        if (filter.type == FilterType::KEYWORD) {
            if (formatted_log.find(filter.pattern) != std::string::npos) {
                match = true;
            }
        } else if (filter.type == FilterType::REGEX) {
            if (std::regex_search(formatted_log, filter.compiled_regex)) {
                match = true;
            }
        }

        if (match) {
            // Use the channel manager to send the log to the target channel
            channelManager_->broadcastToChannel(filter.channel, formatted_log, std::nullopt);
        }
    }
}
```

#### **Step 3: Integrate `IRCFilterManager` into `IRCServer`**

Modify `IRCServer` to use the new manager and a single generic callback.

**Modified Code (`include/IRCServer.h`):**
```cpp
// ... includes ...
#include "IRCFilterManager.h" // Add this

class IRCServer {
    // ...
private:
    // ...
    std::unique_ptr<IRCFilterManager> filterManager_;
    // ...
};
```

**Modified Code (`src/IRCServer.cpp`):**
```cpp
// ... includes ...
#include "IRCFilterManager.h"

IRCServer::IRCServer(int port, std::shared_ptr<LogBuffer> logBuffer)
    : port_(port)
    , listenFd_(-1)
    , running_(false)
    , logBuffer_(logBuffer) {
    
    // ... (rest of constructor)
    threadPool_ = std::make_unique<ThreadPool>(THREAD_POOL_SIZE);
    channelManager_ = std::make_shared<IRCChannelManager>();
    clientManager_ = std::make_shared<IRCClientManager>();
    commandParser_ = std::make_unique<IRCCommandParser>();
    commandHandler_ = std::make_unique<IRCCommandHandler>(this);

    // Initialize the new filter manager
    filterManager_ = std::make_unique<IRCFilterManager>(channelManager_);
}

void IRCServer::start() {
    if (running_.load()) {
        return;
    }
    
    try {
        setupSocket();
        
        channelManager_->initializeLogChannels();
        
        if (logBuffer_) {
            // Remove the old, specific callbacks
            // logBuffer_->registerCallback("#logs-all", ...);
            // logBuffer_->registerCallback("#logs-error", ...);

            // Add a single, generic callback
            logBuffer_->registerCallback("global-filter-callback", 
                [this](const LogEntry& entry) {
                    // Always send to #logs-all
                    channelManager_->distributeLogEntry(entry);

                    // Process through dynamic filters
                    filterManager_->processLogEntry(entry);
                });
        }
        
        running_ = true;
        acceptThread_ = std::thread(&IRCServer::acceptClients, this);
        std::cout << "IRC server started on port " << port_ << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to start IRC server: " << e.what() << std::endl;
        throw;
    }
}

// Add an accessor for the command handler to use
IRCFilterManager* IRCServer::getFilterManager() {
    return filterManager_.get();
}
```

#### **Step 4: Update `IRCCommandHandler` to manage filters**

Add the `!filter` command logic.

**Modified Code (`src/IRCCommandHandler.cpp`):**
```cpp
// In handleCommand, for PRIVMSG:
void IRCCommandHandler::handlePrivmsg(std::shared_ptr<IRCClient> client, const IRCCommandParser::IRCCommand& command) {
    // ... (existing logic for sending messages to channels)

    // Add logic for commands like !filter
    if (!command.trailing.empty() && command.trailing[0] == '!') {
        handleBotCommand(client, command.params[0], command.trailing);
    }
}

// Add a new private method to handle bot commands
void IRCCommandHandler::handleBotCommand(std::shared_ptr<IRCClient> client, const std::string& channel, const std::string& text) {
    std::stringstream ss(text);
    std::string botCmd;
    ss >> botCmd;

    if (botCmd == "!filter") {
        std::string subCmd;
        ss >> subCmd;

        auto filterManager = server_->getFilterManager();
        std::string response;

        if (subCmd == "ADD") {
            std::string targetChannel, typeStr, pattern;
            ss >> targetChannel >> typeStr;
            std::getline(ss, pattern);
            pattern = pattern.substr(pattern.find_first_not_of(" 	")); // trim leading whitespace

            if (targetChannel.empty() || typeStr.empty() || pattern.empty()) {
                response = "Usage: !filter ADD <#channel> <KEYWORD|REGEX> <pattern>";
            } else {
                FilterType type = (typeStr == "REGEX") ? FilterType::REGEX : FilterType::KEYWORD;
                response = filterManager->addFilter(targetChannel, type, pattern);
            }
        } else if (subCmd == "DEL") {
            int id;
            ss >> id;
            if (ss.fail()) {
                response = "Usage: !filter DEL <id>";
            } else {
                response = filterManager->removeFilter(id);
            }
        } else if (subCmd == "LIST") {
            response = filterManager->listFilters();
        } else {
            response = "Unknown filter command. Use ADD, DEL, or LIST.";
        }
        server_->getChannelManager()->sendPrivmsgToClient(client, channel, response);
    }
}
```

## 5. Verification

1.  Compile and run the updated server.
2.  Connect with an IRC client (e.g., `irssi`, or the `test_irc.py` script).
3.  Join `#logs-all` and a custom channel, e.g., `#database`.
    - `JOIN #logs-all`
    - `JOIN #database`
4.  Add a new filter from your IRC client:
    - `PRIVMSG #database :!filter ADD #database KEYWORD FATAL`
5.  Use a separate terminal to send logs to the server (port 9999):
    ```bash
    echo "INFO: User logged in" | nc localhost 9999
    echo "FATAL: Database connection lost" | nc localhost 9999
    echo "WARN: Disk space is low" | nc localhost 9999
    ```
6.  **Observe the results:**
    - `#logs-all` should receive all three log messages.
    - `#database` should only receive the "FATAL: Database connection lost" message.
7.  List the filters to see your rule:
    - `PRIVMSG #database :!filter LIST`
8.  Remove the filter:
    - `PRIVMSG #database :!filter DEL 1` (assuming the ID is 1)
9.  Send the "FATAL" log again and verify it no longer appears in the `#database` channel.

```