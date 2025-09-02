# LogCaster Quick Start Guide

This guide provides detailed, step-by-step instructions to compile, run, and verify both the C and C++ versions of the LogCaster server.

## 1. Prerequisites

Before you begin, ensure you have the following tools installed on your system. These are standard development tools available on most Linux distributions.

- **C/C++ Compiler**: `gcc` and `g++` (or `clang`)
- **Build Tools**: `make` and `cmake`

#### Installation Examples

- **For Debian/Ubuntu-based systems:**
  ```bash
  sudo apt-get update && sudo apt-get install -y build-essential cmake
  ```

- **For Red Hat/CentOS-based systems:**
  ```bash
  sudo yum groupinstall -y "Development Tools"
  sudo yum install -y cmake
  ```

## 2. Building the C Version (LogCaster-C)

The C version uses a standard `Makefile` for a simple and fast build process.

1.  Navigate to the C version's directory:
    ```bash
    cd LogCaster-C
    ```

2.  (Optional) Clean any previous builds:
    ```bash
    make clean
    ```

3.  Compile the project:
    ```bash
    make
    ```

This will compile all source files and create the final executable at `bin/logcaster-c`.

## 3. Building the C++ Version (LogCaster-CPP)

The C++ version uses `CMake` for a more robust and cross-platform-friendly build system.

1.  Navigate to the C++ version's directory:
    ```bash
    cd LogCaster-CPP
    ```

2.  Create a separate build directory (out-of-source build):
    ```bash
    mkdir -p build && cd build
    ```

3.  Run CMake to configure the project:
    ```bash
    cmake ..
    ```

4.  Compile the project:
    ```bash
    make
    ```

This will compile the project and create the final executable at `bin/logcaster-cpp`.

## 4. Running the Server

Both server versions can be started from their respective parent directories (`LogCaster-C` or `LogCaster-CPP`).

### Command-Line Options

Both versions support the following flags for configuration:

-   `-h`: Displays the help message.
-   `-p <port>`: Sets the port for the main log ingestion server (default: 9999).
-   `-P`: Enables write-through persistence to disk.
-   `-d <directory>`: Specifies the directory for log files (default: `./logs`).
-   `-s <size_mb>`: Sets the maximum size (in MB) for a single log file before rotation (default: 10).

The C++ version also supports IRC-specific flags:
-   `-i`: Enables the interactive IRC server (default port: 6667).
-   `-I <port>`: Enables the IRC server on a specific port.

### Execution Examples

- **Run the C server with default settings:**
  ```bash
  ./LogCaster-C/bin/logcaster-c
  ```

- **Run the C++ server on a different port:**
  ```bash
  ./LogCaster-CPP/bin/logcaster-cpp -p 10000
  ```

- **Run the C++ server with persistence and IRC enabled:**
  ```bash
  # Persist logs to /var/log/logcaster, rotate files at 50MB, and enable IRC on port 7777
  ./LogCaster-CPP/bin/logcaster-cpp -P -d /var/log/logcaster -s 50 -i -I 7777
  ```

## 5. Verifying the Setup

Once the server is running, you can test its functionality using a simple tool like `netcat` (`nc`).

1.  **Send a log message:**
    Open a new terminal and run:
    ```bash
    echo "[INFO] User 'admin' logged in successfully." | nc localhost 9999
    ```
    You should see a confirmation in the server's console output.

2.  **Query for statistics:**
    ```bash
    echo "STATS" | nc localhost 9998
    ```
    The server will respond with the current log buffer statistics.

3.  **Connect via IRC (if enabled on C++ version):**
    Use your favorite IRC client to connect to `localhost` on the specified IRC port (e.g., 6667). You can then join channels like `#logs-all` to see a real-time stream of logs.
