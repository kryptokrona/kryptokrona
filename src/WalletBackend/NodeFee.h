// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <NodeRpcProxy/NodeRpcProxy.h>

#include <string>

#include <vector>

namespace NodeFee
{
    std::tuple<uint64_t, std::string> getNodeFee(
        const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon);

    bool isValidFee(uint64_t feeAmount, std::string feeAddress);

    std::vector<std::pair<std::string, uint64_t>> appendFeeTransaction(
        const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon,
        std::vector<std::pair<std::string, uint64_t>> transactions);
}
