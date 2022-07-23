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
file(GLOB_RECURSE Crypto crypto/*)
file(GLOB_RECURSE CryptoNoteCore CryptoNoteCore/* CryptoNoteConfig.h)
file(GLOB_RECURSE CryptoNoteProtocol CryptoNoteProtocol/*)
file(GLOB_RECURSE CryptoTest CryptoTest/*)
file(GLOB_RECURSE Errors Errors/*)
file(GLOB_RECURSE Http HTTP/*)
file(GLOB_RECURSE JsonRpcServer JsonRpcServer/*)
file(GLOB_RECURSE Logging Logging/*)
file(GLOB_RECURSE miner Miner/*)
file(GLOB_RECURSE Mnemonics Mnemonics/*)
file(GLOB_RECURSE Nigel Nigel/*)
file(GLOB_RECURSE NodeRpcProxy NodeRpcProxy/*)
file(GLOB_RECURSE P2p P2p/*)
file(GLOB_RECURSE Rpc Rpc/*)
file(GLOB_RECURSE Serialization Serialization/*)
file(GLOB_RECURSE service WalletService/*)
file(GLOB_RECURSE SubWallets SubWallets/*)
file(GLOB_RECURSE Transfers Transfers/*)
file(GLOB_RECURSE kryptokronad Daemon/*)
file(GLOB_RECURSE Utilities Utilities/*)
file(GLOB_RECURSE Wallet Wallet/*)
file(GLOB_RECURSE WalletApi WalletApi/*)
file(GLOB_RECURSE WalletBackend WalletBackend/*)
file(GLOB_RECURSE zedwallet zedwallet/*)
file(GLOB_RECURSE zedwallet++ zedwallet++/*)

if(MSVC)
    file(GLOB_RECURSE System System/* Platform/Windows/System/*)
elseif(APPLE)
    file(GLOB_RECURSE System System/* Platform/OSX/System/* Platform/Posix/System/*)
else()
    file(GLOB_RECURSE System System/* Platform/Linux/System/* Platform/Posix/System/*)
endif()

# Group the files together in IDEs
source_group("" FILES $${Common} ${Crypto} ${CryptoNoteCore} ${CryptoNoteProtocol} ${kryptokronad} ${JsonRpcServer} ${Http} ${Logging} ${miner} ${Mnemonics} ${Nigel} ${NodeRpcProxy} ${P2p} ${Rpc} ${Serialization} ${System} ${Transfers} ${Wallet} ${WalletApi} ${WalletBackend} ${zedwallet} ${zedwallet++} ${CryptoTest} ${Errors} ${Utilities} ${SubWallets})

# Define a group of files as a library to link against
add_library(blockchain_explorer STATIC ${blockchain_explorer})
add_library(common STATIC ${common})
add_library(Crypto STATIC ${Crypto})
add_library(CryptoNoteCore STATIC ${CryptoNoteCore})
add_library(Errors STATIC ${Errors})
add_library(Http STATIC ${Http})
add_library(JsonRpcServer STATIC ${JsonRpcServer})
add_library(Logging STATIC ${Logging})
add_library(Mnemonics STATIC ${Mnemonics})
add_library(Nigel STATIC ${Nigel})
add_library(NodeRpcProxy STATIC ${NodeRpcProxy})
add_library(P2P STATIC ${CryptoNoteProtocol} ${P2p})
add_library(Rpc STATIC ${Rpc})
add_library(Serialization STATIC ${Serialization})
add_library(SubWallets STATIC ${SubWallets})
add_library(System STATIC ${System})
add_library(Transfers STATIC ${Transfers})
add_library(Utilities STATIC ${Utilities})
add_library(Wallet STATIC ${Wallet})
add_library(WalletBackend STATIC ${WalletBackend})
add_library(WalletService STATIC ${service})

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

add_executable(cryptotest ${CryptoTest} ${CT_SOURCES_OS})
add_executable(miner ${miner} ${MINER_SOURCES_OS})
add_executable(service ${service} ${PG_SOURCES_OS})
add_executable(kryptokronad ${kryptokronad} ${DAEMON_SOURCES_OS})
add_executable(WalletApi ${WalletApi} ${WALLET_API_SOURCES_OS})
add_executable(zedwallet ${zedwallet} ${ZED_WALLET_SOURCES_OS})
add_executable(zedwallet++ ${zedwallet++} ${ZED_WALLET_SOURCES_OS})

if(MSVC)
    target_link_libraries(System ws2_32)
    target_link_libraries(kryptokronad Rpcrt4)
    target_link_libraries(service Rpcrt4)
endif ()

# A bit of hackery so we don't have to do the if/else/ for every target that
# wants to use filesystem
add_library(__filesystem INTERFACE)

# Windows works out of the box
if (APPLE)
    target_link_libraries(__filesystem INTERFACE /usr/local/opt/llvm/lib/libc++fs.a)
elseif (UNIX)
    target_link_libraries(__filesystem INTERFACE stdc++fs)
endif()

if(MSVC)
	target_link_libraries(kryptokronad System CryptoNoteCore rocksdb ${Boost_LIBRARIES})
else()
	target_link_libraries(kryptokronad System CryptoNoteCore rocksdblib ${Boost_LIBRARIES})
endif()

# Add the dependencies we need
target_link_libraries(common __filesystem)
target_link_libraries(CryptoNoteCore common Logging Crypto P2P Rpc Http Serialization System ${Boost_LIBRARIES})
target_link_libraries(cryptotest Crypto common)
target_link_libraries(Errors SubWallets)
target_link_libraries(Logging common)
target_link_libraries(miner CryptoNoteCore Rpc System Http Crypto Errors Utilities)
target_link_libraries(Nigel Errors)
target_link_libraries(P2P CryptoNoteCore upnpc-static)
target_link_libraries(Rpc P2P Utilities)
target_link_libraries(service JsonRpcServer Wallet Mnemonics Errors)
target_link_libraries(SubWallets Utilities CryptoNoteCore)
target_link_libraries(Wallet NodeRpcProxy Transfers CryptoNoteCore common ${Boost_LIBRARIES})
target_link_libraries(WalletApi WalletBackend)
target_link_libraries(WalletBackend Mnemonics CryptoNoteCore Nigel cryptopp-static __filesystem Utilities SubWallets)
target_link_libraries(WalletService Mnemonics)
target_link_libraries(zedwallet Mnemonics Wallet Errors Utilities)
target_link_libraries(zedwallet++ WalletBackend)

# Add dependencies means we have to build the latter before we build the former
# In this case it's because we need to have the current version name rather
# than a cached one
add_dependencies(cryptotest version)
add_dependencies(miner version)
add_dependencies(JsonRpcServer version)
add_dependencies(P2P version)
add_dependencies(Rpc version)
add_dependencies(service version)
add_dependencies(kryptokronad version)
add_dependencies(WalletApi version)
add_dependencies(WalletService version)
add_dependencies(zedwallet version)

# Finally build the binaries
set_property(TARGET kryptokronad PROPERTY OUTPUT_NAME "kryptokronad")
set_property(TARGET zedwallet PROPERTY OUTPUT_NAME "xkrwallet")
set_property(TARGET zedwallet++ PROPERTY OUTPUT_NAME "xkrwallet-beta")
set_property(TARGET service PROPERTY OUTPUT_NAME "kryptokrona-service")
set_property(TARGET miner PROPERTY OUTPUT_NAME "miner")
set_property(TARGET cryptotest PROPERTY OUTPUT_NAME "cryptotest")
set_property(TARGET WalletApi PROPERTY OUTPUT_NAME "wallet-api")

# Additional make targets, can be used to build a subset of the targets
# e.g. make pool will build only kryptokronad and service
add_custom_target(pool DEPENDS kryptokronad service)
add_custom_target(solominer DEPENDS kryptokronad zedwallet miner)
add_custom_target(cli DEPENDS kryptokronad zedwallet)
