// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "database_config.h"

#include <common/util.h>
#include "common/string_tools.h"
#include "crypto/crypto.h"
#include <config/cryptonote_config.h>

using namespace cryptonote;

namespace
{
    const uint64_t MEGABYTE = 1024 * 1024;
}

DataBaseConfig::DataBaseConfig() : dataDir(tools::getDefaultDataDirectory()),
                                   backgroundThreadsCount(DATABASE_DEFAULT_BACKGROUND_THREADS_COUNT),
                                   maxOpenFiles(DATABASE_DEFAULT_MAX_OPEN_FILES),
                                   writeBufferSize(DATABASE_WRITE_BUFFER_MB_DEFAULT_SIZE * MEGABYTE),
                                   readCacheSize(DATABASE_READ_BUFFER_MB_DEFAULT_SIZE * MEGABYTE),
                                   testnet(false),
                                   configFolderDefaulted(false)
{
}

bool DataBaseConfig::init(const std::string dataDirectory, const int backgroundThreads, const int openFiles, const int writeBufferMB, const int readCacheMB)
{
    dataDir = dataDirectory;
    backgroundThreadsCount = backgroundThreads;
    maxOpenFiles = openFiles;
    writeBufferSize = writeBufferMB * MEGABYTE;
    readCacheSize = readCacheMB * MEGABYTE;

    if (dataDir == tools::getDefaultDataDirectory())
    {
        configFolderDefaulted = true;
    }

    return true;
}

bool DataBaseConfig::isConfigFolderDefaulted() const
{
    return configFolderDefaulted;
}

std::string DataBaseConfig::getDataDir() const
{
    return dataDir;
}

uint16_t DataBaseConfig::getBackgroundThreadsCount() const
{
    return backgroundThreadsCount;
}

uint32_t DataBaseConfig::getMaxOpenFiles() const
{
    return maxOpenFiles;
}

uint64_t DataBaseConfig::getWriteBufferSize() const
{
    return writeBufferSize;
}

uint64_t DataBaseConfig::getReadCacheSize() const
{
    return readCacheSize;
}

bool DataBaseConfig::getTestnet() const
{
    return testnet;
}
