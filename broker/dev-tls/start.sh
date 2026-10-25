#!/bin/sh
# Starts the broker (compose.yaml runs this instead of the image's own
# start). The files it needs come from Moodle: see README.md.
set -eu
cd /mosquitto/config

missing=""
for file in certs/ca.crt certs/server.crt certs/server.key passwd; do
  [ -e "$file" ] || missing="$missing $file"
done
if [ -n "$missing" ]; then
  echo "Missing in broker/dev-tls/:$missing. Copy them from Moodle, then restart."
  exit 1
fi

# Mosquitto runs as user mosquitto and must be able to read these files, whoever
# owns the copies from Moodle and whatever their permissions are. It wants its
# password file readable by no one else, too. So it reads copies made inside the
# container, owned by it.
mkdir -p /mosquitto/certs
for file in ca.crt server.crt server.key; do
  install -o mosquitto -g mosquitto -m 0600 "certs/$file" "/mosquitto/certs/$file"
done
install -o mosquitto -g mosquitto -m 0700 passwd /mosquitto/passwd

exec /docker-entrypoint.sh /usr/sbin/mosquitto -c /mosquitto/config/mosquitto.conf
