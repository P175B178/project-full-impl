cmake_minimum_required(VERSION 3.25)
project(warden VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_CLANG_TIDY "Run clang-tidy on every translation unit during build" OFF)
option(BUILD_TESTING "Build the unit tests" ON)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE clang-tidy REQUIRED)
endif()

# Settings for this project's own targets, so third-party code keeps its own.
function(warden_configure_target target)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
    set_target_properties(${target} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
    if(ENABLE_CLANG_TIDY)
        set_target_properties(${target} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--header-filter=^${PROJECT_SOURCE_DIR}/(include|src|tests)/")
    endif()
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

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
    GIT_SHALLOW    TRUE
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(tomlplusplus)

# ── Core library (no hardware deps, testable on host) ──
add_library(warden_core
    src/core/Config.cpp
    src/core/StateMachine.cpp
)
# Public, because other targets include these headers.
target_include_directories(warden_core PUBLIC include src/core)
target_link_libraries(warden_core PUBLIC spdlog::spdlog tomlplusplus::tomlplusplus)
warden_configure_target(warden_core)

# ── Main executable ──
# main.cpp includes the header-only simulation classes in src/sim.
add_executable(warden src/main.cpp)
target_link_libraries(warden PRIVATE warden_core)
warden_configure_target(warden)

# ── Format target ──
find_program(CLANG_FORMAT_EXE clang-format)
if(CLANG_FORMAT_EXE)
    # A glob is fine here: this only lists files to format, not to build.
    file(GLOB_RECURSE ALL_SOURCE_FILES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/tests/*.h"
        "${PROJECT_SOURCE_DIR}/tests/*.cpp"
    )
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES}
        COMMENT "Formatting all source files"
    )
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${ALL_SOURCE_FILES}
        COMMENT "Checking formatting (dry run)"
    )
endif()

# ── Tests ──
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
