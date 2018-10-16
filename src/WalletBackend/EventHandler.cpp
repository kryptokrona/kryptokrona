// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

///////////////////////////////////////
#include <WalletBackend/EventHandler.h>
///////////////////////////////////////

void EventHandler::registerOnSynced(std::function<void(int)> function)
{
    onSyncedFunction = function;
}

void EventHandler::deregisterOnSynced()
{
    /* Reset to a default unhooked up function */
    onSyncedFunction = {};
}

void EventHandler::fireOnSynced(uint64_t blockHeight)
{
    /* Check the event is hooked up */
    if (onSyncedFunction)
    {
        onSyncedFunction(blockHeight);
    }
}
