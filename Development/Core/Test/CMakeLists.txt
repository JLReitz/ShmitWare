include(GoogleTest)

add_library(ShmitCore-Test STATIC)
target_include_directories(ShmitCore-Test
    PUBLIC
    ${CMAKE_SOURCE_DIR}/Development/Core/Test/Include
)
target_link_libraries(ShmitCore-Test
    PUBLIC
    ShmitCore
    gtest_main
    gmock_main
)

# Add ShmitCore-Test source subdirectory
add_subdirectory(Source)