// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

////////////////////////////////////
#include <WalletBackend/SubWallet.h>
////////////////////////////////////

#include <CryptoNoteCore/Account.h>
#include <CryptoNoteCore/CryptoNoteBasicImpl.h>

#include <WalletBackend/Constants.h>
#include <WalletBackend/Utilities.h>

///////////////////////////////////
/* CONSTRUCTORS / DECONSTRUCTORS */
///////////////////////////////////

/* Makes a view only subwallet */
SubWallet::SubWallet(
    const Crypto::PublicKey publicSpendKey,
    const std::string address,
    const uint64_t scanHeight,
    const uint64_t scanTimestamp,
    const bool isPrimaryAddress) :

    m_publicSpendKey(publicSpendKey),
    m_address(address),
    m_syncStartHeight(scanHeight),
    m_syncStartTimestamp(scanTimestamp),
    m_privateSpendKey(Constants::BLANK_SECRET_KEY),
    m_isPrimaryAddress(isPrimaryAddress)
{
}

/* Makes a standard subwallet */
SubWallet::SubWallet(
    const Crypto::PublicKey publicSpendKey,
    const Crypto::SecretKey privateSpendKey,
    const std::string address,
    const uint64_t scanHeight,
    const uint64_t scanTimestamp,
    const bool isPrimaryAddress) :

    m_publicSpendKey(publicSpendKey),
    m_address(address),
    m_syncStartHeight(scanHeight),
    m_syncStartTimestamp(scanTimestamp),
    m_privateSpendKey(privateSpendKey),
    m_isPrimaryAddress(isPrimaryAddress)
{
}

/////////////////////
/* CLASS FUNCTIONS */
/////////////////////

void SubWallet::completeAndStoreTransactionInput(
    const Crypto::KeyDerivation derivation,
    const size_t outputIndex,
    WalletTypes::TransactionInput input,
    const bool isViewWallet)
{
    /* Can't create a key image with a view wallet - but we still store the
       input so we can calculate the balance */
    if (!isViewWallet)
    {
        Crypto::KeyImage keyImage;

        /* Make a temporary key pair */
        CryptoNote::KeyPair tmp;

        /* Get the tmp public key from the derivation, the index,
           and our public spend key */
        Crypto::derive_public_key(
            derivation, outputIndex, m_publicSpendKey, tmp.publicKey
        );

        /* Get the tmp private key from the derivation, the index,
           and our private spend key */
        Crypto::derive_secret_key(
            derivation, outputIndex, m_privateSpendKey, tmp.secretKey
        );

        /* Get the key image from the tmp public and private key */
        Crypto::generate_key_image(
            tmp.publicKey, tmp.secretKey, keyImage
        );

        input.keyImage = keyImage;
    }

    m_unspentInputs.push_back(input);
}

std::tuple<uint64_t, uint64_t> SubWallet::getBalance(
    const uint64_t currentHeight) const
{
    uint64_t unlockedBalance = 0;

    uint64_t lockedBalance = 0;

    for (const auto input : m_unspentInputs)
    {
        /* If an unlock height is present, check if the input is unlocked */
        if (Utilities::isInputUnlocked(input.unlockTime, currentHeight))
        {
            unlockedBalance += input.amount;
        }
        else
        {
            lockedBalance += input.amount;
        }
    }

    return {unlockedBalance, lockedBalance};
}

void SubWallet::reset(const uint64_t scanHeight)
{
    m_syncStartTimestamp = 0;
    m_syncStartHeight = scanHeight;

    /* If the transaction is in the pool, we'll find it when we scan the next
       top block. If it's returned and in an earlier block - too bad, you should
       have set your scan height lower! */
    m_lockedInputs.clear();

    /* Remove inputs which are above the scan height */
    auto it = std::remove_if(m_unspentInputs.begin(), m_unspentInputs.end(),
    [&scanHeight](const auto input)
    {
        return input.blockHeight >= scanHeight;
    });

    if (it != m_unspentInputs.end())
    {
        m_unspentInputs.erase(it);
    }

    it = std::remove_if(m_spentInputs.begin(), m_spentInputs.end(),
    [&scanHeight, this](auto &input)
    {
        /* Input was received after scan height, remove */
        if (input.blockHeight >= scanHeight)
        {
            return true;
        }

        /* Input was received before scan height, but spent after - move back
           into unspent inputs. */
        if (input.spendHeight >= scanHeight)
        {
            input.spendHeight = 0;

            m_unspentInputs.push_back(input);

            return true;
        }

        return false;
    });

    if (it != m_spentInputs.end())
    {
        m_spentInputs.erase(it);
    }
}

bool SubWallet::isPrimaryAddress() const
{
    return m_isPrimaryAddress;
}

std::string SubWallet::address() const
{
    return m_address;
}

std::vector<WalletTypes::TransactionInput> SubWallet::unspentInputs() const
{
    return m_unspentInputs;
}

std::vector<WalletTypes::TransactionInput> SubWallet::lockedInputs() const
{
    return m_lockedInputs;
}

std::vector<WalletTypes::TransactionInput> SubWallet::spentInput() const
{
    return m_spentInputs;
}

bool SubWallet::hasKeyImage(const Crypto::KeyImage keyImage) const
{
    auto it = std::find_if(m_unspentInputs.begin(), m_unspentInputs.end(),
    [&keyImage](const auto &input)
    {
        return input.keyImage == keyImage;
    });

    /* Found the key image */
    if (it != m_unspentInputs.end())
    {
        return true;
    }

    /* Didn't find it in unlocked inputs, check the locked inputs */
    it = std::find_if(m_lockedInputs.begin(), m_lockedInputs.end(),
    [&keyImage](const auto &input)
    {
        return input.keyImage == keyImage;
    });

    return it != m_lockedInputs.end();

    /* Note: We don't need to check the spent inputs - it should never show
       up there, as the same key image can only be used once */
}

Crypto::PublicKey SubWallet::publicSpendKey() const
{
    return m_publicSpendKey;
}

Crypto::SecretKey SubWallet::privateSpendKey() const
{
    return m_privateSpendKey;
}

void SubWallet::markInputAsSpent(
    const Crypto::KeyImage keyImage,
    const uint64_t spendHeight)
{
    /* Find the input */
    auto it = std::find_if(m_unspentInputs.begin(), m_unspentInputs.end(),
    [&keyImage](const auto x)
    {
        return x.keyImage == keyImage;
    });

    if (it != m_unspentInputs.end())
    {
        /* Set the spend height */
        it->spendHeight = spendHeight;

        /* Add to the spent inputs vector */
        m_spentInputs.push_back(*it);

        /* Remove from the unspent vector */
        m_unspentInputs.erase(it);

        return;
    }

    /* Didn't find it, lets try in the locked inputs */
    it = std::find_if(m_lockedInputs.begin(), m_lockedInputs.end(),
    [&keyImage](const auto x)
    {
        return x.keyImage == keyImage;
    });

    if (it != m_lockedInputs.end())
    {
        /* Set the spend height */
        it->spendHeight = spendHeight;

        /* Add to the spent inputs vector */
        m_spentInputs.push_back(*it);

        /* Remove from the locked vector */
        m_lockedInputs.erase(it);

        return;
    }
    
    /* Shouldn't happen */
    throw std::runtime_error("Could not find key image to remove!");
}

void SubWallet::markInputAsLocked(const Crypto::KeyImage keyImage)
{
    /* Find the input */
    auto it = std::find_if(m_unspentInputs.begin(), m_unspentInputs.end(),
    [&keyImage](const auto x)
    {
        return x.keyImage == keyImage;
    });

    /* Shouldn't happen */
    if (it == m_unspentInputs.end())
    {
        throw std::runtime_error("Could not find key image to lock!");
    }

    /* Add to the spent inputs vector */
    m_lockedInputs.push_back(*it);

    /* Remove from the unspent vector */
    m_unspentInputs.erase(it);
}

void SubWallet::removeForkedInputs(const uint64_t forkHeight)
{
    const auto it = std::remove_if(m_spentInputs.begin(), m_spentInputs.end(),
    [&forkHeight, this](auto &input)
    {
        if (input.spendHeight >= forkHeight)
        {
            /* Reset spend height */
            input.spendHeight = 0;

            /* Readd to the unspent vector */
            m_unspentInputs.push_back(input);

            return true;
        }

        return false;
    });

    if (it != m_spentInputs.end())
    {
        m_spentInputs.erase(it);
    }
}

void SubWallet::removeCancelledTransactions(const std::unordered_set<Crypto::Hash> cancelledTransactions)
{
    /* Find the inputs used in the cancelled transactions */
    const auto it = std::remove_if(m_lockedInputs.begin(), m_lockedInputs.end(),
    [&](auto &input)
    {
        if (cancelledTransactions.find(input.parentTransactionHash) != cancelledTransactions.end())
        {
            input.spendHeight = 0;

            /* Re-add the input to the unspent vector now it has been returned
               to our wallet */
            m_unspentInputs.push_back(input);

            return true;
        }

        return false;
    });

    /* Remove the inputs used in the cancelled tranactions */
    if (it != m_lockedInputs.end())
    {
        m_spentInputs.erase(it);
    }
}

std::vector<WalletTypes::TxInputAndOwner> SubWallet::getInputs() const
{
    std::vector<WalletTypes::TxInputAndOwner> inputs;

    for (const auto input : m_unspentInputs)
    {
        inputs.emplace_back(input, m_publicSpendKey, m_privateSpendKey);
    }

    return inputs;
}

uint64_t SubWallet::syncStartHeight() const
{
    return m_syncStartHeight;
}

uint64_t SubWallet::syncStartTimestamp() const
{
    return m_syncStartTimestamp;
}
