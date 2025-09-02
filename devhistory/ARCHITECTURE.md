# LogCaster Architecture

This document provides a high-level overview of the LogCaster system architecture. It covers both the C and C++ implementations, highlighting their core components, interactions, and data flow.

## Core Concepts (Shared Architecture)

Both the C and C++ versions are built around the same fundamental architectural concepts:

1.  **Log Ingestion Endpoint**: A primary TCP server (`LogServer`) listens on a dedicated port (default: 9999) to accept high-volume, incoming log data from clients.

2.  **Query Endpoint**: A secondary TCP server listens on a separate port (default: 9998) to handle interactive queries without interfering with log ingestion.

3.  **Main Event Loop**: The main thread in both implementations uses `select()` to monitor the listener sockets for new connections. This allows the server to remain responsive to new clients while offloading all heavy processing.

4.  **Thread Pool**: A fixed-size pool of worker threads is used to handle all client-related tasks (I/O, parsing, processing). This decouples the main thread from client processing, enabling high concurrency.

5.  **In-Memory Buffer**: A thread-safe, circular buffer (`LogBuffer`) acts as the primary, high-speed storage for recent logs. It is designed to be accessed concurrently by multiple threads.

6.  **Persistence Manager**: An optional component that runs in its own dedicated thread. It consumes logs from a queue and writes them to disk asynchronously, ensuring log durability without blocking the performance-sensitive ingestion path.

## C Implementation (`LogCaster-C`)

The C version is a procedural implementation that relies on POSIX APIs and manual resource management.

### Component Diagram

```
+-----------------+      +--------------------+      +--------------------+      +--------------------+
|   Main Thread   |----->|   Client Job (1)   |----->|                    |----->| Persistence Queue  |
|  (select loop)  |      +--------------------+      |                    |      +--------------------+
+-----------------+      |   Client Job (2)   |      |    Log Buffer      |             |
        |                +--------------------+      | (Circular Array)   |             |
        |                |        ...         |      | (Mutex Protected)  |             v
        |                +--------------------+      |                    |      +--------------------+
        |                |   Client Job (N)   |----->|                    |----->| Writer Thread      |
        |                +--------------------+      +--------------------+      | (Async I/O)        |
        |                      ^                                                     +--------------------+
        |                      |
+-----------------+      +--------------------+
|  Query Client   |----->|   Query Handler    |
| (port 9998)     |      | (Direct Handling)  |
+-----------------+      +--------------------+
```

### Key Characteristics

-   **Data Structures**: Components are managed via `struct` pointers (e.g., `log_server_t`, `thread_pool_t`, `client_job_t`). All memory is managed manually with `malloc()` and `free()`.
-   **Concurrency**: Implemented directly with `pthreads`, `pthread_mutex_t`, and `pthread_cond_t`.
-   **Data Flow**: When a new log client connects, the main thread `malloc`s a `client_job_t` struct, populates it with the client's file descriptor and the server context, and enqueues it to the thread pool. A worker thread dequeues the job, reads data from the socket, and pushes it to both the in-memory `LogBuffer` and the `PersistenceManager`'s write queue.

## C++ Implementation (`LogCaster-CPP`)

The C++ version uses modern C++ features to create a more robust, type-safe, and maintainable object-oriented system.

### Component Diagram

```
+-----------------+      +--------------------+      +--------------------+      +--------------------+
|   LogServer     |----->|   ThreadPool       |----->|     LogBuffer      |----->| PersistenceManager |
| (select loop)   |      | (std::thread)      |      | (std::deque)       |      | (std::thread)      |
+-----------------+      +--------------------+      +--------------------+      +--------------------+
        |                      ^                           ^      ^
        |                      |                           |      |
+-----------------+      +--------------------+      +--------------------+      
|  Query Client   |----->|   QueryHandler     |------|      | (Shared Pointer)
| (port 9998)     |      | (uses ThreadPool)  |      +--------------------+
+-----------------+      +--------------------+                 |
                                                                 |
                                                        +--------------------+
                                                        |     IRCServer      |
                                                        | (std::thread)      |
                                                        +--------------------+
```

### Key Characteristics

-   **Object-Oriented Design**: All components are encapsulated in classes (`LogServer`, `ThreadPool`, `PersistenceManager`, `IRCServer`).
-   **RAII & Smart Pointers**: Resource ownership is managed automatically. `std::unique_ptr` is used for components with a single owner (like `ThreadPool`), while `std::shared_ptr` is used for the `LogBuffer` so it can be safely accessed by both the `LogServer` and the `IRCServer`.
-   **Concurrency**: Implemented with `std::thread`, `std::mutex`, and `std::condition_variable`. Tasks are submitted to the `ThreadPool` as type-safe `std::function` objects, often created from lambdas, which cleanly captures context without manual structs.
-   **Modularity**: The `IRCServer` is a prime example of modularity. It runs in its own thread and interacts with the core logging system non-intrusively via the shared `LogBuffer` and a callback mechanism, demonstrating excellent separation of concerns.

## Data Flow Summary

1.  **Log Ingestion**: A client connects to the **Log Ingestion Port** (9999). The `LogServer`'s main thread accepts the connection and passes the client socket to the `ThreadPool`. A worker thread takes over, reads log data, and pushes it to the `LogBuffer` and (if enabled) the `PersistenceManager`'s queue.

2.  **Log Querying**: A client connects to the **Query Port** (9998). The `LogServer` accepts the connection and passes the task to the `ThreadPool`. A worker thread reads the query string, uses the `QueryHandler` to parse it and search the `LogBuffer`, and writes the results back to the client.

3.  **IRC Interaction (C++ only)**: An IRC client connects to the **IRC Port** (6667). The `IRCServer` (running in its own thread) handles the client. Commands are processed by the `IRCCommandHandler`. If a user requests a log stream or query, the handler interacts with the shared `LogBuffer` to fulfill the request.
