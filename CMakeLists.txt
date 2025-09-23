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