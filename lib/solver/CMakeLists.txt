add_library(solver INTERFACE)

target_link_libraries(solver
    INTERFACE
        engine
        logging
)

target_include_directories(solver
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
