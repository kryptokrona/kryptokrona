// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <functional>

class EventHandler
{
    public:
        void registerOnSynced(std::function<void(int)> function);

        void deregisterOnSynced();

        void fireOnSynced(uint64_t blockHeight);

    private:
        std::function<void(int)> onSyncedFunction;
};
