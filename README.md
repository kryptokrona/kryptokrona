![Kryptokrona](kryptokrona.png)

<p>
  <a href="https://github.com/kryptokrona/kryptokrona/actions/workflows/main-ci.yml">
      <img src="https://github.com/kryptokrona/kryptokrona/actions/workflows/main-ci.yml/badge.svg">
  </a>
</p>

Kryptokrona is a decentralized blockchain from the Nordic based on CryptoNote, which forms the basis for Monero, among others. CryptoNote is a so-called “application layer” protocol further developed by TurtleCoin that enables things like: private transactions, messages and arbitrary data storage, completely decentralized.

# Table of Contents

- [Development Resources](#development-resources)
- [Installation](#installation)
  - [How To Compile](#how-to-compile)
    - [Linux](#linux)
      - [Prerequisites](#prerequisites)
      - [Ubuntu, using GCC](#ubuntu-using-gcc)
      - [Ubuntu, using Clang](#ubuntu-using-clang)
      - [Generic Linux](#generic-linux)
    - [OSX/Apple](#osxapple)
      - [Using GCC](#using-gcc)
      - [Prerequisites](#prerequisites-1)
      - [Building](#building)
    - [Using Clang](#using-clang)
      - [Prerequisites](#prerequisites-2)
      - [Building](#building-1)
    - [Windows](#windows)
      - [Prerequisites](#prerequisites-3)
      - [Building](#building-2)
    - [Raspberry Pi 3 B+ (AARCH64/ARM64)](#raspberry-pi-3-b-aarch64arm64)
      - [Known working images](#known-working-images)
      - [Building](#building-3)
- [Setup testnet](#setup-testnet)
  - [Change config](#change-config)
  - [Install Docker](#install-docker)
  - [Start the orchestration of Docker containers](#start-the-orchestration-of-docker-containers)
  - [Stop all Docker containers](#stop-all-docker-containers)
  - [Start all Docker containers again](#start-all-docker-containers-again)
  - [Removing all data](#removing-all-data)
- [Deploy node](#deploy-node)
- [Thanks](#thanks)
- [Copypasta for license when editing files](#copypasta-for-license-when-editing-files)
  - [Contributors](#contributors)
  - [License](#license)

# Development Resources

- Web: kryptokrona.org
- Mail: mjovanc@protonmail.com
- GitHub: https://github.com/kryptokrona
- Hugin: projectdevelopment board on Hugin Messenger
- It is HIGHLY recommended to join our board on Hugin Messenger if you want to contribute to stay up to date on what is happening on the project.

# Installation

We offer binary images of the latest releases here: https://github.com/kryptokrona/kryptokrona/releases

If you would like to compile yourself, read on.

## How To Compile

### Linux

#### Prerequisites

You will need the following packages: boost, cmake (3.8 or higher), make, and git.

You will also need either GCC/G++, or Clang.

If you are using GCC, you will need GCC-11.0 or higher.

If you are using Clang, you will need Clang 6.0 or higher. You will also need libstdc++\-6.0 or higher.

#### Ubuntu, using GCC

If you are using Ubuntu 22.04 LTS GCC11 and C++11 now comes as default and no need to install this.

- `sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y`
- `sudo apt-get update`
- `sudo apt-get install aptitude python3-pip -y`
- `sudo aptitude install -y build-essential g++-11 gcc-11 git libboost-all-dev`
- `sudo pip3 install cmake`
- `export CC=gcc-11`
- `export CXX=g++-11`
- `git clone -b master --single-branch https://github.com/kryptokrona/kryptokrona`
- `cd kryptokrona`
- `mkdir build`
- `cd build`
- `cmake ..`
- `make`

The binaries will be in the `src` folder when you are complete.

- `cd src`
- `./kryptokrona --version`

#### Ubuntu, using Clang

- `sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y`
- `wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -`

You need to modify the below command for your version of ubuntu - see https://apt.llvm.org/

- Ubuntu 14.04 (Trusty)

* `sudo add-apt-repository "deb https://apt.llvm.org/trusty/ llvm-toolchain-trusty 6.0 main"`

- Ubuntu 16.04 (Xenial)

* `sudo add-apt-repository "deb https://apt.llvm.org/xenial/ llvm-toolchain-xenial 6.0 main"`

- Ubuntu 18.04 (Bionic)

* `sudo add-apt-repository "deb https://apt.llvm.org/bionic/ llvm-toolchain-bionic 6.0 main"`

* `sudo apt-get update`
* `sudo apt-get install aptitude -y`
* `sudo aptitude install -y -o Aptitude::ProblemResolver::SolutionCost='100*canceled-actions,200*removals' build-essential clang-6.0 libstdc++-7-dev git libboost-all-dev python-pip`
* `sudo pip install cmake`
* `export CC=clang-6.0`
* `export CXX=clang++-6.0`
* `git clone -b master --single-branch https://github.com/kryptokrona/kryptokrona`
* `cd kryptokrona`
* `mkdir build`
* `cd build`
* `cmake ..`
* `make`

The binaries will be in the `src` folder when you are complete.

- `cd src`
- `./kryptokrona --version`

#### Generic Linux

Ensure you have the dependencies listed above.

If you want to use clang, ensure you set the environment variables `CC` and `CXX`.
See the ubuntu instructions for an example.

- `git clone -b master --single-branch https://github.com/kryptokrona/kryptokrona`
- `cd turtlecoin`
- `mkdir build`
- `cd build`
- `cmake ..`
- `make`

The binaries will be in the `src` folder when you are complete.

- `cd src`
- `./kryptokrona --version`

### OSX/Apple

#### Prerequisites

- Install XCode and Developer Tools.

#### Using GCC

##### Building

If using M1 chip, switch gcc@8 to gcc@11 when installing through brew.

- `which brew || /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
- `brew install --force cmake boost llvm gcc@8`
- `export CC=gcc-8`
- `export CXX=g++-8`
- `git clone -b master --single-branch https://github.com/kryptokrona/kryptokrona`
- `cd kryptokrona`
- `mkdir build`
- `cd build`
- `cmake ..`
- `make`

The binaries will be in the `src` folder when you are complete.

- `cd src`
- `./kryptokrona --version`

#### Using Clang

##### Building

- `which brew || /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
- `brew install --force cmake boost llvm`
- `export CC=/usr/local/opt/llvm/bin/clang`
- `export CXX=/usr/local/opt/llvm/bin/clang++`
- `git clone -b master --single-branch https://github.com/kryptokrona/kryptokrona`
- `cd kryptokrona`
- `mkdir build`
- `cd build`
- `cmake ..`
- `make`

The binaries will be in the `src` folder when you are complete.

- `cd src`
- `./kryptokrona --version`

### Windows

#### Prerequisites

- Install [Visual Studio 2017 Community Edition](https://www.visualstudio.com/thank-you-downloading-visual-studio/?sku=Community&rel=15&page=inlineinstall)
- When installing Visual Studio, it is **required** that you install **Desktop development with C++**
- Install the latest version of [Boost](https://bintray.com/boostorg/release/download_file?file_path=1.68.0%2Fbinaries%2Fboost_1_68_0-msvc-14.1-64.exe) - Currently Boost 1.68.

#### Building

- From the start menu, open 'x64 Native Tools Command Prompt for vs2017'.
- `cd <your_kryptokrona_directory>`
- `mkdir build`
- `cd build`
- `set PATH="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin";%PATH%`
- `cmake -G "Visual Studio 15 2017 Win64" .. -DBOOST_ROOT=C:/local/boost_1_68_0`

If you have errors on this step about not being able to find the following static libraries, you may need to update your cmake. Open 'Visual Studio Installer' and click 'Update'.

- `MSBuild kryptokrona.sln /p:Configuration=Release /m`

The binaries will be in the `src/Release` folder when you are complete.

- `cd src`
- `cd Release`
- `kryptokrona.exe --version`

### Raspberry Pi 3 B+ (AARCH64/ARM64)

The following images are known to work. Your operation system image **MUST** be 64 bit.

#### Known working images

- https://github.com/Crazyhead90/pi64/releases
- https://fedoraproject.org/wiki/Architectures/ARM/Raspberry_Pi#aarch64_supported_images_for_Raspberry_Pi_3
- https://archlinuxarm.org/platforms/armv8/broadcom/raspberry-pi-3

Once you have a 64 bit image installed, setup proceeds the same as any Linux distribution. Ensure you have at least 2GB of ram, or the build is likely to fail. You may need to setup swap space.

#### Building

- `git clone -b master --single-branch https://github.com/kryptokrona/kryptokrona`
- `cd kryptokrona`
- `mkdir build`
- `cd build`
- `cmake ..`
- `make`

The binaries will be in the `src` folder when you are complete.

- `cd src`
- `./kryptokrona --version`

# Setup testnet

## Change config

Before we start we just need to change slight a bit on the `CryptoNoteConfig.h` header file with some constants so we don't use our main net to test on.

Open up `src/config/CryptoNoteConfig.h`

Then we need to change the constants **P2P_DEFAULT_PORT** and **RPC_DEFAULT_PORT** to:

```cpp
const int      P2P_DEFAULT_PORT                              =  11898;
const int      RPC_DEFAULT_PORT                              =  11899;
```

And put some different letter or number in one of these **CRYPTONOTE_NETWORK** uuids:

```cpp
const static   boost::uuids::uuid CRYPTONOTE_NETWORK         =
{
    {  0xf1, 0x4c, 0xb8, 0xc8, 0xb2, 0x56, 0x45, 0x2e, 0xee, 0xf0, 0xb4, 0x99, 0xab, 0x71, 0x6c, 0xcc  }
};
```

And also we need to comment out seed nodes:

```cpp
const char* const SEED_NODES[] = {
  // "68.183.214.93:11897",//pool1
  // "5.9.250.93:11898"//techy
};
```

Now we are good to go to start with Docker. So if we want to setup our own testnet locally on our computer we will need to install Docker on our computer.

## Install Docker

On Windows or Mac it's enough to install Docker Desktop and we will have everything we need in order to setup. For GNU/Linux however there is a slight different process. We are going through the steps for doing it on a Ubuntu distribution, it should work on all Debian derived distributions. Read below.

Update our headers:

```bash
sudo apt-get update
```

Installing neccessary dependencies for Docker:

```bash
sudo apt-get -y install \
    ca-certificates \
    curl \
    gnupg \
    lsb-release
```

Setting up the keyring:

```bash
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
```

Updating our sources.list for be able to download Docker:

```bash
echo \
"deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu \
$(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
```

Installing Docker and Docker Compose:

```bash
sudo apt-get update
sudo apt-get install -y docker-ce docker-ce-cli containerd.io docker-compose
```

## Start the orchestration of Docker containers

So now we have everything in place in order for us to build and orchestrate up a local testnet. We do not need to install all dependencies mentioned before this section of Docker on your computer. When we start the process of setting up the testnet we will build a Docker Image that installs everything for us automatically.

So to start from scratch we will use the shell script `setup-testnet.sh` but before that we must make sure that all our shell scripts that we need are executable on our system. To make the shell scripts executable:

```bash
sudo chmod +x setup-testnet.sh \
              remove-testnet.sh \
              start-testnet.sh \
              teardown-testnet.sh
```

To start:

- `./setup-testnet.sh`

So now the first time when starting the script it will take a while to compile and link all the files (around 15-20 minutes dependening on how powerful computer you have). So when it's done you will see a lot of output of the Daemons starting on three nodes. The miner do not have any output on the terminal.

## Stop all Docker containers

So all of these containers running will take up some memory and CPU usage on your system so it could be wise to stop these when not using them. To do that just run the shell script:

- `./teardown-testnet.sh`

This makes sure that we still have the image saved locally so we don't need to build it again when we will start it.

## Start all Docker containers again

Instead of using `setup-testnet.sh` file we will use the file `start-testnet.sh`:

- `./start-testnet.sh`

The difference here is that we will not build the image again and thus has to wait a while. Now this will only take seconds. And now we have our testnet up again!

## Removing all data

When we want to do a full cleanup on our system with Docker we can start the script `remove-testnet.sh`:

- `./remove-testnet.sh`

Now we will remove networks, volumes, images and containers existing on our system. If you after removing everything want to start again. Just use the file `setup-testnet.sh` again.

# Deploy node

Before you start you should have set a DNS record of type A to the IP address of your VPS. Then you should use this domain in the shell file below.

To deploy, login to your VPS and copy the file in `deploy/setup-node.sh` to it and make it executable by:

- `sudo chmod +x setup-node.sh`

Edit the two constant values to your values:

```sh
DOMAIN="example.com"
EMAIL="foo@bar.com"
```

Now run the file:

- `./setup-node.sh`

Now this script will take care of basically everything. Just sit back and relax and grab a ☕.

# Thanks

Cryptonote Developers, Bytecoin Developers, Monero Developers, Forknote Project, TurtleCoin Community

# Copypasta for license when editing files

Hi kryptokrona contributor, thanks for forking and sending back Pull Requests. Extensive docs about contributing are in the works or elsewhere. For now this is the bit we need to get into all the files we touch. Please add it to the top of the files, see [src/CryptoNoteConfig.h](https://github.com/turtlecoin/turtlecoin/commit/28cfef2575f2d767f6e512f2a4017adbf44e610e) for an example.

```
// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, Kryptokrona
//
// Please see the included LICENSE file for more information.
```

# Contributors

The following contributors have either helped to start this project, have contributed
code, are actively maintaining it (including documentation), or in other ways
being awesome contributors to this project. **We'd like to take a moment to recognize them.**

[<img src="https://github.com/mjovanc.png?size=72" alt="mjovanc" width="72">](https://github.com/mjovanc)

# License

The license is GPL-3.0 License.
