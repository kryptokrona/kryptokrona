#!/bin/sh
# This file is meant to be ran during the BOOST build process using a 
# cross-compile toolchain for AARCH64

echo "Setting up user compiler gcc-aarch64: ${HOME}/toolchain/gcc-arm-8.2-2018.08-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu-g++"
echo "using gcc : aarch64 : ${HOME}/toolchain/gcc-arm-8.2-2018.08-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu-g++ ; " >> tools/build/v2/user-config.jam