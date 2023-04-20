// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <cryptonote.h>
#include <functional>

namespace cryptonote
{

    enum class TransactionMessageType
    {
        AddTransactionType,
        DeleteTransactionType
    };

    // immutable messages
    struct AddTransaction
    {
        crypto::Hash hash;
    };

    struct DeleteTransaction
    {
        crypto::Hash hash;
    };

    class TransactionPoolMessage
    {
    public:
        TransactionPoolMessage(const AddTransaction &at);
        TransactionPoolMessage(const DeleteTransaction &at);

        // pattern matchin API
        void match(std::function<void(const AddTransaction &)> &&, std::function<void(const DeleteTransaction &)> &&);

        // API with explicit type handling
        TransactionMessageType getType() const;
        AddTransaction getAddTransaction() const;
        DeleteTransaction getDeleteTransaction() const;

    private:
        const TransactionMessageType type;
        union
        {
            const AddTransaction addTransaction;
            const DeleteTransaction deleteTransaction;
        };
    };

}
