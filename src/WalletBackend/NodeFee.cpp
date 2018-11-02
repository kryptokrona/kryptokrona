// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

//////////////////////////////////
#include <WalletBackend/NodeFee.h>
//////////////////////////////////

#include <WalletBackend/ValidateParameters.h>

namespace NodeFee
{

std::tuple<uint64_t, std::string> getNodeFee(const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon)
{
    const uint64_t feeAmount = daemon->feeAmount();

    const std::string feeAddress = daemon->feeAddress();

    if (isValidFee(feeAmount, feeAddress))
    {
        return {feeAmount, feeAddress};
    }

    return {0, std::string()};
}

bool isValidFee(uint64_t feeAmount, std::string feeAddress)
{
    bool allowIntegratedAddresses = false;

    return feeAmount != 0 
        && feeAddress != ""
        && validateAddresses({feeAddress}, allowIntegratedAddresses);
}

std::vector<std::pair<std::string, uint64_t>> appendFeeTransaction(
    const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon,
    std::vector<std::pair<std::string, uint64_t>> transactions)
{
    const auto [feeAmount, feeAddress] = getNodeFee(daemon);

    if (isValidFee(feeAmount, feeAddress))
    {
        transactions.push_back({feeAddress, feeAmount});
    }

    return transactions;
}

} // namespace
