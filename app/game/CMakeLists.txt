include(FetchContent)

FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
    GIT_TAG v6.0.2
)

FetchContent_MakeAvailable(ftxui)

add_executable(game_sp src/game_sp.cpp)
target_link_libraries(game_sp
    PRIVATE
        ftxui::screen
        ftxui::dom
        ftxui::component
        argparse
        engine
        protocol
)

add_executable(game_mp src/game_mp.cpp)
target_link_libraries(game_mp
    PRIVATE
        ftxui::screen
        ftxui::dom
        ftxui::component
        argparse
        engine
        networking
        protocol
)
