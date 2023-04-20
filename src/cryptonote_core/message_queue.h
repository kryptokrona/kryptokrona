// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <queue>

#include "intrusive_linked_list.h"

#include <syst/dispatcher.h>
#include "syst/event.h"
#include "syst/interrupted_exception.h"

namespace cryptonote
{

    template <class MessageType>
    class MessageQueue
    {
    public:
        MessageQueue(syst::Dispatcher &dispatcher);

        const MessageType &front();
        void pop();
        void push(const MessageType &message);

        void stop();

    private:
        friend class IntrusiveLinkedList<MessageQueue<MessageType>>;
        typename IntrusiveLinkedList<MessageQueue<MessageType>>::hook &getHook();
        void wait();

        syst::Dispatcher &dispatcher;
        std::queue<MessageType> messageQueue;
        syst::Event event;
        bool stopped;

        typename IntrusiveLinkedList<MessageQueue<MessageType>>::hook hook;
    };

    template <class MessageQueueContainer, class MessageType>
    class MesageQueueGuard
    {
    public:
        MesageQueueGuard(MessageQueueContainer &container, MessageQueue<MessageType> &messageQueue)
            : container(container), messageQueue(messageQueue)
        {
            container.addMessageQueue(messageQueue);
        }

        ~MesageQueueGuard()
        {
            container.removeMessageQueue(messageQueue);
        }

    private:
        MessageQueueContainer &container;
        MessageQueue<MessageType> &messageQueue;
    };

    template <class MessageType>
    MessageQueue<MessageType>::MessageQueue(syst::Dispatcher &dispatch)
        : dispatcher(dispatch), event(dispatch), stopped(false)
    {
    }

    template <class MessageType>
    void MessageQueue<MessageType>::wait()
    {
        if (messageQueue.empty())
        {
            if (stopped)
            {
                throw syst::InterruptedException();
            }

            event.clear();
            while (!event.get())
            {
                event.wait();

                if (stopped)
                {
                    throw syst::InterruptedException();
                }
            }
        }
    }

    template <class MessageType>
    const MessageType &MessageQueue<MessageType>::front()
    {
        wait();
        return messageQueue.front();
    }

    template <class MessageType>
    void MessageQueue<MessageType>::pop()
    {
        wait();
        messageQueue.pop();
    }

    template <class MessageType>
    void MessageQueue<MessageType>::push(const MessageType &message)
    {
        dispatcher.remoteSpawn([=]() mutable
                               {
    messageQueue.push(std::move(message));
    event.set(); });
    }

    template <class MessageType>
    void MessageQueue<MessageType>::stop()
    {
        stopped = true;
        event.set();
    }

    template <class MessageType>
    typename IntrusiveLinkedList<MessageQueue<MessageType>>::hook &MessageQueue<MessageType>::getHook()
    {
        return hook;
    }
}
