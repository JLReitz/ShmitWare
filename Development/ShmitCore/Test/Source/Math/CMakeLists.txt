# Target unit tests
add_executable(ShmitCore-test-Math
    ${CMAKE_CURRENT_LIST_DIR}/TestRatio.cpp
)

# Link gtest_main and ShmitCore-Test to targets
target_link_libraries(ShmitCore-test-Math
    ShmitCore-Test
)

# Discover all unit tests
gtest_discover_tests(ShmitCore-test-Math)