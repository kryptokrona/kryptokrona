# Build statically
add_definitions(-DSTATICLIB)

# Add external libraries as include dirs, so we can do #include "httplib" for example
include_directories(${CMAKE_SOURCE_DIR}/external/rocksdb/include)
include_directories(${CMAKE_SOURCE_DIR}/external/cpp-linenoise)
include_directories(${CMAKE_SOURCE_DIR}/external/cpp-httplib)
include_directories(${CMAKE_SOURCE_DIR}/external/nlohmann-json)
include_directories(${CMAKE_SOURCE_DIR}/external/rapidjson)
include_directories(${CMAKE_SOURCE_DIR}/external/cxxopts)
include_directories(${CMAKE_SOURCE_DIR}/external/cryptopp)

# Show cmake where the source files are
# Note, if you add remove a source file, you will need to re-run cmake so it
# can find the new file
file(GLOB_RECURSE blockchain_explorer blockchain_explorer/*)
file(GLOB_RECURSE common common/*)
file(GLOB_RECURSE crypto crypto/*)
file(GLOB_RECURSE cryptonote_core cryptonote_core/* cryptonote_config.h)
file(GLOB_RECURSE cryptonote_protocol cryptonote_protocol/*)
file(GLOB_RECURSE crypto_test crypto_test/*)
file(GLOB_RECURSE errors errors/*)
file(GLOB_RECURSE http http/*)
file(GLOB_RECURSE json_rpc_server json_rpc_server/*)
file(GLOB_RECURSE logging logging/*)
file(GLOB_RECURSE miner miner/*)
file(GLOB_RECURSE mnemonics mnemonics/*)
file(GLOB_RECURSE nigel nigel/*)
file(GLOB_RECURSE node_rpc_proxy node_rpc_proxy/*)
file(GLOB_RECURSE p2p p2p/*)
file(GLOB_RECURSE rpc rpc/*)
file(GLOB_RECURSE serialization serialization/*)
file(GLOB_RECURSE service wallet_service/*)
file(GLOB_RECURSE sub_wallets sub_wallets/*)
file(GLOB_RECURSE transfers transfers/*)
file(GLOB_RECURSE kryptokronad daemon/*)
file(GLOB_RECURSE utilities utilities/*)
file(GLOB_RECURSE wallet wallet/*)
file(GLOB_RECURSE wallet_api wallet_api/*)
file(GLOB_RECURSE wallet_backend wallet_backend/*)
file(GLOB_RECURSE zedwallet zedwallet/*)
file(GLOB_RECURSE zedwallet++ zedwallet++/*)

if(MSVC)
    file(GLOB_RECURSE syst syst/* platform/windows/syst/*)
elseif(APPLE)
    file(GLOB_RECURSE syst syst/* platform/osx/syst/* platform/posix/syst/*)
else()
    file(GLOB_RECURSE syst syst/* platform/linux/syst/* platform/posix/syst/*)
endif()

# Group the files together in IDEs
source_group("" FILES $${common} ${crypto} ${cryptonote_core} ${cryptonote_protocol} ${kryptokronad} ${json_rpc_server} ${http} ${logging} ${miner} ${mnemonics} ${Nigel} ${NodeRpcProxy} ${p2p} ${rpc} ${serialization} ${syst} ${transfers} ${wallet} ${wallet_api} ${wallet_backend} ${zedwallet} ${zedwallet++} ${crypto_test} ${errors} ${utilities} ${sub_wallets})

# Define a group of files as a library to link against
add_library(blockchain_explorer STATIC ${blockchain_explorer})
add_library(common STATIC ${common})
add_library(crypto STATIC ${crypto})
add_library(cryptonote_core STATIC ${cryptonote_core})
add_library(errors STATIC ${errors})
add_library(http STATIC ${http})
add_library(json_rpc_server STATIC ${json_rpc_server})
add_library(logging STATIC ${logging})
add_library(mnemonics STATIC ${mnemonics})
add_library(nigel STATIC ${nigel})
add_library(node_rpc_proxy STATIC ${node_rpc_proxy})
add_library(p2p STATIC ${cryptonote_protocol} ${p2p})
add_library(rpc STATIC ${rpc})
add_library(serialization STATIC ${serialization})
add_library(sub_wallets STATIC ${sub_wallets})
add_library(syst STATIC ${syst})
add_library(transfers STATIC ${transfers})
add_library(utilities STATIC ${utilities})
add_library(wallet STATIC ${wallet})
add_library(wallet_backend STATIC ${wallet_backend})
add_library(wallet_service STATIC ${service})

if(MSVC)
    set(DAEMON_SOURCES_OS
        binary_info/daemon.rc
    )
    set(ZED_WALLET_SOURCES_OS
        binary_info/zedwallet.rc
    )
    set(MINER_SOURCES_OS
        binary_info/miner.rc
    )
    set(PG_SOURCES_OS
        binary_info/service.rc
    )
    set(CT_SOURCES_OS
        binary_info/cryptotest.rc
    )
    set(WALLET_API_SOURCES_OS
        binary_info/walletapi.rc
    )
endif()

add_executable(crypto_test ${crypto_test} ${CT_SOURCES_OS})
add_executable(miner ${miner} ${MINER_SOURCES_OS})
add_executable(service ${service} ${PG_SOURCES_OS})
add_executable(kryptokronad ${kryptokronad} ${DAEMON_SOURCES_OS})
add_executable(wallet_api ${wallet_api} ${WALLET_API_SOURCES_OS})
add_executable(zedwallet ${zedwallet} ${ZED_WALLET_SOURCES_OS})
add_executable(zedwallet++ ${zedwallet++} ${ZED_WALLET_SOURCES_OS})

if(MSVC)
    target_link_libraries(syst ws2_32)
    target_link_libraries(kryptokronad Rpcrt4)
    target_link_libraries(service Rpcrt4)
endif()

# A bit of hackery so we don't have to do the if/else/ for every target that
# wants to use filesystem
add_library(__filesystem INTERFACE)

# Windows works out of the box
if(APPLE)
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
        # Apply rules specific to x64 architecture on macOS
        target_link_libraries(__filesystem INTERFACE /usr/local/opt/llvm/lib/c++/libc++abi.a)
    elseif(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        # Apply rules specific to arm64 architecture on macOS (M1)
        target_link_libraries(__filesystem INTERFACE /opt/homebrew/opt/llvm/lib/c++/libc++abi.a)
    else()
        message(FATAL_ERROR "Unsupported macOS architecture")
    endif()
elseif(UNIX)
    target_link_libraries(__filesystem INTERFACE stdc++fs)
endif()

if(MSVC)
    target_link_libraries(kryptokronad syst cryptonote_core rocksdb ${Boost_LIBRARIES})
else()
    target_link_libraries(kryptokronad syst cryptonote_core rocksdblib ${Boost_LIBRARIES})
endif()

# Add the dependencies we need
target_link_libraries(common __filesystem)
target_link_libraries(cryptonote_core common logging crypto p2p rpc http serialization syst ${Boost_LIBRARIES})
target_link_libraries(crypto_test crypto common)
target_link_libraries(errors sub_wallets)
target_link_libraries(logging common)
target_link_libraries(miner cryptonote_core rpc syst http crypto errors utilities)
target_link_libraries(nigel errors)
target_link_libraries(p2p cryptonote_core upnpc-static)
target_link_libraries(rpc p2p utilities)
target_link_libraries(service json_rpc_server wallet mnemonics errors)
target_link_libraries(sub_wallets utilities cryptonote_core)
target_link_libraries(wallet node_rpc_proxy transfers cryptonote_core common ${Boost_LIBRARIES})
target_link_libraries(wallet_api wallet_backend)
target_link_libraries(wallet_backend mnemonics cryptonote_core nigel cryptopp-static __filesystem utilities sub_wallets)
target_link_libraries(wallet_service mnemonics)
target_link_libraries(zedwallet mnemonics wallet errors utilities)
target_link_libraries(zedwallet++ wallet_backend)

# Add dependencies means we have to build the latter before we build the former
# In this case it's because we need to have the current version name rather
# than a cached one
add_dependencies(crypto_test version)
add_dependencies(miner version)
add_dependencies(json_rpc_server version)
add_dependencies(p2p version)
add_dependencies(rpc version)
add_dependencies(service version)
add_dependencies(kryptokronad version)
add_dependencies(wallet_api version)
add_dependencies(wallet_service version)
add_dependencies(zedwallet version)

# Finally build the binaries
set_property(TARGET kryptokronad PROPERTY OUTPUT_NAME "kryptokronad")
set_property(TARGET zedwallet PROPERTY OUTPUT_NAME "xkrwallet")
set_property(TARGET zedwallet++ PROPERTY OUTPUT_NAME "xkrwallet-beta")
set_property(TARGET service PROPERTY OUTPUT_NAME "kryptokrona-service")
set_property(TARGET miner PROPERTY OUTPUT_NAME "miner")
set_property(TARGET crypto_test PROPERTY OUTPUT_NAME "crypto_test")
set_property(TARGET wallet_api PROPERTY OUTPUT_NAME "wallet-api")

# Additional make targets, can be used to build a subset of the targets
# e.g. make pool will build only kryptokronad and service
add_custom_target(pool DEPENDS kryptokronad service)
add_custom_target(solominer DEPENDS kryptokronad zedwallet miner)
add_custom_target(cli DEPENDS kryptokronad zedwallet)
