# this docker file is intended for production usage to setup a node for the 
# kryptokrona blockchain

FROM ubuntu:22.04

COPY . /usr/src/kryptokrona
WORKDIR /usr/src/kryptokrona

# install build dependencies
RUN apt-get update && \
    apt-get install -y \
      build-essential \
      libssl-dev \
      libffi-dev \
      python3-dev \
      gcc-11 \
      g++-11 \
      git \
      cmake \
      librocksdb-dev \
      libboost-all-dev \
      libboost-system1.74.0 \
      libboost-filesystem1.74.0 \
      libboost-thread1.74.0 \
      libboost-date-time1.74.0 \
      libboost-chrono1.74.0 \
      libboost-regex1.74.0 \
      libboost-serialization1.74.0 \
      libboost-program-options1.74.0 \
      libicu70 \
      p7zip-full

# create the build directory
RUN mkdir build
WORKDIR /usr/src/kryptokrona/build

# build and install
RUN cmake -DCMAKE_CXX_FLAGS="-g0 -Os -fPIC -std=gnu++17" .. && make -j$(nproc) --ignore-errors

# create the directory for the daemon files (this can perhaps be removed since we use the volume flag while starting the docker container, but untested!)
RUN mkdir -p src/blockloc

WORKDIR /usr/src/kryptokrona/build/src

# set executable permission on kryptokrona deamon
RUN chmod +x kryptokrona_daemon

EXPOSE 11898

CMD [ "/bin/sh", "-c", "./kryptokrona_daemon --data-dir /usr/src/kryptokrona/build/src/blockloc --p2p-bind-ip 0.0.0.0 --rpc-bind-ip 0.0.0.0 --hide-my-port" ]