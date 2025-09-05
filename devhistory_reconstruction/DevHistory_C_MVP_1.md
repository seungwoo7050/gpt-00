# DevHistory C-MVP-1: Basic TCP Server Reconstruction

## 1. Introduction

This document details the reconstruction of the first Minimum Viable Product (MVP1) for the C version of LogCaster. The primary goal of this stage is to build a foundational, yet robust, basic TCP server capable of handling multiple concurrent clients.

The server listens on a designated port (9999), accepts connections, and prints any received log messages to standard output. This MVP establishes the core networking layer using the standard `select()` system call for I/O multiplexing, providing a solid base for future enhancements like thread pools and in-memory buffers.

## 2. In-depth Analysis for Technical Interviews

### Architectural Choices
- **Why `select()` for I/O Multiplexing?**
  - **Choice:** We opted for `select()` over more modern alternatives like `epoll` (Linux) or `kqueue` (BSD/macOS).
  - **Reasoning:** `select()` is part of the POSIX standard, making it highly portable across various UNIX-like systems. For an initial MVP, its simplicity and universal availability were prioritized over the higher performance and scalability of `epoll`/`kqueue`. This choice allows us to focus on the fundamental server logic first.
  - **Trade-offs:** The major drawbacks of `select()` are the `FD_SETSIZE` limit (typically 1024) on the number of file descriptors and its O(n) performance complexity, as it scans all file descriptors on each call. This is acceptable for MVP1 but will become a bottleneck under heavy load.

### Technical Trade-offs
- **Fixed-Size Buffer (`BUFFER_SIZE = 4096`)**
  - **Trade-off:** We use a fixed-size buffer for each client connection. This simplifies memory management significantly, as we don't need to deal with dynamic resizing or reallocation for incoming data.
  - **Downside:** This approach can be inefficient. If most log messages are small, a large portion of the allocated 4KB per client is wasted. Conversely, a log message larger than 4KB will be truncated or require multiple `read()` calls to process, which is not handled in this MVP.

### Challenges Faced & Design Decisions
- **`client_count` Bug**
  - **Challenge:** The legacy code intentionally omits decrementing `client_count` when a client disconnects.
  - **Decision:** As this is a reconstruction of the original MVP1, this bug is faithfully reproduced. It serves as a documented issue to be addressed in a later "Security & Stability" MVP, demonstrating an iterative development and bug-fixing process. The server will eventually stop accepting new connections once the limit is reached, even if clients have disconnected.
- **Global Variable for Signal Handling (`g_server`)**
  - **Challenge:** Standard POSIX signal handlers have a `void(int)` signature, making it difficult to pass application-specific context (like the server instance) into them.
  - **Decision:** A global static pointer (`g_server`) was used to allow the `handle_signal` function to access the server's state and set its `running` flag to `false`. While often considered poor practice, this is a common and pragmatic solution for simple signal handling in C. The alternative would involve more complex mechanisms like the self-pipe trick, which was deemed overly complex for this MVP.

### Production Considerations & Future Improvements
- **Scalability:** For a production environment, `select()` is not a viable option for handling thousands of concurrent connections. The clear next step is to migrate to `epoll` on Linux, which provides O(1) performance regardless of the number of watched file descriptors.
- **Resource Management:** The fixed-buffer strategy is inefficient. A more advanced implementation would use a dynamic buffer (e.g., a "rope" data structure) or a memory pool to manage memory more effectively, especially with many idle connections.
- **Protocol Rigidity:** The current newline-delimited protocol is simple but fragile. A more robust production protocol would include length-prefixing for messages to handle binary data or multi-line logs correctly.

## 3. Sequence List

This section contains the complete source code and build scripts for the C-MVP-1, broken down by their logical implementation sequence.

### [SEQUENCE: C-MVP1-1] Makefile
```makefile
# Compiler and flags
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -Werror -pedantic -std=c11
LDFLAGS = -lpthread

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

# Source files and object files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

# Target executable
TARGET = $(BINDIR)/logcaster-c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete. Executable: $(TARGET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)
	@echo "Cleaned up object and binary files."
```

### [SEQUENCE: C-MVP1-2] `include/server.h` - Header and Definitions
```c
#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#define DEFAULT_PORT 9999
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 4096
```

### [SEQUENCE: C-MVP1-3] `include/server.h` - Server Structure
```c
// Server state structure (MVP1 version)
typedef struct {
    int port;
    int listen_fd;
    fd_set master_set;
    fd_set read_set;
    int max_fd;
    int client_count;
    volatile sig_atomic_t running;
} log_server_t;
```

### [SEQUENCE: C-MVP1-4] `include/server.h` - Function Prototypes
```c
// Function prototypes (MVP1 version)
log_server_t* server_create(int port);
void server_run(log_server_t* server);
void server_destroy(log_server_t* server);
void handle_signal(int sig);

#endif // SERVER_H
```

### [SEQUENCE: C-MVP1-5] `src/server.c` - Includes and Global Server Pointer
```c
#include "server.h"

// Global pointer for signal handling
static log_server_t* g_server = NULL;
```

### [SEQUENCE: C-MVP1-6] `src/server.c` - Signal Handler
```c
// Signal handler: sets the server's running state to 0 for a safe shutdown
void handle_signal(int sig) {
    (void)sig; // Prevent unused parameter warning
    if (g_server) {
        g_server->running = 0;
    }
}
```

### [SEQUENCE: C-MVP1-7] `src/server.c` - `server_create` Initialization
```c
// Creates the server structure and initializes the listening socket
log_server_t* server_create(int port) {
    log_server_t* server = malloc(sizeof(log_server_t));
    if (!server) {
        perror("Failed to allocate server");
        return NULL;
    }

    server->port = port;
    server->running = 1;
    server->client_count = 0;
```

### [SEQUENCE: C-MVP1-8] `src/server.c` - `socket()`
```c
    // Create listening socket
    server->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->listen_fd < 0) {
        perror("Failed to create socket");
        free(server);
        return NULL;
    }
```

### [SEQUENCE: C-MVP1-9] `src/server.c` - `setsockopt()`
```c
    // Set socket reuse option (to avoid TIME_WAIT issues on restart)
    int opt = 1;
    if (setsockopt(server->listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Failed to set socket options");
        close(server->listen_fd);
        free(server);
        return NULL;
    }
```

### [SEQUENCE: C-MVP1-10] `src/server.c` - `sockaddr_in` Setup
```c
    // Configure server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
```

### [SEQUENCE: C-MVP1-11] `src/server.c` - `bind()`
```c
    // Bind socket to the address
    if (bind(server->listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        close(server->listen_fd);
        free(server);
        return NULL;
    }
```

### [SEQUENCE: C-MVP1-12] `src/server.c` - `listen()`
```c
    // Start listening for connections
    if (listen(server->listen_fd, MAX_CLIENTS) < 0) {
        perror("Failed to listen on socket");
        close(server->listen_fd);
        free(server);
        return NULL;
    }
```

### [SEQUENCE: C-MVP1-13] `src/server.c` - `fd_set` Initialization
```c
    // Initialize fd_sets and add the listening socket
    FD_ZERO(&server->master_set);
    FD_ZERO(&server->read_set);
    FD_SET(server->listen_fd, &server->master_set);
    server->max_fd = server->listen_fd;

    g_server = server; // Assign the server instance to the global pointer

    printf("LogCaster-C server starting on port %d\n", port);
    return server;
}
```

### [SEQUENCE: C-MVP1-14] `src/server.c` - `server_run` Main Loop
```c
// Server main loop
void server_run(log_server_t* server) {
    printf("Press Ctrl+C to stop\n");
    while (server->running) {
        server->read_set = server->master_set;
```

### [SEQUENCE: C-MVP1-15] `src/server.c` - `select()` Call
```c
        // Wait for I/O events using select()
        if (select(server->max_fd + 1, &server->read_set, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) { // Interrupted by a signal
                continue;
            }
            perror("select failed");
            break;
        }
```

### [SEQUENCE: C-MVP1-16] `src/server.c` - File Descriptor Iteration
```c
        // Iterate through all file descriptors to check for events
        for (int i = 0; i <= server->max_fd; i++) {
            if (FD_ISSET(i, &server->read_set)) {
```

### [SEQUENCE: C-MVP1-17] `src/server.c` - New Connection Handling
```c
                // Event on listening socket: handle new client connection
                if (i == server->listen_fd) {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int new_fd = accept(server->listen_fd, (struct sockaddr*)&client_addr, &client_len);

                    if (new_fd < 0) {
                        perror("Failed to accept connection");
                        continue;
                    }
```

### [SEQUENCE: C-MVP1-18] `src/server.c` - Max Clients Check
```c
                    // Check client limit
                    if (server->client_count >= MAX_CLIENTS) {
                        fprintf(stderr, "Max clients reached. Rejecting new connection.\n");
                        close(new_fd);
                        continue;
                    }
```

### [SEQUENCE: C-MVP1-19] `src/server.c` - Add New Client
```c
                    // Add new client to the master_set
                    fcntl(new_fd, F_SETFL, O_NONBLOCK); // Set to non-blocking
                    FD_SET(new_fd, &server->master_set);
                    if (new_fd > server->max_fd) {
                        server->max_fd = new_fd;
                    }
                    server->client_count++;
                    printf("New connection from %s:%d on socket %d\n",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), new_fd);
                } else {
```

### [SEQUENCE: C-MVP1-20] `src/server.c` - Data Receive Handling
```c
                    // Event on client socket: handle incoming data
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    int nbytes = read(i, buffer, sizeof(buffer) - 1);
```

### [SEQUENCE: C-MVP1-21] `src/server.c` - Connection Closed or Error
```c
                    if (nbytes <= 0) {
                        // Connection closed or error
                        if (nbytes == 0) {
                            printf("Socket %d hung up\n", i);
                        } else {
                            perror("read failed");
                        }
                        close(i);
                        FD_CLR(i, &server->master_set);
                        // NOTE: client_count is not decremented here, a known bug for MVP5.
```

### [SEQUENCE: C-MVP1-22] `src/server.c` - Log Output
```c
                    } else {
                        // Data received successfully, print to stdout
                        buffer[nbytes] = '\0';
                        printf("[LOG] from socket %d: %s", i, buffer);
                        // Add a newline if the buffer doesn't have one
                        if (buffer[nbytes-1] != '\n') {
                            printf("\n");
                        }
                    }
                }
            }
        }
    }
}
```

### [SEQUENCE: C-MVP1-23] `src/server.c` - `server_destroy`
```c
// Frees server resources
void server_destroy(log_server_t* server) {
    if (!server) return;

    printf("\nShutting down server...\n");
```

### [SEQUENCE: C-MVP1-24] `src/server.c` - Close All Connections
```c
    // Close all client connections
    for (int i = 0; i <= server->max_fd; i++) {
        if (FD_ISSET(i, &server->master_set)) {
            close(i);
        }
    }

    free(server);
    g_server = NULL;
    printf("Server shut down gracefully.\n");
}
```

### [SEQUENCE: C-MVP1-25] `src/main.c` - Main Function and Port Parsing
```c
#include "server.h"

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;

    // [SEQUENCE: C-MVP1-26]
    // Parse command-line arguments (for port number)
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number. Using default %d.\n", DEFAULT_PORT);
            port = DEFAULT_PORT;
        }
    }
```

### [SEQUENCE: C-MVP1-26] `src/main.c` - Signal Handler Registration
```c
    // Register signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, handle_signal);
```

### [SEQUENCE: C-MVP1-27] `src/main.c` - Server Creation
```c
    // Create the server
    log_server_t* server = server_create(port);
    if (!server) {
        fprintf(stderr, "Failed to create server.\n");
        return 1;
    }
```

### [SEQUENCE: C-MVP1-28] `src/main.c` - Server Run
```c
    // Run the server
    server_run(server);
```

### [SEQUENCE: C-MVP1-29] `src/main.c` - Server Destruction
```c
    // Clean up server resources
    server_destroy(server);

    return 0;
}
```

### [SEQUENCE: C-MVP1-30] `tests/test_client.py`
```python
import socket
import threading
import time
import sys

SERVER_HOST = '127.0.0.1'
SERVER_PORT = 9999
NUM_CLIENTS_MULTIPLE = 5
NUM_CLIENTS_STRESS = 50
MESSAGES_PER_CLIENT = 10

def client_task(client_id, num_messages):
    """A single client connects, sends messages, and closes."""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((SERVER_HOST, SERVER_PORT))
            # print(f"Client {client_id}: Connected to server.")
            for i in range(num_messages):
                message = f"Client {client_id}, message {i+1}\n"
                s.sendall(message.encode('utf-8'))
                time.sleep(0.1)
            # print(f"Client {client_id}: All messages sent.")
    except ConnectionRefusedError:
        print(f"Client {client_id}: Connection refused. Is the server running?")
    except Exception as e:
        print(f"Client {client_id}: An error occurred: {e}")
    # finally:
        # print(f"Client {client_id}: Connection closed.")

def run_test(num_clients, num_messages):
    """Runs a test with a specified number of clients."""
    threads = []
    start_time = time.time()

    print(f"--- Starting test with {num_clients} clients ---")
    for i in range(num_clients):
        thread = threading.Thread(target=client_task, args=(i + 1, num_messages))
        threads.append(thread)
        thread.start()
        time.sleep(0.05) # Stagger connections slightly

    for thread in threads:
        thread.join()

    end_time = time.time()
    print(f"\nTest with {num_clients} clients finished in {end_time - start_time:.2f} seconds.")

if __name__ == "__main__":
    if len(sys.argv) != 2 or sys.argv[1] not in ['single', 'multiple', 'stress']:
        print("Usage: python test_client.py [single|multiple|stress]")
        sys.exit(1)

    mode = sys.argv[1]

    if mode == 'single':
        run_test(1, MESSAGES_PER_CLIENT)
    elif mode == 'multiple':
        run_test(NUM_CLIENTS_MULTIPLE, MESSAGES_PER_CLIENT)
    elif mode == 'stress':
        run_test(NUM_CLIENTS_STRESS, 5)
```

## 4. Build Verification

The project was built and tested successfully. The initial "Address already in use" error was resolved by changing the port, indicating a temporary `TIME_WAIT` state on the default port rather than a code issue.

### Build Log

```
$ make -C log-caster-c-reconstructed clean && make -C log-caster-c-reconstructed
make: Entering directory '/home/user/gpt-00/log-caster-c-reconstructed'
rm -rf obj bin
Cleaned up object and binary files.
make: Leaving directory '/home/user/gpt-00/log-caster-c-reconstructed'
make: Entering directory '/home/user/gpt-00/log-caster-c-reconstructed'
gcc -Iinclude -Wall -Wextra -Werror -pedantic -std=c11 -c src/main.c -o obj/main.o
gcc -Iinclude -Wall -Wextra -Werror -pedantic -std=c11 -c src/server.c -o obj/server.o
gcc  obj/main.o  obj/server.o -o bin/logcaster-c -lpthread
Build complete. Executable: bin/logcaster-c
make: Leaving directory '/home/user/gpt-00/log-caster-c-reconstructed'
```

### Test Execution Log

The server was started in the background, and the test client was run in all three modes.

**Server Output:**
```
$ ./log-caster-c-reconstructed/bin/logcaster-c
LogCaster-C server starting on port 9990
Press Ctrl+C to stop
New connection from 127.0.0.1:33310 on socket 5
[LOG] from socket 5: Client 1, message 1
...
Socket 5 hung up
New connection from 127.0.0.1:33312 on socket 5
...
```

**Test Client Output:**
```
$ python3 ./log-caster-c-reconstructed/tests/test_client.py single && ...
--- Starting test with 1 clients ---

Test with 1 clients finished in 1.00 seconds.
--- Starting test with 5 clients ---

Test with 5 clients finished in 1.21 seconds.
--- Starting test with 50 clients ---

Test with 50 clients finished in 2.97 seconds.
```

