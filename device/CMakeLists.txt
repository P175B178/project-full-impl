cmake_minimum_required(VERSION 3.25)
project(warden VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(warden src/main.cpp)
target_include_directories(warden PRIVATE include)

# Compiler warnings, treated as errors
target_compile_options(warden PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
set_target_properties(warden PROPERTIES COMPILE_WARNING_AS_ERROR ON)
