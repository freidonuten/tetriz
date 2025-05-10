add_library(tui INTERFACE)

target_link_libraries(tui
    INTERFACE
        ftxui::screen
        ftxui::dom
        ftxui::component
        magic_enum
        protocol
)

target_include_directories(tui
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
