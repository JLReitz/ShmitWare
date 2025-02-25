# Target unit tests
add_executable(ShmitCore-test-IO
    ${CMAKE_CURRENT_LIST_DIR}/Session/TestEgress.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Session/TestIngress.cpp
)

# Link gtest_main and ShmitCore-Test to targets
target_link_libraries(ShmitCore-test-IO
    gtest_main
    gmock_main
    ShmitCore-Test
)

# Discover all unit tests
gtest_discover_tests(ShmitCore-test-IO)