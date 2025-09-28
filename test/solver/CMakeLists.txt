add_executable(solver_test
    expand_state_test.cpp
)

target_link_libraries(solver_test
    PRIVATE
        GTest::gtest_main
        solver
)

add_test(NAME solver_test COMMAND solver_test)
