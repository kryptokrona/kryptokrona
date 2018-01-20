# Run with docker run -it
# run screen and then start the daemon and simplewallet in /usr/src/app/turtlecoin/build
# remember to backup your wallet.dat and blockchain db with docker export
# Pull base image.
FROM ubuntu:16.04

# Install.
RUN apt-get update && \
apt-get install -y screen build-essential python-dev gcc-4.9 g++-4.9 git cmake libboost1.58-all-dev librocksdb-dev

# Create app directory
RUN mkdir -p /usr/src/app
WORKDIR /usr/src/app

# Clone repo and build
RUN export CXXFLAGS="-std=gnu++11" && \
git clone https://github.com/turtlecoin/turtlecoin /usr/src/app/turtlecoin && \
mkdir build

WORKDIR /usr/src/app/turtlecoin/build
RUN cmake ..  && make

# Set environment variables.
ENV HOME /root

# Define working directory.
WORKDIR /root

# Define default command.
CMD ["bash"]
