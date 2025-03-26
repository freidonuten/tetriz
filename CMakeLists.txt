cmake_minimum_required(VERSION 3.10)
project(tetriz-server VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(objects OBJECT
    src/Server.cpp
    src/MessageTokenizer.cpp
    src/Response.cpp
    src/protocol_dispatch.cpp
)

add_executable(server src/main.cpp $<TARGET_OBJECTS:objects>)
add_compile_options(-Wall -Wextra -Wpedantic)

enable_testing()
find_package(GTest REQUIRED)
include_directories(src)
include_directories(lib)
add_executable(tests
    test/protocol_serialization.cpp
)
target_link_libraries(tests GTest::GTest GTest::Main $<TARGET_OBJECTS:objects>)
add_test(NAME tests COMMAND tests)
