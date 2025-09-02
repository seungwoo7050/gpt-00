# LogCaster IRC Interface Guide

This guide explains how to connect to and interact with the LogCaster server using its built-in IRC interface. This feature is available only in the C++ version of the server and must be enabled with the `-i` or `-I <port>` command-line flag.

## 1. Connecting to the Server

You can connect to LogCaster using any standard IRC client (e.g., Irssi, HexChat, mIRC, etc.).

- **Host / Address**: `localhost` (or the IP address of the machine running LogCaster)
- **Port**: `6667` (default) or the custom port specified with the `-I` flag.
- **Password**: No password is required.
- **SSL/TLS**: Not supported.

#### Example using Irssi:

```
/connect localhost 6667
```

Once connected, you should set your nickname:

```
/NICK my-admin-nick
```

## 2. Real-time Log Streaming

The primary feature of the IRC interface is the ability to stream logs in real-time by joining special, pre-defined channels. When a log message is received by the main LogCaster server, it is automatically broadcast to the corresponding channel.

To start receiving logs, simply join one of the following channels:

-   `#logs-all`: A firehose of every single log message received by the server.
-   `#logs-error`: Streams only logs that have been identified with an `ERROR` level.
-   `#logs-warning`: Streams only `WARNING` level logs.
-   `#logs-info`: Streams only `INFO` level logs.
-   `#logs-debug`: Streams `DEBUG` level logs.

#### Example:

To monitor all error messages across your infrastructure, execute:

```
/JOIN #logs-error
```

You will now see error logs appear in that channel window as they happen.

## 3. Interactive Querying

You can actively query the in-memory log buffer by sending commands in any channel you have joined. All commands are prefixed with `!` and are sent as a regular channel message.

The results of your query will be sent back to the channel by a bot named `LogBot`.

### `!query`

This is the main command for searching the log buffer. It uses the same powerful syntax as the TCP query port.

**Syntax**:
`!query <parameter>=<value> [<parameter>=<value> ...]`

**Examples:**

-   **Search for a single keyword:**
    ```
    !query keyword=database
    ```

-   **Search for multiple keywords (default is AND):**
    ```
    !query keywords=user,failed,login
    ```

-   **Search using OR logic:**
    ```
    !query keywords=timeout,refused operator=OR
    ```

-   **Search using a regular expression:**
    ```
    !query regex=user_[0-9]+_failed
    ```

-   **Search within a time range (using Unix timestamps):**
    ```
    !query time_from=1677628800 time_to=1677632400
    ```

-   **Combine multiple filters:**
    ```
    !query keywords=payment,error operator=AND regex=.*credit_card.* 
    ```

### Other Commands

-   **`!stats`**: Displays current statistics for the log buffer, including total logs processed, current count in memory, and dropped logs.
-   **`!help`**: Shows a list of available commands and syntax examples.

## 4. Standard IRC Commands

LogCaster supports all the basic IRC commands you would expect, allowing it to function as a lightweight chat and notification server in addition to its logging capabilities.

-   `/JOIN <#channel>`
-   `/PART <#channel>`
-   `/MSG <nickname> <message>`
-   `/NAMES <#channel>`
-   `/TOPIC <#channel> [new topic]`
-   `/QUIT [quit message]`
