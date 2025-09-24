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

add_executable(basic test_basic.cpp)
target_link_libraries(basic
    PRIVATE
        lockfree_queue
        Catch2::Catch2WithMain
)

if(TARGET Catch2::Catch2WithMain)
    target_include_directories(basic
        PRIVATE
            $<TARGET_PROPERTY:Catch2::Catch2WithMain,INTERFACE_INCLUDE_DIRECTORIES>
    )
endif()

add_executable(ABA test_ABA.cpp)
target_link_libraries(ABA
    PRIVATE
        lockfree_queue
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(basic)
catch_discover_tests(ABA)
