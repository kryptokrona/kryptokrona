// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include "CryptoNoteConfig.h"

namespace WalletConfig
{
    /* The prefix your coins address starts with */
    const std::string addressPrefix = "TRTL";

    /* Your coins 'Ticker', e.g. Monero = XMR, Bitcoin = BTC */
    const std::string ticker = "TRTL";

    /* The filename to output the CSV to in save_csv */
    const std::string csvFilename = "transactions.csv";

    /* The filename to read+write the address book to */
    const std::string addressBookFilename = ".addressBook.json";

    /* The name of your deamon */
    const std::string daemonName = "TurtleCoind";

    /* The name to call this wallet */
    const std::string walletName = "Zedwallet";

    /* The full name of your crypto */
    const std::string coinName = "TurtleCoin";


    /* The number of decimals your coin has */
    const int numDecimalPlaces = CryptoNote::parameters
                                           ::CRYPTONOTE_DISPLAY_DECIMAL_POINT;


    /* The length of a standard address for your coin */
    const long unsigned int addressLength = 99;


    /* The mixin value to use with transactions */
    const uint64_t defaultMixin = CryptoNote::parameters::DEFAULT_MIXIN;

    /* The default fee value to use with transactions */
    const uint64_t defaultFee = CryptoNote::parameters::MINIMUM_FEE; 

    /* The minimum fee value to allow with transactions */
    const uint64_t minimumFee = CryptoNote::parameters::MINIMUM_FEE;
}
