// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "p2p_protocol_types.h"

namespace cryptonote {

class NetNodeConfig {
public:
  NetNodeConfig();
  bool init(const std::string interface, const int port, const int external, const bool localIp,
              const bool hidePort, const std::string dataDir, const std::vector<std::string> addPeers,
              const std::vector<std::string> addExclusiveNodes, const std::vector<std::string> addPriorityNodes,
              const std::vector<std::string> addSeedNodes);

  std::string getP2pStateFilename() const;
  bool getTestnet() const;
  std::string getBindIp() const;
  uint16_t getBindPort() const;
  uint16_t getExternalPort() const;
  bool getAllowLocalIp() const;
  std::vector<PeerlistEntry> getPeers() const;
  std::vector<NetworkAddress> getPriorityNodes() const;
  std::vector<NetworkAddress> getExclusiveNodes() const;
  std::vector<NetworkAddress> getSeedNodes() const;
  bool getHideMyPort() const;
  std::string getConfigFolder() const;

private:
  std::string bindIp;
  uint16_t bindPort;
  uint16_t externalPort;
  bool allowLocalIp;
  std::vector<PeerlistEntry> peers;
  std::vector<NetworkAddress> priorityNodes;
  std::vector<NetworkAddress> exclusiveNodes;
  std::vector<NetworkAddress> seedNodes;
  bool hideMyPort;
  std::string configFolder;
  std::string p2pStateFilename;
  bool testnet;
};

} //namespace nodetool
