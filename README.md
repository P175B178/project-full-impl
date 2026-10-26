# Warden

Monitors temperature and humidity on a Raspberry Pi. Reference implementation of the engineering
project for module P175B178 — Modern Application of C++ for Hardware Control.

| Folder | What is in it |
| ------ | ------------- |
| [`device/`](device/README.md) | The program running on the Raspberry Pi |
| [`desktop/`](desktop/README.md) | Warden Console, the desktop program that monitors the devices |
| [`protocol/`](protocol/README.md) | The MQTT message contract both programs follow |
| [`broker/`](broker/README.md) | Local MQTT brokers, and the settings for the course broker |

## Getting Started

1. Open this repository in VS Code.
2. Run **Dev Containers: Reopen in Container** and pick **Device** or **Desktop**. Each opens only
   its own folder; for both, open a second VS Code window. The first **Desktop** build takes a
   while: it downloads Qt and compiles QtMqtt.
3. Press `Ctrl+Shift+B` to build, `F5` to debug.

The brokers run on the host: open the repository in VS Code there, without **Reopen in Container**,
and run the task **Broker: Start (dev)**.
