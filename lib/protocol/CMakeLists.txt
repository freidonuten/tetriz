add_library(protocol INTERFACE)

target_link_libraries(protocol INTERFACE engine)
target_include_directories(protocol
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
