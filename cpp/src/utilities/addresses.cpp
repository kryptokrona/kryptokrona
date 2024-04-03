// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

////////////////////////////////
#include <utilities/addresses.h>
////////////////////////////////

#include <common/base58.h>

#include <config/cryptonote_config.h>

#include <cryptonote_core/cryptonote_basic_impl.h>
#include <cryptonote_core/cryptonote_tools.h>

namespace utilities
{

    /* Will throw an exception if the addresses are invalid. Please check they
       are valid before calling this function. (e.g. use validateAddresses)

       Please note this function does not accept integrated addresses. Please
       extract the payment ID from them before calling this function. */
    std::vector<crypto::PublicKey> addressesToSpendKeys(const std::vector<std::string> addresses)
    {
        std::vector<crypto::PublicKey> spendKeys;

        for (const auto &address : addresses)
        {
            const auto [spendKey, viewKey] = addressToKeys(address);
            spendKeys.push_back(spendKey);
        }

        return spendKeys;
    }

    std::tuple<crypto::PublicKey, crypto::PublicKey> addressToKeys(const std::string address)
    {
        cryptonote::AccountPublicAddress parsedAddress;

        uint64_t prefix;

        /* Failed to parse */
        if (!parseAccountAddressString(prefix, parsedAddress, address))
        {
            throw std::invalid_argument("Address is not valid!");
        }

        /* Incorrect prefix */
        if (prefix != cryptonote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX)
        {
            throw std::invalid_argument("Address is not valid!");
        }

        return {parsedAddress.spendPublicKey, parsedAddress.viewPublicKey};
    }

    /* Assumes address is valid */
    std::tuple<std::string, std::string> extractIntegratedAddressData(const std::string address)
    {
        /* Don't need this */
        uint64_t ignore;

        std::string decoded;

        /* Decode from base58 */
        tools::base58::decode_addr(address, ignore, decoded);

        const uint64_t paymentIDLen = 64;

        /* Grab the payment ID from the decoded address */
        std::string paymentID = decoded.substr(0, paymentIDLen);

        /* The binary array encoded keys are the rest of the address */
        std::string keys = decoded.substr(paymentIDLen, std::string::npos);

        /* Convert keys as string to binary array */
        cryptonote::BinaryArray ba = common::asBinaryArray(keys);

        cryptonote::AccountPublicAddress addr;

        /* Convert from binary array to public keys */
        cryptonote::fromBinaryArray(addr, ba);

        /* Convert the set of extracted keys back into an address */
        const std::string actualAddress = cryptonote::getAccountAddressAsStr(
            cryptonote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX,
            addr);

        return {actualAddress, paymentID};
    }

    std::string publicKeysToAddress(
        const crypto::PublicKey publicSpendKey,
        const crypto::PublicKey publicViewKey)
    {
        return cryptonote::getAccountAddressAsStr(
            cryptonote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX,
            {publicSpendKey, publicViewKey});
    }

    /* Generates a public address from the given private keys */
    std::string privateKeysToAddress(
        const crypto::SecretKey privateSpendKey,
        const crypto::SecretKey privateViewKey)
    {
        crypto::PublicKey publicSpendKey;
        crypto::PublicKey publicViewKey;

        crypto::secret_key_to_public_key(privateSpendKey, publicSpendKey);
        crypto::secret_key_to_public_key(privateViewKey, publicViewKey);

        return cryptonote::getAccountAddressAsStr(
            cryptonote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX,
            {publicSpendKey, publicViewKey});
    }

} // namespace utilities
