cmake_minimum_required (VERSION 2.6)
project (react)

# compiler flags
add_definitions(-std=c99 -Wall -pedantic)

# version
set (REACT_VERSION_MAJOR 0)
set (REACT_VERSION_MINOR 1)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

include_directories("${PROJECT_BINARY_DIR}")

add_executable (react react.c)
