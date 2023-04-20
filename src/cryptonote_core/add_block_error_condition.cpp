// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "add_block_error_condition.h"

namespace cryptonote
{
    namespace error
    {

        AddBlockErrorConditionCategory AddBlockErrorConditionCategory::INSTANCE;

        std::error_condition make_error_condition(AddBlockErrorCondition e)
        {
            return std::error_condition(
                static_cast<int>(e),
                AddBlockErrorConditionCategory::INSTANCE);
        }

    }
}
