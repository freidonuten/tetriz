cmake_minimum_required(VERSION 3.24)
project(tetriz VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic -fdiagnostics-color=always)

add_subdirectory(app/tetrizd)
add_subdirectory(app/tetriz_sp)
add_subdirectory(app/tetriz_mp)

add_subdirectory(lib/engine)
add_subdirectory(lib/logging)
add_subdirectory(lib/networking)
add_subdirectory(lib/protocol)
add_subdirectory(lib/tui)
add_subdirectory(lib/util)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY "https://github.com/google/googletest"
    GIT_TAG        v1.16.0
)

FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY "https://github.com/Neargye/magic_enum"
    GIT_TAG        v0.9.7
)

FetchContent_Declare(
    argparse
    GIT_REPOSITORY "https://github.com/p-ranav/argparse"
    GIT_TAG        v3.2
)

FetchContent_Declare(
    ftxui
    GIT_REPOSITORY "https://github.com/ArthurSonzogni/ftxui"
    GIT_TAG v6.0.2
)

FetchContent_MakeAvailable(googletest magic_enum argparse ftxui)

enable_testing()

add_executable(tests test/main.cpp)
target_link_libraries(tests GTest::gtest_main)

add_test(NAME tests COMMAND tests)
