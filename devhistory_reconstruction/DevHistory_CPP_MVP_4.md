# DevHistory CPP-MVP-4: C++ Persistence Layer

## 1. Introduction

This document details the reconstruction of the fourth Minimum Viable Product (MVP4) for the C++ version of LogCaster. This phase introduces a persistence layer, elevating the server from an in-memory tool to a durable log storage service. This is achieved by leveraging modern C++ features for safe and efficient file I/O, file system management, and concurrency.

**Key Features & Improvements:**
- **Asynchronous Persistence:** A dedicated writer thread handles all disk I/O, ensuring that log ingestion performance is not impacted. This is implemented using standard C++ libraries like `<thread>`, `<mutex>`, and `<condition_variable>`.
- **`<filesystem>` for Portability:** All file system operations (creating directories, renaming files for rotation) are handled using the C++17 `<filesystem>` library, resulting in clean, portable, and platform-independent code.
- **RAII-based File I/O:** C-style `FILE*` pointers are replaced with `std::ofstream`, which manages the file handle automatically. The file is closed safely in the destructor, preventing resource leaks.
- **Configurable via Command Line:** The persistence feature is fully configurable via command-line arguments, allowing users to enable it, set the log directory, and define the maximum file size for rotation.

## 2. In-depth Analysis for Technical Interviews

### Architectural Choices
- **Dependency Injection for Persistence**
  - **Choice:** The `PersistenceManager` is created in `main` and then "injected" into the `LogServer` object via a `setPersistenceManager` method.
  - **Reasoning:** This decouples the `LogServer` from the concrete implementation of the persistence layer. The `LogServer` doesn't need to know *how* the persistence is configured or created; it only needs a valid `PersistenceManager` to call `write()`. This makes the `LogServer` easier to test in isolation (by providing a mock persistence manager) and more flexible for future changes.
- **Asynchronous Queue with `std::condition_variable`**
  - **Choice:** The writer thread uses a `std::condition_variable` to wait for new logs to appear in the queue.
  - **Reasoning:** This is a highly efficient mechanism for producer-consumer scenarios. The writer thread sleeps and consumes zero CPU while waiting. It is only woken up by the main threads when they `notify_one()` after pushing a new log. The use of a `wait_for` with a timeout also allows the queue to be flushed periodically even if no new logs are arriving, ensuring data is written to disk in a timely manner.

### Technical Trade-offs
- **`<filesystem>` vs. POSIX Calls**
  - **Trade-off:** The C++ `<filesystem>` library can result in a slightly larger binary size and may require linking an extra library (`stdc++fs`) on older GCC versions compared to direct POSIX calls (`mkdir`, `rename`).
  - **Reasoning:** The benefits in terms of code clarity, type safety, and cross-platform portability are immense. Path manipulation is handled by a dedicated `std::filesystem::path` object, which prevents common string-based path errors. The library also provides proper error handling through exceptions (`std::filesystem::filesystem_error`).

### Challenges Faced & Design Decisions
- **Linker Errors with `PersistenceManager`**
  - **Challenge:** An early build failed with linker errors, indicating that the `PersistenceManager` constructor, destructor, and `write` method were not found.
  - **Resolution:** The issue was a simple oversight: `src/Persistence.cpp` had not been added to the `SOURCES` list in `CMakeLists.txt`. This highlights the importance of ensuring all new source files are correctly registered with the build system.
- **Test Script Pathing**
  - **Challenge:** The Python test script, which uses `subprocess.Popen`, initially failed because it could not find the C++ executable (`logcaster-cpp`) when run from the project root.
  - **Resolution:** The script was made robust by programmatically determining its own absolute path and using it to construct an absolute path to the build executable. This ensures the test can be run reliably from any location.

## 3. Sequence List

This section contains the complete source code for the C++-MVP-4. Existing sequences are preserved, and new MVP4 sequences are added.

*Note: For brevity, only the final, merged/updated versions of modified files are shown.*

### New Files in MVP4
- `include/Persistence.h` (Sequences [CPP-MVP4-4] to [CPP-MVP4-6])
- `src/Persistence.cpp` (Sequences [CPP-MVP4-7] to [CPP-MVP4-12])

### `CMakeLists.txt` (Updated)
*The `SOURCES` variable was updated to include `src/Persistence.cpp`, and a conditional link for `stdc++fs` was added (Sequences [CPP-MVP4-2], [CPP-MVP4-3]).*

### `include/LogServer.h` (Updated)
*A pointer to the `PersistenceManager` and a setter method were added (Sequences [CPP-MVP4-13] to [CPP-MVP4-15]).*

### `src/LogServer.cpp` (Updated)
*The `handleClientTask` method was updated to call `persistence_->write()` (Sequence [CPP-MVP4-18]).*

### `src/main.cpp` (Updated)
*The `main` function was updated to use `getopt` to parse persistence-related command-line arguments and inject the `PersistenceManager` into the `LogServer` (Sequences [CPP-MVP4-20] to [CPP-MVP4-22]).*

## 4. Build Verification

The project was successfully compiled and tested. The persistence feature was verified by checking for the creation and content of log files.

### Final Build Log
```
$ cd build && cmake .. && make
-- Configuring done
-- Generating done
-- Build files have been written to: /home/user/gpt-00/log-caster-cpp-reconstructed/build
[ 11%] Building CXX object CMakeFiles/logcaster-cpp.dir/src/Persistence.cpp.o
[ 22%] Linking CXX executable logcaster-cpp
[100%] Built target logcaster-cpp
```

### Final Test Execution Log
```
$ python3 ./log-caster-cpp-reconstructed/tests/test_persistence.py
--- Test 1: Persistence disabled ---
OK

--- Test 2: Persistence enabled ---
OK

All MVP4 tests passed!
```
