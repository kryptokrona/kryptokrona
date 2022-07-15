#!/bin/sh

# before running this shell script ensure you have docker and docker compose setup on your computer.
# as well as make sure that docker is running otherwise you will get a lot of errors.

# build the docker image for the kryptokrona daemon, miner and xkrwallet etc 
# (general build used for many purposes)
docker build -t kryptokrona/kryptokrona -f Dockerfile.test .

# start the testnet script
./start-testnet.sh 