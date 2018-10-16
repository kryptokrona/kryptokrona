// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

/////////////////////////////////////////////
#include <WalletBackend/ValidateParameters.h>
/////////////////////////////////////////////

#include <Common/Base58.h>

#include <config/CryptoNoteConfig.h>
#include <config/WalletConfig.h>

#include <CryptoNoteCore/CryptoNoteBasicImpl.h>
#include <CryptoNoteCore/CryptoNoteTools.h>
#include <CryptoNoteCore/Mixins.h>
#include <CryptoNoteCore/TransactionExtra.h>

#include <WalletBackend/Utilities.h>

WalletError validateTransaction(
    const std::vector<std::pair<std::string, uint64_t>> destinations,
    const uint64_t mixin,
    const uint64_t fee,
    const std::string paymentID,
    const std::vector<std::string> subWalletsToTakeFrom,
    const std::string changeAddress,
    const std::shared_ptr<SubWallets> subWallets,
    const uint64_t height)
{
    /* Validate the destinations */
    if (WalletError error = validateDestinations(destinations); error != SUCCESS)
    {
        return error;
    }

    /* OK, destinations are good. Extract the regular addresses from the
       integrated addresses, and the payment ID's. Verify the paymentID's
       don't conflict */
    if (WalletError error = validateIntegratedAddresses(destinations, paymentID); error != SUCCESS)
    {
        return error;
    }

    /* Validate we have enough money for the transaction */
    if (WalletError error = validateAmount(destinations, fee,
            subWalletsToTakeFrom, subWallets); error != SUCCESS)
    {
        return error;
    }

    /* Validate the mixin */
    if (WalletError error = validateMixin(mixin, height); error != SUCCESS)
    {
        return error;
    }

    /* Validate the payment ID */
    if (WalletError error = validatePaymentID(paymentID); error != SUCCESS)
    {
        return error;
    }

    /* Verify the change address is valid and exists in the subwallets */
    if (WalletError error = validateOurAddresses({changeAddress}, subWallets); error != SUCCESS)
    {
        return error;
    }

    return SUCCESS;
}

WalletError validateIntegratedAddresses(
    const std::vector<std::pair<std::string, uint64_t>> destinations,
    std::string paymentID)
{
    for (const auto [address, amount] : destinations)
    {
        if (address.length() != WalletConfig::integratedAddressLength)
        {
            continue;
        }

        /* Grab the address + pid from the integrated address */
        const auto [extractedAddress, extractedPaymentID] 
            = Utilities::extractIntegratedAddressData(address);

        /* No payment ID given, set it to the extracted one */
        if (paymentID == "")
        {
            paymentID = extractedPaymentID;
        }
        else if (paymentID != extractedPaymentID)
        {
            return CONFLICTING_PAYMENT_IDS;
        }
    }

    return SUCCESS;
}

WalletError validatePaymentID(const std::string paymentID)
{
    if (paymentID.empty())
    {
        return SUCCESS;
    }

    if (paymentID.length() != 64)
    {
        return PAYMENT_ID_WRONG_LENGTH;
    }

    std::vector<uint8_t> extra;

    /* Verify the extracted payment ID is valid */
    if (!CryptoNote::createTxExtraWithPaymentId(paymentID, extra))
    {
        return PAYMENT_ID_INVALID;
    }

    return SUCCESS;
}

WalletError validateMixin(const uint64_t mixin, const uint64_t height)
{
    const auto [minMixin, maxMixin] = CryptoNote::Mixins::getMixinAllowableRange(height);

    if (mixin < minMixin)
    {
        return MIXIN_TOO_SMALL;
    }

    if (mixin > maxMixin)
    {
        return MIXIN_TOO_BIG;
    }

    return SUCCESS;
}

WalletError validateAmount(
    const std::vector<std::pair<std::string, uint64_t>> destinations,
    const uint64_t fee,
    const std::vector<std::string> subWalletsToTakeFrom,
    const std::shared_ptr<SubWallets> subWallets)
{
    /* Verify the fee is valid */
    if (fee < CryptoNote::parameters::MINIMUM_FEE)
    {
        return FEE_TOO_SMALL;
    }

    /* Verify the subwallets to take from exist */
    if (WalletError error = validateOurAddresses(subWalletsToTakeFrom, subWallets); error != SUCCESS)
    {
        return ADDRESS_NOT_IN_WALLET;
    }

    /* Get the available balance, using the source addresses */
    uint64_t availableBalance = subWallets->getBalance(
        Utilities::addressesToSpendKeys(subWalletsToTakeFrom),
        /* Take from all if no subwallets specified */
        subWalletsToTakeFrom.empty()
    );

    /* Get the total amount of the transaction */
    uint64_t totalAmount = Utilities::getTransactionSum(destinations) + fee;

    if (totalAmount > availableBalance)
    {
        return NOT_ENOUGH_BALANCE;
    }

    return SUCCESS;
}

WalletError validateDestinations(
    const std::vector<std::pair<std::string, uint64_t>> destinations)
{
    /* Make sure there is at least one destination */
    if (destinations.empty())
    {
        return NO_DESTINATIONS_GIVEN;
    }

    std::vector<std::string> destinationAddresses;

    for (const auto & [destination, amount] : destinations)
    {
        /* Check all of the amounts are > 0 */
        if (amount == 0)
        {
            return AMOUNT_IS_ZERO;
        }

        destinationAddresses.push_back(destination);
    }

    /* Validate the addresses are good [Integrated addresses allowed] */
    if (WalletError error = validateAddresses(destinationAddresses, true); error != SUCCESS)
    {
        return error;
    }
    
    return SUCCESS;
}

WalletError validateAddresses(
    std::vector<std::string> addresses,
    const bool integratedAddressesAllowed)
{
    for (auto &address : addresses)
    {
        /* Address is the wrong length */
        if (address.length() != WalletConfig::standardAddressLength &&
            address.length() != WalletConfig::integratedAddressLength)
        {
            return ADDRESS_WRONG_LENGTH;
        }

        /* Address has the wrong prefix */
        if (address.substr(0, WalletConfig::addressPrefix.length()) !=
            WalletConfig::addressPrefix)
        {
            return ADDRESS_WRONG_PREFIX;
        }

        if (address.length() == WalletConfig::integratedAddressLength)
        {
            if (!integratedAddressesAllowed)
            {
                return ADDRESS_IS_INTEGRATED;
            }

            std::string decoded;

            /* Don't need this */
            uint64_t ignore;

            if (!Tools::Base58::decode_addr(address, ignore, decoded))
            {
                return ADDRESS_NOT_BASE58;
            }

            const uint64_t paymentIDLen = 64;

            /* Grab the payment ID from the decoded address */
            std::string paymentID = decoded.substr(0, paymentIDLen);

            std::vector<uint8_t> extra;

            /* Verify the extracted payment ID is valid */
            if (!CryptoNote::createTxExtraWithPaymentId(paymentID, extra))
            {
                return INTEGRATED_ADDRESS_PAYMENT_ID_INVALID;
            }

            /* The binary array encoded keys are the rest of the address */
            std::string keys = decoded.substr(paymentIDLen, std::string::npos);

            /* Convert keys as string to binary array */
            CryptoNote::BinaryArray ba = Common::asBinaryArray(keys);

            CryptoNote::AccountPublicAddress addr;

            /* Convert from binary array to public keys */
            if (!CryptoNote::fromBinaryArray(addr, ba))
            {
                return ADDRESS_NOT_VALID;
            }

            /* Convert the set of extracted keys back into an address, then
               verify that as a normal address */
            address = CryptoNote::getAccountAddressAsStr(
                CryptoNote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX,
                addr
            );
        }

        /* Not used */
        uint64_t ignore;

        /* Not used */
        CryptoNote::AccountPublicAddress ignore2;

        if (!parseAccountAddressString(ignore, ignore2, address))
        {
            return ADDRESS_NOT_VALID;
        }
    }

    return SUCCESS;
}

WalletError validateOurAddresses(
    const std::vector<std::string> addresses,
    const std::shared_ptr<SubWallets> subWallets)
{
    /* Validate the addresses are valid [Integrated addreses not allowed] */
    if (WalletError error = validateAddresses(addresses, false); error != SUCCESS)
    {
        return error;
    }

    /* Validate we own the addresses */
    for (const auto key : Utilities::addressesToSpendKeys(addresses))
    {
        const auto &keys = subWallets->m_publicSpendKeys;

        if (std::find(keys.begin(), keys.end(), key) == keys.end())
        {
            return ADDRESS_NOT_IN_WALLET;
        }
    }

    return SUCCESS;
}
