# Cross-compilation toolchain for Raspberry Pi 5 (aarch64 / ARM Cortex-A76)
# Uses the cross toolchain in the dev container.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER        aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER      aarch64-linux-gnu-g++)
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)

find_program(CROSS_CXX aarch64-linux-gnu-g++)
if(NOT CROSS_CXX)
    message(FATAL_ERROR
        "Cross-compiler aarch64-linux-gnu-g++ not found. "
        "Install with: apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu")
endif()

# Debian/Ubuntu multiarch installs arm64 packages under /usr/lib/aarch64-linux-gnu/.
# Setting CMAKE_FIND_ROOT_PATH to /usr makes find_library() search there.
set(CMAKE_FIND_ROOT_PATH /usr)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# pkg-config runs on this machine, so point it at the arm64 packages' .pc files.
set(ENV{PKG_CONFIG_LIBDIR} "/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig")

# CMake's compiler checks build a library: a cross-compiled program cannot run here.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
