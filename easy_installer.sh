#!/usr/bin/env bash
# rocksteady, turtlecoin developers 2017-2018
# use this installer to clone-and-compile turtlecoin in one line
# supports Ubuntu 16 LTS

sudo apt-get update
yes "" | sudo apt-get install build-essential python-dev gcc-4.9 g++-4.9 git cmake libboost1.58-all-dev librocksdb-dev
export CXXFLAGS="-std=gnu++11"
git clone https://github.com/turtlecoin/turtlecoin
cd turtlecoin
mkdir build && cd $_
cmake ..
make
