# DevHistory C-MVP-4: Persistence Layer

## 1. Introduction

This document details the reconstruction of the fourth Minimum Viable Product (MVP4) for the C version of LogCaster. This phase introduces a critical feature for data durability: a persistence layer. This transforms the server from a transient, in-memory tool into a robust service capable of preserving logs across restarts.

The core of this MVP is the new `PersistenceManager` module, which handles all disk I/O operations asynchronously to minimize performance impact on the main logging path.

**Key Features & Improvements:**
- **Asynchronous File Writing:** A dedicated "Writer Thread" is implemented to handle all file I/O. The main application threads simply enqueue log messages to a write queue and return immediately, ensuring that log ingestion performance is not blocked by slow disk operations.
- **Log File Rotation:** To prevent log files from growing indefinitely, a size-based rotation mechanism is implemented. When the `current.log` file reaches a configurable size limit, it is renamed with a timestamp, and a new `current.log` is created.
- **Configurable Persistence:** The persistence feature can be enabled and configured via command-line arguments, including the target directory (`-d`) and max file size (`-s`).

## 2. In-depth Analysis for Technical Interviews

### Architectural Choices
- **Asynchronous Writer Thread**
  - **Choice:** A dedicated thread is responsible for all file writing, fed by a producer-consumer queue.
  - **Reasoning:** Disk I/O is orders of magnitude slower than memory operations and is a common source of performance bottlenecks. By making writes asynchronous, the high-performance worker threads that handle client connections can simply hand off the log message to the persistence queue and immediately become available to process the next client. This maintains high throughput for log ingestion, as the application does not have to wait for the `fprintf` and `fflush` calls to complete.
  - **Trade-offs:** The primary trade-off is a small window of potential data loss. If the server crashes after a log has been added to the write queue but before the writer thread has flushed it to disk, that log will be lost. For many logging scenarios, this tiny risk is an acceptable trade-off for the massive performance gain.

### Data Structures & Synchronization
- **Write Queue (Singly Linked List)**
  - **Choice:** A simple, dynamically allocated linked list is used for the write queue.
  - **Reasoning:** Similar to the thread pool's job queue, a linked list is simple to implement and allows for a theoretically unbounded queue size. The logic involves taking the entire queue in one locked operation (`head = manager->write_queue; manager->write_queue = NULL;`) to minimize the time spent holding the mutex, which is a good practice for producer-consumer queues.

### Challenges Faced & Design Decisions
- **`Makefile` and Build Process Instability**
  - **Challenge:** The project was plagued by a series of build failures. The `Makefile` did not reliably create the `obj` and `bin` directories before trying to write to them. Subsequent attempts to fix this with `order-only-prerequisites` also failed.
  - **Resolution:** The `Makefile` was ultimately stabilized by making each rule explicitly responsible for ensuring its output directory exists (e.g., using `@mkdir -p $(@D)`). This made the build process more robust and less dependent on `make`'s implicit ordering.
- **Test Script Pathing Issues**
  - **Challenge:** The Python test scripts, when run from the project root, could not find the server executable in the `bin` directory using relative paths like `./bin/logcaster-c`.
  - **Resolution:** The most robust solution was to make the test script path-independent. The final version of `test_persistence.py` programmatically determines its own location (`os.path.dirname(os.path.abspath(__file__))`) and constructs a reliable absolute path to the server executable. This ensures the tests can be run from any directory.

## 3. Sequence List

This section contains the complete source code for the C-MVP-4. Existing sequences are preserved, and new MVP4 sequences are added.

*Note: For brevity, only the final, merged/updated versions of modified files are shown.*

### New Files in MVP4
- `include/persistence.h` (Sequences [C-MVP4-3] to [C-MVP4-7])
- `src/persistence.c` (Sequences [C-MVP4-8] to [C-MVP4-15])
- `tests/test_persistence.py` (Sequence [C-MVP4-26])

### `include/server.h` (Updated)
*The `log_server_t` struct was updated to include a pointer to the `persistence_manager_t` (Sequence [C-MVP4-17]).*

### `src/server.c` (Updated)
*The `handle_client_job` function was updated to call `persistence_write` (Sequence [C-MVP4-20]). The `server_create` and `server_destroy` functions were updated to manage the persistence manager lifecycle (Sequences [C-MVP4-21], [C-MVP4-22]).*

### `src/main.c` (Updated)
*The `main` function was significantly updated to use `getopt` to parse command-line arguments for configuring the persistence layer (Sequences [C-MVP4-23] to [C-MVP4-25]).*

## 4. Build Verification

The project was successfully compiled and tested after resolving the `Makefile` and test script pathing issues.

### Final Build Log
```
$ make -C log-caster-c-reconstructed clean && make -C log-caster-c-reconstructed
make: Entering directory '/home/user/gpt-00/log-caster-c-reconstructed'
rm -rf obj bin
Clean complete
make: Leaving directory '/home/user/gpt-00/log-caster-c-reconstructed'
make: Entering directory '/home/user/gpt-00/log-caster-c-reconstructed'
Compiling src/log_buffer.c...
...
Compiling src/persistence.c...
...
Linking...
Build complete: bin/logcaster-c
make: Leaving directory '/home/user/gpt-00/log-caster-c-reconstructed'
```

### Final Test Execution Log
```
$ python3 ./log-caster-c-reconstructed/tests/test_persistence.py
--- Test 1: Persistence disabled ---
OK

--- Test 2: Persistence enabled ---
OK

All MVP4 tests passed!
```
