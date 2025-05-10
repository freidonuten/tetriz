add_executable(tetrizd src/main.cpp)

target_link_libraries(tetrizd
    PRIVATE
        logging
        networking
        protocol
        util
)
