# DevHistory CPP-MVP-3: Modern C++ Query Engine

## 1. Introduction

This document details the reconstruction of the third Minimum Viable Product (MVP3) for the C++ version of LogCaster. This phase implements an advanced query engine, bringing the C++ server to functional parity with its C counterpart while fully embracing the power and safety of modern C++.

The core of this MVP is the introduction of a sophisticated query parser and the enhancement of the log buffer to support complex, multi-faceted searches. This is achieved using C++ standard library features to ensure type safety, robust error handling, and expressive code.

**Key Features & Improvements:**
- **`QueryParser` Class:** A new static class responsible for parsing `key=value` query strings into a structured `ParsedQuery` object.
- **`std::regex`:** The POSIX C library is replaced by C++'s `<regex>` for powerful and type-safe regular expression matching.
- **`std::chrono`:** C-style `time_t` is replaced by `<chrono>` for type-safe time-based filtering.
- **`std::optional`:** Used within `ParsedQuery` to safely represent optional parameters like `regex` or time filters, eliminating the need for null pointer checks.
- **Object-Oriented Matching:** The `ParsedQuery` class encapsulates the matching logic itself with a `matches()` method, bundling data and behavior together.

## 2. In-depth Analysis for Technical Interviews

### Architectural Choices
- **Static `QueryParser` Class**
  - **Choice:** The `QueryParser` is designed with a single public `static` method, `parse()`.
  - **Reasoning:** Since parsing is a pure, stateless operation (it takes a string and returns a structured object without modifying any internal state), there is no need to instantiate a `QueryParser` object. A static class serves as a namespace for the parsing functionality, making the design clean and the usage clear: `QueryParser::parse(...)`.
- **`ParsedQuery` with `matches()` Method**
  - **Choice:** The `ParsedQuery` class is not just a data struct; it contains the `matches()` method that performs the actual filtering logic.
  - **Reasoning:** This is a classic example of object-oriented encapsulation. The object that holds the query data is also responsible for knowing how to apply that data as a filter. This improves cohesion and separates concerns, as the `LogBuffer` no longer needs to know the internal details of how to filter by regex or time; it simply delegates that responsibility to the `ParsedQuery` object.

### Technical Trade-offs
- **`std::regex` vs. Other Libraries**
  - **Trade-off:** `std::regex` is part of the C++ standard and requires no external dependencies. However, it has a reputation for being slower than specialized libraries like Google's RE2 or PCRE.
  - **Reasoning:** For this project, the convenience, portability, and type safety of the standard library were prioritized over raw performance. `std::regex` also provides built-in exception handling (`std::regex_error`), which makes the parser more robust against invalid user input.
- **`std::unique_ptr` for `ParsedQuery`**
  - **Choice:** The `QueryParser::parse` method returns a `std::unique_ptr<ParsedQuery>`.
  - **Reasoning:** This clearly communicates ownership semantics. The caller of `parse()` is now the sole owner of the newly created `ParsedQuery` object and is responsible for its lifetime. This prevents memory leaks, as the `unique_ptr` will automatically deallocate the object when it goes out of scope. It's a modern C++ alternative to returning a raw pointer that the caller must remember to `delete`.

### Challenges Faced & Design Decisions
- **Intensive Debugging Cycle**
  - **Challenge:** Similar to the C version, the C++ MVP3 reconstruction faced a persistent and difficult-to-diagnose issue where the server would fail to start, resulting in "Connection refused" errors during testing.
  - **Resolution:** The debugging process involved multiple steps: checking for port conflicts, adding debug logs to the event loop, and refactoring the signal handling mechanism. The root cause was ultimately traced to instability in the `main` function's object lifecycle and signal handling. The final, stable solution reverted to the C++ MVP1 model of using a global `std::unique_ptr` for the server object and a C-style signal handler that calls `.reset()` to ensure a clean, predictable shutdown via RAII.
- **Header-Only `ThreadPool`**
  - **Challenge:** The `ThreadPool::enqueue` method is a template, which means its implementation must be available in every translation unit that uses it.
  - **Decision:** The implementation of `enqueue` was placed in the `ThreadPool.h` header file. While this increases compilation times slightly, it is the standard and necessary approach for template member functions in C++.

## 3. Sequence List

This section contains the complete source code for the C++-MVP-3. Existing MVP1/MVP2 sequences are preserved, and new MVP3 sequences are added.

*Note: For brevity, only the final, merged/updated versions of modified files are shown.*

### New Files in MVP3
- `include/QueryParser.h` (Sequences [CPP-MVP3-3] to [CPP-MVP3-6])
- `src/QueryParser.cpp` (Sequences [CPP-MVP3-7] to [CPP-MVP3-10])

### `CMakeLists.txt` (Updated)
*The `SOURCES` variable was updated to include `src/QueryParser.cpp` (Sequence [CPP-MVP3-2]).*

### `include/LogBuffer.h` (Updated)
*A forward declaration for `ParsedQuery` and the `searchEnhanced` method declaration were added (Sequence [CPP-MVP3-11], [CPP-MVP3-12]).*

### `src/LogBuffer.cpp` (Updated)
*The `searchEnhanced` method was implemented, delegating matching logic to the `ParsedQuery` object (Sequence [CPP-MVP3-14], [CPP-MVP3-15]).*

### `src/QueryHandler.cpp` (Updated)
*The `processQuery` and `handleSearch` methods were updated to use the new `QueryParser`, and the `HELP` command was enhanced (Sequences [CPP-MVP3-16] to [CPP-MVP3-21]).*

## 4. Build Verification

The project was successfully compiled and tested after an extensive debugging process involving port conflicts and signal handling logic.

### Final Build Log
```
$ cd build && cmake .. && make
-- Configuring done
-- Generating done
-- Build files have been written to: /home/user/gpt-00/log-caster-cpp-reconstructed/build
[ 12%] Building CXX object CMakeFiles/logcaster-cpp.dir/src/main.cpp.o
[ 25%] Building CXX object CMakeFiles/logcaster-cpp.dir/src/LogServer.cpp.o
[ 37%] Building CXX object CMakeFiles/logcaster-cpp.dir/src/LogBuffer.cpp.o
[ 50%] Building CXX object CMakeFiles/logcaster-cpp.dir/src/QueryHandler.cpp.o
[ 62%] Building CXX object CMakeFiles/logcaster-cpp.dir/src/QueryParser.cpp.o
[ 75%] Linking CXX executable logcaster-cpp
[100%] Built target logcaster-cpp
```

### Final Test Execution Log
```
$ pkill -f logcaster-cpp; sleep 1; ./build/logcaster-cpp &
$ sleep 2 && python3 tests/test_mvp3.py
--- Test 1: Simple keyword search ---
> QUERY keywords=timeout
FOUND: 2 matches
...
OK

--- Test 2: Regex search ---
> QUERY regex=user_id=[0-9]+
FOUND: 1 matches
...
OK

...

All MVP3 tests passed!
```
