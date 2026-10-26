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

**VS Code task:** `Run` — with two simulated devices, no broker needed

```bash
./build/debug/warden-console
```

**Debug with breakpoints:**

**VS Code launch config:** `Debug` (`F5`) — builds automatically and launches under GDB.

---

## Utilities

### Clean build directories

Useful when builds get into a broken state:

**VS Code task:** `Clean all build directories`

```bash
rm -rf build/
```
