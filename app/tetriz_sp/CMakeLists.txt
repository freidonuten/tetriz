add_executable(tetriz_sp src/main.cpp)

target_link_libraries(tetriz_sp
    PRIVATE
        tui
        engine
        argparse
)
