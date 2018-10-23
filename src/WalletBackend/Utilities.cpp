// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

////////////////////////////////////
#include <WalletBackend/Utilities.h>
////////////////////////////////////

#include <Common/Base58.h>

#include <config/CryptoNoteConfig.h>

#include <CryptoNoteCore/CryptoNoteBasicImpl.h>
#include <CryptoNoteCore/CryptoNoteTools.h>

namespace Utilities
{

/* Will throw an exception if the addresses are invalid. Please check they
   are valid before calling this function. (e.g. use validateAddresses)
   
   Please note this function does not accept integrated addresses. Please
   extract the payment ID from them before calling this function. */
std::vector<Crypto::PublicKey> addressesToViewKeys(const std::vector<std::string> addresses)
{
    std::vector<Crypto::PublicKey> viewKeys;

    CryptoNote::AccountPublicAddress parsedAddress;

    uint64_t prefix;

    for (const auto &address : addresses)
    {
        /* Failed to parse */
        if (!parseAccountAddressString(prefix, parsedAddress, address))
        {
            throw std::invalid_argument("Address is not valid!");
        }

        /* Incorrect prefix */
        if (prefix != CryptoNote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX)
        {
            throw std::invalid_argument("Address is not valid!");
        }

        viewKeys.push_back(parsedAddress.viewPublicKey);
    }

    return viewKeys;
}

std::vector<Crypto::PublicKey> addressesToSpendKeys(const std::vector<std::string> addresses)
{
    std::vector<Crypto::PublicKey> spendKeys;

    CryptoNote::AccountPublicAddress parsedAddress;

    uint64_t prefix;

    for (const auto &address : addresses)
    {
        /* Failed to parse */
        if (!parseAccountAddressString(prefix, parsedAddress, address))
        {
            throw std::invalid_argument("Address is not valid!");
        }

        /* Incorrect prefix */
        if (prefix != CryptoNote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX)
        {
            throw std::invalid_argument("Address is not valid!");
        }

        spendKeys.push_back(parsedAddress.spendPublicKey);
    }

    return spendKeys;
}

/* Assumes the address is valid */
std::tuple<Crypto::PublicKey, Crypto::PublicKey> addressToKeys(const std::string address)
{
    Crypto::PublicKey publicSpendKey = addressesToSpendKeys({address})[0];
    Crypto::PublicKey publicViewKey = addressesToViewKeys({address})[0];

    return {publicSpendKey, publicViewKey};
}

uint64_t getTransactionSum(const std::vector<std::pair<std::string, uint64_t>> destinations)
{
    uint64_t amountSum = 0;

    /* TODO: Overflow */
    for (const auto & [destination, amount] : destinations)
    {
        amountSum += amount;
    }

    return amountSum;
}

/* Assumes address is valid */
std::tuple<std::string, std::string> extractIntegratedAddressData(const std::string address)
{
    /* Don't need this */
    uint64_t ignore;

    std::string decoded;

    /* Decode from base58 */
    Tools::Base58::decode_addr(address, ignore, decoded);

    const uint64_t paymentIDLen = 64;

    /* Grab the payment ID from the decoded address */
    std::string paymentID = decoded.substr(0, paymentIDLen);

    /* The binary array encoded keys are the rest of the address */
    std::string keys = decoded.substr(paymentIDLen, std::string::npos);

    /* Convert keys as string to binary array */
    CryptoNote::BinaryArray ba = Common::asBinaryArray(keys);

    CryptoNote::AccountPublicAddress addr;

    /* Convert from binary array to public keys */
    CryptoNote::fromBinaryArray(addr, ba);

    /* Convert the set of extracted keys back into an address */
    const std::string actualAddress = CryptoNote::getAccountAddressAsStr(
        CryptoNote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX,
        addr
    );

    return {actualAddress, paymentID};
}

std::string publicKeysToAddress(
    const Crypto::PublicKey publicSpendKey,
    const Crypto::PublicKey publicViewKey)
{
    return CryptoNote::getAccountAddressAsStr(
        CryptoNote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX,
        { publicSpendKey, publicViewKey }
    );
}

/* Generates a public address from the given private keys */
std::string privateKeysToAddress(
    const Crypto::SecretKey privateSpendKey,
    const Crypto::SecretKey privateViewKey)
{
    Crypto::PublicKey publicSpendKey;
    Crypto::PublicKey publicViewKey;

    Crypto::secret_key_to_public_key(privateSpendKey, publicSpendKey);
    Crypto::secret_key_to_public_key(privateViewKey, publicViewKey);

    return CryptoNote::getAccountAddressAsStr(
        CryptoNote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX,
        { publicSpendKey, publicViewKey }
    );
}

/* Round value to the nearest multiple (rounding down) */
uint64_t getLowerBound(const uint64_t val, const uint64_t nearestMultiple)
{
    uint64_t remainder = val % nearestMultiple;

    return val - remainder;
}

/* Round value to the nearest multiple (rounding down) */
uint64_t getUpperBound(const uint64_t val, const uint64_t nearestMultiple)
{
    return getLowerBound(val, nearestMultiple) + nearestMultiple;
}

bool isInputUnlocked(
    const uint64_t unlockTime,
    const uint64_t currentHeight)
{
    /* Might as well return fast with the case that is true for nearly all
       transactions (excluding coinbase) */
    if (unlockTime == 0)
    {
        return true;
    }

    /* if unlockTime is greater than this amount, we treat it as a
       timestamp, otherwise we treat it as a block height */
    if (unlockTime >= CryptoNote::parameters::CRYPTONOTE_MAX_BLOCK_NUMBER)
    {
        const uint64_t currentTimeAdjusted = static_cast<uint64_t>(std::time(nullptr))
            + CryptoNote::parameters::CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_SECONDS;

        return currentTimeAdjusted >= unlockTime;
    }

    const uint64_t currentHeightAdjusted = unlockTime
        + CryptoNote::parameters::CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_BLOCKS;

    return currentHeightAdjusted >= unlockTime;
}

} // namespace Utilities
