cmake_minimum_required(VERSION 3.12)

option(BUILD_TEST_EXE "Build test executable" OFF)

project(dll-hot-reload LANGUAGES CXX)

if (MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.29.30129 AND CMAKE_VERSION VERSION_GREATER 3.20.3)
    # this change happened in CMake 3.20.4
    set(CMAKE_CXX_STANDARD 23) # /std:c++latest - unlocks the non stable cpp20 features. For new 16.11 versions
else ()
    set(CMAKE_CXX_STANDARD 20) # /std:c++latest for msvc and -std=c++20 for everyone else.
endif ()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory("src")
