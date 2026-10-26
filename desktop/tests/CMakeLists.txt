# Qt Test: only the tests need it.
find_package(Qt6 REQUIRED COMPONENTS Test)

# Test-only dependency, fetched here rather than in the top-level CMakeLists.txt.
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.16.0
    GIT_SHALLOW    TRUE
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(googletest)

add_executable(warden_console_tests
    test_device_model.cpp
    test_message_router.cpp
    test_observation.cpp
    test_topics.cpp
)

target_link_libraries(warden_console_tests PRIVATE
    warden_core
    Qt6::Test
    GTest::gtest_main
)

target_compile_definitions(warden_console_tests PRIVATE
    # The message examples both programs are tested against
    WARDEN_PROTOCOL_EXAMPLES="${PROJECT_SOURCE_DIR}/../protocol/examples"
)

warden_configure_target(warden_console_tests)

include(GoogleTest)
gtest_discover_tests(warden_console_tests)
