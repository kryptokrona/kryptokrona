// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <cstdint>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "common/std_input_stream.h"
#include "common/std_output_stream.h"
#include "serialization/binary_input_stream_serializer.h"
#include "serialization/binary_output_stream_serializer.h"

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

// Crash-safety helpers for the on-disk main-chain storage (blocks.bin /
// blockindexes.bin). The vector is durable across an *unclean* process exit
// (segfault / SIGKILL) as long as each block's data + index entry reach the OS
// page cache before its count header does, and the files are fsync'd on a
// cadence so a power loss can only ever lose a re-syncable tail. See push_back.
namespace swapped_vector_detail
{
    // Open a second descriptor to an existing file purely so we can fdatasync
    // the underlying inode without disturbing the std::fstream read/write state.
    inline int platformOpenRW(const std::string &name)
    {
#ifdef _WIN32
        return _open(name.c_str(), _O_RDWR | _O_BINARY);
#else
        return ::open(name.c_str(), O_RDWR);
#endif
    }

    inline void platformSync(int fd)
    {
        if (fd < 0)
        {
            return;
        }
#ifdef _WIN32
        _commit(fd);
#else
        // fsync (not fdatasync) for portability: macOS has no fdatasync. The
        // extra metadata sync is negligible at DISK_SYNC_INTERVAL cadence.
        fsync(fd);
#endif
    }

    inline void platformClose(int fd)
    {
        if (fd < 0)
        {
            return;
        }
#ifdef _WIN32
        _close(fd);
#else
        ::close(fd);
#endif
    }

    // Force pending blocks to disk at least this often (in push/pop operations).
    // Consistency (openability) is preserved every block by ordered flushes;
    // this bound only limits how many blocks a power loss can discard.
    const uint64_t DISK_SYNC_INTERVAL = 100;
}

template <class T>
class SwappedVector
{
public:
    typedef T value_type;

    class const_iterator
    {
    public:
        typedef ptrdiff_t difference_type;
        typedef std::random_access_iterator_tag iterator_category;
        typedef const T *pointer;
        typedef const T &reference;
        typedef T value_type;

        const_iterator()
        {
        }

        const_iterator(SwappedVector *swappedVector, size_t index) : m_swappedVector(swappedVector), m_index(index)
        {
        }

        bool operator!=(const const_iterator &other) const
        {
            return m_index != other.m_index;
        }

        bool operator<(const const_iterator &other) const
        {
            return m_index < other.m_index;
        }

        bool operator<=(const const_iterator &other) const
        {
            return m_index <= other.m_index;
        }

        bool operator==(const const_iterator &other) const
        {
            return m_index == other.m_index;
        }

        bool operator>(const const_iterator &other) const
        {
            return m_index > other.m_index;
        }

        bool operator>=(const const_iterator &other) const
        {
            return m_index >= other.m_index;
        }

        const_iterator &operator++()
        {
            ++m_index;
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator i = *this;
            ++m_index;
            return i;
        }

        const_iterator &operator--()
        {
            --m_index;
            return *this;
        }

        const_iterator operator--(int)
        {
            const_iterator i = *this;
            --m_index;
            return i;
        }

        const_iterator &operator+=(difference_type n)
        {
            m_index += n;
            return *this;
        }

        const_iterator &operator-=(difference_type n)
        {
            m_index -= n;
            return *this;
        }

        const_iterator operator+(difference_type n) const
        {
            return const_iterator(m_swappedVector, m_index + n);
        }

        friend const_iterator operator+(difference_type n, const const_iterator &i)
        {
            return const_iterator(i.m_swappedVector, n + i.m_index);
        }

        difference_type operator-(const const_iterator &other) const
        {
            return m_index - other.m_index;
        }

        const_iterator &operator-(difference_type n) const
        {
            return const_iterator(m_swappedVector, m_index - n);
        }

        const T &operator*() const
        {
            return (*m_swappedVector)[m_index];
        }

        const T *operator->() const
        {
            return &(*m_swappedVector)[m_index];
        }

        const T &operator[](difference_type offset) const
        {
            return (*m_swappedVector)[m_index + offset];
        }

        size_t index() const
        {
            return m_index;
        }

    private:
        SwappedVector *m_swappedVector;
        size_t m_index;
    };

    SwappedVector();
    // SwappedVector(const SwappedVector&) = delete;
    ~SwappedVector();
    // SwappedVector& operator=(const SwappedVector&) = delete;

    bool open(const std::string &itemFileName, const std::string &indexFileName, size_t poolSize);
    void close();

    bool empty() const;
    uint64_t size() const;
    const_iterator begin();
    const_iterator end();
    const T &operator[](uint64_t index);
    const T &front();
    const T &back();
    void clear();
    void pop_back();
    void push_back(const T &item);

private:
    struct ItemEntry;
    struct CacheEntry;

    struct ItemEntry
    {
    public:
        T item;
        typename std::list<CacheEntry>::iterator cacheIter;
    };

    struct CacheEntry
    {
    public:
        typename std::map<uint64_t, ItemEntry>::iterator itemIter;
    };

    std::fstream m_itemsFile;
    std::fstream m_indexesFile;
    size_t m_poolSize;
    std::vector<uint64_t> m_offsets;
    uint64_t m_itemsFileSize;
    std::map<uint64_t, ItemEntry> m_items;
    std::list<CacheEntry> m_cache;
    uint64_t m_cacheHits;
    uint64_t m_cacheMisses;

    // Crash-safety: companion descriptors used only for fdatasync, plus a
    // counter that triggers a durable sync every DISK_SYNC_INTERVAL writes.
    int m_itemsSyncFd;
    int m_indexesSyncFd;
    uint64_t m_pendingSync;

    T *prepare(uint64_t index);
    // Flush the fstream buffers to the OS and fdatasync both files to disk.
    void syncToDisk();
};

template <class T>
SwappedVector<T>::SwappedVector() : m_itemsSyncFd(-1), m_indexesSyncFd(-1), m_pendingSync(0)
{
}

template <class T>
SwappedVector<T>::~SwappedVector()
{
    close();
}

template <class T>
bool SwappedVector<T>::open(const std::string &itemFileName, const std::string &indexFileName, size_t poolSize)
{
    if (poolSize == 0)
    {
        return false;
    }

    // Drop any descriptors left over from a previous open() on this instance.
    swapped_vector_detail::platformClose(m_itemsSyncFd);
    swapped_vector_detail::platformClose(m_indexesSyncFd);
    m_itemsSyncFd = -1;
    m_indexesSyncFd = -1;
    m_pendingSync = 0;

    m_itemsFile.open(itemFileName, std::ios::in | std::ios::out | std::ios::binary);
    m_indexesFile.open(indexFileName, std::ios::in | std::ios::out | std::ios::binary);
    if (m_itemsFile && m_indexesFile)
    {
        uint64_t count;
        m_indexesFile.read(reinterpret_cast<char *>(&count), sizeof count);
        if (!m_indexesFile)
        {
            return false;
        }

        std::vector<uint64_t> offsets;
        uint64_t itemsFileSize = 0;
        for (uint64_t i = 0; i < count; ++i)
        {
            uint32_t itemSize;
            m_indexesFile.read(reinterpret_cast<char *>(&itemSize), sizeof itemSize);
            if (!m_indexesFile)
            {
                // Self-healing: an unclean exit can leave the count header ahead
                // of the index entries that were actually persisted. Rather than
                // refusing to boot ("Failed to load main chain storage"), keep the
                // blocks we can read and let the daemon re-sync the torn tail from
                // peers. The stale on-disk count is overwritten by the next push.
                std::cerr << "SwappedVector: index truncated at entry " << i << " of " << count
                          << "; recovering with " << i << " blocks and re-syncing the rest." << std::endl;
                m_indexesFile.clear();
                break;
            }

            offsets.emplace_back(itemsFileSize);
            itemsFileSize += itemSize;
        }

        m_offsets.swap(offsets);
        m_itemsFileSize = itemsFileSize;
    }
    else
    {
        m_itemsFile.open(itemFileName, std::ios::out | std::ios::binary);
        m_itemsFile.close();
        m_itemsFile.open(itemFileName, std::ios::in | std::ios::out | std::ios::binary);
        m_indexesFile.open(indexFileName, std::ios::out | std::ios::binary);
        uint64_t count = 0;
        m_indexesFile.write(reinterpret_cast<char *>(&count), sizeof count);
        if (!m_indexesFile)
        {
            return false;
        }

        m_indexesFile.close();
        m_indexesFile.open(indexFileName, std::ios::in | std::ios::out | std::ios::binary);
        m_offsets.clear();
        m_itemsFileSize = 0;
    }

    // Companion descriptors for durable fdatasync (both files now exist).
    m_itemsSyncFd = swapped_vector_detail::platformOpenRW(itemFileName);
    m_indexesSyncFd = swapped_vector_detail::platformOpenRW(indexFileName);

    m_poolSize = poolSize;
    m_items.clear();
    m_cache.clear();
    m_cacheHits = 0;
    m_cacheMisses = 0;
    return true;
}

template <class T>
void SwappedVector<T>::close()
{
    // A no-op previously: on an unclean exit nothing was ever flushed. Make a
    // clean close fully durable and release the companion descriptors.
    syncToDisk();

    if (m_itemsFile.is_open())
    {
        m_itemsFile.close();
    }

    if (m_indexesFile.is_open())
    {
        m_indexesFile.close();
    }

    swapped_vector_detail::platformClose(m_itemsSyncFd);
    swapped_vector_detail::platformClose(m_indexesSyncFd);
    m_itemsSyncFd = -1;
    m_indexesSyncFd = -1;
}

template <class T>
void SwappedVector<T>::syncToDisk()
{
    // Push fstream buffers into the OS page cache, then force both files to
    // stable storage. Items are synced before indexes so the on-disk state can
    // never reference block data that has not yet been persisted.
    if (m_itemsFile.is_open())
    {
        m_itemsFile.flush();
    }

    if (m_indexesFile.is_open())
    {
        m_indexesFile.flush();
    }

    swapped_vector_detail::platformSync(m_itemsSyncFd);
    swapped_vector_detail::platformSync(m_indexesSyncFd);
    m_pendingSync = 0;
}

template <class T>
bool SwappedVector<T>::empty() const
{
    return m_offsets.empty();
}

template <class T>
uint64_t SwappedVector<T>::size() const
{
    return m_offsets.size();
}

template <class T>
typename SwappedVector<T>::const_iterator SwappedVector<T>::begin()
{
    return const_iterator(this, 0);
}

template <class T>
typename SwappedVector<T>::const_iterator SwappedVector<T>::end()
{
    return const_iterator(this, m_offsets.size());
}

template <class T>
const T &SwappedVector<T>::operator[](uint64_t index)
{
    auto itemIter = m_items.find(index);
    if (itemIter != m_items.end())
    {
        if (itemIter->second.cacheIter != --m_cache.end())
        {
            m_cache.splice(m_cache.end(), m_cache, itemIter->second.cacheIter);
        }

        ++m_cacheHits;
        return itemIter->second.item;
    }

    if (index >= m_offsets.size())
    {
        throw std::runtime_error("SwappedVector::operator[]");
    }

    if (!m_itemsFile)
    {
        throw std::runtime_error("SwappedVector::operator[]");
    }

    m_itemsFile.seekg(m_offsets[index]);
    T tempItem;

    common::StdInputStream stream(m_itemsFile);
    cryptonote::BinaryInputStreamSerializer archive(stream);
    serialize(tempItem, archive);

    T *item = prepare(index);
    std::swap(tempItem, *item);
    ++m_cacheMisses;
    return *item;
}

template <class T>
const T &SwappedVector<T>::front()
{
    return operator[](0);
}

template <class T>
const T &SwappedVector<T>::back()
{
    return operator[](m_offsets.size() - 1);
}

template <class T>
void SwappedVector<T>::clear()
{
    if (!m_indexesFile)
    {
        throw std::runtime_error("SwappedVector::clear");
    }

    m_indexesFile.seekp(0);
    uint64_t count = 0;
    m_indexesFile.write(reinterpret_cast<char *>(&count), sizeof count);
    if (!m_indexesFile)
    {
        throw std::runtime_error("SwappedVector::clear");
    }

    syncToDisk();

    m_offsets.clear();
    m_itemsFileSize = 0;
    m_items.clear();
    m_cache.clear();
}

template <class T>
void SwappedVector<T>::pop_back()
{
    if (!m_indexesFile)
    {
        throw std::runtime_error("SwappedVector::pop_back");
    }

    m_indexesFile.seekp(0);
    uint64_t count = m_offsets.size() - 1;
    m_indexesFile.write(reinterpret_cast<char *>(&count), sizeof count);
    if (!m_indexesFile)
    {
        throw std::runtime_error("SwappedVector::pop_back");
    }

    // Lowering the count is always consistent on its own; flush it out and sync
    // on the usual cadence.
    m_indexesFile.flush();
    if (++m_pendingSync >= swapped_vector_detail::DISK_SYNC_INTERVAL)
    {
        syncToDisk();
    }

    m_itemsFileSize = m_offsets.back();
    m_offsets.pop_back();
    auto itemIter = m_items.find(m_offsets.size());
    if (itemIter != m_items.end())
    {
        m_cache.erase(itemIter->second.cacheIter);
        m_items.erase(itemIter);
    }
}

template <class T>
void SwappedVector<T>::push_back(const T &item)
{
    uint64_t itemsFileSize;

    {
        if (!m_itemsFile)
        {
            throw std::runtime_error("SwappedVector::push_back");
        }

        m_itemsFile.seekp(m_itemsFileSize);

        common::StdOutputStream stream(m_itemsFile);
        cryptonote::BinaryOutputStreamSerializer archive(stream);
        serialize(const_cast<T &>(item), archive);

        itemsFileSize = m_itemsFile.tellp();

        // Barrier 1: the block data must reach the OS before anything in the
        // index references it, so a crash can never leave a dangling entry.
        m_itemsFile.flush();
    }

    {
        if (!m_indexesFile)
        {
            throw std::runtime_error("SwappedVector::push_back");
        }

        m_indexesFile.seekp(sizeof(uint64_t) + sizeof(uint32_t) * m_offsets.size());
        uint32_t itemSize = static_cast<uint32_t>(itemsFileSize - m_itemsFileSize);
        m_indexesFile.write(reinterpret_cast<char *>(&itemSize), sizeof itemSize);
        if (!m_indexesFile)
        {
            throw std::runtime_error("SwappedVector::push_back");
        }

        // Barrier 2: the size entry must reach the OS before the count header
        // that will make it live. This is what guarantees count <= entries on
        // disk after any unclean exit, so open() always succeeds.
        m_indexesFile.flush();

        m_indexesFile.seekp(0);
        uint64_t count = m_offsets.size() + 1;
        m_indexesFile.write(reinterpret_cast<char *>(&count), sizeof count);
        if (!m_indexesFile)
        {
            throw std::runtime_error("SwappedVector::push_back");
        }

        // Commit the new count.
        m_indexesFile.flush();
    }

    m_offsets.push_back(m_itemsFileSize);
    m_itemsFileSize = itemsFileSize;

    if (++m_pendingSync >= swapped_vector_detail::DISK_SYNC_INTERVAL)
    {
        syncToDisk();
    }

    T *newItem = prepare(m_offsets.size() - 1);
    *newItem = item;
}

template <class T>
T *SwappedVector<T>::prepare(uint64_t index)
{
    if (m_items.size() == m_poolSize)
    {
        auto cacheIter = m_cache.begin();
        m_items.erase(cacheIter->itemIter);
        m_cache.erase(cacheIter);
    }

    auto itemIter = m_items.insert(std::make_pair(index, ItemEntry()));
    CacheEntry cacheEntry = {itemIter.first};
    auto cacheIter = m_cache.insert(m_cache.end(), cacheEntry);
    itemIter.first->second.cacheIter = cacheIter;
    return &itemIter.first->second.item;
}
