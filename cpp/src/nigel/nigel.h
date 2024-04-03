// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <atomic>

#include "httplib.h"

#include <rpc/core_rpc_server_commands_definitions.h>

#include <string>

#include <thread>

#include <unordered_set>

#include "wallet_types.h"

class Nigel
{
public:
    /////////////////////////
    /* Public Constructors */
    /////////////////////////

    Nigel(
        const std::string daemonHost,
        const uint16_t daemonPort);

    Nigel(
        const std::string daemonHost,
        const uint16_t daemonPort,
        const std::chrono::seconds timeout);

    ~Nigel();

    /////////////////////////////
    /* Public member functions */
    /////////////////////////////

    void init();

    void swapNode(const std::string daemonHost, const uint16_t daemonPort);

    /* Returns whether we've received info from the daemon at some point */
    bool isOnline() const;

    uint64_t localDaemonBlockCount() const;

    uint64_t networkBlockCount() const;

    uint64_t peerCount() const;

    uint64_t hashrate() const;

    std::tuple<uint64_t, std::string> nodeFee() const;

    std::tuple<std::string, uint16_t> nodeAddress() const;

    std::tuple<bool, std::vector<wallet_types::WalletBlockInfo>> getWalletSyncData(
        const std::vector<crypto::Hash> blockHashCheckpoints,
        uint64_t startHeight,
        uint64_t startTimestamp) const;

    /* Returns a bool on success or not */
    bool getTransactionsStatus(
        const std::unordered_set<crypto::Hash> transactionHashes,
        std::unordered_set<crypto::Hash> &transactionsInPool,
        std::unordered_set<crypto::Hash> &transactionsInBlock,
        std::unordered_set<crypto::Hash> &transactionsUnknown) const;

    std::tuple<bool, std::vector<cryptonote::RandomOuts>> getRandomOutsByAmounts(
        const std::vector<uint64_t> amounts,
        const uint64_t requestedOuts) const;

    /* {success, connectionError} */
    std::tuple<bool, bool> sendTransaction(
        const cryptonote::Transaction tx) const;

    std::tuple<bool, std::unordered_map<crypto::Hash, std::vector<uint64_t>>>
    getGlobalIndexesForRange(
        const uint64_t startHeight,
        const uint64_t endHeight) const;

private:
    //////////////////////////////
    /* Private member functions */
    //////////////////////////////

    void stop();

    void backgroundRefresh();

    bool getDaemonInfo();

    bool getFeeInfo();

    //////////////////////////////
    /* Private member variables */
    //////////////////////////////

    /* Stores our http client (Don't really care about it launching threads
       and making our functions non const) */
    std::shared_ptr<httplib::Client> m_httpClient = nullptr;

    /* Runs a background refresh on height, hashrate, etc */
    std::thread m_backgroundThread;

    /* If we should stop the background thread */
    std::atomic<bool> m_shouldStop = false;

    /* The amount of blocks the daemon we're connected to has */
    std::atomic<uint64_t> m_localDaemonBlockCount = 0;

    /* The amount of blocks the network has */
    std::atomic<uint64_t> m_networkBlockCount = 0;

    /* The amount of peers we're connected to */
    std::atomic<uint64_t> m_peerCount = 0;

    /* The hashrate (based on the last local block the daemon has synced) */
    std::atomic<uint64_t> m_lastKnownHashrate = 0;

    /* The address to send the node fee to (May be "") */
    std::string m_nodeFeeAddress;

    /* The fee the node charges */
    uint64_t m_nodeFeeAmount = 0;

    /* The timeout on requests */
    std::chrono::seconds m_timeout;

    /* The daemon hostname */
    std::string m_daemonHost;

    /* The daemon port */
    uint16_t m_daemonPort;
};
