/*
Copyright (C) 2018, The TurtleCoin developers

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <SimpleWallet/ColouredMsg.h>
#include <SimpleWallet/Types.h>

#include <Wallet/WalletGreen.h>

void printPrivateKeys(CryptoNote::WalletGreen &wallet, bool viewWallet);

void help(bool viewWallet);

void reset(CryptoNote::INode &node, std::shared_ptr<WalletInfo> &walletInfo);

void blockchainHeight(CryptoNote::INode &node, CryptoNote::WalletGreen &wallet);

void balance(CryptoNote::INode &node, CryptoNote::WalletGreen &wallet,
             bool viewWallet);

void exportKeys(std::shared_ptr<WalletInfo> &walletInfo);
