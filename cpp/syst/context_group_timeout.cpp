// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "context_group_timeout.h"
#include <syst/interrupted_exception.h>

using namespace syst;

ContextGroupTimeout::ContextGroupTimeout(Dispatcher &dispatcher, ContextGroup &contextGroup, std::chrono::nanoseconds timeout) : workingContextGroup(dispatcher),
                                                                                                                                 timeoutTimer(dispatcher)
{
    workingContextGroup.spawn([&, timeout]
                              {
    try {
      timeoutTimer.sleep(timeout);
      contextGroup.interrupt();
    } catch (InterruptedException&) {
    } });
}
