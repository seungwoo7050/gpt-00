# LogCaster: High-Performance C/C++ Log Server

![Language](https://img.shields.io/badge/language-C%20%26%20C%2B%2B-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Build](https://img.shields.io/badge/build-passing-brightgreen)

LogCaster is a high-performance, concurrent log aggregation server developed in parallel C and C++ versions. It is designed to handle massive volumes of log data from numerous clients simultaneously while providing a powerful real-time query interface.

This project serves as a practical case study comparing the trade-offs between C for raw performance and manual control, and Modern C++ (17/20) for safety, maintainability, and high-level abstractions.

## Key Features

- **Dual Implementations**: The entire feature set is built in both C (using POSIX APIs) and Modern C++, allowing for direct comparison.
- **High-Performance Concurrency**: Both versions use a thread pool architecture to handle thousands of concurrent client connections without blocking the main server loop.
- **Advanced Query System**: A dedicated query port allows for complex log retrieval using keywords, `AND`/`OR` operators, time ranges, and POSIX/std::regex regular expressions.
- **Asynchronous Persistence**: An optional persistence layer writes logs to disk via a dedicated I/O thread, ensuring log durability without impacting ingestion performance.
- **Log Rotation**: Automatically rotates log files based on size to manage disk space effectively.
- **Interactive IRC Interface (C++)**: The C++ version includes a built-in IRC server, allowing operators to connect with any IRC client to query logs and stream them in real-time to channels.

## Project Structure

The repository is divided into two main components:

- **`LogCaster-C/`**: A pure C implementation focusing on performance and direct system call interaction. It relies on `pthreads` and manual memory management.
- **`LogCaster-CPP/`**: A Modern C++17 implementation emphasizing safety, RAII, and object-oriented design. It heavily utilizes the STL, `std::thread`, `std::filesystem`, and smart pointers.

## Quick Start

### Prerequisites

- `gcc` and `g++` (or `clang`)
- `make`
- `cmake` (for the C++ version)

### Build

#### C Version

```bash
cd LogCaster-C
make
```

#### C++ Version

```bash
cd LogCaster-CPP
mkdir -p build && cd build
cmake ..
make
```

### Run

#### C Version (Basic)

```bash
./LogCaster-C/bin/logcaster-c
```

#### C++ Version (with Persistence and IRC enabled)

```bash
# This command starts the server, enables persistence in the ./logs directory,
# sets a 20MB file rotation size, and enables the IRC server on the default port.
./LogCaster-CPP/bin/logcaster-cpp -P -d ./logs -s 20 -i
```

## Basic Usage

#### Sending Logs (Port 9999)

Use `netcat` or any TCP client to send logs. Each line is treated as a new log entry.

```bash
echo "[ERROR] Database connection failed: timeout" | nc localhost 9999
```

#### Querying Logs (Port 9998)

Send commands to the query port to retrieve logs, stats, or help.

```bash
# Get server statistics
echo "STATS" | nc localhost 9998

# Perform a keyword search
echo "QUERY keyword=Database" | nc localhost 9998

# Perform a regex search
echo "QUERY regex=.*timeout" | nc localhost 9998
```

#### Using the IRC Interface (Port 6667)

1.  Connect your favorite IRC client (e.g., Irssi, HexChat) to `localhost:6667`.
2.  Set your nickname: `/NICK myadmin`
3.  Join a log channel: `/JOIN #logs-error` to see a real-time stream of error logs.
4.  Query directly in a channel: `!query keyword=timeout`

## Documentation

For a deeper understanding of the project, please refer to the full documentation set:

- **[DevHistory.md](DevHistory.md)**: A detailed chronicle of the entire development process, from MVP1 to the final version.
- **[ARCHITECTURE.md](ARCHITECTURE.md)**: A high-level overview of the system design and component interactions.
- **[QUICKSTART.md](QUICKSTART.md)**: A detailed guide to getting the server built and running with all options.
- **[IRC_USAGE_GUIDE.md](IRC_USAGE_GUIDE.md)**: A complete guide to using the interactive IRC interface.
- **[DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md)**: Instructions and best practices for deploying LogCaster.

## Contributing

Contributions are welcome! Please read **[CONTRIBUTING.md](CONTRIBUTING.md)** for details on our code of conduct and the process for submitting pull requests.

## License

This project is licensed under the MIT License - see the **[LICENSE.md](LICENSE.md)** file for details.
