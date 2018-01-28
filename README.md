![TurtleCoin](https://user-images.githubusercontent.com/34389545/34746835-428ac812-f55b-11e7-8e78-d342ac45d904.png)

### How To Compile

#### Ubuntu 16.xx LTS
- `sudo apt-get update`
- `sudo apt-get -y install build-essential python-dev gcc-4.9 g++-4.9 git cmake libboost1.58-all-dev librocksdb-dev`
- `export CXXFLAGS="-std=gnu++11"`
- `git clone https://github.com/turtlecoin/turtlecoin`
- `cd turtlecoin`
- `mkdir build && cd $_`
- `cmake ..`
- `make`

#### Windows 10
- `TODO: Windows 10 fuckery and magic`

#### Apple

##### Prerequisites

- Install [cmake](https://cmake.org/). See
  [here](https://stackoverflow.com/questions/23849962/cmake-installer-for-mac-fails-to-create-usr-bin-symlinks)
  if you are unable call `cmake` from the terminal after installing.
- Install the [boost](http://www.boost.org/) libraries. Either compile boost
  manually or run `brew install boost`.
- Install Xcode and Developer Tools.

##### Building

- `git clone https://github.com/turtlecoin/turtlecoin`
- `cd turtlecoin`
- `mkdir build && cd $_`
- `cmake ..` or `cmake -DBOOST_ROOT=<path_to_boost_install> ..` when building
  from a specific boost install
- `make`

The binaries will be in `./src` after compilation is complete.

#### Thanks
Cryptonote Developers, Bytecoin Developers, Forknote Project, TurtleCoin Community
