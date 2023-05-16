<a href="https://github.com/kryptokrona/kryptokrona">
  <img align="right" width="200" height="120" alt="Kryptokrona" src="resources/kryptokrona.svg">
</a>

# kryptokrona
[![build](https://img.shields.io/github/actions/workflow/status/kryptokrona/kryptokrona/master-ci.yml?branch=master)](https://github.com/kryptokrona/kryptokrona/actions/workflows/master-ci.yml)
[![release](https://img.shields.io/github/v/release/kryptokrona/kryptokrona)](https://img.shields.io/github/v/release/kryptokrona/kryptokrona)
[![license](https://img.shields.io/badge/license-GPLv3-blue.svg)](https://opensource.org/licenses/GPLv3)
[![discord](https://img.shields.io/discord/562673808582901793?label=discord)](https://discord.gg/VTgsTGS9b7)

Kryptokrona is a decentralized blockchain from the Nordic based on CryptoNote, which forms the basis for Monero, among others. CryptoNote is a so-called “application layer” protocol further developed by TurtleCoin that enables things like: private transactions, messages and arbitrary data storage, completely decentralized.

## Table of Contents

- [Development Resources](#development-resources)
- [Getting Help](#getting-help)
- [Reporting Issues](#reporting-issues)
- [Versioning](#versioning)
- [CI/CD](#cicd)
- [Installation](#installation)
  - [Prerequisites](#prerequisites)
  - [Ubuntu](#ubuntu)
    - [Build using GCC](#build-using-gcc)
    - [Build using Clang](#build-using-clang)
  - [Generic Linux](#generic-linux)
    - [Build](#build)
  - [OSX/Apple](#osxapple)
    - [Prerequisites](#prerequisites-1)
    - [Build using GCC](#build-using-gcc-1)
    - [Build using Clang](#build-using-clang-1)
  - [Windows](#windows)
    - [Prerequisites](#prerequisites-2)
    - [Build using Visual C++ 2019](#build-using-visual-c-2019)
    - [Build using Visual C++ 2022](#build-using-visual-c-2022)
  - [Raspberry Pi 3 B+ (AARCH64/ARM64)](#raspberry-pi-3-b-aarch64arm64)
    - [Known working images](#known-working-images)
    - [Build](#build-1)
- [Setup testnet](#setup-testnet)
  - [Install Docker](#install-docker)
  - [Start the orchestration of Docker containers](#start-the-orchestration-of-docker-containers)
  - [Stop all Docker containers](#stop-all-docker-containers)
  - [Start all Docker containers again](#start-all-docker-containers-again)
  - [Removing all data](#removing-all-data)
- [Deploy node](#deploy-node)
- [Checklist before release](#checklist-before-release)
- [Help and Support](#help-and-support)
- [Thanks](#thanks)
- [Copypasta for license when editing files](#copypasta-for-license-when-editing-files)
- [Contributors](#contributors)
- [License](#license)

## Development Resources

- Web: kryptokrona.org
- Docs: https://docs.kryptokrona.org
- GitHub: https://github.com/kryptokrona
- Hugin: Kryptokrona group on Hugin Messenger `33909fb89783fb15b5c2df50ff7107c112c3b232681f77814c024c909c07e932`r
- It is HIGHLY recommended to join our board on Hugin Messenger if you want to contribute to stay up to date on what is happening on the project.
- Twitter: https://twitter.com/kryptokrona

## Getting Help

Are you having trouble with Kryptokrona? We want to help!

- Read through all documentation on our Wiki: https://docs.kryptokrona.org

- If you are upgrading, read the release notes for upgrade instructions and "new and noteworthy" features.

- Ask a question we monitor stackoverflow.com for questions tagged with kryptokrona. You can also chat with the community on Hugin or Discord.

- Report bugs with Kryptokrona at https://github.com/kryptokrona/kryptokrona/issues.

- Join the Discussion and be part of the community at Discord: https://discord.gg/VTgsTGS9b7

## Reporting Issues

Kryptokrona uses GitHub’s integrated issue tracking system to record bugs and feature requests. If you want to raise an issue, please follow the recommendations below:

- Before you log a bug, please search the issue tracker to see if someone has already reported the problem.

- If the issue doesn’t already exist, create a new issue.

- Please provide as much information as possible with the issue report. We like to know the Kryptokrona version, operating system, and JVM version you’re using.

- If you need to paste code or include a stack trace, use Markdown. ``` escapes before and after your text.

- If possible, try to create a test case or project that replicates the problem and attach it to the issue.

## Versioning

Kryptokrona uses [Semantic Versioning Standard 2.0.0](https://semver.org/).

Given a version number MAJOR.MINOR.PATCH, increment the:

- MAJOR version when you make incompatible API changes
- MINOR version when you add functionality in a backwards compatible manner
- PATCH version when you make backwards compatible bug fixes

Additional labels for pre-release and build metadata are available as extensions to the MAJOR.MINOR.PATCH format.

## CI/CD

This project is automatically built and tested using GitHub Actions. We have two pipelines:

- **Kryptokrona Main Pipeline** - This is the pipeline that runs the code merged into our main branch.
- **Kryptokrona Pull Request Pipeline** - This is the pipeline that runs each time a pull request come in so the reviewer has some help evaluating the code status.

The Kryptokrona Main Pipeline do everything the Kryptokrona Pull Request Pipeline does in addition to generate Doxygen and building and publishing a Docker Image to
the project tagged by the project name, owner, repository and short form of commit SHA value.

The purpose of publishing prepared Docker images is to make it faster and easier to deploy a Kryptokrona node/pool.

## Installation

We offer binary images of the latest releases here: https://github.com/kryptokrona/kryptokrona/releases

If you would like to compile yourself, read on.

### Prerequisites

You will need the following packages: boost, cmake (3.8 or higher), make, and git.

You will also need either GCC/G++, or Clang.

If you are using GCC, you will need GCC-11.0 or higher.

If you are using Clang, you will need Clang 6.0 or higher. You will also need libstdc++\-6.0 or higher.

### Ubuntu

#### Build using GCC

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

#### Build using Clang

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

### Generic Linux

#### Build

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

#### Build using GCC

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

#### Build using Clang

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

Note that if you are using x64 you need to set the flag by running cmake with:

`cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 ..`

Or with macOS m1:

`cmake -DCMAKE_OSX_ARCHITECTURES=arm64 ..`

The binaries will be in the `src` folder when you are complete.

- `cd src`
- `./kryptokrona --version`

### Windows

#### Prerequisites

- Install [Visual Studio 2019 Community Edition](https://visualstudio.microsoft.com/vs/older-downloads/)
- Install [Visual Studio 2022 Community Edition](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&channel=Release&version=VS2022&source=VSLandingPage&cid=2030&passive=false)
- When installing Visual Studio, it is **required** that you install **Desktop development with C++**.  Select MSVC v141 Build Tools.
- Install the latest version of [Boost](https://sourceforge.net/projects/boost/files/boost-binaries/1.68.0/boost_1_68_0-msvc-14.1-64.exe/download) - Currently Boost 1.68.

#### Build using Visual C++ 2019

- From the start menu, open 'x64 Native Tools Command Prompt for vs2019'.
- `cd <your_kryptokrona_directory>`
- `mkdir build`
- `cd build`
- `set PATH="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin";%PATH%`
- `cmake -G "Visual Studio 16 2019" -A x64 .. -DBOOST_ROOT=C:/local/boost_1_68_0`

If you have errors on this step about not being able to find the following static libraries, you may need to update your cmake. Open 'Visual Studio Installer' and click 'Update'.

- `MSBuild kryptokrona.sln /p:Configuration=Release /p:PlatformToolset=v141`

#### Build using Visual C++ 2022

- From the start menu, open 'x64 Native Tools Command Prompt for vs2022'.
- `cd <your_kryptokrona_directory>`
- `mkdir build`
- `cd build`
- `set PATH="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin";%PATH%`
- `cmake -G "Visual Studio 17 2022" .. -DBOOST_ROOT=C:/local/boost_1_68_0 `

If you have errors on this step about not being able to find the following static libraries, you may need to update your cmake. Open 'Visual Studio Installer' and click 'Update'.

- `MSBuild kryptokrona.sln /p:Configuration=Release /p:PlatformToolset=v141 /p:Platform="x64" /m`


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

#### Build

- `git clone -b master --single-branch https://github.com/kryptokrona/kryptokrona`
- `cd kryptokrona`
- `mkdir build`
- `cd build`
- `cmake ..`
- `make`

The binaries will be in the `src` folder when you are complete.

- `cd src`
- `./kryptokrona --version`

## Setup testnet

### Install Docker

On Windows or Mac it's enough to install Docker Desktop and we will have everything we need in order to setup. For GNU/Linux however there is a slightly different process. We are going through the steps for doing it on a Ubuntu distribution, it should work on all Debian derived distributions. Read below.

Update our headers:

```bash
sudo apt-get update
```

Installing necessary dependencies for Docker:

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

### Start the orchestration of Docker containers

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

So now the first time when starting the script it will take a while to compile and link all the files (around 15-20 minutes depending on how powerful computer you have). So when it's done you will see a lot of output of the Daemons starting on three nodes. The miner do not have any output on the terminal.

### Stop all Docker containers

So all of these containers running will take up some memory and CPU usage on your system so it could be wise to stop these when not using them. To do that just run the shell script:

- `./teardown-testnet.sh`

This makes sure that we still have the image saved locally so we don't need to build it again when we will start it.

### Start all Docker containers again

Instead of using `setup-testnet.sh` file we will use the file `start-testnet.sh`:

- `./start-testnet.sh`

The difference here is that we will not build the image again and thus has to wait a while. Now this will only take seconds. And now we have our testnet up again!

### Removing all data

When we want to do a full cleanup on our system with Docker we can start the script `remove-testnet.sh`:

- `./remove-testnet.sh`

Now we will remove networks, volumes, images and containers existing on our system. If you after removing everything want to start again. Just use the file `setup-testnet.sh` again.

Check also out the article made by Marcus Cvjeticanin here: https://medium.com/coinsbench/setup-a-testnet-with-kryptokrona-blockchain-41b5f9ffd86

## Deploy node

To deploy your own node, please follow the instructions on our Wiki: [https://docs.kryptokrona.org/guides/kryptokrona/deploy-your-own-full-node](https://docs.kryptokrona.org/guides/kryptokrona/deploy-your-own-full-node)

## Checklist before release

1. Edit the file in `config/version.h.in` and bump up the version.
2. Create PR and wait for review + merge.
3. Create a tag on master `git tag v*.*.*`
4. Push the changes `git push origin <tag_name>` to trigger a new build and to publish to various package managers.

## Help and Support

For questions and support please use the channel #support in [Kryptokrona](https://discord.gg/mkRpVgDubC) Discord server. The issue tracker is for bug reports and feature discussions only.

## Thanks

Cryptonote Developers, Bytecoin Developers, Monero Developers, Forknote Project, TurtleCoin Community, Kryptokrona Developers

## Copypasta for license when editing files

Hi Kryptokrona contributor, thanks for forking and sending back Pull Requests. Extensive docs about contributing are in the works or elsewhere. For now this is the bit we need to get into all the files we touch. Please add it to the top of the files, see [src/cryptonote_config.h](https://github.com/turtlecoin/turtlecoin/commit/28cfef2575f2d767f6e512f2a4017adbf44e610e) for an example.

```
// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.
```

## Contributors

The following contributors have either helped to start this project, have contributed
code, are actively maintaining it (including documentation), or in other ways
being awesome contributors to this project. **We'd like to take a moment to recognize them.**

[<img src="https://github.com/mjovanc.png?size=72" alt="mjovanc" width="72">](https://github.com/mjovanc)
[<img src="https://github.com/f-r00t.png?size=72" alt="f-r00t" width="72">](https://github.com/f-r00t)
[<img src="https://github.com/n9lsjr.png?size=72" alt="n9lsjr" width="72">](https://github.com/n9lsjr)
[<img src="https://github.com/TechyGuy17.png?size=72" alt="TechyGuy17" width="72">](https://github.com/TechyGuy17)


## License

The GPL-3.0 License.
