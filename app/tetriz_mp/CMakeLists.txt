add_executable(tetriz_mp src/main.cpp)

target_link_libraries(tetriz_mp
    PRIVATE
        tui
        engine
        argparse
        protocol
        networking
)
