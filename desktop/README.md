# Desktop

Warden Console: a Qt desktop program that monitors Warden devices through the MQTT broker.

Everything below runs in the **Desktop** dev container, which opens this folder, `desktop/`: all
paths are relative to it. The program's window shows on the container's own small desktop: open
http://localhost:6080 (password `vscode`) in a browser, or in VS Code via **Simple Browser: Show**.

> [!TIP]
> Throughout this guide, VS Code tasks are run via `Ctrl+Shift+P` → **Tasks: Run Task**.

---

## Local build

**VS Code task:** `Build` (`Ctrl+Shift+B`)

```bash
cmake --preset debug && cmake --build --preset debug
```

**VS Code task:** `Run` — connects to the broker (see [MQTT broker](#mqtt-broker))

```bash
LSAN_OPTIONS=suppressions=lsan.supp:print_suppressions=0 ./build/debug/warden-console
```

**VS Code task:** `Run (simulated)` — two simulated devices instead of a broker's

```bash
LSAN_OPTIONS=suppressions=lsan.supp:print_suppressions=0 ./build/debug/warden-console --simulate
```

Debug builds run with AddressSanitizer and UndefinedBehaviorSanitizer; `lsan.supp` leaves out a
few objects Qt itself keeps until the program ends.

**Debug with breakpoints:**

**VS Code launch configs:** `Debug` (`F5`) and `Debug (simulated)` — build automatically and launch
under GDB.

---

## Tests

**VS Code task:** `Run tests` — also **Tasks: Run Test Task**

```bash
ctest --preset debug
```

---

## Window layout

The window's layout is in Qt Designer forms, `src/app/*.ui`. Open one in Qt Widgets Designer by
double-clicking it in VS Code (Qt extension).

---

## MQTT broker

Run the broker on your computer: open the repository in a VS Code window on the host and run the
task **Broker: Start (dev)** (see [`broker/`](../broker/README.md)). The console connects to
`host.docker.internal:1883` in the dev container (set in `WARDEN_BROKER_HOST`), and to
`localhost:1883` elsewhere.

To try it without a device, publish `protocol/examples/observation-temperature.json` on
`sensors/group-1/temperature` from MQTT Explorer.

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

**VS Code task:** `Workflow: Verify` — formatting, clang-tidy, and Debug and Release builds with
tests, as CI (`.github/workflows/desktop.yml`) runs them

```bash
cmake --preset debug && cmake --build --preset debug --target format-check && cmake --workflow --preset tidy && cmake --build --preset debug && ctest --preset debug && cmake --workflow --preset release
```

Each check also has its own task: `Workflow: Tidy` and `Workflow: Release build and test`.

### Clean build directories

Useful when builds get into a broken state:

**VS Code task:** `Clean all build directories`

```bash
rm -rf build/
```
