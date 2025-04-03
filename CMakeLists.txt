cmake_minimum_required(VERSION 3.10)
project(tetriz VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXT_LIBRARY_PATH ${PROJECT_SOURCE_DIR}/lib)
set(INT_LIBRARY_PATH ${PROJECT_SOURCE_DIR}/src)

add_library(libserver OBJECT
    src/epoll.cpp
    src/Server.cpp
    src/MessageTokenizer.cpp
    src/Response.cpp
    src/protocol_dispatch.cpp
)

add_subdirectory(src/game)

find_package(GTest REQUIRED)

add_executable(server src/main.cpp $<TARGET_OBJECTS:libserver>)
add_executable(server_ng src/server/main.cpp $<TARGET_OBJECTS:libserver>)
add_executable(tests test/protocol_serialization.cpp)

add_compile_options(-Wall -Wextra -Wpedantic)

include_directories(src)
include_directories(lib)

target_link_libraries(tests GTest::GTest GTest::Main $<TARGET_OBJECTS:libserver>)

enable_testing()
add_test(NAME tests COMMAND tests)
