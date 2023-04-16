#!/bin/sh

# Abort on any error (including if wait-for-it fails).
set -e

# Wait for the backend to be up, if we know where it is.
if [ -n "$NODE_HOST" ]; then
  /usr/src/kryptokrona/wait-for-it.sh "$NODE_HOST:${NODE_PORT:NODE_PORT}"
fi

# Run the main container command.
exec "$@"