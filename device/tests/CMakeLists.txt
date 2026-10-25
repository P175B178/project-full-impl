# Test-only dependency, fetched here rather than in the top-level CMakeLists.txt.
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.16.0
    GIT_SHALLOW    TRUE
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(googletest)

add_executable(warden_tests
    test_state_machine.cpp
)

target_link_libraries(warden_tests PRIVATE
    warden_core
    GTest::gtest_main
)

warden_configure_target(warden_tests)
# Most tests do not care when a reading was taken, so they leave its timestamp out.
target_compile_options(warden_tests PRIVATE -Wno-missing-field-initializers)

include(GoogleTest)
gtest_discover_tests(warden_tests)
