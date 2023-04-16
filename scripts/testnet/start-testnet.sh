#!/bin/sh

# orchestrate up the testnet with docker compose
# before up, add -d flag to avoid output on terminal
docker-compose up

# if you use the -d flag on previous command uncomment this line below
#docker ps