cmake_minimum_required(VERSION 3.0)

project(kumquat C)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(src)
include_directories(third_party/glew)
include_directories(third_party/stb)

add_definitions(-DGLEW_STATIC)

add_definitions(-DPLATFORM_LINUX)

add_subdirectory(src)
add_subdirectory(third_party)
