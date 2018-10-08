// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

////////////////////////////////////////
#include <WalletBackend/WalletBackend.h>
////////////////////////////////////////

#include <config/WalletConfig.h>

#include <CryptoNoteCore/Mixins.h>

#include <WalletBackend/Utilities.h>
#include <WalletBackend/ValidateParameters.h>

//////////////////////////
/* NON MEMBER FUNCTIONS */
//////////////////////////

namespace
{
    /* Split each amount into uniform amounts, e.g.
       1234567 = 1000000 + 200000 + 30000 + 4000 + 500 + 60 + 7 */
    std::vector<uint64_t> splitAmountIntoDenominations(uint64_t amount)
    {
        std::vector<uint64_t> splitAmounts;

        int multiplier = 1;

        while (amount > 0)
        {
            uint64_t denomination = multiplier * (amount % 10);

            /* If we have for example, 1010 - we want 1000 + 10,
               not 1000 + 0 + 10 + 0 */
            if (amount != 0)
            {
                splitAmounts.push_back(denomination);
            }

            amount /= 10;

            multiplier *= 10;
        }

        return splitAmounts;
    }
    
} // namespace

/////////////////////
/* CLASS FUNCTIONS */
/////////////////////

/* A basic send transaction, the most common transaction, one destination,
   default fee, default mixin, default change address
   
   WARNING: This is NOT suitable for multi wallet containers, as the change
   address is undefined - it will return to one of the subwallets, but it
   is not defined which, since they are not ordered by creation time or
   anything like that.
   
   If you want to return change to a specific wallet, use
   sendTransactionAdvanced() */
std::tuple<WalletError, Crypto::Hash> WalletBackend::sendTransactionBasic(
    const std::string destination,
    const uint64_t amount,
    const std::string paymentID)
{
    const std::unordered_map<std::string, uint64_t> destinations = {
        {destination, amount}
    };

    const uint64_t mixin = CryptoNote::Mixins::getDefaultMixin(
        m_daemon->getLastKnownBlockHeight()
    );

    const uint64_t fee = WalletConfig::defaultFee;

    /* Assumes the container has at least one subwallet - this is true as long
       as the static constructors were used */
    const std::string changeAddress = m_subWallets->getDefaultChangeAddress();

    return sendTransactionAdvanced(
        destinations, mixin, fee, paymentID, {}, changeAddress
    );
}

std::tuple<WalletError, Crypto::Hash> WalletBackend::sendTransactionAdvanced(
    const std::unordered_map<std::string, uint64_t> destinations,
    const uint64_t mixin,
    const uint64_t fee,
    const std::string paymentID,
    const std::vector<std::string> addressesToTakeFrom,
    const std::string changeAddress)
{
    /* Validate the transaction input parameters */
    const WalletError error = validateTransaction(
        destinations, mixin, fee, paymentID, addressesToTakeFrom, changeAddress,
        *m_subWallets, m_daemon->getLastKnownBlockHeight()
    );

    if (error)
    {
        return {error, Crypto::Hash()};
    }

    /* TODO: Convert integrated address to standard + payment ID */

    /* If no address to take from is given, we will take from all available. */
    const bool takeFromAllSubWallets = addressesToTakeFrom.empty();

    /* The total amount we are sending */
    const uint64_t amount = getTransactionSum(destinations);

    /* Convert the addresses to public spend keys */
    const std::vector<Crypto::PublicKey> subWalletsToTakeFrom
        = addressesToSpendKeys(addressesToTakeFrom);

    /* The transaction 'inputs' - key images we have previously received */
    const auto inputs = m_subWallets->getTransactionInputsForAmount(
        amount, takeFromAllSubWallets, subWalletsToTakeFrom
    );

    std::map<std::string, std::vector<uint64_t>> splitAmounts;

    /* Split each amount into uniform amounts, e.g.
       1234567 = 1000000 + 200000 + 30000 + 4000 + 500 + 60 + 7 */
    for (const auto & [address, amount] : destinations)
    {
        splitAmounts[address] = splitAmountIntoDenominations(amount);
    }

    return {SUCCESS, Crypto::Hash()};
}
