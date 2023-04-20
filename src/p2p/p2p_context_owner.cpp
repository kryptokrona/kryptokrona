// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "p2p_context_owner.h"
#include <cassert>
#include "p2p_context.h"

namespace cryptonote
{

    P2pContextOwner::P2pContextOwner(P2pContext *ctx, ContextList &contextList) : contextList(contextList)
    {
        contextIterator = contextList.insert(contextList.end(), ContextList::value_type(ctx));
    }

    P2pContextOwner::P2pContextOwner(P2pContextOwner &&other) : contextList(other.contextList), contextIterator(other.contextIterator)
    {
        other.contextIterator = contextList.end();
    }

    P2pContextOwner::~P2pContextOwner()
    {
        if (contextIterator != contextList.end())
        {
            contextList.erase(contextIterator);
        }
    }

    P2pContext &P2pContextOwner::get()
    {
        assert(contextIterator != contextList.end());
        return *contextIterator->get();
    }

    P2pContext *P2pContextOwner::operator->()
    {
        return &get();
    }

}
