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
./build/debug/warden
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
tests, as CI (`.github/workflows/device.yml`) runs them

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
