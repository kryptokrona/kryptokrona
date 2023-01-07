// Copyright 2014-2018 The Monero Developers
// Copyright 2018 The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "CryptoNote.h"

#include <tuple>

#include <vector>

#include <errors/errors.h>

namespace Mnemonics
{
    std::tuple<Error, Crypto::SecretKey> MnemonicToPrivateKey(const std::string words);

    std::tuple<Error, Crypto::SecretKey> MnemonicToPrivateKey(const std::vector<std::string> words);

    std::string PrivateKeyToMnemonic(const Crypto::SecretKey privateKey);

    bool HasValidChecksum(const std::vector<std::string> words);

    std::string GetChecksumWord(const std::vector<std::string> words);

    std::vector<int> GetWordIndexes(const std::vector<std::string> words);
}
