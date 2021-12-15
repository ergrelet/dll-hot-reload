cmake_minimum_required(VERSION 3.14)

option(BUILD_TEST_EXE "Build test executable" OFF)

project(dll-hot-reload LANGUAGES CXX)

include(FetchContent)

if (MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.29.30129 AND CMAKE_VERSION VERSION_GREATER 3.20.3)
    # this change happened in CMake 3.20.4
    set(CMAKE_CXX_STANDARD 23) # /std:c++latest - unlocks the non stable cpp20 features. For new 16.11 versions
else ()
    set(CMAKE_CXX_STANDARD 20) # /std:c++latest for msvc and -std=c++20 for everyone else.
endif ()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# inipp
FetchContent_Declare(
    inipp
    GIT_REPOSITORY https://github.com/mcmtroffaes/inipp.git
    GIT_TAG        46248e4e93a2e63f9a1d0d8d9ad40bd6b3725df5
)
FetchContent_MakeAvailable(inipp)

# Blackbone
FetchContent_Declare(
    BlackBone
    GIT_REPOSITORY https://github.com/DarthTon/Blackbone.git
    GIT_TAG        a672509b5458efeb68f65436259b96fa8cd4dcfc
)
FetchContent_MakeAvailable(BlackBone)
add_subdirectory("${blackbone_SOURCE_DIR}/src/BlackBone")
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    target_link_libraries(BlackBone PRIVATE "${blackbone_SOURCE_DIR}/src/3rd_party/DIA/lib/amd64/diaguids.lib")
elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    target_link_libraries(BlackBone PRIVATE "${blackbone_SOURCE_DIR}/src/3rd_party/DIA/lib/diaguids.lib")
endif()
target_include_directories(BlackBone PUBLIC "${blackbone_SOURCE_DIR}/src/3rd_party")
target_include_directories(BlackBone INTERFACE "${blackbone_SOURCE_DIR}/src")

add_subdirectory("src")
