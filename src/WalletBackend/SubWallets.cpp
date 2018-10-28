// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

/////////////////////////////////////
#include <WalletBackend/SubWallets.h>
/////////////////////////////////////

#include <config/CryptoNoteConfig.h>

#include <CryptoNoteCore/Currency.h>

#include <ctime>

#include <random>

#include <WalletBackend/Utilities.h>

//////////////////////////
/* NON MEMBER FUNCTIONS */
//////////////////////////

namespace
{

    uint64_t getCurrentTimestampAdjusted()
    {
        /* Get the current time as a unix timestamp */
        std::time_t time = std::time(nullptr);

        /* Take the amount of time a block can potentially be in the past/future */
        std::initializer_list<uint64_t> limits =
        {
            CryptoNote::parameters::CRYPTONOTE_BLOCK_FUTURE_TIME_LIMIT,
            CryptoNote::parameters::CRYPTONOTE_BLOCK_FUTURE_TIME_LIMIT_V3,
            CryptoNote::parameters::CRYPTONOTE_BLOCK_FUTURE_TIME_LIMIT_V4
        };

        /* Get the largest adjustment possible */
        uint64_t adjust = std::max(limits);

        /* Take the earliest timestamp that will include all possible blocks */
        return time - adjust;
    }

    /* Converts a height to a timestamp */
    uint64_t scanHeightToTimestamp(const uint64_t scanHeight)
    {
        if (scanHeight == 0)
        {
            return 0;
        }

        /* Get the amount of seconds since the blockchain launched */
        uint64_t secondsSinceLaunch = scanHeight * 
                                      CryptoNote::parameters::DIFFICULTY_TARGET;

        /* Get the genesis block timestamp and add the time since launch */
        uint64_t timestamp = CryptoNote::parameters::GENESIS_BLOCK_TIMESTAMP
                           + secondsSinceLaunch;

        /* Don't make timestamp too large or daemon throws an error */
        if (timestamp >= getCurrentTimestampAdjusted())
        {
            return getCurrentTimestampAdjusted();
        }

        return timestamp;
    }
    
} // namespace

///////////////////////////////////
/* CONSTRUCTORS / DECONSTRUCTORS */
///////////////////////////////////

SubWallets::SubWallets()
{
}

/* Makes a new subwallet */
SubWallets::SubWallets(
    const Crypto::SecretKey privateSpendKey,
    const Crypto::SecretKey privateViewKey,
    const std::string address,
    const uint64_t scanHeight,
    const bool newWallet) :

    m_privateViewKey(privateViewKey)
{
    Crypto::PublicKey publicSpendKey;

    Crypto::secret_key_to_public_key(privateSpendKey, publicSpendKey);

    const uint64_t timestamp = newWallet ? getCurrentTimestampAdjusted() : 0;

    const bool isPrimaryAddress = true;

    m_subWallets[publicSpendKey] = SubWallet(
        publicSpendKey, privateSpendKey, address, scanHeight, timestamp,
        isPrimaryAddress
    );

    m_publicSpendKeys.push_back(publicSpendKey);
}

/* Makes a new view only subwallet */
SubWallets::SubWallets(
    const Crypto::SecretKey privateViewKey,
    const std::string address,
    const uint64_t scanHeight,
    const bool newWallet) :

    m_privateViewKey(privateViewKey)
{
    const auto [publicSpendKey, publicViewKey] = Utilities::addressToKeys(address);

    const uint64_t timestamp = newWallet ? getCurrentTimestampAdjusted() : 0;

    const bool isPrimaryAddress = true;

    m_subWallets[publicSpendKey] = SubWallet(
        publicSpendKey, address, scanHeight, timestamp, isPrimaryAddress
    );

    m_publicSpendKeys.push_back(publicSpendKey);
}

/////////////////////
/* CLASS FUNCTIONS */
/////////////////////

void SubWallets::addSubWallet()
{
    CryptoNote::KeyPair spendKey;

    /* Generate a spend key */
    Crypto::generate_keys(spendKey.publicKey, spendKey.secretKey);

    const std::string address = Utilities::privateKeysToAddress(
        spendKey.secretKey, m_privateViewKey
    );

    const bool isPrimaryAddress = false;

    m_subWallets[spendKey.publicKey] = SubWallet(
        spendKey.publicKey, spendKey.secretKey, address, 0,
        getCurrentTimestampAdjusted(), isPrimaryAddress
    );
}

void SubWallets::importSubWallet(
    const Crypto::SecretKey privateSpendKey,
    const uint64_t scanHeight,
    const bool newWallet)
{
    Crypto::PublicKey publicSpendKey;

    Crypto::secret_key_to_public_key(privateSpendKey, publicSpendKey);

    uint64_t timestamp = newWallet ? getCurrentTimestampAdjusted() : 0;

    const std::string address = Utilities::privateKeysToAddress(
        privateSpendKey, m_privateViewKey
    );

    const bool isPrimaryAddress = false;

    m_subWallets[publicSpendKey] = SubWallet(
        publicSpendKey, privateSpendKey, address, scanHeight, timestamp,
        isPrimaryAddress
    );

    m_publicSpendKeys.push_back(publicSpendKey);
}

/* Gets the starting height, and timestamp to begin the sync from. Only one of
   these will be non zero, which will the the lowest one (ignoring null values).

   So, if for example, one subwallet has a start height of 400,000, and another
   has a timestamp of something corresponding to 300,000, we would return
   zero for the start height, and the timestamp corresponding to 300,000.

   Alternatively, if the timestamp corresponded to 500,000, we would return
   400,000 for the height, and zero for the timestamp. */
std::tuple<uint64_t, uint64_t> SubWallets::getMinInitialSyncStart() const
{
    /* Get the smallest sub wallet (by timestamp) */
    auto minElementByTimestamp = *std::min_element(m_subWallets.begin(), m_subWallets.end(),
    [](const auto &lhs, const auto &rhs)
    {
        return lhs.second.m_syncStartTimestamp < rhs.second.m_syncStartTimestamp;
    });

    const uint64_t minTimestamp = minElementByTimestamp.second.m_syncStartTimestamp;

    /* Get the smallest sub wallet (by height) */
    auto minElementByHeight = *std::min_element(m_subWallets.begin(), m_subWallets.end(),
    [](const auto &lhs, const auto &rhs)
    {
        return lhs.second.m_syncStartHeight < rhs.second.m_syncStartHeight;
    });

    const uint64_t minHeight = minElementByHeight.second.m_syncStartHeight;

    /* One or both of the values are zero, caller will use whichever is non
       zero */
    if (minHeight == 0 || minTimestamp == 0)
    {
        return {minHeight, minTimestamp};
    }

    /* Convert timestamp to height so we can compare them, then return the min
       of the two, and set the other to zero */
    const uint64_t timestampFromHeight = scanHeightToTimestamp(minHeight);

    if (timestampFromHeight < minTimestamp)
    {
        return {minHeight, 0};
    }
    else
    {
        return {0, minTimestamp};
    }
}

void SubWallets::addTransaction(const WalletTypes::Transaction tx)
{
    /* If we sent this transaction, we will input it into the transactions
       vector instantly. This lets us display the data to the user, and then
       when the transaction actually comes in, we will update the transaction
       with the block infomation. */
    const auto it = std::find_if(m_transactions.begin(), m_transactions.end(),
    [tx](const auto transaction)
    {
        return tx.hash == transaction.hash;
    });

    /* Found it, update the transaction, don't update the balance */
    if (it != m_transactions.end())
    {
        *it = tx;
        return;
    }

    m_transactions.push_back(tx);
}

void SubWallets::completeAndStoreTransactionInput(
    const Crypto::PublicKey publicSpendKey,
    const Crypto::KeyDerivation derivation,
    const size_t outputIndex,
    WalletTypes::TransactionInput input)
{
    const auto subWallet = m_subWallets.find(publicSpendKey);

    /* Check it exists */
    if (subWallet != m_subWallets.end())
    {
        subWallet->second.completeAndStoreTransactionInput(
            derivation, outputIndex, input
        );
    }
}

std::tuple<bool, Crypto::PublicKey>
    SubWallets::getKeyImageOwner(const Crypto::KeyImage keyImage) const
{
    for (const auto & [publicKey, subWallet] : m_subWallets)
    {
        /* See if the sub wallet contains the key image */
        auto it = std::find_if(subWallet.m_transactionInputs.begin(),
                               subWallet.m_transactionInputs.end(),
        [&keyImage](const auto &input)
        {
            return input.keyImage == keyImage;
        });

        /* Found the key image */
        if (it != subWallet.m_transactionInputs.end())
        {
            return {true, subWallet.m_publicSpendKey};
        }
    }

    return {false, Crypto::PublicKey()};
}

/* Remember if the transaction suceeds, we need to remove these key images
   so we don't double spend.
   
   This may throw if you don't validate the user has enough balance, and
   that each of the subwallets exist. */
std::tuple<std::vector<WalletTypes::TxInputAndOwner>, uint64_t>
        SubWallets::getTransactionInputsForAmount(
    const uint64_t amount,
    const bool takeFromAll,
    std::vector<Crypto::PublicKey> subWalletsToTakeFrom) const
{
    /* If we're able to take from every subwallet, set the wallets to take from
       to all our public spend keys */
    if (takeFromAll)
    {
        subWalletsToTakeFrom = m_publicSpendKeys;
    }

    std::vector<SubWallet> wallets;

    /* Loop through each public key and grab the associated wallet */
    for (const auto &publicKey : subWalletsToTakeFrom)
    {
        wallets.push_back(m_subWallets.at(publicKey));
    }

    std::vector<WalletTypes::TxInputAndOwner> availableInputs;

    /* Copy the transaction inputs from this sub wallet to inputs */
    for (const auto &subWallet : wallets)
    {
        for (const auto input : subWallet.m_transactionInputs)
        {
            /* Can't spend an input we've already spent, or one that is
               currently in the transaction pool*/
            if (input.isSpent || input.isLocked)
            {
                continue;
            }

            availableInputs.emplace_back(
                input, subWallet.m_publicSpendKey, subWallet.m_privateSpendKey
            );
        }
    }

    /* Shuffle the inputs */
    std::shuffle(availableInputs.begin(), availableInputs.end(), std::random_device{});

    uint64_t foundMoney = 0;

    std::vector<WalletTypes::TxInputAndOwner> inputsToUse;

    /* Loop through each input */
    for (const auto &walletAmount : availableInputs)
    {
        /* Add each input */
        inputsToUse.push_back(walletAmount);

        foundMoney += walletAmount.input.amount;

        /* Keep adding until we have enough money for the transaction */
        if (foundMoney >= amount)
        {
            return {inputsToUse, foundMoney};
        }
    }

    /* Not enough money to cover the transaction */
    throw std::invalid_argument("Not enough funds found!");
}

/* Remember if the transaction suceeds, we need to remove these key images
   so we don't double spend. */
std::tuple<std::vector<WalletTypes::TxInputAndOwner>, uint64_t, uint64_t>
    SubWallets::getFusionTransactionInputs(
    const bool takeFromAll,
    std::vector<Crypto::PublicKey> subWalletsToTakeFrom,
    const uint64_t mixin) const
{
    /* If we're able to take from every subwallet, set the wallets to take from
       to all our public spend keys */
    if (takeFromAll)
    {
        subWalletsToTakeFrom = m_publicSpendKeys;
    }

    std::vector<SubWallet> wallets;

    /* Loop through each public key and grab the associated wallet */
    for (const auto &publicKey : subWalletsToTakeFrom)
    {
        wallets.push_back(m_subWallets.at(publicKey));
    }

    std::vector<WalletTypes::TxInputAndOwner> availableInputs;

    /* Copy the transaction inputs from this sub wallet to inputs */
    for (const auto &subWallet : wallets)
    {
        for (const auto input : subWallet.m_transactionInputs)
        {
            /* Can't spend an input we've already spent, or one that is
               currently in the transaction pool*/
            if (input.isSpent || input.isLocked)
            {
                continue;
            }

            availableInputs.emplace_back(
                input, subWallet.m_publicSpendKey, subWallet.m_privateSpendKey
            );
        }
    }

    /* Get an approximation of the max amount of inputs we can include in this
       transaction */
    uint64_t maxInputsToTake = CryptoNote::Currency::getApproximateMaximumInputCount(
        CryptoNote::parameters::FUSION_TX_MAX_SIZE,
        CryptoNote::parameters::FUSION_TX_MIN_IN_OUT_COUNT_RATIO,
        mixin
    );

    /* Shuffle the inputs */
    std::shuffle(availableInputs.begin(), availableInputs.end(), std::random_device{});

    /* Split the inputs into buckets based on what power of ten they are in
       (For example, [1, 2, 5, 7], [20, 50, 80, 80], [100, 600, 700]) */
    std::unordered_map<uint64_t, std::vector<WalletTypes::TxInputAndOwner>> buckets;

    for (const auto &walletAmount : availableInputs)
    {
        /* Find out how many digits the amount has, i.e. 1337 has 4 digits,
           420 has 3 digits */
        int numberOfDigits = log10(walletAmount.input.amount);

        /* Insert the amount into the correct bucket */
        buckets[numberOfDigits].push_back(walletAmount);
    }

    /* Find the buckets which have enough inputs to meet the fusion tx
       requirements */
    std::vector<std::vector<WalletTypes::TxInputAndOwner>> fullBuckets;

    for (const auto [amount, bucket] : buckets)
    {
        /* Skip the buckets with not enough items */
        if (bucket.size() >= CryptoNote::parameters::FUSION_TX_MIN_INPUT_COUNT)
        {
            fullBuckets.push_back(bucket);
        }
    }

    /* Shuffle the full buckets */
    std::shuffle(fullBuckets.begin(), fullBuckets.end(), std::random_device{});

    /* The buckets to pick inputs from */
    std::vector<std::vector<WalletTypes::TxInputAndOwner>> bucketsToTakeFrom;

    /* We have full buckets, take the first full bucket */
    if (!fullBuckets.empty())
    {
        bucketsToTakeFrom = { fullBuckets.front() };
    }
    /* Otherwise just use all buckets */
    else
    {
        for (const auto [amount, bucket] : buckets)
        {
            bucketsToTakeFrom.push_back(bucket);
        }
    }

    std::vector<WalletTypes::TxInputAndOwner> inputsToUse;

    uint64_t foundMoney = 0;

    /* Loop through each bucket (Remember we're only looping through one if
       we've got a full bucket) */
    for (const auto bucket : bucketsToTakeFrom)
    {
        /* Loop through each input in this bucket */
        for (const auto &walletAmount : bucket)
        {
            /* Add each input */
            inputsToUse.push_back(walletAmount);

            foundMoney += walletAmount.input.amount;

            /* Got enough inputs, return */
            if (inputsToUse.size() >= maxInputsToTake)
            {
                return {inputsToUse, maxInputsToTake, foundMoney};
            }
        }
    }

    return {inputsToUse, maxInputsToTake, foundMoney};
}

/* Gets the primary address, which is the first address created with the
   wallet */
std::string SubWallets::getDefaultChangeAddress() const
{
    const auto it = 
    std::find_if(m_subWallets.begin(), m_subWallets.end(), [](const auto subWallet)
    {
        return subWallet.second.m_isPrimaryAddress;
    });

    if (it == m_subWallets.end())
    {
        throw std::runtime_error("This container has no primary address!");
    }

    return it->second.m_address;
}

/* Will throw if the public keys given don't exist */
std::tuple<uint64_t, uint64_t> SubWallets::getBalance(
    std::vector<Crypto::PublicKey> subWalletsToTakeFrom,
    const bool takeFromAll,
    const uint64_t currentHeight) const
{
    /* If we're able to take from every subwallet, set the wallets to take from
       to all our public spend keys */
    if (takeFromAll)
    {
        subWalletsToTakeFrom = m_publicSpendKeys;
    }

    uint64_t unlockedBalance = 0;

    uint64_t lockedBalance = 0;

    /* TODO: Overflow */
    for (const auto pubKey : subWalletsToTakeFrom)
    {
        const auto [unlocked, locked] = m_subWallets.at(pubKey).getBalance(currentHeight);

        unlockedBalance += unlocked;
        lockedBalance += locked;
    }

    return {unlockedBalance, lockedBalance};
}

/* Mark a key image as spent, no longer can be used in transactions */
void SubWallets::markInputAsSpent(
    const Crypto::KeyImage keyImage,
    const Crypto::PublicKey publicKey,
    const uint64_t spendHeight)
{
    /* Grab a reference to the transaction inputs so we don't have to type
       this each time */
    auto &inputs = m_subWallets.at(publicKey).m_transactionInputs;

    /* Find the input */
    auto it = std::find_if(inputs.begin(), inputs.end(), [&keyImage](const auto x)
    {
        return x.keyImage == keyImage;
    });

    /* Shouldn't happen */
    if (it == inputs.end())
    {
        throw std::runtime_error("Could not find key image to remove!");
    }

    it->isLocked = false;

    it->isSpent = true;
    it->spendHeight = spendHeight;
}

/* Mark a key image as locked, can no longer be used in transactions till it
   returns from the pool, or we find it in a block, in which case we will
   mark it as spent. */
void SubWallets::markInputAsLocked(
    const Crypto::KeyImage keyImage,
    const Crypto::PublicKey publicKey)
{
    /* Grab a reference to the transaction inputs so we don't have to type
       this each time */
    auto &inputs = m_subWallets.at(publicKey).m_transactionInputs;

    /* Find the input */
    auto it = std::find_if(inputs.begin(), inputs.end(), [&keyImage](const auto x)
    {
        return x.keyImage == keyImage;
    });

    /* Shouldn't happen */
    if (it == inputs.end())
    {
        throw std::runtime_error("Could not find key image to remove!");
    }

    it->isLocked = true;
}

/* Remove transactions and key images that occured on a forked chain */
void SubWallets::removeForkedTransactions(uint64_t forkHeight)
{
    /* std::remove_if doesn't actually remove anything, it just moves the
       removed items to the end, and gives an iterator to the 'new' end.
       We then call std::erase(newEnd, oldEnd) to remove the items */
    auto newEnd = std::remove_if(m_transactions.begin(), m_transactions.end(),
    [forkHeight](auto tx)
    {
        /* Remove the transaction if it's height is >= than the fork height */
        return tx.blockHeight >= forkHeight;
    });

    m_transactions.erase(newEnd, m_transactions.end());

    /* Loop through each subwallet */
    for (auto & [publicKey, subWallet] : m_subWallets)
    {
        auto &inputs = subWallet.m_transactionInputs;

        for (auto &input : inputs)
        {
            /* If an input was spent on a forked chain, mark it as unspent, 
               and re-add the spent balance to the owner */
            if (input.isSpent && input.spendHeight >= forkHeight)
            {
                input.isSpent = false;
                input.spendHeight = 0;
            }
        }

        /* Remove the tx input if it's height is >= fork height */
        auto newInputsEnd = std::remove_if(inputs.begin(), inputs.end(),
        [forkHeight](auto input)
        {
            return input.blockHeight >= forkHeight;
        });

        inputs.erase(newInputsEnd, inputs.end());
    }
}

Crypto::SecretKey SubWallets::getPrivateViewKey() const
{
    return m_privateViewKey;
}
