add_library(engine INTERFACE)

target_link_libraries(engine
    INTERFACE
        magic_enum
        util
)
target_include_directories(engine
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
