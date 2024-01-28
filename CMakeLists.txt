cmake_minimum_required(VERSION 3.10)
project(tetriz-server VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(objects OBJECT
    src/Player.cpp
    src/Server.cpp
    src/Room.cpp
    src/Protocol.cpp
    src/MessageTokenizer.cpp
    src/Response.cpp
)
add_executable(server src/main.cpp $<TARGET_OBJECTS:objects>)

enable_testing()
find_package(GTest REQUIRED)
include_directories(src)
add_executable(tests
    test/protocol_serialization.cpp
)
target_link_libraries(tests GTest::GTest GTest::Main $<TARGET_OBJECTS:objects>)
add_test(NAME tests COMMAND tests)
