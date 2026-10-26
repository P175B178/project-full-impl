cmake_minimum_required(VERSION 3.25)
project(warden_console VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt 6 from the dev container (installed under /opt/qt, found through
# CMAKE_PREFIX_PATH).
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(warden-console
    src/main.cpp
    src/app/MainWindow.cpp
    src/messages/DeviceGateway.cpp
    src/core/DeviceModel.cpp
    src/core/DeviceState.cpp
    src/messages/MessageRouter.cpp
    src/messages/Observation.cpp
    src/messages/Topics.cpp
    src/sim/SimulatedGateway.cpp
)
target_include_directories(warden-console PRIVATE src)
target_link_libraries(warden-console PRIVATE Qt6::Widgets)
# Inject project version so the program can report it without hardcoding.
target_compile_definitions(warden-console PRIVATE WARDEN_CONSOLE_VERSION="${PROJECT_VERSION}")

# Compiler warnings, treated as errors
target_compile_options(warden-console PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
set_target_properties(warden-console PROPERTIES COMPILE_WARNING_AS_ERROR ON)
