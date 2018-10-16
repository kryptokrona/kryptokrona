// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

/////////////////////////////////////////////
#include <WalletBackend/WalletSynchronizer.h>
/////////////////////////////////////////////

#include <Common/StringTools.h>

#include <crypto/crypto.h>

#include <future>

//////////////////////////
/* NON MEMBER FUNCTIONS */
//////////////////////////

namespace {
} // namespace

///////////////////////////////////
/* CONSTRUCTORS / DECONSTRUCTORS */
///////////////////////////////////

/* Default constructor */
WalletSynchronizer::WalletSynchronizer() :
    m_shouldStop(false),
    m_startTimestamp(0),
    m_startHeight(0)
{
}

/* Parameterized constructor */
WalletSynchronizer::WalletSynchronizer(
    const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon,
    const uint64_t startHeight,
    const uint64_t startTimestamp,
    const Crypto::SecretKey privateViewKey,
    const std::shared_ptr<EventHandler> eventHandler) :

    m_daemon(daemon),
    m_shouldStop(false),
    m_startHeight(startHeight),
    m_startTimestamp(startTimestamp),
    m_privateViewKey(privateViewKey),
    m_eventHandler(eventHandler)
{
}

/* Move constructor */
WalletSynchronizer::WalletSynchronizer(WalletSynchronizer && old)
{
    /* Call the move assignment operator */
    *this = std::move(old);
}

/* Move assignment operator */
WalletSynchronizer & WalletSynchronizer::operator=(WalletSynchronizer && old)
{
    /* Stop any running threads */
    stop();

    m_daemon = std::move(old.m_daemon);

    m_blockDownloaderThread = std::move(old.m_blockDownloaderThread);
    m_transactionSynchronizerThread = std::move(old.m_transactionSynchronizerThread);

    m_shouldStop.store(old.m_shouldStop.load());

    m_blockDownloaderStatus = std::move(old.m_blockDownloaderStatus);
    m_transactionSynchronizerStatus = std::move(old.m_transactionSynchronizerStatus);

    m_startTimestamp = std::move(old.m_startTimestamp);
    m_startHeight = std::move(old.m_startHeight);

    m_privateViewKey = std::move(old.m_privateViewKey);

    m_eventHandler = std::move(old.m_eventHandler);

    return *this;
}

/* Deconstructor */
WalletSynchronizer::~WalletSynchronizer()
{
    stop();
}

/////////////////////
/* CLASS FUNCTIONS */
/////////////////////

/* Launch the worker thread in the background. It's safest to do this in a 
   seperate function, so everything in the constructor gets initialized,
   and if we do any inheritance, things don't go awry. */
void WalletSynchronizer::start()
{
    if (m_daemon == nullptr)
    {
        throw std::runtime_error("Daemon has not been initialized!");
    }

    m_blockDownloaderThread = std::thread(
        &WalletSynchronizer::downloadBlocks, this
    );

    m_transactionSynchronizerThread = std::thread(
        &WalletSynchronizer::findTransactionsInBlocks, this
    );
}

void WalletSynchronizer::stop()
{
    /* If either of the threads are running (and not detached) */
    if (m_blockDownloaderThread.joinable() || 
        m_transactionSynchronizerThread.joinable())
    {
        /* Tell the threads to stop */
        m_shouldStop.store(true);

        /* Stop the block processing queue so the threads don't hang trying
           to push/pull from the queue */
        m_blockProcessingQueue.stop();

        /* Wait for the block downloader thread to finish (if applicable) */
        if (m_blockDownloaderThread.joinable())
        {
            m_blockDownloaderThread.join();
        }

        /* Wait for the transaction synchronizer thread to finish (if applicable) */
        if (m_transactionSynchronizerThread.joinable())
        {
            m_transactionSynchronizerThread.join();
        }
    }
}

/* Remove any transactions at this height or above, they were on a forked
   chain */
void WalletSynchronizer::removeForkedTransactions(const uint64_t forkHeight)
{
    m_subWallets->removeForkedTransactions(forkHeight);
}

/* Find inputs that belong to us (i.e., outgoing transactions) */
uint64_t WalletSynchronizer::processTransactionInputs(
    const std::vector<CryptoNote::KeyInput> keyInputs,
    std::unordered_map<Crypto::PublicKey, int64_t> &transfers,
    const uint64_t blockHeight)
{
    uint64_t sumOfInputs = 0;

    for (const auto keyInput : keyInputs)
    {
        sumOfInputs += keyInput.amount;
        
        /* See if any of the sub wallets contain this key image. If they do,
           it means this keyInput is an outgoing transfer from that wallet.
           
           We grab the spendKey so we can index the transfers array and then
           notify the subwallets all at once */
        auto [found, publicSpendKey] = m_subWallets->getKeyImageOwner(
            keyInput.keyImage
        );

        if (found)
        {
            /* Take the amount off the current amount (If a key doesn't exist,
               it will default to zero, so this is just setting the value
               to the negative amount in that case */
            transfers[publicSpendKey] -= keyInput.amount;

            /* The transaction has been spent, discard the key image so we
               don't double spend it */
            m_subWallets->removeSpentKeyImage(keyInput.keyImage, publicSpendKey);
        }
    }

    return sumOfInputs;
}

/* Find outputs that belong to us (i.e., incoming transactions) */
std::tuple<bool, uint64_t> WalletSynchronizer::processTransactionOutputs(
    const std::vector<WalletTypes::KeyOutput> keyOutputs,
    const Crypto::PublicKey txPublicKey,
    std::unordered_map<Crypto::PublicKey, int64_t> &transfers,
    const uint64_t blockHeight,
    const std::vector<uint32_t> globalIndexes)
{
    Crypto::KeyDerivation derivation;

    /* Generate the key derivation from the random tx public key, and our private
       view key */
    if (!Crypto::generate_key_derivation(txPublicKey, m_privateViewKey,
                                         derivation))
    {
        return {false, 0};
    }

    /* The sum of all the outputs in the transaction */
    uint64_t sumOfOutputs = 0;

    for (size_t outputIndex = 0; outputIndex < keyOutputs.size(); outputIndex++)
    {
        uint64_t amount = keyOutputs[outputIndex].amount;

        /* Add the amount to the sum of outputs, used for calculating fee later */
        sumOfOutputs += amount;

        Crypto::PublicKey spendKey;

        /* Derive the spend key from the transaction, using the previous
           derivation */
        if (!Crypto::underive_public_key(
            derivation, outputIndex, keyOutputs[outputIndex].key, spendKey))
        {
            return {false, 0};
        }

        const auto spendKeys = m_subWallets->m_publicSpendKeys;

        /* See if the derived spend key matches any of our spend keys */
        auto ourSpendKey = std::find(spendKeys.begin(), spendKeys.end(),
                                     spendKey);
        
        /* If it does, the transaction belongs to us */
        if (ourSpendKey != spendKeys.end())
        {
            /* Add the amount to the current amount (If a key doesn't exist,
               it will default to zero, so this is just setting the value
               to the amount in that case */
            transfers[*ourSpendKey] += amount;

            /* TODO: Don't need to do this in a view wallet */
            WalletTypes::TransactionInput input;

            input.amount = amount;
            input.blockHeight = blockHeight;
            input.transactionPublicKey = txPublicKey;
            input.transactionIndex = outputIndex;
            input.globalOutputIndex = globalIndexes[outputIndex];
            input.key = keyOutputs[outputIndex].key;

            /* We need to fill in the key image of the transaction input -
               we'll let the subwallet do this since we need the private spend
               key. We use the key images to detect outgoing transactions,
               and we use the transaction inputs to make transactions ourself */
            m_subWallets->completeAndStoreTransactionInput(
                *ourSpendKey, derivation, outputIndex, input
            );
        }
    }

    return {true, sumOfOutputs};
}

void WalletSynchronizer::processCoinbaseTransaction(
    const WalletTypes::RawCoinbaseTransaction rawTX,
    const uint64_t blockTimestamp,
    const uint64_t blockHeight)
{
    /* TODO: Input is a uint64_t, but we store it as an int64_t so it can be
       negative - need to handle overflow */
    std::unordered_map<Crypto::PublicKey, int64_t> transfers;

    processTransactionOutputs(
        rawTX.keyOutputs, rawTX.transactionPublicKey, transfers, blockHeight,
        rawTX.globalIndexes
    );

    /* Process any transactions we found belonging to us */
    if (!transfers.empty())
    {
        /* Coinbase transactions don't have a fee */
        uint64_t fee = 0;

        /* Coinbase transactions can't have payment ID's */
        std::string paymentID;

        /* Form the actual transaction */
        WalletTypes::Transaction tx(
            transfers, rawTX.hash, fee, blockTimestamp, blockHeight, paymentID
        );

        /* Store the transaction */
        m_subWallets->addTransaction(tx);
    }
}

/* Find the inputs and outputs of a transaction that belong to us */
void WalletSynchronizer::processTransaction(
    const WalletTypes::RawTransaction rawTX,
    const uint64_t blockTimestamp,
    const uint64_t blockHeight)
{
    /* TODO: Input is a uint64_t, but we store it as an int64_t so it can be
       negative - need to handle overflow */
    std::unordered_map<Crypto::PublicKey, int64_t> transfers;

    /* Finds the sum of inputs, addds the amounts that belong to us to the
       transfers map */
    const uint64_t sumOfInputs = processTransactionInputs(
        rawTX.keyInputs, transfers, blockHeight
    );

    /* Finds the sum of outputs, adds the amounts that belong to us to the
       transfers map, and stores any key images that belong to us */
    const auto [success, sumOfOutputs] = processTransactionOutputs(
        rawTX.keyOutputs, rawTX.transactionPublicKey, transfers, blockHeight,
        rawTX.globalIndexes
    );

    /* Failed to parse a key */
    if (!success)
    {
        return;
    }

    /* Process any transactions we found belonging to us */
    if (!transfers.empty())
    {
        /* Fee is the difference between inputs and outputs */
        const uint64_t fee = sumOfInputs - sumOfOutputs;

        /* Form the actual transaction */
        const WalletTypes::Transaction tx(
            transfers, rawTX.hash, fee, blockTimestamp, blockHeight,
            rawTX.paymentID
        );

        /* Store the transaction */
        m_subWallets->addTransaction(tx);
    }
}

void WalletSynchronizer::findTransactionsInBlocks()
{
    while (!m_shouldStop.load())
    {
        WalletTypes::WalletBlockInfo b = m_blockProcessingQueue.pop_back();

        /* Could have stopped between entering the loop and getting a block */
        if (m_shouldStop.load())
        {
            return;
        }

        /* Chain forked, invalidate previous transactions */
        if (m_transactionSynchronizerStatus.getHeight() >= b.blockHeight)
        {
            removeForkedTransactions(b.blockHeight);
        }

        /* Process the coinbase transaction */
        processCoinbaseTransaction(b.coinbaseTransaction, b.blockTimestamp,
                                   b.blockHeight);

        /* Process the rest of the transactions */
        for (const auto &tx : b.transactions)
        {
            processTransaction(tx, b.blockTimestamp, b.blockHeight);
        }

        /* Make sure to do this at the end, once the transactions are fully
           processed! Otherwise, we could miss a transaction depending upon
           when we save */
        m_transactionSynchronizerStatus.storeBlockHash(
            b.blockHash, b.blockHeight
        );

        if (b.blockHeight >= m_daemon->getLastKnownBlockHeight())
        {
            m_eventHandler->fireOnSynced(b.blockHeight);
        }
    }
}

void WalletSynchronizer::downloadBlocks()
{
    /* Stores the results from the getWalletSyncData call */
    std::vector<WalletTypes::WalletBlockInfo> newBlocks;

    std::promise<std::error_code> errorPromise;

    /* Once the function is complete, set the error value from the promise */
    auto callback = [&errorPromise](std::error_code e)
    {
        errorPromise.set_value(e);
    };

    /* While we haven't been told to stop */
    while (!m_shouldStop.load())
    {
        /* Re-assign promise (can't reuse) */
        errorPromise = std::promise<std::error_code>();

        /* Get the std::future */
        auto error = errorPromise.get_future();

        /* The block hashes to try begin syncing from */
        auto blockCheckpoints = m_blockDownloaderStatus.getBlockHashCheckpoints();

        m_daemon->getWalletSyncData(
            std::move(blockCheckpoints), m_startHeight, m_startTimestamp,
            newBlocks, callback
        );

        while (true)
        {
            /* Don't hang if the daemon doesn't respond */
            auto status = error.wait_for(std::chrono::milliseconds(50));

            if (m_shouldStop.load())
            {
                return;
            }
            
            /* queryBlocks() has returned */
            if (status == std::future_status::ready)
            {
                break;
            }
        }

        auto err = error.get();

        if (err)
        {
            std::cout << "Failed to download blocks from daemon: " << err << ", "
                      << err.message() << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        else
        {
            /* If we get no blocks, we are fully synced. Sleep a bit so we
               don't spam the daemon. */
            if (newBlocks.empty())
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            std::cout << "Syncing blocks: " << newBlocks[0].blockHeight
                      << std::endl;

            for (const auto &block : newBlocks)
            {
                if (m_shouldStop.load())
                {
                    return;
                }

                /* Store that we've downloaded the block */
                m_blockDownloaderStatus.storeBlockHash(block.blockHash,
                                                       block.blockHeight);

                /* Add the block to the queue for processing */
                m_blockProcessingQueue.push_front(block);
            }

            /* Empty the vector so we're not re-iterating the old ones */
            newBlocks.clear();
        }
    }
}

void WalletSynchronizer::initializeAfterLoad(
    const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon,
    const std::shared_ptr<EventHandler> eventHandler)
{
    m_daemon = daemon;
    m_eventHandler = eventHandler;
}
