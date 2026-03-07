cmake_minimum_required(VERSION 3.14)
project(cmap C)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
add_compile_options(-Wall -Wextra -Werror)

option(USE_SIMPLE_HASH "Use simple built-in hash instead of xxHash" OFF)

# Library
add_library(map STATIC map.c)
target_include_directories(map PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(map PRIVATE MAP_ENABLE_DIAGNOSTIC)

if(USE_SIMPLE_HASH)
    target_compile_definitions(map PUBLIC USE_SIMPLE_HASH)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XXHASH REQUIRED libxxhash)
    target_include_directories(map PRIVATE ${XXHASH_INCLUDE_DIRS})
    target_link_libraries(map PUBLIC ${XXHASH_LIBRARIES})
    target_link_directories(map PUBLIC ${XXHASH_LIBRARY_DIRS})
endif()

# Test executables (only when building standalone)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_executable(test_functional test/test_functional.c)
    target_compile_definitions(test_functional PRIVATE MAP_ENABLE_DIAGNOSTIC)
    target_link_libraries(test_functional PRIVATE map)

    add_executable(test_perf test/test_perf.c)
    target_compile_definitions(test_perf PRIVATE MAP_ENABLE_DIAGNOSTIC)
    target_link_libraries(test_perf PRIVATE map)
endif()
