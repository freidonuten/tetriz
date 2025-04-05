include(FetchContent)

FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
    GIT_TAG v6.0.2
)

FetchContent_MakeAvailable(ftxui)

add_executable(game_sp game_sp.cpp)
target_include_directories(game_sp PRIVATE ${EXT_LIBRARY_PATH})
target_include_directories(game_sp PRIVATE ${INT_LIBRARY_PATH})
target_link_libraries(game_sp
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
)

add_executable(game_mp game_mp.cpp)
target_include_directories(game_mp PRIVATE ${EXT_LIBRARY_PATH})
target_include_directories(game_mp PRIVATE ${INT_LIBRARY_PATH})
target_link_libraries(game_mp
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
)
