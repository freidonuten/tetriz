add_library(logging STATIC)

target_sources(logging PRIVATE src/logger.cpp)
target_include_directories(logging PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(logging PUBLIC magic_enum)
