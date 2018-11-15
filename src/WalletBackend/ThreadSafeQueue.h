// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <condition_variable>

#include <mutex>

#include <queue>

#include <WalletBackend/Constants.h>

template <typename T>
class ThreadSafeQueue
{
    public:
        ThreadSafeQueue() :
            m_shouldStop(false)
        {
        }

        void push(T item)
        {
            /* Aquire the lock */
            std::unique_lock<std::mutex> lock(m_mutex);
			
            /* Stopping, don't push data */
            if (m_shouldStop)
            {
                return;
            }

            if (m_queue.size() >= Constants::MAXIMUM_SYNC_QUEUE_SIZE)
            {
                m_consumedBlock.wait(lock, [&]
                {
                    /* Stopping, don't block */
                    if (m_shouldStop)
                    {
                        return true;
                    }

                    /* Wait for the queue size to fall below the maximum size
                       before pushing our data */
                    return m_queue.size() < Constants::MAXIMUM_SYNC_QUEUE_SIZE;
                });
            }

            /* Add the item to the front of the queue */
            m_queue.push(item);

            /* Unlock the mutex before notifying, so it doesn't block after
               waking up */
            lock.unlock();

            /* Notify the consumer that we have some data */
            m_haveData.notify_all();
        }

        /* Take an item from the front of the queue */
        T pop()
        {
            /* Aquire the lock */
            std::unique_lock<std::mutex> lock(m_mutex);
			
            T item;

            /* Stopping, don't return data */
            if (m_shouldStop)
            {
                return item;
            }
                
            /* Wait for data to become available (releases the lock whilst
               it's not, so we don't block the producer) */
            m_haveData.wait(lock, [&]
            { 
                /* Stopping, don't block */
                if (m_shouldStop)
                {
                    return true;
                }

                return !m_queue.empty();
            });

            /* Stopping, don't return data */
            if (m_shouldStop)
            {
                return item;
            }

            /* Get the first item in the queue */
            item = m_queue.front();

            /* Remove the first item from the queue */
            m_queue.pop();

            /* Unlock the mutex before notifying, so it doesn't block after
               waking up */
            lock.unlock();

            m_consumedBlock.notify_all();
			
            /* Return the item */
            return item;
        }

        /* Stop the queue if something is waiting on it, so we don't block
           whilst closing */
        void stop()
        {
            /* Make sure the queue knows to return */
            m_shouldStop = true;

            /* Wake up anything waiting on data */
            m_haveData.notify_all();

	    /* Make sure not to call .unlock() on the mutex here - it's
	       undefined behaviour if it isn't locked. */

            m_consumedBlock.notify_all();
        }

        void start()
        {
            m_shouldStop = false;
        }

    private:
        /* The deque data structure */
        std::queue<T> m_queue;

        /* The mutex, to ensure we have atomic access to the queue */
        std::mutex m_mutex;

        /* Whether we have data or not */
        std::condition_variable m_haveData;

        /* Triggered when a block is consumed */
        std::condition_variable m_consumedBlock;

        /* Whether we're stopping */
        std::atomic<bool> m_shouldStop;
};
