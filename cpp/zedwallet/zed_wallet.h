// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <node_rpc_proxy/node_rpc_proxy.h>

#include <zedwallet/types.h>

int main(int argc, char **argv);

void run(cryptonote::WalletGreen &wallet, cryptonote::INode &node,
         Config &config);
