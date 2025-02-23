# Target unit tests
add_executable(ShmitCore-test-Data
    ${CMAKE_CURRENT_LIST_DIR}/TestEncode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TestDecode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TestFields.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TestPacket.cpp
)

# Link gtest_main and ShmitCore-Test to targets
target_link_libraries(ShmitCore-test-Data
    ShmitCore-Test
)

# Discover all unit tests
gtest_discover_tests(ShmitCore-test-Data)