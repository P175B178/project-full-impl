cmake_minimum_required(VERSION 3.25)
project(warden VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Settings for this project's own targets, so third-party code keeps its own.
function(warden_configure_target target)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
    set_target_properties(${target} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
endfunction()

# ── Dependencies ──
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.15.3
    GIT_SHALLOW    TRUE
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(spdlog)

# ── Core library (no hardware deps, testable on host) ──
add_library(warden_core
    src/core/StateMachine.cpp
)
# Public, because other targets include these headers.
target_include_directories(warden_core PUBLIC include src/core)
target_link_libraries(warden_core PUBLIC spdlog::spdlog)
warden_configure_target(warden_core)

# ── Main executable ──
# main.cpp includes the header-only simulation classes in src/sim.
add_executable(warden src/main.cpp)
target_link_libraries(warden PRIVATE warden_core)
warden_configure_target(warden)
