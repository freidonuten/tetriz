cmake_minimum_required(VERSION 3.10)
project(tetriz VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(libserver OBJECT
    src/epoll.cpp
    src/Server.cpp
    src/MessageTokenizer.cpp
    src/Response.cpp
    src/protocol_dispatch.cpp
)

find_package(GTest REQUIRED)
find_package(ftxui 5 REQUIRED)

add_executable(server src/main.cpp $<TARGET_OBJECTS:libserver>)
add_executable(game_sp src/game/game_sp.cpp)
add_executable(tests test/protocol_serialization.cpp)

add_compile_options(-Wall -Wextra -Wpedantic)

include_directories(src)
include_directories(lib)

target_link_libraries(tests GTest::GTest GTest::Main $<TARGET_OBJECTS:libserver>)
target_link_libraries(game_sp
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
)

enable_testing()
add_test(NAME tests COMMAND tests)
