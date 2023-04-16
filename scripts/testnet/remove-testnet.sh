#!/bin/sh
# This file is intended to remove everything on your computer with docker images, networks,
# containers and volumes. 
# If you have any other docker containers running beforehand or images that you want to keep. 
# Don't use this script! USE WITH CAUTION!

docker stop $(docker ps -qa); docker rm $(docker ps -qa); docker rmi -f $(docker images -qa); docker volume rm $(docker volume ls -q); docker network rm $(docker network ls -q)
