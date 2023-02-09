#!/bin/sh

# before running this shell script ensure you have docker and docker compose setup on your computer.
# as well as make sure that docker is running otherwise you will get a lot of errors.

# build the docker image for the kryptokrona daemon, miner and xkrwallet etc 
# (general build used for many purposes)

# uncomment the line below if you _need_ to compile it locally if you made some changes that is
# not pushed to master 

#docker build -t kryptokrona/kryptokrona-testnet -f Dockerfile.test .

# pull latest testnet image (uncomment this if you want to build yourself)
docker pull ghcr.io/kryptokrona/kryptokrona-testnet:latest

# start the testnet script
./start-testnet.sh 