# Lock-Free Queue

This project incorporates the audio lock-free queue used for real-time audio processing in the audio mixer. 
The lock-free queue component has been extracted separately as a header only lib for reuse.

## Prerequisites
- CMake : 3.14
- C++20

## Notes
- The constructor parameter `requested` is a hint; the queue capacity is adjusted to the smallest power of two >= requested (minimum 2). See [`lockfree::queue`](include/lockfree_queue.h).
- The library is header-only and requires C++23.
- Tests live in [tests/CMakeLists.txt](tests/CMakeLists.txt) and examples/tests are [tests/test_basic.cpp](tests/test_basic.cpp) and [tests/test_ABA.cpp](tests/test_ABA.cpp).
- CI is configured at [.github/workflows/CI.yml](.github/workflows/CI.yml).

## Use in your project

### FetchContent
```cmake
FetchContent_Declare(
    lockfree_queue
    GIT_REPOSITORY https://github.com/hu4nghe/lockfree_queue.git
    GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(lockfree_queue)
```

### Or install
```sh
cmake -S . -B build_install -DCMAKE_INSTALL_PREFIX="/path/to/install" -Dbuild_tests=OFF

cmake --install build_install --prefix install
```
```cmake
find_package(lockfree_queue REQUIRED CONFIG
    PATHS /path/to/install/lib/cmake/lockfree_queue
)
```
### Don't forget to link got include path(even it is header only)

```cmake
target_link_libraries(myapp INTERFACE lockfree_queue::lockfree_queue)
```

Build with tests
```sh
cmake -S . -B build -Dbuild_tests=ON

cmake --build build --parallel

ctest --test-dir build --output-on-failure
```