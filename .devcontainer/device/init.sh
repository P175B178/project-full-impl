#!/bin/sh
if [ ! -f device/.env ]; then
  cp device/.env.sample device/.env
  echo "Created device/.env from device/.env.sample — fill in your Pi connection details."
fi
