# LogCaster Deployment Guide

This guide provides instructions and best practices for deploying and running the LogCaster server in a stable, long-running, production-like environment.

## 1. System Requirements

### Operating System
- A modern Linux distribution is recommended (e.g., Ubuntu 20.04+, CentOS/RHEL 7+, Debian 10+).

### Hardware
- **Minimum**: 1 CPU core, 512MB RAM.
- **Recommended**: 2+ CPU cores, 2GB+ RAM to handle high traffic and a large in-memory log buffer.

### File Descriptors
LogCaster is designed to handle a large number of concurrent connections. Most default Linux distributions have a low limit for open file descriptors per process (often 1024). This is a critical bottleneck that must be addressed for any serious use.

1.  **Check the current limit:**
    ```bash
    ulimit -n
    ```

2.  **Set the limit for the current session (temporary):**
    ```bash
    ulimit -n 65536
    ```

3.  **Set the limit permanently:**
    Edit the `/etc/security/limits.conf` file and add the following lines, replacing `logcaster_user` with the user that will run the server process.
    ```
    # /etc/security/limits.conf
    logcaster_user soft nofile 65536
    logcaster_user hard nofile 65536
    ```
    You will need to log out and log back in for this change to take effect.

## 2. Choosing a Version (C vs. C++)

-   **C Version (`logcaster-c`)**: Choose this for environments where minimizing binary size and memory footprint is the absolute top priority and you are comfortable with the trade-offs in safety inherent to C.
-   **C++ Version (`logcaster-cpp`)**: **Recommended for most use cases.** It provides greater stability, safety (RAII, smart pointers, type-safe containers), and advanced features like the IRC interface, with a negligible performance difference for this I/O-bound application.

## 3. Configuration

Configuration is managed via command-line flags. For production, it is recommended to run the server with persistence enabled.

-   `-p <port>`: The primary port for log ingestion (default: 9999). Ensure this port is open in your firewall for your log sources.
-   `-P`: **Enables persistence.** Highly recommended for any production use case to prevent data loss on restart.
-   `-d <dir>`: Specifies the directory for log files (e.g., `/var/log/logcaster`). Ensure the user running LogCaster has write permissions to this directory.
-   `-s <size_mb>`: The maximum size (in MB) for a single log file before it is rotated. This should be tuned based on your expected log volume and disk space.
-   `-i` / `-I <port>`: Enables the interactive IRC server (C++ only). It is strongly recommended to **expose this port only to trusted internal networks**, as it provides direct access to log data.

## 4. Running as a Systemd Service

For long-term operation, LogCaster should be managed by a process supervisor like `systemd`. This ensures it starts automatically on boot and restarts on failure.

1.  **Create a user for LogCaster:**
    ```bash
    sudo useradd -r -s /bin/false logcaster_user
    ```

2.  **Create a systemd unit file.**
    Create a file at `/etc/systemd/system/logcaster.service`:

    ```ini
    [Unit]
    Description=LogCaster C++ Log Aggregation Server
    After=network.target

    [Service]
    # Replace with the actual user/group you created
    User=logcaster_user
    Group=logcaster_user

    # Absolute path to the executable and desired flags
    ExecStart=/opt/logcaster/LogCaster-CPP/bin/logcaster-cpp -P -d /var/log/logcaster -s 100 -i
    
    Restart=on-failure
    RestartSec=5s

    # Set the required file descriptor limit
    LimitNOFILE=65536

    [Install]
    WantedBy=multi-user.target
    ```

3.  **Install and run the service:**
    ```bash
    # Reload systemd to recognize the new service file
    sudo systemctl daemon-reload

    # Start the service
    sudo systemctl start logcaster

    # Enable the service to start on boot
    sudo systemctl enable logcaster

    # Check the status of the service
    sudo systemctl status logcaster
    ```

## 5. Monitoring

-   **STATS Command**: Periodically connect to the query port (9998) and issue the `STATS` command to monitor the number of active clients, total logs processed, and dropped logs. This can be integrated with monitoring scripts.
-   **IRC Interface**: For the C++ version, operators can join the IRC server for a real-time, interactive view of the server's health and log flow.
-   **File System**: Monitor the disk usage of the persistence directory (`-d`) to ensure it does not fill up.
-   **Process Health**: Use `systemctl status logcaster` to check if the process is active and view its latest logs.
