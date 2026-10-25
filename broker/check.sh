#!/bin/sh
# Connects to a TLS broker the way your programs will, with BROKER_ADDRESS,
# BROKER_USER and BROKER_PASSWORD from .env and certs/ca.crt, and says whether
# that works. Run by the tasks "Broker: Check (dev TLS)" and "Broker: Check
# (course)": see README.md.
set -u

out=$(mosquitto_sub -h "$BROKER_ADDRESS" -p 8883 --cafile /certs/ca.crt \
  -u "$BROKER_USER" -P "$BROKER_PASSWORD" -t 'sensors/#' -d -C 1 -W 5 2>&1)

# With -d, mosquitto_sub prints the broker's answer to the login: CONNACK (0)
# means accepted.
if echo "$out" | grep -q "CONNACK (0)"; then
  echo "Connected to $BROKER_ADDRESS:8883 as $BROKER_USER: TLS and login work."
else
  echo "Could not connect to $BROKER_ADDRESS:8883 as $BROKER_USER:"
  echo "$out" | grep -v " sending "
  exit 1
fi
