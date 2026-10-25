cmake_minimum_required(VERSION 3.25)
project(desktop VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt 6 from the dev container (installed under /opt/qt, found through
# CMAKE_PREFIX_PATH). Add more modules here as you need them, e.g. Charts, Mqtt.
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(desktop src/main.cpp)
target_link_libraries(desktop PRIVATE Qt6::Widgets)

# Compiler warnings, treated as errors
target_compile_options(desktop PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
set_target_properties(desktop PROPERTIES COMPILE_WARNING_AS_ERROR ON)
