# Extension Guide 02: Implementing Unit Tests with Google Test

## 1. Objective

This guide focuses on significantly improving the project's robustness and code quality by introducing C++ unit tests. The current test suite consists only of Python-based integration tests, which validate the application's behavior from an external perspective. We will integrate the Google Test framework to enable fine-grained testing of individual classes and functions in isolation.

As a primary example, we will write unit tests for the `QueryParser` class to verify its logic for parsing various query strings.

## 2. Current Limitation (Legacy Code)

The project currently lacks a C++ unit testing framework. The `tests/` directory contains only Python scripts:

```
/tests
├── test_client.py
├── test_irc.py
├── test_mvp2.py
├── test_mvp3.py
├── test_persistence.py
└── test_security.py
```

These scripts perform **integration testing** or **end-to-end testing**. They start the compiled server and interact with it over the network. While valuable, this approach has limitations:

-   **Lack of Granularity:** It's difficult to test specific internal functions or edge cases of a single class like `QueryParser` without the overhead of the full server and network stack.
-   **Incomplete Coverage:** It is impractical to cover all logical branches of every C++ class from the outside.
-   **Slow Feedback Loop:** Running the full server for every test is slower than executing lightweight unit tests.

The `CMakeLists.txt` file has no configuration for a test runner or framework, confirming the absence of a native testing setup.

## 3. Proposed Solution: Google Test Integration

Google Test is a popular, feature-rich C++ test framework. We will integrate it into our project using CMake's `FetchContent` module. This allows CMake to automatically download and configure Google Test during the project build process, avoiding the need for manual installation.

Our strategy is:
1.  Update `CMakeLists.txt` to fetch Google Test and define a test executable.
2.  Create a new C++ test file, `tests/QueryParser_test.cpp`.
3.  Write specific tests for the `QueryParser::parse` method, covering valid inputs, invalid inputs, and complex cases.
4.  Use CTest, CMake's test driver program, to run the compiled tests.

## 4. Implementation Guidance

#### **Step 1: Modify `CMakeLists.txt` to Integrate Google Test**

This is the most critical step. We need to instruct CMake to download Google Test, build it, and link it to our test executable.

**Modified Code (`CMakeLists.txt`):**
```cmake
# [SEQUENCE: CPP-MVP1-1]
# CMake 최소 요구 버전 설정
cmake_minimum_required(VERSION 3.14) # GTest FetchContent needs at least 3.14

# [SEQUENCE: CPP-MVP1-2]
# 프로젝트 이름 및 버전 설정
project(LogCaster-CPP VERSION 1.0)

# ... (existing set(SOURCES ...), add_executable(logcaster-cpp ...), etc.) ...

# [SEQUENCE: CPP-MVP1-6]
# [SEQUENCE: CPP-MVP2-3]
# 스레드 라이브러리 링크
find_package(Threads REQUIRED)
target_link_libraries(logcaster-cpp PRIVATE Threads::Threads)

# ===================================================================
# NEW SECTION: GOOGLE TEST INTEGRATION
# ===================================================================

# Enable testing with CTest
enable_testing()

# Include FetchContent module to download dependencies
include(FetchContent)

# Declare the Google Test dependency
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# Make the dependency available to the project
# This will download and configure Google Test
FetchContent_MakeAvailable(googletest)

# Create the test executable
add_executable(run_tests
    tests/QueryParser_test.cpp
    # Add other test files here in the future
)

# Link the test executable against our main application code
# This allows tests to access classes like QueryParser
# We link against the target, not the sources, for better dependency management
target_link_libraries(run_tests PRIVATE logcaster-cpp)

# Link the test executable against Google Test
target_link_libraries(run_tests PRIVATE GTest::gtest_main)

# Add the test to CTest
# This command defines a test named 'UnitTests' that runs the 'run_tests' executable
include(GoogleTest)
gtest_discover_tests(run_tests)

# ===================================================================

# [SEQUENCE: CPP-MVP4-3]
# 구형 GCC/G++ 컴파일러를 위한 stdc++fs 라이브러리 링크
# ... (rest of the file) ...
```

#### **Step 2: Create `tests/QueryParser_test.cpp`**

Create a new file to house the unit tests for `QueryParser`.

**New File (`tests/QueryParser_test.cpp`):**
```cpp
#include <gtest/gtest.h>
#include "QueryParser.h" // The class we are testing

// Test case for a simple, valid query with only keywords
TEST(QueryParserTest, HandlesSimpleKeywordQuery) {
    std::string query = "QUERY keywords=error,timeout";
    auto result = QueryParser::parse(query);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->keywords.size(), 2);
    EXPECT_EQ(result->keywords[0], "error");
    EXPECT_EQ(result->keywords[1], "timeout");
    EXPECT_EQ(result->op, QueryOperator::AND); // Default operator
}

// Test case for a query with a different operator
TEST(QueryParserTest, HandlesOrOperator) {
    std::string query = "QUERY keywords=warn,debug operator=OR";
    auto result = QueryParser::parse(query);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->keywords.size(), 2);
    EXPECT_EQ(result->op, QueryOperator::OR);
}

// Test case for a query with a regex pattern
TEST(QueryParserTest, HandlesRegexQuery) {
    std::string query = "QUERY regex=^FATAL:.*\[[0-9]+\]$";
    auto result = QueryParser::parse(query);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->keywords.empty());
    ASSERT_TRUE(result->regex_pattern.has_value());
    EXPECT_EQ(result->regex_pattern.value(), "^FATAL:.*\[[0-9]+\]$");
}

// Test case for a full, complex query
TEST(QueryParserTest, HandlesComplexQuery) {
    std::string query = "QUERY keywords=db,conn,fail operator=OR regex=timeout time_from=1672531200 time_to=1672617600";
    auto result = QueryParser::parse(query);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->keywords.size(), 3);
    EXPECT_EQ(result->op, QueryOperator::OR);
    ASSERT_TRUE(result->regex_pattern.has_value());
    EXPECT_EQ(result->regex_pattern.value(), "timeout");
    ASSERT_TRUE(result->time_from.has_value());
    EXPECT_EQ(result->time_from.value(), 1672531200);
    ASSERT_TRUE(result->time_to.has_value());
    EXPECT_EQ(result->time_to.value(), 1672617600);
}

// Test case for an invalid query (missing QUERY prefix)
TEST(QueryParserTest, RejectsQueryWithoutPrefix) {
    std::string query = "keywords=error";
    auto result = QueryParser::parse(query);
    ASSERT_FALSE(result.has_value());
}

// Test case for an invalid parameter
TEST(QueryParserTest, RejectsQueryWithInvalidParameter) {
    std::string query = "QUERY invalid_param=some_value";
    auto result = QueryParser::parse(query);
    ASSERT_FALSE(result.has_value());
}

// Test case for an empty query
TEST(QueryParserTest, RejectsEmptyQuery) {
    std::string query = "QUERY";
    auto result = QueryParser::parse(query);
    ASSERT_FALSE(result.has_value());
}
```

## 5. Verification

After making the changes to `CMakeLists.txt` and adding the new test file, you can build and run your unit tests.

1.  **Configure CMake (if you haven't already):**
    This command tells CMake to read your `CMakeLists.txt`, create a build system in the `build` directory, and download Google Test.
    ```bash
    cmake -S . -B build
    ```

2.  **Build the project and the tests:**
    This command compiles both your main application (`logcaster-cpp`) and your test executable (`run_tests`).
    ```bash
    cmake --build build
    ```

3.  **Run the tests using CTest:**
    Navigate into the build directory and run CTest. CTest will automatically find and execute the tests you defined.
    ```bash
    cd build
    ctest --verbose
    ```

**Expected Output:**

You should see output from CTest indicating that it found and ran your tests, with a summary at the end showing that all tests passed.

```
UpdateCTestConfiguration  from :/home/user/project/build/DartConfiguration.tcl
UpdateCTestConfiguration  from :/home/user/project/build/DartConfiguration.tcl
Test project /home/user/project/build
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end

Running 7 test(s)...
Test #1: QueryParserTest.HandlesSimpleKeywordQuery ...   Passed    0.00 sec
Test #2: QueryParserTest.HandlesOrOperator ...   Passed    0.00 sec
Test #3: QueryParserTest.HandlesRegexQuery ...   Passed    0.00 sec
Test #4: QueryParserTest.HandlesComplexQuery ...   Passed    0.00 sec
Test #5: QueryParserTest.RejectsQueryWithoutPrefix ...   Passed    0.00 sec
Test #6: QueryParserTest.RejectsQueryWithInvalidParameter ...   Passed    0.00 sec
Test #7: QueryParserTest.RejectsEmptyQuery ...   Passed    0.00 sec

100% tests passed, 0 tests failed out of 7

Total Test time (real) =   0.01 sec
```

This confirms that the unit testing framework is successfully integrated and your first unit tests are working correctly.
