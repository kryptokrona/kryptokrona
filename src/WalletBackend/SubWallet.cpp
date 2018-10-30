// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

////////////////////////////////////
#include <WalletBackend/SubWallet.h>
////////////////////////////////////

#include <CryptoNoteCore/Account.h>
#include <CryptoNoteCore/CryptoNoteBasicImpl.h>

#include <WalletBackend/Utilities.h>

//////////////////////////
/* NON MEMBER FUNCTIONS */
//////////////////////////

namespace {
} // namespace

///////////////////////////////////
/* CONSTRUCTORS / DECONSTRUCTORS */
///////////////////////////////////

SubWallet::SubWallet()
{
}

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

    m_unspentInputs.erase(it);

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

    m_spentInputs.erase(it);
}
