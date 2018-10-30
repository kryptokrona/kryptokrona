// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

/////////////////////////////////////
#include <WalletBackend/SubWallets.h>
/////////////////////////////////////

#include <config/CryptoNoteConfig.h>

#include <CryptoNoteCore/Currency.h>

#include <ctime>

#include <mutex>

#include <random>

#include <WalletBackend/Utilities.h>

///////////////////////////////////
/* CONSTRUCTORS / DECONSTRUCTORS */
///////////////////////////////////

/* Makes a new subwallet */
SubWallets::SubWallets(
    const Crypto::SecretKey privateSpendKey,
    const Crypto::SecretKey privateViewKey,
    const std::string address,
    const uint64_t scanHeight,
    const bool newWallet) :

    m_privateViewKey(privateViewKey),
    m_isViewWallet(false)
{
    Crypto::PublicKey publicSpendKey;

    Crypto::secret_key_to_public_key(privateSpendKey, publicSpendKey);

    const uint64_t timestamp = newWallet ? Utilities::getCurrentTimestampAdjusted() : 0;

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

    m_privateViewKey(privateViewKey),
    m_isViewWallet(true)
{
    const auto [publicSpendKey, publicViewKey] = Utilities::addressToKeys(address);

    const uint64_t timestamp = newWallet ? Utilities::getCurrentTimestampAdjusted() : 0;

    const bool isPrimaryAddress = true;

    m_subWallets[publicSpendKey] = SubWallet(
        publicSpendKey, address, scanHeight, timestamp, isPrimaryAddress
    );

    m_publicSpendKeys.push_back(publicSpendKey);
}

/* Copy constructor */
SubWallets::SubWallets(const SubWallets &other) :
    m_subWallets(other.m_subWallets),
    m_transactions(other.m_transactions),
    m_lockedTransactions(other.m_lockedTransactions),
    m_privateViewKey(other.m_privateViewKey),
    m_isViewWallet(other.m_isViewWallet)
{
}

/////////////////////
/* CLASS FUNCTIONS */
/////////////////////

WalletError SubWallets::addSubWallet()
{
    /* This generates a private spend key - incompatible with view wallets */
    if (m_isViewWallet)
    {
        return ILLEGAL_VIEW_WALLET_OPERATION;
    }

    std::scoped_lock lock(m_mutex);

    CryptoNote::KeyPair spendKey;

    /* Generate a spend key */
    Crypto::generate_keys(spendKey.publicKey, spendKey.secretKey);

    const std::string address = Utilities::privateKeysToAddress(
        spendKey.secretKey, m_privateViewKey
    );

    const bool isPrimaryAddress = false;

    const uint64_t scanHeight = 0;

    m_subWallets[spendKey.publicKey] = SubWallet(
        spendKey.publicKey, spendKey.secretKey, address, scanHeight,
        Utilities::getCurrentTimestampAdjusted(), isPrimaryAddress
    );

    return SUCCESS;
}

WalletError SubWallets::importSubWallet(
    const Crypto::SecretKey privateSpendKey,
    const uint64_t scanHeight,
    const bool newWallet)
{
    /* Can't add a private spend key to a view wallet */
    if (m_isViewWallet)
    {
        return ILLEGAL_VIEW_WALLET_OPERATION;
    }

    std::scoped_lock lock(m_mutex);

    Crypto::PublicKey publicSpendKey;

    Crypto::secret_key_to_public_key(privateSpendKey, publicSpendKey);

    uint64_t timestamp = newWallet ? Utilities::getCurrentTimestampAdjusted() : 0;

    const std::string address = Utilities::privateKeysToAddress(
        privateSpendKey, m_privateViewKey
    );

    const bool isPrimaryAddress = false;

    if (m_subWallets.find(publicSpendKey) != m_subWallets.end())
    {
        return SUBWALLET_ALREADY_EXISTS;
    }

    m_subWallets[publicSpendKey] = SubWallet(
        publicSpendKey, privateSpendKey, address, scanHeight, timestamp,
        isPrimaryAddress
    );

    m_publicSpendKeys.push_back(publicSpendKey);

    return SUCCESS;
}

WalletError SubWallets::importViewSubWallet(
    const Crypto::PublicKey publicSpendKey,
    const uint64_t scanHeight,
    const bool newWallet)
{
    /* Can't have view/non view wallets in one container */
    if (!m_isViewWallet)
    {
        return ILLEGAL_NON_VIEW_WALLET_OPERATION;
    }

    std::scoped_lock lock(m_mutex);

    if (m_subWallets.find(publicSpendKey) != m_subWallets.end())
    {
        return SUBWALLET_ALREADY_EXISTS;
    }

    uint64_t timestamp = newWallet ? Utilities::getCurrentTimestampAdjusted() : 0;

    Crypto::PublicKey publicViewKey;

    Crypto::secret_key_to_public_key(m_privateViewKey, publicViewKey);

    const std::string address = Utilities::publicKeysToAddress(
        publicSpendKey, publicViewKey
    );

    const bool isPrimaryAddress = false;

    m_subWallets[publicSpendKey] = SubWallet(
        publicSpendKey, address, scanHeight, timestamp, isPrimaryAddress
    );

    m_publicSpendKeys.push_back(publicSpendKey);

    return SUCCESS;
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
    std::scoped_lock lock(m_mutex);

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
    const uint64_t timestampFromHeight = Utilities::scanHeightToTimestamp(minHeight);

    if (timestampFromHeight < minTimestamp)
    {
        return {minHeight, 0};
    }
    else
    {
        return {0, minTimestamp};
    }
}

void SubWallets::addUnconfirmedTransaction(const WalletTypes::Transaction tx)
{
    std::scoped_lock lock(m_mutex);

    m_lockedTransactions.push_back(tx);
}

void SubWallets::addTransaction(const WalletTypes::Transaction tx)
{
    std::scoped_lock lock(m_mutex);

    /* If we sent this transaction, we will input it into the transactions
       vector instantly. This lets us display the data to the user, and then
       when the transaction actually comes in, we will update the transaction
       with the block infomation. */
    const auto it = std::remove_if(m_lockedTransactions.begin(), m_lockedTransactions.end(),
    [tx](const auto transaction)
    {
        return tx.hash == transaction.hash;
    });

    if (it != m_lockedTransactions.end())
    {
        /* Remove from the locked container */
        m_lockedTransactions.erase(it);
    }

    m_transactions.push_back(tx);
}

void SubWallets::completeAndStoreTransactionInput(
    const Crypto::PublicKey publicSpendKey,
    const Crypto::KeyDerivation derivation,
    const size_t outputIndex,
    WalletTypes::TransactionInput input)
{
    std::scoped_lock lock(m_mutex);

    const auto it = m_subWallets.find(publicSpendKey);

    /* Check it exists */
    if (it != m_subWallets.end())
    {
        /* If we have a view wallet, don't attempt to derive the key image */
        it->second.completeAndStoreTransactionInput(
            derivation, outputIndex, input, m_isViewWallet
        );
    }
}

std::tuple<bool, Crypto::PublicKey>
    SubWallets::getKeyImageOwner(const Crypto::KeyImage keyImage) const
{
    /* View wallet can't generate key images */
    if (m_isViewWallet)
    {
        return {false, Crypto::PublicKey()};
    }

    std::scoped_lock lock(m_mutex);

    for (const auto & [publicKey, subWallet] : m_subWallets)
    {
        /* See if the sub wallet contains the key image */
        auto it = std::find_if(subWallet.m_unspentInputs.begin(),
                               subWallet.m_unspentInputs.end(),
        [&keyImage](const auto &input)
        {
            return input.keyImage == keyImage;
        });

        /* Found the key image */
        if (it != subWallet.m_unspentInputs.end())
        {
            return {true, subWallet.m_publicSpendKey};
        }

        /* Didn't find it in unlocked inputs, check the locked inputs */
        it = std::find_if(subWallet.m_lockedInputs.begin(),
                          subWallet.m_lockedInputs.end(),
        [&keyImage](const auto &input)
        {
            return input.keyImage == keyImage;
        });

        if (it != subWallet.m_unspentInputs.end())
        {
            return {true, subWallet.m_publicSpendKey};
        }

        /* Note: We don't need to check the spent inputs - it should never show
           up there, as the same key image can only be used once */
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
    /* Can't send transactions with a view wallet */
    throwIfViewWallet();

    std::scoped_lock lock(m_mutex);

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
        for (const auto input : subWallet.m_unspentInputs)
        {
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
    /* Can't send transactions with a view wallet */
    throwIfViewWallet();

    std::scoped_lock lock(m_mutex);

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
        for (const auto input : subWallet.m_unspentInputs)
        {
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
    std::scoped_lock lock(m_mutex);

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
    std::scoped_lock lock(m_mutex);

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
    /* A view wallet can't generate key images, so can't determine when an
       input is spent */
    throwIfViewWallet();

    std::scoped_lock lock(m_mutex);

    /* Grab a reference to the transaction inputs so we don't have to type
       this each time */
    auto &unspent = m_subWallets.at(publicKey).m_unspentInputs;
    auto &locked = m_subWallets.at(publicKey).m_lockedInputs;
    auto &spent = m_subWallets.at(publicKey).m_spentInputs;

    /* Find the input */
    auto it = std::find_if(unspent.begin(), unspent.end(), [&keyImage](const auto x)
    {
        return x.keyImage == keyImage;
    });

    if (it != unspent.end())
    {
        /* Set the spend height */
        it->spendHeight = spendHeight;

        /* Add to the spent inputs vector */
        spent.push_back(*it);

        /* Remove from the unspent vector */
        unspent.erase(it);

        return;
    }

    /* Didn't find it, lets try in the locked inputs */
    it = std::find_if(locked.begin(), locked.end(), [&keyImage](const auto x)
    {
        return x.keyImage == keyImage;
    });

    if (it != locked.end())
    {
        /* Set the spend height */
        it->spendHeight = spendHeight;

        /* Add to the spent inputs vector */
        spent.push_back(*it);

        /* Remove from the locked vector */
        locked.erase(it);

        return;
    }
    
    /* Shouldn't happen */
    throw std::runtime_error("Could not find key image to remove!");
}

/* Mark a key image as locked, can no longer be used in transactions till it
   returns from the pool, or we find it in a block, in which case we will
   mark it as spent. */
void SubWallets::markInputAsLocked(
    const Crypto::KeyImage keyImage,
    const Crypto::PublicKey publicKey)
{
    /* View wallets can't have locked inputs (can't spend) */
    throwIfViewWallet();

    std::scoped_lock lock(m_mutex);

    /* Grab a reference to the transaction inputs so we don't have to type
       this each time */
    auto &inputs = m_subWallets.at(publicKey).m_unspentInputs;

    /* Find the input */
    auto it = std::find_if(inputs.begin(), inputs.end(), [&keyImage](const auto x)
    {
        return x.keyImage == keyImage;
    });

    /* Shouldn't happen */
    if (it == inputs.end())
    {
        throw std::runtime_error("Could not find key image to lock!");
    }

    /* Add to the locked vector */
    m_subWallets.at(publicKey).m_lockedInputs.push_back(*it);

    /* Remove from the unspent vector */
    inputs.erase(it);
}

/* Remove transactions and key images that occured on a forked chain */
void SubWallets::removeForkedTransactions(uint64_t forkHeight)
{
    std::scoped_lock lock(m_mutex);

    /* std::remove_if doesn't actually remove anything, it just moves the
       removed items to the end, and gives an iterator to the 'new' end.
       We then call std::erase(newEnd, oldEnd) to remove the items */
    auto newEnd = std::remove_if(m_transactions.begin(), m_transactions.end(),
    [forkHeight](auto tx)
    {
        /* Remove the transaction if it's height is >= than the fork height */
        return tx.blockHeight >= forkHeight;
    });

    m_transactions.erase(newEnd);

    /* Loop through each subwallet */
    for (auto & [publicKey, subWallet] : m_subWallets)
    {
        const auto it = 
        std::remove_if(subWallet.m_spentInputs.begin(), subWallet.m_spentInputs.end(),
        [&forkHeight, &subWallet = subWallet](auto &input)
        {
            if (input.spendHeight >= forkHeight)
            {
                /* Reset spend height */
                input.spendHeight = 0;

                /* Readd to the unspent vector */
                subWallet.m_unspentInputs.push_back(input);

                return true;
            }

            return false;
        });

        /* Remove any inputs which are no longer spent */
        subWallet.m_spentInputs.erase(it);
    }
}

void SubWallets::removeCancelledTransactions(
    const std::unordered_set<Crypto::Hash> cancelledTransactions)
{
    /* View wallets don't have locked transactions (can't spend) */
    throwIfViewWallet();

    std::scoped_lock lock(m_mutex);

    /* Find any cancelled transactions */
    const auto newEnd = std::remove_if(m_lockedTransactions.begin(), m_lockedTransactions.end(),
    [&cancelledTransactions](const auto &tx)
    {
        return cancelledTransactions.find(tx.hash) != cancelledTransactions.end();
    });

    /* Remove the cancelled transactions */
    m_lockedTransactions.erase(newEnd);

    for (auto &[pubKey, subWallet] : m_subWallets)
    {
        std::vector<WalletTypes::TransactionInput> &locked = subWallet.m_lockedInputs;

        /* Find the inputs used in the cancelled transactions */
        const auto it = std::remove_if(locked.begin(), locked.end(),
        [&cancelledTransactions, &subWallet = subWallet](auto &input)
        {
            if (cancelledTransactions.find(input.hashOfContainingTransaction) != cancelledTransactions.end())
            {
                input.spendHeight = 0;

                /* Re-add the input to the unspent vector now it has been returned
                   to our wallet */
                subWallet.m_unspentInputs.push_back(input);

                return true;
            }

            return false;
        });

        /* Remove the inputs used in the cancelled tranactions */
        locked.erase(it);
    }
}

Crypto::SecretKey SubWallets::getPrivateViewKey() const
{
    return m_privateViewKey;
}

std::unordered_set<Crypto::Hash> SubWallets::getLockedTransactionsHashes() const
{
    /* Can't have locked transactions in a view wallet (can't spend) */
    throwIfViewWallet();

    std::scoped_lock lock(m_mutex);

    std::unordered_set<Crypto::Hash> result;

    for (const auto transaction : m_lockedTransactions)
    {
        result.insert(transaction.hash);
    }

    return result;
}

void SubWallets::throwIfViewWallet() const
{
    if (m_isViewWallet)
    {
        throw std::runtime_error("Wallet is a view wallet, but this function cannot be called in a view wallet");
    }
}

bool SubWallets::isViewWallet() const
{
    return m_isViewWallet;
}

void SubWallets::reset(const uint64_t scanHeight)
{
    std::scoped_lock lock(m_mutex);

    /* If the transaction is in the pool, we'll find it when we scan the next
       top block. If it's returned and in an earlier block - too bad, you should
       have set your scan height lower! */
    m_lockedTransactions.clear();

    /* Find transactions that are above the scan height, and remove them */
    const auto it = std::remove_if(m_transactions.begin(), m_transactions.end(), 
    [&scanHeight](const auto tx)
    {
        return tx.blockHeight >= scanHeight;
    });

    m_transactions.erase(it);

    for (auto &[pubKey, subWallet] : m_subWallets)
    {
        subWallet.reset(scanHeight);
    }
}
