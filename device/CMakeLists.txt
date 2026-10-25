cmake_minimum_required(VERSION 3.25)
project(warden VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_HARDWARE "Build hardware drivers (Raspberry Pi only)" OFF)
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

FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG        v2.5.0
    GIT_SHALLOW    TRUE
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(CLI11)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
    GIT_SHALLOW    TRUE
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(nlohmann_json)

# System libraries are found through pkg-config.
find_package(PkgConfig REQUIRED)

# ── Core library (no hardware, no MQTT: testable on host) ──
add_library(warden_core
    src/core/Config.cpp
    src/core/ReadingProcessor.cpp
    src/core/StateMachine.cpp
    src/core/WardenApp.cpp
)
# Public, because other targets include these headers.
target_include_directories(warden_core PUBLIC include src/core)
target_link_libraries(warden_core PUBLIC spdlog::spdlog tomlplusplus::tomlplusplus)
warden_configure_target(warden_core)

# ── Hardware drivers ──
if(ENABLE_HARDWARE)
    # libgpiod 2 replaced the whole API: version 1 would give confusing compile errors.
    pkg_check_modules(GPIOD REQUIRED IMPORTED_TARGET libgpiod>=2)

    add_library(warden_hardware
        src/hardware/buzzer/GpioBuzzer.cpp
        src/hardware/buzzer/GpioPin.cpp
        src/hardware/led/StatusLed.cpp
        src/hardware/led/SysfsLed.cpp
        src/hardware/sensor/Dht22Sensor.cpp
    )
    target_link_libraries(warden_hardware PUBLIC warden_core PkgConfig::GPIOD)
    warden_configure_target(warden_hardware)
endif()

# ── MQTT ──
# Everything warden does over MQTT. Also in the simulation: warden always connects to a broker.
pkg_check_modules(MOSQUITTO REQUIRED IMPORTED_TARGET libmosquitto)

add_library(warden_mqtt
    src/mqtt/Commands.cpp
    src/mqtt/MosquittoClient.cpp
    src/mqtt/MqttPublisher.cpp
    src/mqtt/Payloads.cpp
)
target_include_directories(warden_mqtt PUBLIC src/mqtt)
target_link_libraries(warden_mqtt PUBLIC warden_core PkgConfig::MOSQUITTO PRIVATE nlohmann_json::nlohmann_json)
warden_configure_target(warden_mqtt)

# ── Main executable ──
# main.cpp includes the header-only simulation classes in src/sim.
add_executable(warden src/main.cpp)
# Inject project version so --version can report it without hardcoding.
target_compile_definitions(warden PRIVATE WARDEN_VERSION="${PROJECT_VERSION}")
target_link_libraries(warden PRIVATE warden_core warden_mqtt CLI11::CLI11)
warden_configure_target(warden)
if(ENABLE_HARDWARE)
    target_link_libraries(warden PRIVATE warden_hardware)
    # Same name as the CMake option, so main.cpp can use #ifdef ENABLE_HARDWARE.
    target_compile_definitions(warden PRIVATE ENABLE_HARDWARE)
endif()

# ── Install rules ──
# Relative to the root of the target filesystem, so it installs /usr/bin/warden
# and /etc/warden/warden.toml. For that, the install prefix defaults to /.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix" FORCE)
endif()
install(TARGETS warden RUNTIME DESTINATION usr/bin)
install(FILES config/warden.toml DESTINATION etc/warden)

# ── Packaging (CPack) ──
# The archive name gets the target CPU: warden-1.0.0-aarch64 for the Pi.
set(CPACK_PACKAGE_FILE_NAME "warden-${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Environmental monitor for Raspberry Pi")
set(CPACK_GENERATOR "TGZ")
# From / rather than CPack's default /usr for .deb, which would put the config in
# /usr/etc/warden.
set(CPACK_PACKAGING_INSTALL_PREFIX "/")

if(ENABLE_HARDWARE)
    list(APPEND CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "warden")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgpiod3, libmosquitto1") # libgpiod v2 API = libgpiod.so.3
    # CPack labels the .deb with the architecture of the machine that builds it
    # (dpkg --print-architecture), which is wrong when cross-compiling for the Pi.
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
    endif()
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION
        "Monitors temperature and humidity via DHT22 on a Raspberry Pi.\n Signals state through built-in LEDs and a buzzer.")
    # A conffile, so upgrading the package keeps local edits.
    file(WRITE "${PROJECT_BINARY_DIR}/conffiles" "/etc/warden/warden.toml\n")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${PROJECT_BINARY_DIR}/conffiles")
endif()

include(CPack)

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
