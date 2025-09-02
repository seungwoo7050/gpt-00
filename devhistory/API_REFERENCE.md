# LogCaster API Reference

This document provides a reference for the key public APIs in both the C and C++ implementations of LogCaster. It is intended for developers who wish to extend, integrate, or better understand the core components of the project.

## C Version (`LogCaster-C`)

The C version exposes its API through a set of header files, each corresponding to a major component. The API is procedural, based on manipulating `struct` pointers.

### `server.h`

Manages the main server lifecycle and network events.

-   `log_server_t* server_create(int port)`
    -   Creates and allocates the main `log_server_t` structure. Initializes all sub-components (`ThreadPool`, `LogBuffer`, etc.) to their default states.
-   `void server_destroy(log_server_t* server)`
    -   Frees all resources associated with the server, including the thread pool, log buffer, and persistence manager.
-   `int server_init(log_server_t* server)`
    -   Initializes and binds the TCP listener sockets for both log ingestion and queries.
-   `void server_run(log_server_t* server)`
    -   Starts the main blocking event loop, which uses `select()` to wait for new connections.

### `thread_pool.h`

A generic, reusable thread pool for concurrent task execution.

-   `thread_pool_t* thread_pool_create(int thread_count)`
    -   Creates the thread pool structure and starts the specified number of worker `pthread`s.
-   `void thread_pool_destroy(thread_pool_t* pool)`
    -   Gracefully shuts down the thread pool, waiting for all active jobs to complete before freeing resources.
-   `int thread_pool_add_job(thread_pool_t* pool, void (*function)(void*), void* arg)`
    -   Submits a new job to the pool. The job consists of a function pointer and a `void*` argument for context.

### `log_buffer.h`

A thread-safe, in-memory circular buffer for log entries.

-   `log_buffer_t* log_buffer_create(size_t capacity)`
    -   Allocates and initializes the circular buffer with a given capacity.
-   `int log_buffer_push(log_buffer_t* buffer, const char* message)`
    -   Atomically pushes a new log message into the buffer. If the buffer is full, the oldest entry is overwritten.
-   `int log_buffer_search_enhanced(log_buffer_t* buffer, const parsed_query_t* query, char*** results, int* count)`
    -   Performs a thread-safe search of the buffer based on the criteria in a `parsed_query_t` object.

### `persistence.h`

Manages asynchronous writing of logs to disk.

-   `persistence_manager_t* persistence_create(const persistence_config_t* config)`
    -   Creates the persistence manager and starts its dedicated I/O writer thread.
-   `int persistence_write(persistence_manager_t* manager, const char* log_entry, time_t timestamp)`
    -   Adds a log message to a thread-safe queue to be written to disk asynchronously.

---

## C++ Version (`LogCaster-CPP`)

The C++ API is object-oriented, with functionality encapsulated into classes. Resource management is handled automatically through RAII and smart pointers.

### `LogServer.h`

The central orchestrator class for the logging and query server.

-   `class LogServer`
    -   **`LogServer(int port)`**: The constructor initializes all member components (`ThreadPool`, `LogBuffer`, etc.) using `std::make_unique` and `std::make_shared`.
    -   **`~LogServer()`**: The destructor automatically handles the cleanup of all resources.
    -   **`void start()`**: Starts the main server event loop.
    -   **`void setPersistenceManager(std::unique_ptr<PersistenceManager> persistence)`**: An example of setter dependency injection for optional components.

### `ThreadPool.h`

A generic, type-safe, and reusable thread pool.

-   `class ThreadPool`
    -   **`ThreadPool(size_t numThreads)`**: Constructor starts the worker `std::thread`s.
    -   **`~ThreadPool()`**: Destructor gracefully joins all worker threads.
    -   **`template<...> auto enqueue(F&& f, Args&&... args)`**: A variadic template function that accepts any callable (function, lambda, etc.) and its arguments. It returns a `std::future` allowing the caller to retrieve the task's result asynchronously.

### `LogBuffer.h`

A thread-safe, in-memory log buffer using STL containers.

-   `class LogBuffer`
    -   **`void push(const std::string& message)`**: Pushes a new log message into the internal `std::deque` with mutex protection.
    -   **`std::vector<std::string> searchEnhanced(const ParsedQuery& query) const`**: Performs a search based on a `ParsedQuery` object and returns the results by value.
    -   **`void registerChannelCallback(...)`**: A key function for the IRC integration. It allows other components (like the `IRCServer`) to subscribe to log events that match a specific filter, enabling real-time streaming.

### `Persistence.h`

Manages asynchronous disk I/O using modern C++ features.

-   `class PersistenceManager`
    -   **`PersistenceManager(const PersistenceConfig& config)`**: Constructor creates the log directory using `std::filesystem` and starts the writer `std::thread`.
    -   **`~PersistenceManager()`**: Destructor automatically signals the writer thread to stop, joins it, and closes the `std::ofstream` file handle.
    -   **`void write(const std::string& message, ...)`**: Queues a log message to the internal `std::queue` for asynchronous writing.

### `IRCServer.h`

A self-contained IRC server that integrates with the logging system.

-   `class IRCServer`
    -   **`IRCServer(int port, std::shared_ptr<LogBuffer> logBuffer)`**: The constructor takes a `std::shared_ptr` to the `LogBuffer`, allowing it to safely share access to the log data with the `LogServer`.
    -   **`void start()`**: Starts the IRC server's main accept loop in a new, detached `std::thread`.
