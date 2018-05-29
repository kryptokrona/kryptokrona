#!/usr/bin/env bash
# rocksteady, turtlecoin developers 2017-2018
# use this installer to clone-and-compile turtlecoin
# supports Ubuntu 18 LTS

sudo apt-get update
sudo apt-get install build-essential python-dev gcc g++ git cmake libboost-all-dev librocksdb-dev -y
git clone https://github.com/turtlecoin/turtlecoin
cd turtlecoin
mkdir build && cd $_
cmake ..
make
