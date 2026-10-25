# Debian 13 (Trixie) — desktop build host for the Qt program
FROM debian:13-slim

ENV DEBIAN_FRONTEND=noninteractive

# Qt version installed via aqtinstall. Bump together with the paths in
# devcontainer.json. See https://download.qt.io/online/qtsdkrepository/
ARG QT_VERSION=6.8.2

RUN apt-get update && apt-get install -y --no-install-recommends \
    bash \
    # ── Compilers and build tools (Trixie: gcc/g++ 14.2, clang 19.1) ──
    # CMake and Ninja are needed here already: they build QtMqtt below.
    build-essential \
    gcc \
    g++ \
    clang \
    cmake \
    ninja-build \
    # ── Debugging (gdb 16.3) ──
    gdb \
    # ── Version control ──
    git \
    # ── Python (for aqtinstall) ──
    python3 \
    python3-venv \
    # ── Qt runtime dependencies: xcb platform plugin, OpenGL, fonts ──
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libxcb-cursor0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-randr0 \
    libxcb-render-util0 \
    libxcb-shape0 \
    libxcb-xinerama0 \
    libxcb-xkb1 \
    libxkbcommon-x11-0 \
    libfontconfig1 \
    libdbus-1-3 \
    # ── Misc utilities ──
    curl \
    ca-certificates \
    file \
    less \
    tree \
    nano \
    jq \
    zip \
    unzip \
 && rm -rf /var/lib/apt/lists/*

# The Qt stack is installed with aqtinstall: Widgets/Test from the base, plus the
# qtcharts add-on module. QtMqtt is NOT published as a prebuilt binary in the Qt
# online repository (and is not in Debian), so it is built from source below.
ENV QT_ROOT=/opt/qt
RUN python3 -m venv /opt/aqt \
 && /opt/aqt/bin/pip install --no-cache-dir aqtinstall \
 && /opt/aqt/bin/aqt install-qt linux desktop "${QT_VERSION}" linux_gcc_64 \
      -m qtcharts \
      -O "${QT_ROOT}"

# Build QtMqtt from source against the Qt installed above and install it into the
# same prefix, so CMake's find_package(Qt6 COMPONENTS Mqtt) resolves it. The tag
# tracks QT_VERSION; bump both together.
RUN git clone --depth 1 --branch "v${QT_VERSION}" \
      https://github.com/qt/qtmqtt.git /tmp/qtmqtt \
 && cmake -S /tmp/qtmqtt -B /tmp/qtmqtt-build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH="${QT_ROOT}/${QT_VERSION}/gcc_64" \
      -DCMAKE_INSTALL_PREFIX="${QT_ROOT}/${QT_VERSION}/gcc_64" \
 && cmake --build /tmp/qtmqtt-build \
 && cmake --install /tmp/qtmqtt-build \
 && rm -rf /tmp/qtmqtt /tmp/qtmqtt-build

# Let CMake find Qt and put Qt tools (qmake, uic, moc, designer) on PATH.
ENV CMAKE_PREFIX_PATH=${QT_ROOT}/${QT_VERSION}/gcc_64
ENV PATH=${QT_ROOT}/${QT_VERSION}/gcc_64/bin:${PATH}
