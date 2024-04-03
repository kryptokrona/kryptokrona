// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <p2p/p2p_protocol_types.h>

#include <vector>

class Peerlist
{
public:
    Peerlist(std::vector<PeerlistEntry> &peers, size_t maxSize);

    /* Gets the size of the peer list */
    size_t count() const;

    /* Gets a peer list entry, indexed by time */
    bool get(PeerlistEntry &entry, size_t index) const;

    /* Trim the peer list, removing the oldest ones */
    void trim();

private:
    std::vector<PeerlistEntry> &m_peers;

    const size_t m_maxSize;
};
