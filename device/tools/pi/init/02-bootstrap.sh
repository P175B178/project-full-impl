#!/usr/bin/env bash
set -euo pipefail

# ONE-TIME: Sets up the Pi: installs and configures what warden needs.
# Safe to run multiple times — all steps are idempotent.
#
# Usage:
#   tools/pi/init/02-bootstrap.sh

: "${PI_USER:?PI_USER not set — copy .env.sample to .env and configure it.}"
: "${PI_HOST:?PI_HOST not set — copy .env.sample to .env and configure it.}"

PORT="${PI_PORT:-22}"
PI="${PI_USER}@${PI_HOST}"

echo "==> Bootstrapping Raspberry Pi: ${PI} (port ${PORT})"

ssh -p "${PORT}" -o StrictHostKeyChecking=accept-new "${PI}" bash -s <<'EOF'
set -euo pipefail

echo "==> Updating apt index"
sudo apt-get update

echo "==> Installing runtime dependencies"
sudo apt-get install -y --no-install-recommends \
  ca-certificates \
  rsync

echo "==> Creating app directory"
mkdir -p "$HOME/apps/warden/config"

echo "==> Done"
uname -m
EOF

echo "Bootstrap complete."
