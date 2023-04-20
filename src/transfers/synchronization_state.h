// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "common_types.h"
#include "istream_serializable.h"
#include "serialization/iserializer.h"
#include <vector>
#include <map>

namespace cryptonote
{

    class SynchronizationState : public IStreamSerializable
    {
    public:
        virtual ~SynchronizationState(){};

        struct CheckResult
        {
            bool detachRequired;
            uint32_t detachHeight;
            bool hasNewBlocks;
            uint32_t newBlockHeight;
        };

        typedef std::vector<crypto::Hash> ShortHistory;

        explicit SynchronizationState(const crypto::Hash &genesisBlockHash)
        {
            m_blockchain.push_back(genesisBlockHash);
        }

        ShortHistory getShortHistory(uint32_t localHeight) const;
        CheckResult checkInterval(const BlockchainInterval &interval) const;

        void detach(uint32_t height);
        void addBlocks(const crypto::Hash *blockHashes, uint32_t height, uint32_t count);
        uint32_t getHeight() const;
        const std::vector<crypto::Hash> &getKnownBlockHashes() const;

        // IStreamSerializable
        virtual void save(std::ostream &os) override;
        virtual void load(std::istream &in) override;

        // serialization
        cryptonote::ISerializer &serialize(cryptonote::ISerializer &s, const std::string &name);

    private:
        std::vector<crypto::Hash> m_blockchain;
    };

}
