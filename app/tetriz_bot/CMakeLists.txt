add_executable(tetriz_bot src/main.cpp)

target_link_libraries(tetriz_bot
    PRIVATE
        tui
        engine
        logging
        argparse
        solver
)
