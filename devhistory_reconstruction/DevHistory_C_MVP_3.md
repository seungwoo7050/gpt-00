# DevHistory C-MVP-3: Enhanced Query Capabilities

## 1. Introduction

This document details the reconstruction of the third Minimum Viable Product (MVP3) for the C version of LogCaster. This phase significantly enhances the server's core value proposition: real-time log querying. It transforms the server from a simple log store into a powerful data analysis tool by introducing a sophisticated query system.

The primary goals of this stage are:
1.  **Advanced Query Parser:** A new `QueryParser` module is introduced to systematically parse complex query strings.
2.  **Multi-faceted Filtering:** The server now supports filtering logs based on multiple criteria simultaneously:
    -   **Regex Search:** Via a `regex=` parameter for POSIX regular expression matching.
    -   **Time Range:** Via `time_from=` and `time_to=` parameters using Unix timestamps.
    -   **Multiple Keywords:** Via a `keywords=` parameter with `AND`/`OR` logic specified by an `operator=` parameter.
3.  **HELP Command:** A new `HELP` command is added to the query interface to explain the new, richer query syntax to users.

This MVP adheres to the Separation of Concerns principle by isolating parsing logic into its own module, enhancing the modularity and maintainability of the codebase.

## 2. In-depth Analysis for Technical Interviews

### Architectural Choices
- **Dedicated Query Parser Module**
  - **Choice:** Query parsing logic was extracted from the `QueryHandler` and placed into its own module (`query_parser.c` and `query_parser.h`).
  - **Reasoning:** As the query language grew in complexity, embedding the parsing logic directly within the query handler would have created a monolithic, hard-to-maintain function. By creating a dedicated parser, we isolate the complex string manipulation and state management involved in parsing. The `QueryHandler` is now only responsible for receiving a query string, passing it to the parser, and dispatching the structured result to the `LogBuffer`.

### Data Structures & Algorithms
- **`parsed_query_t` Struct**
  - **Choice:** A struct, `parsed_query_t`, was created to hold the structured output of the parser.
  - **Reasoning:** This provides a clean, well-defined interface between the parser and its consumers. Instead of passing around numerous individual variables (keywords, regex strings, times), all query parameters are neatly encapsulated in a single object. This makes function signatures cleaner and the code easier to reason about.
- **`strtok_r` for Tokenization**
  - **Choice:** The re-entrant version of `strtok`, `strtok_r`, was used to parse the query string.
  - **Reasoning:** `strtok` is not thread-safe because it uses a static internal buffer. Since our server is multi-threaded, using `strtok` could lead to race conditions. `strtok_r` is the correct, thread-safe choice as it requires the caller to provide a `saveptr` variable to maintain state, making it safe to use across multiple threads.

### Challenges Faced & Design Decisions
- **`strtok_r` Memory Corruption**
  - **Challenge:** The most significant challenge in this MVP was a persistent server crash that was difficult to debug. The server would fail to start, but the root cause was not immediately obvious. After extensive debugging (including adding print statements and isolating the issue), the problem was traced to memory handling in the `query_parser_parse` function.
  - **Initial Flawed Approach:** The initial implementation passed a pointer to the middle of a `strdup`-ed copy of the query string to `strtok_r`. `strtok_r` modifies the string it parses in-place. This combination of pointer arithmetic and in-place modification of a dynamically allocated string led to memory corruption and unpredictable crashes.
  - **Resolution:** The problem was solved by adopting a more defensive programming strategy. Instead of complex pointer manipulation, the final version of the parser now makes a separate, clean `strdup`-ed copy of each key and value *after* they have been tokenized. This ensures that each component of the query is working with its own independent, safely allocated memory, completely eliminating the corruption issue.
- **Compiler Warnings as Errors (`-Werror`)**
  - **Challenge:** The strict compiler flag `-Werror` caused the build to fail on what would normally be warnings. For instance, an incompatible pointer type between `const struct parsed_query*` and `const parsed_query_t*` (caused by an incorrect forward declaration) halted the build.
  - **Reasoning:** While challenging during development, using `-Werror` is a best practice. It forces developers to address potential issues early, leading to cleaner, more correct, and more portable code. It prevented us from ignoring a subtle type issue that could have caused problems later.

## 3. Sequence List

This section contains the complete source code for the C-MVP-3. Existing MVP1/MVP2 sequences are preserved, and new MVP3 sequences are added.

*Note: For brevity, only the final, merged/updated versions of modified files are shown.*

### New Files in MVP3
- `include/query_parser.h` (Sequences [C-MVP3-3] to [C-MVP3-7])
- `src/query_parser.c` (Sequences [C-MVP3-8] to [C-MVP3-14])
- `tests/test_mvp3.py` (Sequence [C-MVP3-23])

### `include/log_buffer.h` (Updated)
```c
// [SEQUENCE: C-MVP2-18]
#ifndef LOG_BUFFER_H
#define LOG_BUFFER_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

// [SEQUENCE: C-MVP3-15]
#include "query_parser.h"

#define DEFAULT_BUFFER_SIZE 10000

// [SEQUENCE: C-MVP2-19]
// ... (log_entry_t struct)

// [SEQUENCE: C-MVP2-20]
// ... (log_buffer_t struct)

// [SEQUENCE: C-MVP2-21]
// ... (lifecycle functions)

// [SEQUENCE: C-MVP2-22]
// ... (push function)

// [SEQUENCE: C-MVP3-16]
// Enhanced search function added in MVP3
int log_buffer_search_enhanced(log_buffer_t* buffer, const parsed_query_t* query, char*** results, int* count);

// [SEQUENCE: C-MVP2-23]
// ... (status functions)

#endif // LOG_BUFFER_H
```

### `src/log_buffer.c` (Updated)
*This file was updated to include the `log_buffer_search_enhanced` function (Sequence [C-MVP3-18]), which uses `query_matches_log` to filter results.*

### `src/query_handler.c` (Updated)
*This file was updated to use the new query parser (Sequence [C-MVP3-21]) and to handle the new `HELP` command (Sequence [C-MVP3-20]).*

## 4. Build Verification

The project was successfully compiled and tested after a lengthy debugging session to resolve memory corruption issues in the query parser and several build issues with the `Makefile`.

### Final Build Log
```
$ make -C log-caster-c-reconstructed clean && make -C log-caster-c-reconstructed
make: Entering directory '/home/user/gpt-00/log-caster-c-reconstructed'
rm -rf obj bin
Clean complete
make: Leaving directory '/home/user/gpt-00/log-caster-c-reconstructed'
make: Entering directory '/home/user/gpt-00/log-caster-c-reconstructed'
Compiling src/log_buffer.c...
Compiling src/main.c...
Compiling src/query_handler.c...
Compiling src/query_parser.c...
Compiling src/server.c...
Compiling src/thread_pool.c...
Linking...
Build complete: bin/logcaster-c
make: Leaving directory '/home/user/gpt-00/log-caster-c-reconstructed'
```

### Final Test Execution Log
```
$ pkill -f logcaster-c; sleep 1; ./bin/logcaster-c &
$ sleep 2 && python3 tests/test_mvp3.py
--- Test 1: Simple keyword search ---
> QUERY keywords=timeout
FOUND: 2 matches
[2025-07-28 00:00:00] 2025-07-28 WARNING: Configuration value 'timeout' is deprecated, use 'connection_timeout' instead
[2025-07-28 00:00:00] 2025-07-28 ERROR: Failed to process user request for user_id=123, reason: timeout
OK

--- Test 2: Regex search ---
> QUERY regex=user_id=[0-9]+
FOUND: 1 matches
[2025-07-28 00:00:00] 2025-07-28 ERROR: Failed to process user request for user_id=123, reason: timeout
OK

--- Test 3: Multi-keyword AND search ---
> QUERY keywords=Failed,timeout operator=AND
FOUND: 1 matches
[2025-07-28 00:00:00] 2025-07-28 ERROR: Failed to process user request for user_id=123, reason: timeout
OK

--- Test 4: Multi-keyword OR search ---
> QUERY keywords=startup,failure operator=OR
FOUND: 2 matches
[2025-07-28 00:00:00] 2025-07-28 INFO: Application startup successful
[2025-07-28 00:00:00] 2025-07-28 ERROR: Critical failure in payment module, transaction_id=xyz-789
OK

...

All MVP3 tests passed!
```
