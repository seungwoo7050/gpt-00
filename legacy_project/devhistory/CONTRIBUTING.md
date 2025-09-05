# Contributing to LogCaster

First off, thank you for considering contributing to LogCaster! Your help is greatly appreciated. Whether it's fixing a bug, adding a new feature, or improving documentation, every contribution is valuable.

This document provides guidelines to help make the contribution process smooth and effective for everyone.

## Code of Conduct

This project and everyone participating in it is governed by our [Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior.

## How to Contribute

### Reporting Bugs

If you find a bug, please ensure the bug was not already reported by searching on GitHub under [Issues](https://github.com/your-repo/logcaster/issues). If you're unable to find an open issue addressing the problem, open a new one. Be sure to include a **title and clear description**, as much relevant information as possible, and a **code sample or an executable test case** demonstrating the expected behavior that is not occurring.

### Suggesting Enhancements

If you have an idea for a new feature or an improvement to an existing one, please open an issue with the `enhancement` label. Clearly describe the proposed enhancement, why it would be beneficial, and if possible, provide a code snippet or pseudo-code for how it might be implemented.

### Your First Code Contribution

Unsure where to begin? You can start by looking through `good first issue` and `help wanted` issues:

- **Good First Issues** - Issues which should only require a few lines of code, and a test or two.
- **Help Wanted** - Issues which should be a bit more involved than `good first issue`s.

### Development Workflow

1.  **Fork** the repository on GitHub.
2.  **Clone** your fork locally: `git clone https://github.com/your-username/logcaster.git`
3.  **Create a branch** for your changes. Please use a descriptive branch name like `feature/add-rate-limiting` or `fix/query-parser-leak`.
    ```bash
    git checkout -b feature/my-new-feature
    ```
4.  **Make your changes** locally. See the coding style guidelines below.
5.  **Run tests** to ensure nothing has broken (see Testing section).
6.  **Commit** your changes with a clear and descriptive commit message.
7.  **Push** your branch to your fork on GitHub:
    ```bash
    git push origin feature/my-new-feature
    ```
8.  **Open a Pull Request** to the `main` branch of the original LogCaster repository. Provide a clear title and description for your PR, linking to any relevant issues.

## Coding Style

Consistency is key. Please try to match the style of the existing code as much as possible.

-   **C Version (`LogCaster-C/`)**:
    -   Follow the established procedural style using `structs` and functions.
    -   Adhere to POSIX standards for system calls and `pthreads`.
    -   Manual memory management must be done carefully. Every `malloc` must have a corresponding `free`.
    -   Use comments to explain *why* something is done, not *what* is being done.

-   **C++ Version (`LogCaster-CPP/`)**:
    -   Embrace Modern C++17 features.
    -   Use RAII (Resource Acquisition Is Initialization) for all resource management. Prefer smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers.
    -   Use STL containers (`std::vector`, `std::string`, `std::map`, etc.) instead of C-style arrays.
    -   Use `override` and `final` where appropriate.
    -   Use lambdas for concise, localized functions, especially for thread pool tasks.

## Testing

Each version has its own set of Python-based integration tests. Before submitting a pull request, please ensure your changes do not break any existing tests.

-   **To run C version tests:**
    ```bash
    # From the root directory
    cd LogCaster-C/tests
    python3 test_client.py
    python3 test_security.py
    ```

-   **To run C++ version tests:**
    ```bash
    # From the root directory
    cd LogCaster-CPP/
    python3 test_irc.py
    ```

If you add a new feature, please add a corresponding test file that validates its functionality.
