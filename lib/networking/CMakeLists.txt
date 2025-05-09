add_library(networking OBJECT)

target_sources(networking PRIVATE src/epoll.cpp)

target_include_directories(networking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(networking
    PUBLIC
        logging
        magic_enum
)
