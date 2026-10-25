# Engineering Project

Project template for module P175B178 — Modern Application of C++ for Hardware Control.

## Repository Layout

```text
device/       Program running on the Raspberry Pi (milestones 1–2)
desktop/      Qt desktop program (milestone 3)
protocol/     MQTT message contract shared by both programs
broker/       Local MQTT brokers, and the settings for the course broker
.devcontainer/
  device/     Dev container for device/
  desktop/    Dev container for desktop/
```

`device/` and `desktop/` are independent programs, each its own CMake project. They share no
code, only the message contract in `protocol/`.

## Getting Started

1. Open this repository in VS Code
2. Run **Dev Containers: Reopen in Container** (install the
   [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)
   extension if you haven't) and pick **Device** or **Desktop**
3. Press `Ctrl+Shift+B` to build, `F5` to debug

From a terminal in either folder:

```bash
cmake --preset debug          # configure, once
cmake --build --preset debug  # build → build/debug/
```

Each container opens only its own folder (`device/` or `desktop/`), with its own VS Code tasks,
debug configuration and settings in that folder's `.vscode/`. The rest of the repository is still
there, one level up — open `protocol/` or this README with **File > Open File**. To work on both
programs at once, open a second VS Code window in the other container. The first **Desktop**
container build takes a while: it downloads Qt and compiles the QtMqtt module.

The **Desktop** container runs a small Linux desktop for Qt windows: open http://localhost:6080
(password `vscode`) in a browser, or in VS Code via **Simple Browser: Show**. The starter program
opens an empty window there.

## MQTT Broker

[`broker/`](broker/README.md) has local [Mosquitto](https://mosquitto.org/) brokers to develop
against, with [MQTT Explorer](https://mqtt-explorer.com/) to watch them, and the settings for the
course broker. They run on the host: open this repository in a VS Code window there (without
**Reopen in Container**) and run the task **Broker: Start (dev)**.

## Provided Interfaces

`device/include/interfaces/` contains abstract interfaces that define the hardware contract for this
project:

- `Reading.h` — temperature + humidity data struct
- `Sensor.h` — sensor reading with `std::expected` error handling
- `Led.h` — LED color (Green, Orange, Red), shown steadily or blinking
- `Buzzer.h` — short and long beep sequences

These declarations match the hardware and the project requirements. You may extend them — add
namespaces, methods, documentation — but do not rename or remove the existing methods or change
their signatures.

## Programs

- [device/README.md](device/README.md) — the program running on the Raspberry Pi

## Message Contract

`protocol/` describes the MQTT topics and messages both programs use, with one example file per
message. See [protocol/README.md](protocol/README.md).
