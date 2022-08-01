// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <atomic>

#include <common/observer_manager.h>

#include "cryptonote_core/icore.h"

#include "cryptonote_protocol/cryptonote_protocol_definitions.h"
#include "cryptonote_protocol/cryptonote_protocol_handler_common.h"
#include "cryptonote_protocol/icryptonote_protocol_observer.h"
#include "cryptonote_protocol/icryptonote_protocol_query.h"

#include "p2p/p2p_protocol_definitions.h"
#include "p2p/net_node_common.h"
#include "p2p/connection_context.h"

#include <logging/logger_ref.h>

namespace system
{
  class Dispatcher;
}

namespace cryptonote
{
  class Currency;

  class CryptoNoteProtocolHandler : public ICryptoNoteProtocolHandler
  {
  public:

    CryptoNoteProtocolHandler(const Currency& currency, System::Dispatcher& dispatcher, ICore& rcore, IP2pEndpoint* p_net_layout, std::shared_ptr<Logging::ILogger> log);

    virtual bool addObserver(ICryptoNoteProtocolObserver* observer) override;
    virtual bool removeObserver(ICryptoNoteProtocolObserver* observer) override;

    void set_p2p_endpoint(IP2pEndpoint* p2p);
    // ICore& get_core() { return m_core; }
    virtual bool isSynchronized() const override { return m_synchronized; }
    void log_connections();

    // Interface t_payload_net_handler, where t_payload_net_handler is template argument of nodetool::node_server
    void stop();
    bool start_sync(CryptoNoteConnectionContext& context);
    void onConnectionOpened(CryptoNoteConnectionContext& context);
    void onConnectionClosed(CryptoNoteConnectionContext& context);
    CoreStatistics getStatistics();
    bool get_payload_sync_data(CORE_SYNC_DATA& hshd);
    bool process_payload_sync_data(const CORE_SYNC_DATA& hshd, CryptoNoteConnectionContext& context, bool is_inital);
    int handleCommand(bool is_notify, int command, const BinaryArray& in_buff, BinaryArray& buff_out, CryptoNoteConnectionContext& context, bool& handled);
    virtual size_t getPeerCount() const override;
    virtual uint32_t getObservedHeight() const override;
    virtual uint32_t getBlockchainHeight() const override;
    void requestMissingPoolTransactions(const CryptoNoteConnectionContext& context);

  private:
    //----------------- commands handlers ----------------------------------------------
    int handle_notify_new_block(int command, NOTIFY_NEW_BLOCK::request& arg, CryptoNoteConnectionContext& context);
    int handle_notify_new_transactions(int command, NOTIFY_NEW_TRANSACTIONS::request& arg, CryptoNoteConnectionContext& context);
    int handle_request_get_objects(int command, NOTIFY_REQUEST_GET_OBJECTS::request& arg, CryptoNoteConnectionContext& context);
    int handle_response_get_objects(int command, NOTIFY_RESPONSE_GET_OBJECTS::request& arg, CryptoNoteConnectionContext& context);
    int handle_request_chain(int command, NOTIFY_REQUEST_CHAIN::request& arg, CryptoNoteConnectionContext& context);
    int handle_response_chain_entry(int command, NOTIFY_RESPONSE_CHAIN_ENTRY::request& arg, CryptoNoteConnectionContext& context);
    int handleRequestTxPool(int command, NOTIFY_REQUEST_TX_POOL::request& arg, CryptoNoteConnectionContext& context);
    int handle_notify_new_lite_block(int command, NOTIFY_NEW_LITE_BLOCK::request& arg, CryptoNoteConnectionContext& context);
    int handle_notify_missing_txs(int command, NOTIFY_MISSING_TXS::request& arg, CryptoNoteConnectionContext& context);

    //----------------- i_cryptonote_protocol ----------------------------------
    virtual void relayBlock(NOTIFY_NEW_BLOCK::request& arg) override;
    virtual void relayTransactions(const std::vector<BinaryArray>& transactions) override;

    //----------------------------------------------------------------------------------
    uint32_t get_current_blockchain_height();
    bool request_missing_objects(CryptoNoteConnectionContext& context, bool check_having_blocks);
    bool on_connection_synchronized();
    void updateObservedHeight(uint32_t peerHeight, const CryptoNoteConnectionContext& context);
    void recalculateMaxObservedHeight(const CryptoNoteConnectionContext& context);
    int processObjects(CryptoNoteConnectionContext& context, std::vector<RawBlock>&& rawBlocks, const std::vector<CachedBlock>& cachedBlocks);
    Logging::LoggerRef logger;

private:
    int doPushLiteBlock(NOTIFY_NEW_LITE_BLOCK::request block, CryptoNoteConnectionContext& context, std::vector<BinaryArray> missingTxs);

  private:

    System::Dispatcher& m_dispatcher;
    ICore& m_core;
    const Currency& m_currency;

    p2p_endpoint_stub m_p2p_stub;
    IP2pEndpoint* m_p2p;
    std::atomic<bool> m_synchronized;
    std::atomic<bool> m_stop;

    mutable std::mutex m_observedHeightMutex;
    uint32_t m_observedHeight;
    
    mutable std::mutex m_blockchainHeightMutex;
    uint32_t m_blockchainHeight;

    std::atomic<size_t> m_peersCount;
    Tools::ObserverManager<ICryptoNoteProtocolObserver> m_observerManager;
  };
}
