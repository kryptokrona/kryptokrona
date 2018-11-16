// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

////////////////////////////////////////////
#include <WalletBackend/JsonSerialization.h>
////////////////////////////////////////////

#include <tuple>

#include <Common/StringTools.h>

#include <WalletBackend/Constants.h>
#include <WalletBackend/SubWallet.h>
#include <WalletBackend/SubWallets.h>
#include <WalletBackend/SynchronizationStatus.h>
#include <WalletBackend/WalletBackend.h>
#include <WalletBackend/WalletSynchronizer.h>

using nlohmann::json;

///////////////
/* SubWallet */
///////////////

void to_json(json &j, const SubWallet &s)
{
    j = s.toJson();
}

void from_json(const json &j, SubWallet &s)
{
    s.fromJson(j);
}

/* We use member functions so we can access the private variables, and the
   functions above are defined so json() can auto convert between types */
json SubWallet::toJson() const
{
    return
    {
        {"publicSpendKey", m_publicSpendKey},
        {"privateSpendKey", m_privateSpendKey},
        {"address", m_address},
        {"syncStartTimestamp", m_syncStartTimestamp},
        {"unspentInputs", m_unspentInputs},
        {"lockedInputs", m_lockedInputs},
        {"spentInputs", m_spentInputs},
        {"syncStartHeight", m_syncStartHeight},
        {"isPrimaryAddress", m_isPrimaryAddress},
    };
}

void SubWallet::fromJson(const json &j)
{
    m_publicSpendKey = j.at("publicSpendKey").get<Crypto::PublicKey>();
    m_privateSpendKey = j.at("privateSpendKey").get<Crypto::SecretKey>();
    m_address = j.at("address").get<std::string>();
    m_syncStartTimestamp = j.at("syncStartTimestamp").get<uint64_t>();
    m_unspentInputs = j.at("unspentInputs").get<std::vector<WalletTypes::TransactionInput>>();
    m_lockedInputs = j.at("lockedInputs").get<std::vector<WalletTypes::TransactionInput>>();
    m_spentInputs = j.at("spentInputs").get<std::vector<WalletTypes::TransactionInput>>();
    m_syncStartHeight = j.at("syncStartHeight").get<uint64_t>();
    m_isPrimaryAddress = j.at("isPrimaryAddress").get<bool>();
}

///////////////
/* SubWallets */
///////////////

void to_json(json &j, const SubWallets &s)
{
    j = s.toJson();
}

void from_json(const json &j, SubWallets &s)
{
    s.fromJson(j);
}

json SubWallets::toJson() const
{
    return
    {
        {"publicSpendKeys", m_publicSpendKeys},
        {"subWallet", subWalletsToVector(m_subWallets)},
        {"transactions", m_transactions},
        {"lockedTransactions", m_lockedTransactions},
        {"privateViewKey", m_privateViewKey},
        {"isViewWallet", m_isViewWallet},
    };
}

void SubWallets::fromJson(const json &j)
{
    m_publicSpendKeys = j.at("publicSpendKeys").get<std::vector<Crypto::PublicKey>>();
    m_subWallets = vectorToSubWallets(j.at("subWallet").get<std::vector<SubWallet>>());
    m_transactions = j.at("transactions").get<std::vector<WalletTypes::Transaction>>();
    m_lockedTransactions = j.at("lockedTransactions").get<std::vector<WalletTypes::Transaction>>();
    m_privateViewKey = j.at("privateViewKey").get<Crypto::SecretKey>();
    m_isViewWallet = j.at("isViewWallet").get<bool>();
}

///////////////////
/* WalletBackend */
///////////////////

void to_json(json &j, const WalletBackend &w)
{
    j = w.toJson();
}

void from_json(const json &j, WalletBackend &w)
{
    w.fromJson(j);
}

json WalletBackend::toJson() const
{
    return
    {
        {"walletFileFormatVersion", Constants::WALLET_FILE_FORMAT_VERSION},
        {"subWallets", *m_subWallets},
        {"walletSynchronizer", *m_walletSynchronizer}
    };
}

WalletError WalletBackend::fromJson(const json &j)
{
    uint16_t version = j.at("walletFileFormatVersion").get<uint16_t>();

    if (version != Constants::WALLET_FILE_FORMAT_VERSION)
    {
        return UNSUPPORTED_WALLET_FILE_FORMAT_VERSION;
    }

    m_subWallets = std::make_shared<SubWallets>(
        j.at("subWallets").get<SubWallets>()
    );

    m_walletSynchronizer = std::make_shared<WalletSynchronizer>(
        j.at("walletSynchronizer").get<WalletSynchronizer>()
    );

    return SUCCESS;
}

WalletError WalletBackend::fromJson(
    const json &j,
    const std::string filename,
    const std::string password,
    const std::string daemonHost,
    const uint16_t daemonPort)
{
    if (WalletError error = fromJson(j); error != SUCCESS)
    {
        return error;
    }

    m_filename = filename;
    m_password = password;

    m_daemon = std::make_shared<CryptoNote::NodeRpcProxy>(
        daemonHost, daemonPort, m_logger->getLogger()
    );

    return init();
}

/* Declaration of to_json and from_json have to be in the same namespace as
   the type itself was declared in */
namespace Crypto
{
    ///////////////////////
    /* Crypto::SecretKey */
    ///////////////////////

    void to_json(json &j, const SecretKey &s)
    {
        j = Common::podToHex(s);
    }

    void from_json(const json &j, SecretKey &s)
    {
        Common::podFromHex(j.get<std::string>(), s.data);
    }

    ///////////////////////
    /* Crypto::PublicKey */
    ///////////////////////

    void to_json(json &j, const PublicKey &p)
    {
        j = Common::podToHex(p);
    }

    void from_json(const json &j, PublicKey &p)
    {
        Common::podFromHex(j.get<std::string>(), p.data);
    }

    //////////////////
    /* Crypto::Hash */
    //////////////////

    void to_json(json &j, const Hash &h)
    {
        j = Common::podToHex(h);
    }

    void from_json(const json &j, Hash &h)
    {
        Common::podFromHex(j.get<std::string>(), h.data);
    }

    //////////////////////
    /* Crypto::KeyImage */
    //////////////////////

    void to_json(json &j, const KeyImage &k)
    {
        j = Common::podToHex(k);
    }

    void from_json(const json &j, KeyImage &k)
    {
        Common::podFromHex(j.get<std::string>(), k.data);
    }
}

////////////////////////
/* WalletSynchronizer */
///////////////////////

void to_json(json &j, const WalletSynchronizer &w)
{
    j = w.toJson();
}

void from_json(const json &j, WalletSynchronizer &w)
{
    w.fromJson(j);
}

json WalletSynchronizer::toJson() const
{
    return
    {
        {"transactionSynchronizerStatus", m_transactionSynchronizerStatus},
        {"startTimestamp", m_startTimestamp},
        {"startHeight", m_startHeight},
        {"privateViewKey", m_privateViewKey}
    };
}

void WalletSynchronizer::fromJson(const json &j)
{
    m_blockDownloaderStatus = j.at("transactionSynchronizerStatus").get<SynchronizationStatus>();

    m_transactionSynchronizerStatus = m_blockDownloaderStatus;

    m_startTimestamp = j.at("startTimestamp").get<uint64_t>();
    m_startHeight = j.at("startHeight").get<uint64_t>();

    m_privateViewKey = j.at("privateViewKey").get<Crypto::SecretKey>();
}

///////////////////////////
/* SynchronizationStatus */
///////////////////////////

void to_json(json &j, const SynchronizationStatus &s)
{
    j = s.toJson();
}

void from_json(const json &j, SynchronizationStatus &s)
{
    s.fromJson(j);
}

json SynchronizationStatus::toJson() const
{
    return
    {
        {"blockHashCheckpoints", m_blockHashCheckpoints},
        {"lastKnownBlockHashes", m_lastKnownBlockHashes},
        {"lastKnownBlockHeight", m_lastKnownBlockHeight}
    };
}

void SynchronizationStatus::fromJson(const json &j)
{
    m_blockHashCheckpoints = j.at("blockHashCheckpoints").get<std::deque<Crypto::Hash>>();
    m_lastKnownBlockHashes = j.at("lastKnownBlockHashes").get<std::deque<Crypto::Hash>>();
    m_lastKnownBlockHeight = j.at("lastKnownBlockHeight").get<uint64_t>();
}

namespace WalletTypes
{
    ///////////////////////
    /* TransactionInput  */
    ///////////////////////

    void to_json(json &j, const TransactionInput &t)
    {
        j = {
            {"keyImage", t.keyImage},
            {"amount", t.amount},
            {"blockHeight", t.blockHeight},
            {"transactionPublicKey", t.transactionPublicKey},
            {"transactionIndex", t.transactionIndex},
            {"globalOutputIndex", t.globalOutputIndex},
            {"key", t.key},
            {"spendHeight", t.spendHeight},
            {"unlockTime", t.unlockTime},
            {"parentTransactionHash", t.parentTransactionHash},
        };
    }

    void from_json(const json &j, TransactionInput &t)
    {
        t.keyImage = j.at("keyImage").get<Crypto::KeyImage>();
        t.amount = j.at("amount").get<int64_t>();
        t.blockHeight = j.at("blockHeight").get<uint64_t>();
        t.transactionPublicKey = j.at("transactionPublicKey").get<Crypto::PublicKey>();
        t.transactionIndex = j.at("transactionIndex").get<uint64_t>();
        t.globalOutputIndex = j.at("globalOutputIndex").get<uint64_t>();
        t.key = j.at("key").get<Crypto::PublicKey>();
        t.spendHeight = j.at("spendHeight").get<uint64_t>();
        t.unlockTime = j.at("unlockTime").get<uint64_t>();
        t.parentTransactionHash = j.at("parentTransactionHash").get<Crypto::Hash>();
    }

    //////////////////////////////
    /* WalletTypes::Transaction */
    //////////////////////////////

    void to_json(json &j, const Transaction &t)
    {
        j = json {
            {"transfers", transfersToVector(t.transfers)},
            {"hash", t.hash},
            {"fee", t.fee},
            {"blockHeight", t.blockHeight},
            {"timestamp", t.timestamp},
            {"paymentID", t.paymentID},
            {"unlockTime", t.unlockTime},
            {"isCoinbaseTransaction", t.isCoinbaseTransaction},
        };
    }

    void from_json(const json &j, Transaction &t)
    {
        t.transfers = vectorToTransfers(j.at("transfers").get<std::vector<Transfer>>());
        t.hash = j.at("hash").get<Crypto::Hash>();
        t.fee = j.at("fee").get<uint64_t>();
        t.blockHeight = j.at("blockHeight").get<uint64_t>();
        t.timestamp = j.at("timestamp").get<uint64_t>();
        t.paymentID = j.at("paymentID").get<std::string>();
        t.unlockTime = j.at("unlockTime").get<uint64_t>();
        t.isCoinbaseTransaction = j.at("isCoinbaseTransaction").get<bool>();
    }
}

//////////////
/* Transfer */
//////////////

void to_json(json &j, const Transfer &t)
{
    j = {
        {"publicKey", t.publicKey},
        {"amount", t.amount},
    };
}

void from_json(const json &j, Transfer &t)
{
    t.publicKey = j.at("publicKey").get<Crypto::PublicKey>();
    t.amount = j.at("amount").get<int64_t>();
}

/* std::map / std::unordered_map don't work great in json - they get serialized
   like this for example: 

"transfers": [
    [
          {
                "publicKey": "95b86472ef19dcd7e787031ae4e749226c4ea672c8d41ca960cc1b8cd7d4766f"
          },
          100
    ]
]

This is not very helpful, as it's not obvious that '100' is the amount, and
it's not easy to access, as there's no key to access it by.

So, lets instead convert to a vector of structs when converting to json, to
make it easier for people using the wallet file in different languages to
use */

std::vector<Transfer> transfersToVector(std::unordered_map<Crypto::PublicKey, int64_t> transfers)
{
    std::vector<Transfer> vector;

    for (const auto & [publicKey, amount] : transfers)
    {
        Transfer t;
        t.publicKey = publicKey;
        t.amount = amount;

        vector.push_back(t);
    }

    return vector;
}

std::unordered_map<Crypto::PublicKey, int64_t> vectorToTransfers(std::vector<Transfer> vector)
{
    std::unordered_map<Crypto::PublicKey, int64_t> transfers;

    for (const auto &transfer : vector)
    {
        transfers[transfer.publicKey] = transfer.amount;
    }

    return transfers;
}

std::vector<SubWallet> subWalletsToVector(std::unordered_map<Crypto::PublicKey, SubWallet> subWallets)
{
    std::vector<SubWallet> vector;

    for (const auto & [publicKey, subWallet] : subWallets)
    {
        vector.push_back(subWallet);
    }

    return vector;
}

std::unordered_map<Crypto::PublicKey, SubWallet> vectorToSubWallets(std::vector<SubWallet> vector)
{
    std::unordered_map<Crypto::PublicKey, SubWallet> transfers;

    for (const auto &s : vector)
    {
        transfers[s.publicSpendKey()] = s;
    }

    return transfers;
}
