# LogCaster Troubleshooting Guide

This guide provides solutions to common problems you might encounter while deploying and running the LogCaster server.

---

### Problem: Server fails to start with "address already in use"

-   **Symptom**: The server exits immediately upon launch, and the log (or console output) shows an error message like `bind: Address already in use`.

-   **Cause**: Another process is already using the port LogCaster is trying to bind to (e.g., 9999, 9998, or 6667).

-   **Solution**:
    1.  Identify the process using the required port. Replace `<port>` with the port number from the error message.
        ```bash
        sudo lsof -i :<port>
        # Or
        sudo netstat -tulnp | grep <port>
        ```
    2.  If the conflicting process is not essential, stop it.
    3.  Alternatively, run LogCaster on a different port using the `-p` (for logs), or `-I` (for IRC) flags.

---

### Problem: Server starts but immediately refuses new connections

-   **Symptom**: The server process is running, but any attempt to connect (e.g., with `netcat`) is immediately rejected.

-   **Cause**: This is almost always caused by the operating system's **file descriptor limit** for the process being too low. The server uses one file descriptor for each connection, and if it hits the limit, it cannot accept any more.

-   **Solution**:
    1.  Check the current limit for the running process or user: `ulimit -n`.
    2.  Increase the limit. For a permanent solution, edit `/etc/security/limits.conf` and ensure the `LimitNOFILE` directive is set in your `systemd` service file, as described in the **[DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md)**.
        ```ini
        # In your logcaster.service file
        [Service]
        LimitNOFILE=65536
        ```
    3.  Restart the LogCaster service (`sudo systemctl restart logcaster`) after applying the changes.

---

### Problem: Log files are not being created in the persistence directory

-   **Symptom**: You are running the server, but the directory specified with `-d` remains empty.

-   **Cause 1**: Persistence is not enabled.
    -   **Solution**: Ensure the `-P` flag is included in your startup command or `ExecStart` line in your `systemd` service file.

-   **Cause 2**: Incorrect file permissions.
    -   **Solution**: The user running the LogCaster process must have write permissions for the target directory. Verify permissions with `ls -ld /path/to/your/log/dir` and grant access if needed:
        ```bash
        sudo chown -R logcaster_user:logcaster_group /path/to/your/log/dir
        sudo chmod -R 755 /path/to/your/log/dir
        ```

---

### Problem: High memory usage that grows over time

-   **Symptom**: The server's memory consumption continuously increases and does not stabilize.

-   **Cause**: This could be due to a very high volume of logs keeping the in-memory buffer full, or a potential memory leak.

-   **Solution**:
    1.  **Check Buffer Status**: Connect to the query port (9998) and run the `STATS` command. 
        ```bash
        echo "STATS" | nc localhost 9998
        ```
        If the `Current=` value is consistently equal to the buffer's capacity (default 10,000), the memory usage is likely normal and expected. If this is too high for your system, you can reduce the `DEFAULT_BUFFER_SIZE` constant in the source code (`log_buffer.h`) and recompile.

    2.  **Check for Leaks (C Version)**: The C version is more susceptible to memory leaks. If memory usage grows unboundedly even when the log count is stable, this may indicate a leak. Run the server under `valgrind` to diagnose the issue:
        ```bash
        valgrind --leak-check=full ./LogCaster-C/bin/logcaster-c
        ```
        Please report any leaks found as a bug on the project's issue tracker.

---

### Problem: The C++ server fails to start with a `std::filesystem` error

-   **Symptom**: The server fails on startup with an error related to `std::filesystem`.

-   **Cause**: Your system's C++ standard library may have incomplete or buggy support for `<filesystem>`. This can happen on older compilers or systems (e.g., GCC versions before 9).

-   **Solution**:
    1.  Ensure you are using a modern C++ compiler (GCC 9+ or Clang 7+).
    2.  When compiling, you may need to explicitly link against the filesystem library by adding `lstdc++fs` to the linker flags. The included `CMakeLists.txt` attempts to do this automatically, but manual intervention may be required on some systems.
