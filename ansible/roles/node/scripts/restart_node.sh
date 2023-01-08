#!/bin/bash
# script file used to restart a node if it gets stuck

docker stop kryptokrona-node
docker rm -f kryptokrona-node
docker run -d --network host --name kryptokrona-node -v /root/.kryptokrona:/root/.kryptokrona ghcr.io/kryptokrona/kryptokrona:e16f2fa /bin/sh -c '/usr/src/kryptokrona/start.sh'