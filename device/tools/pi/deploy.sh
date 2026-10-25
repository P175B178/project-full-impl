#!/usr/bin/env bash
set -euo pipefail

# Deploys the warden binary and config to the Pi over SSH.
#
# Usage:
#   tools/pi/deploy.sh                        # deploy build/pi-debug/warden
#   tools/pi/deploy.sh build/pi-release/warden

: "${PI_USER:?PI_USER not set — copy .env.sample to .env and configure it.}"
: "${PI_HOST:?PI_HOST not set — copy .env.sample to .env and configure it.}"

PORT="${PI_PORT:-22}"
PI="${PI_USER}@${PI_HOST}"
REMOTE_APPS="/home/${PI_USER}/apps"
REMOTE_DIR="${REMOTE_APPS}/warden"
LOCAL_BIN="${1:-build/pi-debug/warden}"

if [[ ! -f "${LOCAL_BIN}" ]]; then
  echo "Binary not found: ${LOCAL_BIN}" >&2
  exit 2
fi

SSH_OPTS=(-p "${PORT}" -o StrictHostKeyChecking=accept-new)
RSYNC_RSH="ssh -p ${PORT} -o StrictHostKeyChecking=accept-new"

echo "==> Deploying ${LOCAL_BIN} -> ${PI}:${REMOTE_DIR}/warden"
ssh "${SSH_OPTS[@]}" "${PI}" "mkdir -p '${REMOTE_DIR}/config'"
rsync -av -e "${RSYNC_RSH}" "${LOCAL_BIN}" "${PI}:${REMOTE_DIR}/warden"
rsync -av -e "${RSYNC_RSH}" config/ "${PI}:${REMOTE_DIR}/config/"
ssh "${SSH_OPTS[@]}" "${PI}" "chmod +x '${REMOTE_DIR}/warden'"
# The group's password: readable by its owner only.
ssh "${SSH_OPTS[@]}" "${PI}" "if [ -f '${REMOTE_DIR}/config/mqtt-password' ]; then chmod 600 '${REMOTE_DIR}/config/mqtt-password'; fi"

# The brokers' CA certificates, where warden.toml's relative ca_file finds them:
# ~/apps on the Pi is laid out like the repository, ~/apps/warden being device/.
for setup in dev-tls course; do
  ca="../broker/${setup}/certs/ca.crt"
  if [[ -f "${ca}" ]]; then
    ssh "${SSH_OPTS[@]}" "${PI}" "mkdir -p '${REMOTE_APPS}/broker/${setup}/certs'"
    rsync -av -e "${RSYNC_RSH}" "${ca}" "${PI}:${REMOTE_APPS}/broker/${setup}/certs/ca.crt"
  fi
done

echo "==> Deploy complete."
