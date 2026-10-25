cmake_minimum_required(VERSION 3.25)
project(device VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(device src/main.cpp)
target_include_directories(device PRIVATE include)

# Compiler warnings, treated as errors
target_compile_options(device PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
set_target_properties(device PROPERTIES COMPILE_WARNING_AS_ERROR ON)
