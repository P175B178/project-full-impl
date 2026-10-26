cmake_minimum_required(VERSION 3.25)
project(warden_console VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_CLANG_TIDY "Run clang-tidy on every translation unit during build" OFF)
option(BUILD_TESTING "Build the unit tests" ON)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE clang-tidy REQUIRED)
endif()

# Qt 6 from the dev container (installed under /opt/qt, found through
# CMAKE_PREFIX_PATH).
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

# Settings for this project's own targets, so third-party code keeps its own.
function(warden_configure_target target)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
    set_target_properties(${target} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
    if(ENABLE_CLANG_TIDY)
        # The header filter leaves out the code Qt generates into the build directory.
        set_target_properties(${target} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--header-filter=^${PROJECT_SOURCE_DIR}/(src|tests)/")
    endif()
endfunction()

# ── Core and messages ──
# Parsers and models: Qt6::Core only, no widgets and no broker, so the unit
# tests can run them headless.
add_library(warden_core STATIC
    src/core/DeviceModel.cpp
    src/core/DeviceState.cpp
    src/messages/DeviceGateway.cpp
    src/messages/JsonMessage.cpp
    src/messages/MessageRouter.cpp
    src/messages/Observation.cpp
    src/messages/StatusMessage.cpp
    src/messages/Topics.cpp
)
target_include_directories(warden_core PUBLIC src)
target_link_libraries(warden_core PUBLIC Qt6::Core)
warden_configure_target(warden_core)

# ── Main executable ──
qt_add_executable(warden-console
    src/main.cpp
    src/app/Colors.cpp
    src/app/MainWindow.cpp
    src/app/StateColorDelegate.cpp
    src/sim/SimulatedGateway.cpp
)
target_link_libraries(warden-console PRIVATE warden_core Qt6::Widgets)
# Inject project version so the program can report it without hardcoding.
target_compile_definitions(warden-console PRIVATE WARDEN_CONSOLE_VERSION="${PROJECT_VERSION}")
warden_configure_target(warden-console)

# ── Format target ──
find_program(CLANG_FORMAT_EXE clang-format)
if(CLANG_FORMAT_EXE)
    # A glob is fine here: this only lists files to format, not to build.
    file(GLOB_RECURSE ALL_SOURCE_FILES CONFIGURE_DEPENDS
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
