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
find_package(Qt6 REQUIRED COMPONENTS Widgets Charts Mqtt)
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
    src/core/AlertLogModel.cpp
    src/core/CsvExport.cpp
    src/core/DeviceModel.cpp
    src/core/DeviceMonitor.cpp
    src/core/DeviceState.cpp
    src/core/HistoryModel.cpp
    src/messages/CommandSerializer.cpp
    src/messages/ConfigMessage.cpp
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
    src/app/BrokerSettingsStore.cpp
    src/app/ChartPanel.cpp
    src/app/Colors.cpp
    src/app/ConnectionDialog.cpp
    src/app/ConnectionDialog.ui
    src/app/DeviceDetailPanel.cpp
    src/app/DeviceDetailPanel.ui
    src/app/DeviceListPanel.cpp
    src/app/DeviceListPanel.ui
    src/app/GroupFilterProxyModel.cpp
    src/app/LogPanel.cpp
    src/app/MainWindow.cpp
    src/app/MainWindow.ui
    src/app/StateColorDelegate.cpp
    src/app/ViolationHighlightDelegate.cpp
    src/app/ZoomableChartView.cpp
    src/mqtt/MqttConnection.cpp
    src/sim/SimulatedGateway.cpp
)
target_link_libraries(warden-console PRIVATE warden_core Qt6::Widgets Qt6::Charts Qt6::Mqtt)
# Inject project version so the program can report it without hardcoding.
target_compile_definitions(warden-console PRIVATE WARDEN_CONSOLE_VERSION="${PROJECT_VERSION}")
warden_configure_target(warden-console)

# ── Install rules ──
# The package installs into /usr. Debian has Qt, Qt Charts included, but no
# QtMqtt, so the package carries its own copy of libQt6Mqtt, in a folder of
# its own that the executable finds through its RPATH.
include(GNUInstallDirs)
set(WARDEN_PRIVATE_LIBDIR "${CMAKE_INSTALL_LIBDIR}/warden-console")
set_target_properties(warden-console PROPERTIES INSTALL_RPATH "$ORIGIN/../${WARDEN_PRIVATE_LIBDIR}")
install(TARGETS warden-console RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(IMPORTED_RUNTIME_ARTIFACTS Qt6::Mqtt LIBRARY DESTINATION ${WARDEN_PRIVATE_LIBDIR})
install(FILES packaging/warden-console.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

# ── Packaging (CPack) ──
set(CPACK_PACKAGE_NAME "warden-console")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Desktop console for Warden devices")
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "warden")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
# Debian's own Qt, the same version as the dev container's. Named by hand:
# dpkg-shlibdeps cannot work them out from a Qt installed under /opt/qt, which
# no Debian package owns.
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libqt6widgets6 (>= ${Qt6_VERSION}), libqt6charts6 (>= ${Qt6_VERSION}), libqt6network6 (>= ${Qt6_VERSION})")
# The bundled QtMqtt uses Qt's private API, which may change with any Qt version: it needs
# exactly the Qt it was built against.
string(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS ", qt6-base-private-abi (= ${Qt6_VERSION})")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION
    "Monitors Warden devices through an MQTT broker.\n Shows their readings, states and trends, and changes their configuration.")
include(CPack)

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
