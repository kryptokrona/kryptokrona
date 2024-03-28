# Kryptokrona
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
