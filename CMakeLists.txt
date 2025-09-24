cmake_minimum_required(VERSION 3.28)
project(lockfree_queue
    VERSION 1.0.0
    DESCRIPTION "Lock-free queue used in audio mixer project"
    LANGUAGES CXX
)

add_library(lockfree_queue INTERFACE)

target_include_directories(lockfree_queue
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(lockfree_queue INTERFACE cxx_std_23)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/lockfree_queueConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(DIRECTORY include/ DESTINATION include)

install(TARGETS lockfree_queue
    EXPORT lockfree_queueTargets
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/lockfree_queueConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/lockfree_queueConfig.cmake"
    INSTALL_DESTINATION lib/cmake/lockfree_queue
)

install(EXPORT lockfree_queueTargets
    FILE lockfree_queueTargets.cmake
    NAMESPACE lockfree_queue::
    DESTINATION lib/cmake/lockfree_queue
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/lockfree_queueConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/lockfree_queueConfigVersion.cmake"
    DESTINATION lib/cmake/lockfree_queue
)
# optional : build test
option(build_tests "Build lockfree_queue tests" OFF)

if(build_tests)
    add_subdirectory(tests)
endif()