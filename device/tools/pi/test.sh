#!/usr/bin/env bash
set -euo pipefail

# Deploys and runs the unit tests on the Pi over SSH.
# The pi-debug preset builds them; they test warden_core only, so no GPIO,
# LEDs or sensor are needed.
#
# Usage:
#   tools/pi/test.sh
#   tools/pi/test.sh path/to/warden_tests   # a different test binary

: "${PI_USER:?PI_USER not set — copy .env.sample to .env and configure it.}"
: "${PI_HOST:?PI_HOST not set — copy .env.sample to .env and configure it.}"

PORT="${PI_PORT:-22}"
PI="${PI_USER}@${PI_HOST}"
LOCAL_TEST_BIN="${1:-build/pi-debug/tests/warden_tests}"

if [[ ! -f "${LOCAL_TEST_BIN}" ]]; then
  echo "Test binary not found: ${LOCAL_TEST_BIN}" >&2
  echo "Build first: cmake --preset pi-debug && cmake --build --preset pi-debug" >&2
  exit 2
fi

REMOTE_DIR="/home/${PI_USER}/apps/warden/tests"
REMOTE_TEST_BIN="${REMOTE_DIR}/warden_tests"
# Some tests load files from the source tree, which does not exist on the Pi:
# config/warden.toml and the message examples in ../protocol/examples. Copy them
# next to the tests and pass their paths instead.
REMOTE_CFG="${REMOTE_DIR}/warden.toml"
REMOTE_EXAMPLES="${REMOTE_DIR}/protocol-examples"

SSH_OPTS=(-p "${PORT}" -o StrictHostKeyChecking=accept-new)
RSYNC_RSH="ssh -p ${PORT} -o StrictHostKeyChecking=accept-new"

echo "==> Deploying ${LOCAL_TEST_BIN} -> ${PI}:${REMOTE_TEST_BIN}"
ssh "${SSH_OPTS[@]}" "${PI}" "mkdir -p '${REMOTE_DIR}'"
rsync -av -e "${RSYNC_RSH}" "${LOCAL_TEST_BIN}" "${PI}:${REMOTE_TEST_BIN}"
rsync -av -e "${RSYNC_RSH}" config/warden.toml "${PI}:${REMOTE_CFG}"
rsync -av -e "${RSYNC_RSH}" ../protocol/examples/ "${PI}:${REMOTE_EXAMPLES}/"
ssh "${SSH_OPTS[@]}" "${PI}" "chmod +x '${REMOTE_TEST_BIN}'"

echo "==> Running tests on ${PI}..."
exec ssh "${SSH_OPTS[@]}" "${PI}" \
  "WARDEN_CONFIG_FILE='${REMOTE_CFG}' WARDEN_PROTOCOL_EXAMPLES='${REMOTE_EXAMPLES}' '${REMOTE_TEST_BIN}'"
