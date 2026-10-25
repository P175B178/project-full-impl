# Warden

Monitors temperature and humidity via a DHT22 sensor on a Raspberry Pi, and
signals the state through the Pi's built-in LEDs and a buzzer.

Everything below runs in the **Device** dev container, which opens this folder, `device/`: all
paths are relative to it.

> [!TIP]
> Throughout this guide, VS Code tasks are run via `Ctrl+Shift+P` → **Tasks: Run Task**.

---

## Local build

**VS Code task:** `Local: Build` (`Ctrl+Shift+B`)

```bash
cmake --preset debug && cmake --build --preset debug
```

**VS Code task:** `Local: Run (simulated)` — with a simulated sensor

```bash
./build/debug/warden -c config/warden.toml
```

Debug builds run with AddressSanitizer and UndefinedBehaviorSanitizer.

**Debug with breakpoints:**

**VS Code launch config:** `Local: Debug` (`F5`) — builds automatically and launches under GDB.

---

## Tests

**VS Code task:** `Local: Run tests` — also **Tasks: Run Test Task**

```bash
ctest --preset debug
```

---

## Configuration

`warden` reads `/etc/warden/warden.toml`, where an installed `warden` looks for it, or the file
given with `-c`/`--config` (`--help` lists all options). `config/warden.toml` documents every
setting. A misspelled setting falls back to its default, so check the startup log after editing the
file.

For your own settings, such as your computer's LAN address, copy it to a name ending in
`.local.toml`, which `.gitignore` covers:

```bash
cp config/warden.toml config/warden.local.toml
./build/debug/warden -c config/warden.local.toml
```

---

## MQTT broker

Run the broker on your computer: open the repository in a VS Code window on the host and run the
task **Broker: Start (dev)** (see [`broker/`](../broker/README.md)).

Start it before `warden`, which stops if it cannot reach the broker.

The broker is set in `warden.toml`, under `[mqtt]`. From the Pi, set `host` to this computer's LAN
address (`ipconfig` on Windows, `ip addr` on Linux).

---

## Raspberry Pi

### One-time setup

1. Set your Pi's details in `.env`, created from `.env.sample` on the first container start and
   never committed, then reopen the container (**Dev Containers: Reopen in Container**):

   ```bash
   PI_HOST=<Pi IP address>
   PI_USER=<your username on the Pi>
   PI_PASSWORD=<your Pi password>
   PI_PORT=22
   ```

2. Install your SSH key on the Pi: **VS Code task:** `Pi init: 1. SSH key`
   (`tools/pi/init/01-setup-ssh.sh`).

3. Set up what `warden` needs on the Pi (safe to run again): **VS Code task:**
   `Pi init: 2. Bootstrap` (`tools/pi/init/02-bootstrap.sh`). Reboot the Pi if it asks.

### Build

**VS Code task:** `Pi: Build` — debug, with the tests

```bash
cmake --preset pi-debug && cmake --build --preset pi-debug
```

**VS Code task:** `Pi: Build (Release)`

```bash
cmake --preset pi-release && cmake --build --preset pi-release
```

### Deploy and run

**VS Code task:** `Pi: Run` — builds, deploys and runs

```bash
cmake --preset pi-debug && cmake --build --preset pi-debug
tools/pi/deploy.sh
tools/pi/run.sh   # Ctrl+C stops it; or: tools/pi/run.sh <config file on the Pi>
```

**VS Code task:** `Pi: Deploy` — without building

```bash
tools/pi/deploy.sh                           # build/pi-debug/warden
tools/pi/deploy.sh build/pi-release/warden   # or the release build
```

**VS Code task:** `Pi: Run tests` — builds, and runs the tests on the Pi

```bash
cmake --preset pi-debug && cmake --build --preset pi-debug
tools/pi/test.sh
```

### Remote debugging

**VS Code launch config:** `Pi: Remote Debug` (`F5`, picked in the debug dropdown) — starts
gdbserver on the Pi (task `Pi: Start gdbserver`, `tools/pi/debug.sh`) and connects to it. Run
`Pi: Build` and `Pi: Deploy` first, so the program on the Pi matches the debug symbols.

### Shell

**VS Code task:** `Pi: SSH`

```bash
tools/pi/connect.sh
```

---

## Packaging a release

**VS Code task:** `Workflow: Package` — an optimized Pi build, packaged as `.deb` and `.tar.gz` in
`package/`

```bash
rm -rf package && cmake --workflow --preset package
```

Install the `.deb` on the Pi:

```bash
scp package/warden_1.0.0_arm64.deb "$PI_USER@$PI_HOST:"   # in the dev container
sudo apt install ./warden_1.0.0_arm64.deb                 # on the Pi
```

It installs `/usr/bin/warden` and its config, `/etc/warden/warden.toml`; upgrading keeps your
changes to the config.

CI builds the package too: download the `.deb` from the run's summary page on GitHub.

---

## Utilities

### Formatting

clang-format formats C++ files on save in VS Code. The pre-commit hook in `.githooks/` at the
repository root stops a commit whose staged C++ files are not formatted, and formats those without
unstaged changes for you to review and stage. The dev container sets it up; elsewhere, run once:
`git config core.hooksPath .githooks`.

**VS Code task:** `Format all sources`

```bash
cmake --preset debug && cmake --build --preset debug --target format
```

### Verify before committing

**VS Code task:** `Workflow: Verify` — formatting, clang-tidy, Debug and Release builds with tests,
and clang-tidy on the Pi drivers, as CI (`.github/workflows/device.yml`) runs them

```bash
cmake --preset debug && cmake --build --preset debug --target format-check && cmake --workflow --preset tidy && cmake --build --preset debug && ctest --preset debug && cmake --workflow --preset release && cmake --workflow --preset tidy-hardware
```

Each check also has its own task: `Workflow: Tidy`, `Workflow: Tidy hardware drivers` and
`Workflow: Release build and test`.

### Clean build directories

Useful when builds get into a broken state:

**VS Code task:** `Clean all build directories`

```bash
rm -rf build/
```
