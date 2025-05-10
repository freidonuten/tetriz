add_library(logging INTERFACE)

target_link_libraries(logging INTERFACE magic_enum)
target_include_directories(logging
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
