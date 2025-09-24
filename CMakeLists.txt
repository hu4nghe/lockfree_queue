cmake_minimum_required(VERSION 3.28)
project(lockfree_queue
        VERSION 1.0.0
        DESCRIPTION "This project incorporates the audio lock-free queue used for real-time audio processing in the audio mixer. "
        LANGUAGES CXX)

add_library(lockfree_queue INTERFACE)

target_include_directories(lockfree_queue
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(lockfree_queue INTERFACE cxx_std_23)

# Test

# Find or fetch Catch2
find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.10.0
    )
    FetchContent_MakeAvailable(Catch2)
endif()
 
# basic tests
add_executable(basic tests/test_basic.cpp)
target_include_directories(basic
  PRIVATE
    ${catch2_SOURCE_DIR}/src
)
target_link_libraries(basic PRIVATE lockfree_queue Catch2::Catch2WithMain)

# ABA test
add_executable(ABA tests/test_ABA.cpp)
target_link_libraries(ABA PRIVATE lockfree_queue Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(basic)
catch_discover_tests(ABA)
