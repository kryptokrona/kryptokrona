![image](https://user-images.githubusercontent.com/34389545/35821974-62e0e25c-0a70-11e8-87dd-2cfffeb6ed47.png)

### Build Status

Linux + OSX - https://travis-ci.org/ZedPea/turtlecoin.svg?branch=master
Windows - https://ci.appveyor.com/api/projects/status/a1efx1ynae1qpxe4?svg=true

### How To Compile 

#### Ubuntu 16.04+ and MacOS 10.10+

There is a bash installation script for Ubuntu 16.04+ and MacOS 10.10+ which can be used to checkout and build the project from source:

`$ curl -sL "https://raw.githubusercontent.com/turtlecoin/turtlecoin/master/multi_installer.sh" | bash `

On Ubuntu you will be asked for sudo rights to install software. The binaries will be in `./src` after compilation is complete.

This script can be used from inside the git repository to build the project from the checked out source, `./multi_installer.sh`

See the script for more installation details and please consider extending it for your operating system and distribution!


#### Windows 10

##### Prerequisites
- Install [Visual Studio 2017 Community Edition](https://www.visualstudio.com/thank-you-downloading-visual-studio/?sku=Community&rel=15&page=inlineinstall)
- When installing Visual Studio, it is **required** that you install **Desktop development with C++** and the **VC++ v140 toolchain** when selecting features. The option to install the v140 toolchain can be found by expanding the "Desktop development with C++" node on the right. You will need this for the project to build correctly.
- Install [Boost 1.59.0](https://sourceforge.net/projects/boost/files/boost-binaries/1.59.0/), ensuring you download the installer for MSVC 14.

##### Building

- From the start menu, open 'x64 Native Tools Command Prompt for vs2017'.
- `cd <your_turtlecoin_directory>`
- `mkdir build`
- `cd build`
- `cmake -G "Visual Studio 14 Win64" .. -DBOOST_ROOT=D:/Boost/boost_1_59_0` (Or your boost installed dir.)
- `MSBuild ByteCoin.sln /p:Configuration=Release /m`
- If all went well, it will complete successfully, and you will find all your binaries in the '..\build\src\Release' directory.
- Additionally, a `.sln` file will have been created in the `build` directory. If you wish to open the project in Visual Studio with this, you can.


#### Apple

##### Prerequisites

- Install [cmake](https://cmake.org/). See [here](https://stackoverflow.com/questions/23849962/cmake-installer-for-mac-fails-to-create-usr-bin-symlinks) if you are unable call `cmake` from the terminal after installing.
- Install the [boost](http://www.boost.org/) libraries. Either compile boost manually or run `brew install boost`.
- Install XCode and Developer Tools.

##### Building

- `git clone https://github.com/turtlecoin/turtlecoin`
- `cd turtlecoin`
- `mkdir build && cd $_`
- `cmake ..` or `cmake -DBOOST_ROOT=<path_to_boost_install> ..` when building
  from a specific boost install
- `make`

The binaries will be in `./src` after compilation is complete.

#### Thanks
Cryptonote Developers, Bytecoin Developers, Monero Developers, Forknote Project, TurtleCoin Community
