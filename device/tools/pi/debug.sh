#!/usr/bin/env bash
set -euo pipefail

# Starts gdbserver on the Pi for remote debugging from VS Code.
#
# Usage:
#   tools/pi/debug.sh
#
# Keep this terminal open while debugging. VS Code connects via the
# "Pi: Remote Debug" launch configuration.

: "${PI_USER:?PI_USER not set — copy .env.sample to .env and configure it.}"
: "${PI_HOST:?PI_HOST not set — copy .env.sample to .env and configure it.}"

PORT="${PI_PORT:-22}"
DEBUG_PORT=2345 # must match miDebuggerServerAddress in .vscode/launch.json
PI="${PI_USER}@${PI_HOST}"
REMOTE_BIN="/home/${PI_USER}/apps/warden/warden"
REMOTE_CFG="/home/${PI_USER}/apps/warden/config/warden.toml"

SSH_OPTS=(-p "${PORT}" -o StrictHostKeyChecking=accept-new)

echo "==> Checking if debug port ${DEBUG_PORT} is already in use..."
PORT_IN_USE=$(
  ssh "${SSH_OPTS[@]}" "${PI}" "
    command -v lsof >/dev/null 2>&1 &&
    lsof -nP -iTCP:${DEBUG_PORT} -sTCP:LISTEN >/dev/null 2>&1 &&
    echo yes || echo no
  "
)

if [[ "${PORT_IN_USE}" == "yes" ]]; then
  echo "==> Port ${DEBUG_PORT} in use — killing existing process..."
  ssh "${SSH_OPTS[@]}" "${PI}" \
    "lsof -t -iTCP:${DEBUG_PORT} -sTCP:LISTEN | xargs -r kill -9"
fi

echo "==> Starting gdbserver on port ${DEBUG_PORT}..."
ssh "${SSH_OPTS[@]}" "${PI}" \
  "gdbserver 0.0.0.0:${DEBUG_PORT} '${REMOTE_BIN}' --config '${REMOTE_CFG}'" &
DEBUG_PID=$!

# Wait until gdbserver is accepting connections
READY=false
for _ in $(seq 1 15); do
  if (exec 3<>/dev/tcp/${PI_HOST}/${DEBUG_PORT}) 2>/dev/null; then
    READY=true
    sleep 1
    break
  fi
  if ! kill -0 ${DEBUG_PID} 2>/dev/null; then
    echo "Error: gdbserver exited unexpectedly." >&2
    exit 1
  fi
  sleep 1
done

if [[ "${READY}" != "true" ]]; then
  echo "Error: gdbserver did not accept connections on port ${DEBUG_PORT} within 15 s." >&2
  kill ${DEBUG_PID} 2>/dev/null || true
  exit 1
fi

echo "==> Debug server ready. Leave this terminal open."
wait ${DEBUG_PID}
