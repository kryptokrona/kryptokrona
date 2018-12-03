// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

////////////////////////////////////////
#include <WalletBackend/WalletBackend.h>
////////////////////////////////////////

#include <Common/Base58.h>
#include <Common/FileSystemShim.h>

#include <config/CryptoNoteConfig.h>

#include <CryptoNoteCore/Account.h>
#include <CryptoNoteCore/CryptoNoteTools.h>
#include <CryptoNoteCore/CryptoNoteBasicImpl.h>

#include <cryptopp/aes.h>
#include <cryptopp/algparam.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/sha.h>
#include <cryptopp/pwdbased.h>

#include <fstream>

#include <future>

#include "json.hpp"

#include <Mnemonics/Mnemonics.h>

#include <NodeRpcProxy/NodeErrors.h>

#include <WalletBackend/Constants.h>
#include <WalletBackend/JsonSerialization.h>
#include <WalletBackend/NodeFee.h>
#include <WalletBackend/Transfer.h>
#include <WalletBackend/Utilities.h>
#include <WalletBackend/ValidateParameters.h>

using json = nlohmann::json;

//////////////////////////
/* NON MEMBER FUNCTIONS */
//////////////////////////

/* Anonymous namespace so it doesn't clash with anything else */
namespace {

/* Check data has the magic indicator from first : last, and remove it if
   it does. Else, return an error depending on where we failed */
template <class Buffer, class Identifier>
WalletError hasMagicIdentifier(
    Buffer &data,
    const Identifier &identifier,
    const WalletError tooSmallError,
    const WalletError wrongIdentifierError)
{
    /* Check we've got space for the identifier */
    if (data.size() < identifier.size())
    {
        return tooSmallError;
    }

    if (!std::equal(identifier.begin(), identifier.end(), data.begin()))
    {
        return wrongIdentifierError;
    }

    /* Remove the identifier from the string */
    data.erase(data.begin(), data.begin() + identifier.size());

    return SUCCESS;
}

/* Check the wallet filename for the new wallet to be created is valid */
WalletError checkNewWalletFilename(std::string filename)
{
    /* Check the file doesn't exist */
    if (std::ifstream(filename))
    {
        return WALLET_FILE_ALREADY_EXISTS;
    }

    /* Check we can open the file */
    if (!std::ofstream(filename))
    {
        return INVALID_WALLET_FILENAME;
    }

    /* Don't leave random files around if we fail later down the road */
    fs::remove(filename);
    
    return SUCCESS;
}

} // namespace

///////////////////////////////////
/* CONSTRUCTORS / DECONSTRUCTORS */
///////////////////////////////////

/* Constructor */
WalletBackend::WalletBackend()
{
    m_logManager = std::make_shared<Logging::LoggerManager>();

    m_logger = std::make_shared<Logging::LoggerRef>(
        *m_logManager, "WalletBackend"
    );

    m_eventHandler = std::make_shared<EventHandler>();

    /* Remember to correctly initialize the daemon - 
    we can't do it here since we don't have the host/port, and the json
    serialization uses the default constructor */
}

/* Deconstructor */
WalletBackend::~WalletBackend()
{
    /* Save, but only if the non default constructor was used - else things
       will be uninitialized, and crash */
    if (m_daemon != nullptr)
    {
        save();
    }
}

/* Standard Constructor */
WalletBackend::WalletBackend(
    const std::string filename,
    const std::string password,
    const Crypto::SecretKey privateSpendKey,
    const Crypto::SecretKey privateViewKey,
    const uint64_t scanHeight,
    const bool newWallet,
    const std::string daemonHost,
    const uint16_t daemonPort) :

    m_filename(filename),
    m_password(password)
{
    m_logManager = std::make_shared<Logging::LoggerManager>();

    m_logger = std::make_shared<Logging::LoggerRef>(
        *m_logManager, "WalletBackend"
    );

    m_daemon = std::make_shared<CryptoNote::NodeRpcProxy>(
        daemonHost, daemonPort, m_logger->getLogger()
    );

    /* Generate the address from the two private keys */
    std::string address = Utilities::privateKeysToAddress(
        privateSpendKey, privateViewKey
    );

    m_eventHandler = std::make_shared<EventHandler>();

    m_subWallets = std::make_shared<SubWallets>(
        privateSpendKey, privateViewKey, address, scanHeight, newWallet
    );
}

/* View Wallet Constructor */
WalletBackend::WalletBackend(
    const std::string filename,
    const std::string password,
    const Crypto::SecretKey privateViewKey,
    const std::string address,
    const uint64_t scanHeight,
    const std::string daemonHost,
    const uint16_t daemonPort) :

    m_filename(filename),
    m_password(password)
{
    m_logManager = std::make_shared<Logging::LoggerManager>();

    m_logger = std::make_shared<Logging::LoggerRef>(
        *m_logManager, "WalletBackend"
    );

    m_daemon = std::make_shared<CryptoNote::NodeRpcProxy>(
        daemonHost, daemonPort, m_logger->getLogger()
    );

    bool newWallet = false;

    m_eventHandler = std::make_shared<EventHandler>();

    m_subWallets = std::make_shared<SubWallets>(
        privateViewKey, address, scanHeight, newWallet
    );
}

//////////////////////
/* STATIC FUNCTIONS */
//////////////////////

std::tuple<WalletError, std::string> WalletBackend::createIntegratedAddress(
    const std::string address,
    const std::string paymentID)
{
    if (WalletError error = validatePaymentID(paymentID); error != SUCCESS)
    {
        return {error, std::string()};
    }

    const bool allowIntegratedAddresses = false;

    if (WalletError error = validateAddresses({address}, allowIntegratedAddresses); error != SUCCESS)
    {
        return {error, std::string()};
    }

    uint64_t prefix;

    CryptoNote::AccountPublicAddress addr;

    /* Get the private + public key from the address */
    CryptoNote::parseAccountAddressString(prefix, addr, address);

    /* Pack as a binary array */
    CryptoNote::BinaryArray ba;
    CryptoNote::toBinaryArray(addr, ba);
    std::string keys = Common::asString(ba);

    /* Encode prefix + paymentID + keys as an address */
    const std::string integratedAddress = Tools::Base58::encode_addr(
        CryptoNote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX,
        paymentID + keys
    );

    return {SUCCESS, integratedAddress};
}

/* Imports a wallet from a mnemonic seed. Returns the wallet class,
   or an error. */
std::tuple<WalletError, std::shared_ptr<WalletBackend>> WalletBackend::importWalletFromSeed(
    const std::string mnemonicSeed,
    const std::string filename,
    const std::string password,
    const uint64_t scanHeight,
    const std::string daemonHost,
    const uint16_t daemonPort)
{
    /* Check the filename is valid */
    if (WalletError error = checkNewWalletFilename(filename); error != SUCCESS)
    {
        return {error, nullptr}; 
    }

    /* Convert the mnemonic into a private spend key */
    auto [mnemonicError, privateSpendKey] = Mnemonics::MnemonicToPrivateKey(mnemonicSeed);

    if (mnemonicError)
    {
        return {mnemonicError, nullptr}; 
    }

    Crypto::SecretKey privateViewKey;

    /* Derive the private view key from the private spend key */
    CryptoNote::AccountBase::generateViewFromSpend(
        privateSpendKey, privateViewKey
    );

    /* Just defining here so it's more obvious what we're doing in the
       constructor */
    bool newWallet = false;

    const std::shared_ptr<WalletBackend> wallet(new WalletBackend(
        filename, password, privateSpendKey, privateViewKey,
        scanHeight, newWallet, daemonHost, daemonPort
    ));

    WalletError daemonInitError = wallet->init();

    /* Save to disk */
    WalletError error = wallet->save();

    if (daemonInitError)
    {
        return {daemonInitError, wallet};
    }

    return {error, wallet};
}

/* Imports a wallet from a private spend key and a view key. Returns
   the wallet class, or an error. */
std::tuple<WalletError, std::shared_ptr<WalletBackend>> WalletBackend::importWalletFromKeys(
    const Crypto::SecretKey privateSpendKey,
    const Crypto::SecretKey privateViewKey,
    const std::string filename,
    const std::string password,
    const uint64_t scanHeight,
    const std::string daemonHost,
    const uint16_t daemonPort)
{
    /* Check the filename is valid */
    if (WalletError error = checkNewWalletFilename(filename); error != SUCCESS)
    {
        return {error, nullptr}; 
    }

    /* Just defining here so it's more obvious what we're doing in the
       constructor */
    bool newWallet = false;

    const std::shared_ptr<WalletBackend> wallet(new WalletBackend(
        filename, password, privateSpendKey, privateViewKey, scanHeight,
        newWallet, daemonHost, daemonPort
    ));

    WalletError daemonInitError = wallet->init();

    /* Save to disk */
    WalletError error = wallet->save();

    if (daemonInitError)
    {
        return {daemonInitError, wallet};
    }

    return {error, wallet};
}

/* Imports a view wallet from a private view key and an address.
   Returns the wallet class, or an error. */
std::tuple<WalletError, std::shared_ptr<WalletBackend>> WalletBackend::importViewWallet(
    const Crypto::SecretKey privateViewKey,
    const std::string address,
    const std::string filename,
    const std::string password,
    const uint64_t scanHeight,
    const std::string daemonHost,
    const uint16_t daemonPort)
{
    /* Check the filename is valid */
    if (WalletError error = checkNewWalletFilename(filename); error != SUCCESS)
    {
        return {error, nullptr}; 
    }

    const std::shared_ptr<WalletBackend> wallet(new WalletBackend(
        filename, password, privateViewKey, address, scanHeight, daemonHost,
        daemonPort
    ));

    WalletError daemonInitError = wallet->init();

    /* Save to disk */
    WalletError error = wallet->save();

    if (daemonInitError)
    {
        return {daemonInitError, wallet};
    }

    return {error, wallet};
}

/* Creates a new wallet with the given filename and password */
std::tuple<WalletError, std::shared_ptr<WalletBackend>> WalletBackend::createWallet(
    const std::string filename,
    const std::string password,
    const std::string daemonHost,
    const uint16_t daemonPort)
{
    /* Check the filename is valid */
    if (WalletError error = checkNewWalletFilename(filename); error != SUCCESS)
    {
        return {error, nullptr};
    }
	
    CryptoNote::KeyPair spendKey;
    Crypto::SecretKey privateViewKey;
    Crypto::PublicKey publicViewKey;

    /* Generate a spend key */
    Crypto::generate_keys(spendKey.publicKey, spendKey.secretKey);

    /* Derive the view key from the spend key */
    CryptoNote::AccountBase::generateViewFromSpend(
        spendKey.secretKey, privateViewKey, publicViewKey
    );

    /* Just defining here so it's more obvious what we're doing in the
       constructor */
    bool newWallet = true;
    uint64_t scanHeight = 0;

    const std::shared_ptr<WalletBackend> wallet(new WalletBackend(
        filename, password, spendKey.secretKey, privateViewKey,
        scanHeight, newWallet, daemonHost, daemonPort
    ));
	
    WalletError daemonInitError = wallet->init();

    /* Save to disk */
    WalletError error = wallet->save();

    if (daemonInitError)
    {
        return {daemonInitError, wallet};
    }
	
    return {error, wallet};
}

/* Opens a wallet already on disk with the given filename + password */
std::tuple<WalletError, std::shared_ptr<WalletBackend>> WalletBackend::openWallet(
    const std::string filename,
    const std::string password,
    const std::string daemonHost,
    const uint16_t daemonPort)
{
    /* Open in binary mode, since we have encrypted data */
    std::ifstream file(filename, std::ios_base::binary);

    /* Check we successfully opened the file */
    if (!file)
    {
        return {FILENAME_NON_EXISTENT, nullptr}; 
    }

    /* Read file into a buffer */
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
                             (std::istreambuf_iterator<char>()));

    /* Check that the decrypted data has the 'isAWallet' identifier,
       and remove it it does. If it doesn't, return an error. */
    WalletError error = hasMagicIdentifier(
        buffer, Constants::IS_A_WALLET_IDENTIFIER,
        NOT_A_WALLET_FILE, NOT_A_WALLET_FILE
    );

    if (error)
    {
        return {error, nullptr}; 
    }

    using namespace CryptoPP;

    /* The salt we use for both PBKDF2, and AES decryption */
    byte salt[16];

    /* Check the file is large enough for the salt */
    if (buffer.size() < sizeof(salt))
    {
        return {WALLET_FILE_CORRUPTED, nullptr};
    }

    /* Copy the salt to the salt array */
    std::copy(buffer.begin(), buffer.begin() + sizeof(salt), salt);

    /* Remove the salt, don't need it anymore */
    buffer.erase(buffer.begin(), buffer.begin() + sizeof(salt));

    /* The key we use for AES decryption, generated with PBKDF2 */
    byte key[16];

    /* Using SHA256 as the algorithm */
    PKCS5_PBKDF2_HMAC<SHA256> pbkdf2;

    /* Generate the AES Key using pbkdf2 */
    pbkdf2.DeriveKey(
        key, sizeof(key), 0, (byte *)password.c_str(),
        password.size(), salt, sizeof(salt), Constants::PBKDF2_ITERATIONS
    );

    CBC_Mode<AES>::Decryption cbcDecryption;

    /* Initialize our decrypter with the key and salt/iv */
    cbcDecryption.SetKeyWithIV(key, sizeof(key), salt);

    /* This will store the decrypted data */
    std::string decryptedData;

    try
    {
        /* Decrypt, handling padding */
        StringSource((byte *)buffer.data(), buffer.size(), true, new StreamTransformationFilter(
            cbcDecryption, new StringSink(decryptedData))
        );
    }
    /* do NOT report an alternate error for invalid padding. It allows them
       to do a padding oracle attack, I believe. Just report the wrong password
       error. */
    catch (const CryptoPP::Exception &)
    {
        return {WRONG_PASSWORD, nullptr};
    }

    /* Check that the decrypted data has the 'isCorrectPassword' identifier,
       and remove it it does. If it doesn't, return an error. */
    error = hasMagicIdentifier(
        decryptedData, Constants::IS_CORRECT_PASSWORD_IDENTIFIER,
        WALLET_FILE_CORRUPTED, WRONG_PASSWORD
    );

    if (error)
    {
        return {error, nullptr}; 
    }

    try
    {
        /* Parse the json */
        const json walletJson = json::parse(decryptedData);

        /* Make our wallet object */
        const auto wallet = std::make_shared<WalletBackend>();

        /* Initialize it from the json (We could do this in less steps, but it
           requires a move/copy constructor) */
        error = wallet->fromJson(
            walletJson, filename, password, daemonHost, daemonPort
        );

        const bool dumpJson = false;

        /* For debugging purposes */
        if (dumpJson)
        {
            std::ofstream o("walletData.json");

            o << std::setw(4) << walletJson << std::endl;
        }

        return {error, wallet};
    }
    catch (const json::parse_error &)
    {
        return {WALLET_FILE_CORRUPTED, nullptr};
    }
}

/////////////////////
/* CLASS FUNCTIONS */
/////////////////////

WalletError WalletBackend::init()
{
    if (m_daemon == nullptr)
    {
        throw std::runtime_error("Daemon has not been initialized!");
    }

    std::promise<std::error_code> errorPromise;
    std::future<std::error_code> error = errorPromise.get_future();

    auto callback = [&errorPromise](std::error_code e) 
    {
        errorPromise.set_value(e);
    };

    m_daemon->init(callback);

    const auto errCode = error.get();

    WalletError returnCode = SUCCESS;

    if (errCode.value() == CryptoNote::NodeError::TIMEOUT)
    {
        returnCode = DAEMON_INIT_TIMEOUT;
    }
    else if (errCode)
    {
        return FAILED_TO_INIT_DAEMON;
    }

    /* Init the wallet synchronizer if it hasn't been loaded from the wallet
       file */
    if (m_walletSynchronizer == nullptr)
    {
        auto [startHeight, startTimestamp] = m_subWallets->getMinInitialSyncStart();

        m_walletSynchronizer = std::make_shared<WalletSynchronizer>(
            m_daemon, 
            startHeight,
            startTimestamp,
            m_subWallets->getPrivateViewKey(),
            m_eventHandler
        );
    }
    /* If it has, just initialize the stuff we can't from file */
    else
    {
        m_walletSynchronizer->initializeAfterLoad(m_daemon, m_eventHandler);
    }

    m_walletSynchronizer->m_subWallets = m_subWallets;

    /* Launch the wallet sync process in a background thread */
    m_walletSynchronizer->start();

    return returnCode;
}

WalletError WalletBackend::save() const
{
    /* Stop the wallet synchronizer, so we're not in an invalid state */
    if (m_walletSynchronizer != nullptr)
    {
        m_walletSynchronizer->stop();
    }

    WalletError error = unsafeSave();

    /* Continue syncing */
    if (m_walletSynchronizer != nullptr)
    {
        m_walletSynchronizer->start();
    }

    return error;
}

/* Unsafe because it doesn't lock any data structures - need to stop the
   blockchain synchronizer first (Call save()) */
WalletError WalletBackend::unsafeSave() const
{
    /* Add an identifier to the start of the string so we can verify the wallet
       has been correctly decrypted */
    std::string identiferAsString(
        Constants::IS_CORRECT_PASSWORD_IDENTIFIER.begin(),
        Constants::IS_CORRECT_PASSWORD_IDENTIFIER.end()
    );

    /* Serialize wallet to json */
    json walletJson = *this;

    /* Add magic identifier, and get json as a string */
    std::string walletData = identiferAsString + walletJson.dump();

    using namespace CryptoPP;

    /* The key we use for AES encryption, generated with PBKDF2 */
    byte key[16];

    /* The salt we use for both PBKDF2, and AES Encryption */
    byte salt[16];

    /* Generate 16 random bytes for the salt */
    Crypto::generate_random_bytes(16, salt);

    /* Using SHA256 as the algorithm */
    PKCS5_PBKDF2_HMAC<SHA256> pbkdf2;

    /* Generate the AES Key using pbkdf2 */
    pbkdf2.DeriveKey(
        key, sizeof(key), 0, (byte *)m_password.c_str(),
        m_password.size(), salt, sizeof(salt), Constants::PBKDF2_ITERATIONS
    );

    CBC_Mode<AES>::Encryption cbcEncryption;

    /* Initialize our encryptor with the key and salt/iv */
    cbcEncryption.SetKeyWithIV(key, sizeof(key), salt);

    /* This will store the encrypted data */
    std::string encryptedData;

    /* Encrypt, and pad */
    StringSource(walletData, true, new StreamTransformationFilter(
        cbcEncryption, new StringSink(encryptedData))
    );

    std::ofstream file(m_filename, std::ios_base::binary);

    if (!file)
    {
        return INVALID_WALLET_FILENAME;
    }

    std::string saltString = std::string(salt, salt + sizeof(salt));

    /* Write the isAWalletIdentifier to the file, so when we open it we can
       verify that it is a wallet file */
    std::copy(Constants::IS_A_WALLET_IDENTIFIER.begin(),
              Constants::IS_A_WALLET_IDENTIFIER.end(),
              std::ostreambuf_iterator<char>(file));

    /* Write the salt to the file, so we can use it to unencrypt the file
       later. Note that the salt is unencrypted. */
    std::copy(std::begin(salt), std::end(salt),
              std::ostreambuf_iterator<char>(file));

    /* Write the encrypted wallet data to the file */
    std::copy(encryptedData.begin(), encryptedData.end(),
              std::ostreambuf_iterator<char>(file));

    return SUCCESS;
}

/* Get the balance for one subwallet (error, unlocked, locked) */
std::tuple<WalletError, uint64_t, uint64_t> WalletBackend::getBalance(
    const std::string address) const
{
    /* Verify the address is good, and one of our subwallets */
    if (WalletError error = validateOurAddresses({address}, m_subWallets); error != SUCCESS)
    {
        return {error, 0, 0};
    }

    const auto [unlockedBalance, lockedBalance] = m_subWallets->getBalance(
        Utilities::addressesToSpendKeys({address}),
        false,
        m_daemon->getLastKnownBlockHeight()
    );

    return {SUCCESS, unlockedBalance, lockedBalance};
}

/* Gets the combined balance for all wallets in the container */
std::tuple<uint64_t, uint64_t> WalletBackend::getTotalBalance() const
{
    const bool takeFromAll = true;

    /* Get combined balance from every container */
    return m_subWallets->getBalance(
        {}, takeFromAll, m_daemon->getLastKnownBlockHeight()
    );
}

uint64_t WalletBackend::getTotalUnlockedBalance() const
{
    const auto [unlockedBalance, lockedBalance] = getTotalBalance();

    return unlockedBalance;
}

/* This is simply a wrapper for Transfer::sendTransactionBasic - we need to
   pass in the daemon and subwallets instance */
std::tuple<WalletError, Crypto::Hash> WalletBackend::sendTransactionBasic(
    const std::string destination,
    const uint64_t amount,
    const std::string paymentID)
{
    return SendTransaction::sendTransactionBasic(
        destination, amount, paymentID, m_daemon, m_subWallets
    );
}

std::tuple<WalletError, Crypto::Hash> WalletBackend::sendTransactionAdvanced(
    const std::vector<std::pair<std::string, uint64_t>> destinations,
    const uint64_t mixin,
    const uint64_t fee,
    const std::string paymentID,
    const std::vector<std::string> subWalletsToTakeFrom,
    const std::string changeAddress)
{
    return SendTransaction::sendTransactionAdvanced(
        destinations, mixin, fee, paymentID, subWalletsToTakeFrom,
        changeAddress, m_daemon, m_subWallets
    );
}

std::tuple<WalletError, Crypto::Hash> WalletBackend::sendFusionTransactionBasic()
{
    return SendTransaction::sendFusionTransactionBasic(m_daemon, m_subWallets);
}

std::tuple<WalletError, Crypto::Hash> WalletBackend::sendFusionTransactionAdvanced(
    const uint64_t mixin,
    const std::vector<std::string> subWalletsToTakeFrom,
    const std::string destination)
{
    return SendTransaction::sendFusionTransactionAdvanced(
        mixin, subWalletsToTakeFrom, destination, m_daemon, m_subWallets
    );
}

void WalletBackend::reset(uint64_t scanHeight, uint64_t timestamp)
{
    /* Though the wallet synchronizer can support both a timestamp and a
       scanheight, we need a fixed scan height to cut transactions from.
       Since a transaction in block 10 could have a timestamp before a
       transaction in block 9, we can't rely on timestamps to reset accurately. */
    if (timestamp != 0)
    {
        scanHeight = Utilities::timestampToScanHeight(timestamp);
        timestamp = 0;
    }

    /* Empty the sync status and reset the start height */
    m_walletSynchronizer->reset(scanHeight);

    /* Reset transactions, inputs, etc */
    m_subWallets->reset(scanHeight);

    /* Save the resetted wallet - don't need safe save, already stopped wallet
       synchronizer */
    unsafeSave();

    /* Start the sync process back up */
    m_walletSynchronizer->start();
}

WalletError WalletBackend::addSubWallet()
{
    /* Stop the wallet synchronizer, so we're not in an invalid state */
    m_walletSynchronizer->stop();

    /* Add the sub wallet */
    WalletError error = m_subWallets->addSubWallet(); 

    /* Don't need to use the safe save that stops the synchronizer since
       we've done it ourselves */
    unsafeSave();

    /* Continue syncing, syncing the new wallet as well now */
    m_walletSynchronizer->start();

    return error;
}

WalletError WalletBackend::importSubWallet(
    const Crypto::SecretKey privateSpendKey,
    const uint64_t scanHeight,
    const bool newWallet)
{
    /* Stop the wallet synchronizer, so we're not in an invalid state */
    m_walletSynchronizer->stop();

    /* Add the sub wallet */
    WalletError error = m_subWallets->importSubWallet(
        privateSpendKey, scanHeight, newWallet
    ); 

    /* If we're not making a new wallet, check if we need to reset the scan
       height of the wallet synchronizer, to pick up the new wallet data
       from the requested height */
    if (!newWallet)
    {
        uint64_t currentHeight = m_walletSynchronizer->getCurrentScanHeight();

        if (currentHeight >= scanHeight)
        {
            /* Empty the sync status and reset the start height */
            m_walletSynchronizer->reset(scanHeight);

            /* Reset transactions, inputs, etc */
            m_subWallets->reset(scanHeight);
        }
    }

    /* Don't need to use the safe save that stops the synchronizer since
       we've done it ourselves */
    unsafeSave();

    /* Continue syncing, syncing the new wallet as well now */
    m_walletSynchronizer->start();

    return error;
}

WalletError WalletBackend::importViewSubWallet(
    const Crypto::PublicKey publicSpendKey,
    const uint64_t scanHeight,
    const bool newWallet)
{
    /* Stop the wallet synchronizer, so we're not in an invalid state */
    m_walletSynchronizer->stop();

    /* Add the sub wallet */
    WalletError error = m_subWallets->importViewSubWallet(
        publicSpendKey, scanHeight, newWallet
    ); 

    /* If we're not making a new wallet, check if we need to reset the scan
       height of the wallet synchronizer, to pick up the new wallet data
       from the requested height */
    if (!newWallet)
    {
        uint64_t currentHeight = m_walletSynchronizer->getCurrentScanHeight();

        if (currentHeight >= scanHeight)
        {
            /* Empty the sync status and reset the start height */
            m_walletSynchronizer->reset(scanHeight);

            /* Reset transactions, inputs, etc */
            m_subWallets->reset(scanHeight);
        }
    }

    /* Don't need to use the safe save that stops the synchronizer since
       we've done it ourselves */
    unsafeSave();

    /* Continue syncing, syncing the new wallet as well now */
    m_walletSynchronizer->start();

    return error;
}

bool WalletBackend::isViewWallet() const
{
    return m_subWallets->isViewWallet();
}

std::string WalletBackend::getWalletLocation() const
{
    return m_filename;
}

std::string WalletBackend::getPrimaryAddress() const
{
    return m_subWallets->getPrimaryAddress();
}

std::tuple<uint64_t, uint64_t, uint64_t> WalletBackend::getSyncStatus() const
{
    /* The last block the wallet has synced */
    uint64_t walletBlockCount = m_walletSynchronizer->getCurrentScanHeight();

    /* The last block the daemon has synced */
    uint64_t localDaemonBlockCount = m_daemon->getLastLocalBlockHeight();

    /* The last block on the network, that the daemon is aware of */
    uint64_t networkBlockCount = m_daemon->getLastKnownBlockHeight();

    return {walletBlockCount, localDaemonBlockCount, networkBlockCount};
}

std::string WalletBackend::getWalletPassword() const
{
    return m_password;
}

WalletError WalletBackend::changePassword(const std::string newPassword)
{
    /* Saving is a tad slow because of pbkdf2, might as well take the
       optimization here */
    if (m_password == newPassword)
    {
        return SUCCESS;
    }

    m_password = newPassword;

    return save();
}

/* Returns all the private spend keys, and the single private view key */
std::tuple<std::vector<Crypto::SecretKey>, Crypto::SecretKey> WalletBackend::getAllPrivateKeys() const
{
    return {m_subWallets->getPrivateSpendKeys(), m_subWallets->getPrivateViewKey()};
}

/* Returns the private spend key for the primary address, and the shared private view key */
std::tuple<Crypto::SecretKey, Crypto::SecretKey> WalletBackend::getPrimaryAddressPrivateKeys() const
{
    return {m_subWallets->getPrimaryPrivateSpendKey(), m_subWallets->getPrivateViewKey()};
}

std::tuple<bool, std::string> WalletBackend::getMnemonicSeed() const
{
    const auto [privateSpendKey, privateViewKey] = getPrimaryAddressPrivateKeys();

    Crypto::SecretKey derivedPrivateViewKey;

    /* Derive the view key from the spend key, and check if it matches the
       actual view key */
    CryptoNote::AccountBase::generateViewFromSpend(
        privateSpendKey,
        derivedPrivateViewKey
    );

    if (derivedPrivateViewKey != privateViewKey)
    {
        return {false, std::string()};
    }

    return {true, Mnemonics::PrivateKeyToMnemonic(privateSpendKey)};
}

std::vector<WalletTypes::Transaction> WalletBackend::getTransactions() const
{
    return m_subWallets->getTransactions();
}

std::vector<WalletTypes::Transaction> WalletBackend::getUnconfirmedTransactions() const
{
    return m_subWallets->getUnconfirmedTransactions();
}

WalletTypes::WalletStatus WalletBackend::getStatus() const
{
    const auto [walletBlockCount, localDaemonBlockCount, networkBlockCount]
        = getSyncStatus();

    WalletTypes::WalletStatus status;

    status.walletBlockCount = walletBlockCount;
    status.localDaemonBlockCount = localDaemonBlockCount;
    status.networkBlockCount = networkBlockCount;

    status.peerCount = static_cast<uint32_t>(m_daemon->getPeerCount());

    uint64_t difficulty = m_daemon->getLastLocalBlockHeaderInfo().difficulty;

    status.lastKnownHashrate = static_cast<uint64_t>(
        difficulty / CryptoNote::parameters::DIFFICULTY_TARGET
    );

    return status;
}

/* Returns transactions in the range [startHeight, endHeight - 1] - so if
   we give 1, 100, it will return transactions from block 1 to block 99 */
std::vector<WalletTypes::Transaction> WalletBackend::getTransactionsRange(
    const uint64_t startHeight, const uint64_t endHeight) const
{
    std::vector<WalletTypes::Transaction> result;

    const auto transactions = getTransactions();

    std::copy_if(transactions.begin(), transactions.end(), std::back_inserter(result),
    [&startHeight, &endHeight](const auto tx)
    {
        return tx.blockHeight >= startHeight && tx.blockHeight < endHeight;
    });

    return result;
}

std::tuple<uint64_t, std::string> WalletBackend::getNodeFee() const
{
    return NodeFee::getNodeFee(m_daemon);
}

WalletError WalletBackend::swapNode(std::string daemonHost, uint16_t daemonPort)
{
    /* Stop the wallet synchronizer, since we're replacing the daemon it uses */
    m_walletSynchronizer->stop();

    /* Reinit proxy with new daemon */
    m_daemon = std::make_shared<CryptoNote::NodeRpcProxy>(
        daemonHost, daemonPort, m_logger->getLogger()
    );

    std::promise<std::error_code> errorPromise;
    std::future<std::error_code> error = errorPromise.get_future();

    auto callback = [&errorPromise](std::error_code e) 
    {
        errorPromise.set_value(e);
    };

    /* Init new daemon */
    m_daemon->init(callback);

    /* Give the synchronizer the new daemon */
    m_walletSynchronizer->swapNode(m_daemon);

    if (error.get())
    {
        return FAILED_TO_INIT_DAEMON;
    }
    
    /* Continue syncing */
    m_walletSynchronizer->start();

    return SUCCESS;
}
