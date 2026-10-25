# Test-only dependency, fetched here rather than in the top-level CMakeLists.txt.
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.16.0
    GIT_SHALLOW    TRUE
    FIND_PACKAGE_ARGS
)
set(INSTALL_GTEST OFF) # keep it out of installs and packages
FetchContent_MakeAvailable(googletest)

add_executable(warden_tests
    test_averaging.cpp
    test_blocking_queue.cpp
    test_commands.cpp
    test_config.cpp
    test_mqtt_publisher.cpp
    test_mqtt_subscriber.cpp
    test_payloads.cpp
    test_range.cpp
    test_reading_processor.cpp
    test_signal_transition.cpp
    test_state_machine.cpp
    test_warden_app.cpp
)

target_link_libraries(warden_tests PRIVATE
    warden_core
    warden_mqtt
    nlohmann_json::nlohmann_json
    GTest::gtest_main
    GTest::gmock
)

target_compile_definitions(warden_tests PRIVATE
    WARDEN_CONFIG_FILE="${PROJECT_SOURCE_DIR}/config/warden.toml"
    # The message examples both programs are tested against
    WARDEN_PROTOCOL_EXAMPLES="${PROJECT_SOURCE_DIR}/../protocol/examples"
)

warden_configure_target(warden_tests)
# Most tests do not care when a reading was taken, so they leave its timestamp out.
target_compile_options(warden_tests PRIVATE -Wno-missing-field-initializers)

include(GoogleTest)
# Find the tests when ctest runs rather than right after the build: listing
# them means running the test binary, which this machine cannot do for the
# cross-compiled Pi build (tools/pi/test.sh runs that one on the Pi).
gtest_discover_tests(warden_tests DISCOVERY_MODE PRE_TEST)
