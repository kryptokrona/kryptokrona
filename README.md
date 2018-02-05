![TurtleCoin](https://user-images.githubusercontent.com/34389545/34746835-428ac812-f55b-11e7-8e78-d342ac45d904.png)

### How To Compile

#### Ubuntu 16.04+ and MacOS 10.10+

There is a bash installation script for Ubuntu 16.04+ and MacOS 10.10+ which can be used to checkout and build the project from source:

`$ curl -sL "https://raw.githubusercontent.com/turtlecoin/turtlecoin/master/multi_installer.sh" | bash `

On Ubuntu you will be asked for sudo rights to install software. The binaries will be in `./src` after compilation is complete.

This script can be used from inside the git repository to build the project from the checked out source, `./multi_installer.sh`

See the script for more installation details and please consider extending it for your operating system and distribution!


#### Windows 10

##### Prerequisites
- Install [Visual Studios 2017 Community Edition](https://www.visualstudio.com/thank-you-downloading-visual-studio/?sku=Community&rel=15&page=inlineinstall)
- When installing Visual Studios, it is absolutely important you install C++ capabilities, and the vc++ v140 toolchain when selecting features. You will need this for cmake, MSBuild and other commands.
- Install [Boost 1.59.0](https://sourceforge.net/projects/boost/files/boost-binaries/1.59.0/), ensure to download the installer for  MSVC 14.

##### Building

- Use the start menu or similar to open 'x64 Native Tools Command Prompt for vs2017' command prompt.
- `cd <your_turtlecoin_directory>`
- `mkdir build`
- `cd build`
- `cmake -G "Visual Studio 14 Win64" .. -DBOOST_ROOT=D:/Boost/boost_1_59_0` (Or your boost installed dir.)
- `MSBuild ByteCoin.sln /p:Configuration=Release`
- At this point, this will create a .sln file in the 'build' directory. Open this .sln in Visual Studios 2017 and click 'Build Solution' under the 'Build' Menu Item.
- If all went well, it will complete successfully, and you will find all your binaries in the '..\build\src\Debug' directory, or the '..\build\src\Release' directory if you built with release enabled.


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
Cryptonote Developers, Bytecoin Developers, Forknote Project, TurtleCoin Community


### Thanks
Cryptonote Developers, Bytecoin Developers, Forknote Project, TurtleCoin Community
