#!/usr/bin/env bash
set -euo pipefail

# Runs the deployed warden binary on the Pi over SSH.
#
# Usage:
#   tools/pi/run.sh
#   tools/pi/run.sh /path/to/warden.toml  # custom config path on the Pi

: "${PI_USER:?PI_USER not set — copy .env.sample to .env and configure it.}"
: "${PI_HOST:?PI_HOST not set — copy .env.sample to .env and configure it.}"

PORT="${PI_PORT:-22}"
PI="${PI_USER}@${PI_HOST}"
REMOTE_DIR="/home/${PI_USER}/apps/warden"
REMOTE_CFG="${1:-${REMOTE_DIR}/config/warden.toml}"

SSH_OPTS=(-p "${PORT}" -o StrictHostKeyChecking=accept-new)

echo "==> Running ${REMOTE_DIR}/warden on ${PI}..."
# -t gives the remote command a terminal, so Ctrl+C here reaches warden on the
# Pi as SIGINT. Without it, Ctrl+C only ends the local ssh and warden keeps running.
exec ssh -t "${SSH_OPTS[@]}" "${PI}" "${REMOTE_DIR}/warden" --config "${REMOTE_CFG}"
