cmake_minimum_required(VERSION 3.8.0)
project(ShmitCore VERSION 0.1.0 LANGUAGES CXX)

# TODO see if these are acceptable
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_FLAGS -pedantic -Wformat -Winline -Wreturn-type -Wsign-compare -Wsign-conversion -Wswitch -Wunused)
set(C_FLAGS -Wstrict-prototypes -Wtraditional)
set(CXX_FLAGS -Wreorder)

set(SHMITCORE_FLAGS "")
set(SHMITCORE_DEBUG_FLAGS "-Og")
set(SHMITCORE_RELEASE_FLAGS "-O2")

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    list(APPEND SHMITCORE_FLAGS ${SHMITCORE_DEBUG_FLAGS})
else()
    list(APPEND SHMITCORE_FLAGS ${SHMITCORE_RELEASE_FLAGS})
endif()

include(CMake/Platform.cmake)

add_library(ShmitCore STATIC)
target_compile_options(ShmitCore
    PRIVATE
    "${SHMITCORE_FLAGS}"
    "${COMMON_FLAGS}"
    "$<$<COMPILE_LANGUAGE:C>:${C_FLAGS}>"
    "$<$<COMPILE_LANGUAGE:CXX>:${CXX_FLAGS}>"
)
target_link_libraries(ShmitCore
    PUBLIC
    ShmitCore-target-platform
)

# Add top level include to Shmitcore
set(SHMITCORE_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/Development/Include)
target_include_directories(ShmitCore
    PUBLIC
    ${SHMITCORE_INCLUDE_PATH}
)

# This step marks headers as source files for the benefit of other IDEs (Visual Studio, XCode, etc.).
# They should pick up on it and add them automatically to the file list.
# Admittedly, globbing these files is probably not the best solution. This may need to be re-done sometime.
file(GLOB_RECURSE SHMITCORE_HEADERS
    "${SHMITCORE_INCLUDE_PATH}/*.h"
    "${SHMITCORE_INCLUDE_PATH}/*.hpp"
)
target_sources(ShmitCore
    PRIVATE
    ${SHMITCORE_HEADERS}
)

# Add ShmitCore source directories
add_subdirectory(Help)
add_subdirectory(IO)
add_subdirectory(Platform)

# Add ShmitCore tests
add_subdirectory(Test)
