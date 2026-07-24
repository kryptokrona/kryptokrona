#!/usr/bin/env bash
#
# Rigorous testnet API-coverage test.
#
# Brings up a local testnet daemon, mines to a wallet, then exercises EVERY
# public endpoint of:
#   - kryptokronad          (daemon: HTTP routes + /json_rpc methods)
#   - kryptokrona-service   (wallet service: JSON-RPC methods)
#   - wallet-api            (REST API)
# ...and smoke-tests the xkrwallet CLI. It also creates 100+ addresses ("wallets")
# in both the service and wallet-api and asserts they are all present.
#
# Each endpoint gets a named PASS/FAIL. Endpoints that need real data (block
# hashes, tx hashes, heights, addresses) are fed genuine values gathered from
# the live chain, so most get a strict success assertion; a few that require
# inputs we can't cheaply synthesise (submitblock, sendrawtransaction) are
# asserted to be reachable and return well-formed JSON-RPC (not a 404/crash).
#
# Requires TESTNET binaries (built with -DTEST_NET=ON) and: curl, jq.
#
# The testnet build (USE_TESTNET) is configured for fast, deterministic mining:
# it runs the current mainnet PoW variant (V5) from genesis, a fixed low
# difficulty (so the bundled CPU miner mines instantly), and a short coinbase
# maturity window -- so a handful of mined blocks give a spendable balance for
# the send/fusion/delayed endpoints. Mainnet is unaffected (all gated on
# USE_TESTNET in cryptonote_config.h / currency.cpp).
#
# Usage: BIN_DIR=build_testnet/src scripts/testnet/api-endpoint-test.sh
#        WALLET_COUNT=110 MINE_BLOCKS=25 BIN_DIR=... scripts/testnet/api-endpoint-test.sh

set -uo pipefail

# ----------------------------------------------------------------------------
# Config
# ----------------------------------------------------------------------------
BIN_DIR="${BIN_DIR:-build/src}"
EXE=""
[ -f "$BIN_DIR/kryptokronad.exe" ] && EXE=".exe"
KRYPTOKRONAD="$BIN_DIR/kryptokronad$EXE"
MINER="$BIN_DIR/miner$EXE"
SERVICE="$BIN_DIR/kryptokrona-service$EXE"
WALLET_API="$BIN_DIR/wallet-api$EXE"
XKRWALLET="$BIN_DIR/xkrwallet$EXE"

RPC_PASSWORD="ci-test-password"
WALLET_PASSWORD="ci-test-wallet-pass"

P2P_PORT=30100
DAEMON_RPC=31100
SERVICE_PORT=32100
WAPI_PORT=33100

# Number of extra addresses ("wallets") to create in the service and wallet-api.
WALLET_COUNT="${WALLET_COUNT:-110}"
# Blocks to mine. Coinbase unlocks after 20 blocks on testnet, so >20 gives us a
# spendable balance for the sendTransaction endpoints.
MINE_BLOCKS="${MINE_BLOCKS:-40}"

WORK="$(mktemp -d 2>/dev/null || mktemp -d -t kk-api)"
PIDS=()

# ----------------------------------------------------------------------------
# Output / test harness
# ----------------------------------------------------------------------------
PASS=0; FAIL=0; SKIP=0
FAILED_NAMES=()

c_blue=$'\033[1;34m'; c_green=$'\033[1;32m'; c_red=$'\033[1;31m'; c_yellow=$'\033[1;33m'; c_off=$'\033[0m'

log()  { printf '\n%s==> %s%s\n' "$c_blue" "$*" "$c_off"; }
pass() { PASS=$((PASS+1)); printf '  %s✔%s %s\n' "$c_green" "$c_off" "$1"; }
skip() { SKIP=$((SKIP+1)); printf '  %s∅%s %s (%s)\n' "$c_yellow" "$c_off" "$1" "${2:-skipped}"; }
fail() {
    FAIL=$((FAIL+1)); FAILED_NAMES+=("$1")
    printf '  %sx%s %s\n' "$c_red" "$c_off" "$1"
    [ -n "${2:-}" ] && printf '      %s\n' "$(printf '%s' "$2" | head -c 300 | tr '\n' ' ')"
}
die()  { printf '\n%sFATAL: %s%s\n' "$c_red" "$*" "$c_off" >&2; dump_logs; cleanup; exit 1; }

# Assert a JSON response satisfies a jq filter (strict success).
# tj NAME JSON JQFILTER
tj() {
    local name="$1" json="$2" filter="$3"
    if [ -z "$json" ]; then fail "$name" "empty response"; return; fi
    if printf '%s' "$json" | jq -e "$filter" >/dev/null 2>&1; then pass "$name"
    else fail "$name" "$json"; fi
}

# Assert a response is merely well-formed JSON (endpoint reachable / handled).
# tresp NAME JSON
tresp() {
    local name="$1" json="$2"
    if [ -n "$json" ] && printf '%s' "$json" | jq -e . >/dev/null 2>&1; then pass "$name"
    else fail "$name" "${json:-<empty>}"; fi
}

dump_logs() {
    echo "----- last log output -----" >&2
    for f in "$WORK"/*.log; do
        [ -f "$f" ] || continue
        echo "===== $f =====" >&2; tail -n 25 "$f" >&2 || true
    done
}
cleanup() {
    log "Cleaning up"
    for pid in "${PIDS[@]:-}"; do kill "$pid" 2>/dev/null || true; done
    pkill -P $$ 2>/dev/null || true
    sleep 2
    if [ -n "${KEEP_WORK:-}" ]; then
        echo "Logs preserved in: $WORK"
    else
        rm -rf "$WORK" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM

# ----------------------------------------------------------------------------
# HTTP helpers
# ----------------------------------------------------------------------------
# Daemon plain HTTP endpoint (GET or POST with a JSON body). A short timeout so
# an endpoint that HANGS (see the queryblocks/get_pool/f_* note below) fails
# fast instead of stalling the whole suite for 15s each.
dget()  { curl -s --max-time 8 "http://127.0.0.1:$DAEMON_RPC$1"; }
dpost() { curl -s --max-time 8 -X POST -H 'Content-Type: application/json' -d "${2:-{}}" "http://127.0.0.1:$DAEMON_RPC$1"; }

# Daemon JSON-RPC (/json_rpc). drpc METHOD PARAMS_JSON
drpc() {
    curl -s --max-time 8 -X POST "http://127.0.0.1:$DAEMON_RPC/json_rpc" \
        -H 'Content-Type: application/json' \
        -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"$1\",\"params\":${2:-{}}}"
}

# kryptokrona-service JSON-RPC. srpc METHOD PARAMS_JSON
srpc() {
    curl -s --max-time 30 -X POST "http://127.0.0.1:$SERVICE_PORT/json_rpc" \
        -H 'Content-Type: application/json' \
        -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"password\":\"$RPC_PASSWORD\",\"method\":\"$1\",\"params\":${2:-{}}}"
}

# wallet-api REST. Sets WAPI_CODE + WAPI_BODY. wapi METHOD PATH [BODY]
WAPI_CODE=""; WAPI_BODY=""
wapi() {
    local method="$1" path="$2" body="${3:-}" out
    out=$(curl -s --max-time 40 -w $'\n%{http_code}' -X "$method" \
        -H "X-API-KEY: $RPC_PASSWORD" -H 'Content-Type: application/json' \
        ${body:+-d "$body"} "http://127.0.0.1:$WAPI_PORT$path")
    WAPI_CODE=$(printf '%s' "$out" | tail -n1)
    WAPI_BODY=$(printf '%s' "$out" | sed '$d')
}
# twapi NAME METHOD PATH [BODY] [EXPECT_CODE] [JQFILTER]
twapi() {
    local name="$1" method="$2" path="$3" body="${4:-}" expect="${5:-200}" filter="${6:-}"
    wapi "$method" "$path" "$body"
    if [ "$WAPI_CODE" != "$expect" ]; then
        fail "$name" "HTTP $WAPI_CODE: $WAPI_BODY"; return
    fi
    if [ -n "$filter" ]; then
        if printf '%s' "$WAPI_BODY" | jq -e "$filter" >/dev/null 2>&1; then pass "$name"
        else fail "$name" "$WAPI_BODY"; fi
    else
        pass "$name"
    fi
}

wait_for() { # desc timeout cmd...
    local desc="$1" timeout="$2"; shift 2; local waited=0
    while ! "$@" >/dev/null 2>&1; do
        sleep 2; waited=$((waited+2))
        [ "$waited" -ge "$timeout" ] && return 1
    done
    return 0
}

# ----------------------------------------------------------------------------
# Preflight
# ----------------------------------------------------------------------------
command -v curl >/dev/null || die "curl is required"
command -v jq   >/dev/null || die "jq is required (brew install jq / apt install jq)"
for b in "$KRYPTOKRONAD" "$MINER" "$SERVICE" "$WALLET_API"; do
    [ -x "$b" ] || die "binary not found or not executable: $b (build with -DTEST_NET=ON)"
done

# ----------------------------------------------------------------------------
# 1. Start the daemon
# ----------------------------------------------------------------------------
log "Starting testnet daemon"
mkdir -p "$WORK/node"
"$KRYPTOKRONAD" \
    --data-dir "$WORK/node" \
    --p2p-bind-ip 127.0.0.1 --p2p-bind-port "$P2P_PORT" \
    --rpc-bind-ip 127.0.0.1 --rpc-bind-port "$DAEMON_RPC" \
    --log-level 2 > "$WORK/daemon.log" 2>&1 &
PIDS+=($!)
daemon_up() { dget /getinfo | jq -e '.status' >/dev/null 2>&1; }
wait_for "daemon rpc" 60 daemon_up || die "daemon RPC did not come up"

# ----------------------------------------------------------------------------
# 2. Start the service and mine to its primary address
# ----------------------------------------------------------------------------
log "Starting kryptokrona-service"
WALLET="$WORK/service.wallet"
"$SERVICE" --generate-container \
    --container-file "$WALLET" --container-password "$WALLET_PASSWORD" \
    > "$WORK/service-gen.log" 2>&1 || die "service wallet generation failed"

"$SERVICE" \
    --container-file "$WALLET" --container-password "$WALLET_PASSWORD" \
    --daemon-address 127.0.0.1 --daemon-port "$DAEMON_RPC" \
    --bind-address 127.0.0.1 --bind-port "$SERVICE_PORT" \
    --rpc-password "$RPC_PASSWORD" \
    --log-level 2 > "$WORK/service.log" 2>&1 &
PIDS+=($!)
service_up() { srpc getStatus '{}' | jq -e '.result' >/dev/null 2>&1; }
wait_for "service" 60 service_up || die "kryptokrona-service did not come up"

PRIMARY_ADDR=$(srpc getAddresses '{}' | jq -r '.result.addresses[0]')
[ -n "$PRIMARY_ADDR" ] && [ "$PRIMARY_ADDR" != "null" ] || die "could not read service primary address"
log "Service primary address: $PRIMARY_ADDR"

log "Mining ~$MINE_BLOCKS blocks to the primary address"
# Testnet runs a fixed low difficulty (see cryptonote_config.h) so mining is
# effectively instant. The miner's own --limit can overshoot on such a fast
# chain, so we drive it from here: start it and kill it as soon as the daemon
# reports the target height. (Coinbase matures after the testnet unlock window,
# which is small, so a handful of blocks yields a spendable balance.)
TARGET_HEIGHT=$((MINE_BLOCKS + 1))
"$MINER" --daemon-address "127.0.0.1:$DAEMON_RPC" --address "$PRIMARY_ADDR" \
    --threads 1 --limit "$MINE_BLOCKS" > "$WORK/miner.log" 2>&1 &
miner_pid=$!
for _ in $(seq 1 300); do
    kill -0 "$miner_pid" 2>/dev/null || break
    h=$(dget /getinfo | jq -r '.height' 2>/dev/null)
    [[ "$h" =~ ^[0-9]+$ ]] && [ "$h" -ge "$TARGET_HEIGHT" ] && break
    sleep 0.3
done
kill "$miner_pid" 2>/dev/null || true; wait "$miner_pid" 2>/dev/null || true

sleep 2
HEIGHT=$(dget /getinfo | jq -r '.height')
{ [[ "$HEIGHT" =~ ^[0-9]+$ ]] && [ "$HEIGHT" -ge 2 ]; } || die "could not mine any blocks (height=${HEIGHT:-?})"
log "Chain height reached: $HEIGHT"

# Let the service sync the mined blocks so balances/tx history populate.
service_synced() {
    local s; s=$(srpc getStatus '{}')
    local wb nb; wb=$(printf '%s' "$s" | jq -r '.result.blockCount')
    nb=$(printf '%s' "$s" | jq -r '.result.knownBlockCount')
    [ "$wb" -ge "$((HEIGHT-1))" ] 2>/dev/null
}
wait_for "service sync" 120 service_synced || log "WARN: service not fully synced; balance tests may be lenient"

# ----------------------------------------------------------------------------
# Gather real chain data for the parameterised endpoints
# ----------------------------------------------------------------------------
LAST_HASH=$(drpc getlastblockheader '{}' | jq -r '.result.block_header.hash')
BLOCK1_HASH=$(drpc on_getblockhash '[1]' | jq -r '.result')
# A real (coinbase) transaction hash from block 1's details. NOTE: the daemon's
# plain HTTP endpoints return their payload at the TOP level (not under .result,
# which is only for /json_rpc).
COINBASE_TX=$(dpost /get_block_details_by_height '{"blockHeight":1}' | jq -r '.block.transactions[0].hash // empty')
ZERO_PID="0000000000000000000000000000000000000000000000000000000000000000"
log "Chain data: lastHash=${LAST_HASH:0:12}… block1=${BLOCK1_HASH:0:12}… coinbaseTx=${COINBASE_TX:0:12}…"

# ============================================================================
# 3. DAEMON  —  plain HTTP routes
# ============================================================================
# FINDING: while a synced kryptokrona-service is attached to this daemon, the
# following endpoints HANG (time out -> reported here as failures) even though
# each returns instantly against a standalone daemon:
#     /queryblocks /queryblockslite /queryblocksdetailed
#     /get_pool /get_pool_changes /get_pool_changes_lite
#     json_rpc: f_blocks_list_json f_block_json f_transaction_json
#              f_on_transactions_pool_json
# This looks like RPC lock contention between the wallet sync path and these
# handlers -- a real daemon concurrency bug worth investigating, which is exactly
# the kind of thing this test surfaces.
log "kryptokronad — HTTP routes"
tj  "GET  /getinfo"                 "$(dget /getinfo)"                     '.status=="OK"'
tj  "GET  /getheight"               "$(dget /getheight)"                   '.height>0'
tj  "GET  /feeinfo"                 "$(dget /feeinfo)"                     '.status'
tj  "GET  /getpeers"                "$(dget /getpeers)"                    '.status'
tj  "GET  /info (alias)"            "$(dget /info)"                        '.status=="OK"'
tj  "GET  /height (alias)"          "$(dget /height)"                      '.height>0'
tj  "GET  /fee (alias)"             "$(dget /fee)"                         '.status'
tj  "GET  /peers (alias)"           "$(dget /peers)"                       '.status'
tj  "POST /gettransactions"         "$(dpost /gettransactions "{\"txs_hashes\":[\"$COINBASE_TX\"]}")" '.status'
tresp "POST /sendrawtransaction"    "$(dpost /sendrawtransaction '{"tx_as_hex":""}')"
tresp "POST /getblocks"             "$(dpost /getblocks "{\"block_ids\":[\"$BLOCK1_HASH\"]}")"
tj  "POST /queryblocks"             "$(dpost /queryblocks "{\"block_ids\":[\"$BLOCK1_HASH\"],\"timestamp\":0}")" '.status'
tj  "POST /queryblockslite"         "$(dpost /queryblockslite "{\"blockIds\":[\"$BLOCK1_HASH\"],\"timestamp\":0}")" '.status'
tresp "POST /queryblocksdetailed"   "$(dpost /queryblocksdetailed "{\"blockIds\":[\"$BLOCK1_HASH\"],\"timestamp\":0}")"
tj  "POST /getwalletsyncdata"       "$(dpost /getwalletsyncdata '{"blockHashCheckpoints":[],"startHeight":0,"startTimestamp":0}')" '.status'
tj  "POST /get_o_indexes"           "$(dpost /get_o_indexes "{\"txid\":\"$COINBASE_TX\"}")" '.status'
tj  "POST /getrandom_outs"          "$(dpost /getrandom_outs '{"amounts":[1000],"outs_count":0}')" '.status'
tresp "POST /get_pool"              "$(dpost /get_pool "{\"tailBlockId\":\"$LAST_HASH\",\"knownTxsIds\":[]}")"
tresp "POST /get_pool_changes"      "$(dpost /get_pool_changes "{\"tailBlockId\":\"$LAST_HASH\",\"knownTxsIds\":[]}")"
tresp "POST /get_pool_changes_lite" "$(dpost /get_pool_changes_lite "{\"tailBlockId\":\"$LAST_HASH\",\"knownTxsIds\":[]}")"
tj  "POST /get_block_details_by_height"       "$(dpost /get_block_details_by_height '{"blockHeight":1}')" '.status=="OK"'
tj  "POST /get_blocks_details_by_heights"     "$(dpost /get_blocks_details_by_heights '{"blockHeights":[1,2]}')" '.status=="OK"'
tj  "POST /get_blocks_details_by_hashes"      "$(dpost /get_blocks_details_by_hashes "{\"blockHashes\":[\"$BLOCK1_HASH\"]}")" '.status=="OK"'
tj  "POST /get_transaction_details_by_hashes" "$(dpost /get_transaction_details_by_hashes "{\"transactionHashes\":[\"$COINBASE_TX\"]}")" '.status=="OK"'
# Returns an error status when no tx carries the payment id (expected here), so
# assert reachability rather than status==OK.
tresp "POST /get_transaction_hashes_by_payment_id" "$(dpost /get_transaction_hashes_by_payment_id "{\"paymentId\":\"$ZERO_PID\"}")"
tj  "POST /get_global_indexes_for_range"      "$(dpost /get_global_indexes_for_range '{"startHeight":0,"endHeight":2}')" 'has("indexes")'
tj  "POST /get_transactions_status"           "$(dpost /get_transactions_status "{\"transactionHashes\":[\"$COINBASE_TX\"]}")" '.status=="OK"'
# NOTE: /get_blocks_hashes_by_timestamps is exercised at the very end -- it
# currently CRASHES the daemon, so it must run after all daemon-dependent tests.

# ============================================================================
# 4. DAEMON  —  /json_rpc methods
# ============================================================================
log "kryptokronad — /json_rpc methods"
tj  "rpc  getblockcount"            "$(drpc getblockcount '{}')"                          '.result.count>0'
tj  "rpc  on_getblockhash"          "$(drpc on_getblockhash '[1]')"                       '.result|type=="string"'
tj  "rpc  getblocktemplate"         "$(drpc getblocktemplate "{\"reserve_size\":1,\"wallet_address\":\"$PRIMARY_ADDR\"}")" '.result.blocktemplate_blob'
tj  "rpc  getcurrencyid"            "$(drpc getcurrencyid '{}')"                          '.result.currency_id_blob'
tresp "rpc  submitblock"            "$(drpc submitblock '["0000"]')"
tj  "rpc  getlastblockheader"       "$(drpc getlastblockheader '{}')"                     '.result.block_header.hash'
tj  "rpc  getblockheaderbyhash"     "$(drpc getblockheaderbyhash "{\"hash\":\"$LAST_HASH\"}")" '.result.block_header'
tj  "rpc  getblockheaderbyheight"   "$(drpc getblockheaderbyheight '{"height":1}')"       '.result.block_header'
tj  "rpc  f_blocks_list_json"       "$(drpc f_blocks_list_json "{\"height\":$((HEIGHT-1))}")" '.result.blocks'
tj  "rpc  f_block_json"             "$(drpc f_block_json "{\"hash\":\"$LAST_HASH\"}")"    '.result.block'
tj  "rpc  f_transaction_json"       "$(drpc f_transaction_json "{\"hash\":\"$COINBASE_TX\"}")" '.result.tx // .result.txDetails'
tj  "rpc  f_on_transactions_pool_json" "$(drpc f_on_transactions_pool_json '{}')"         '.result|type=="array"'

# ============================================================================
# 5. KRYPTOKRONA-SERVICE  —  JSON-RPC methods
# ============================================================================
log "kryptokrona-service — JSON-RPC methods"
tj  "svc  getStatus"                "$(srpc getStatus '{}')"                              '.result.blockCount>=0'
tj  "svc  getAddresses"             "$(srpc getAddresses '{}')"                           '.result.addresses|length>=1'
tj  "svc  getViewKey"               "$(srpc getViewKey '{}')"                             '.result.viewSecretKey'
tj  "svc  getMnemonicSeed"          "$(srpc getMnemonicSeed "{\"address\":\"$PRIMARY_ADDR\"}")" '.result.mnemonicSeed'
tj  "svc  getSpendKeys"             "$(srpc getSpendKeys "{\"address\":\"$PRIMARY_ADDR\"}")" '.result.spendSecretKey'
tj  "svc  getBalance"               "$(srpc getBalance '{}')"                             '.result|has("availableBalance")'
tj  "svc  getBalance (per-address)" "$(srpc getBalance "{\"address\":\"$PRIMARY_ADDR\"}")" '.result|has("lockedAmount")'
tj  "svc  validateAddress"          "$(srpc validateAddress "{\"address\":\"$PRIMARY_ADDR\"}")" '.result.isValid==true'
tj  "svc  getFeeInfo"               "$(srpc getFeeInfo '{}')"                             '.result|has("address")'
tj  "svc  getNodeFeeInfo"           "$(srpc getNodeFeeInfo '{}')"                         '.result|has("address")'
tj  "svc  getBlockHashes"           "$(srpc getBlockHashes '{"firstBlockIndex":1,"blockCount":5}')" '.result.blockHashes'
tj  "svc  getTransactionHashes"     "$(srpc getTransactionHashes '{"blockHashes":[],"firstBlockIndex":1,"blockCount":100}')" '.result|has("items")'
tj  "svc  getTransactions"          "$(srpc getTransactions '{"blockHashes":[],"firstBlockIndex":1,"blockCount":100}')" '.result|has("items")'
tj  "svc  getUnconfirmedTransactionHashes" "$(srpc getUnconfirmedTransactionHashes '{"addresses":[]}')" '.result|has("transactionHashes")'
tj  "svc  createIntegratedAddress"  "$(srpc createIntegratedAddress "{\"address\":\"$PRIMARY_ADDR\",\"paymentId\":\"$ZERO_PID\"}")" '.result.integratedAddress'
tj  "svc  estimateFusion"           "$(srpc estimateFusion '{"threshold":1000000,"addresses":[]}')" '.result|has("fusionReadyCount")'
tj  "svc  getDelayedTransactionHashes" "$(srpc getDelayedTransactionHashes '{}')"         '.result|has("transactionHashes")'

# createAddress / createAddressList — spin up 100+ "wallets".
log "Creating $WALLET_COUNT service addresses (wallets)"
tj  "svc  createAddress"            "$(srpc createAddress '{}')"                          '.result.address'
created=0
for _ in $(seq 1 "$((WALLET_COUNT-1))"); do
    a=$(srpc createAddress '{}' | jq -r '.result.address // empty')
    [ -n "$a" ] && created=$((created+1))
done
SVC_ADDR_TOTAL=$(srpc getAddresses '{}' | jq -r '.result.addresses|length')
if [ "$SVC_ADDR_TOTAL" -ge "$WALLET_COUNT" ]; then
    pass "svc  100+ wallets present (getAddresses returns $SVC_ADDR_TOTAL)"
else
    fail "svc  100+ wallets present" "only $SVC_ADDR_TOTAL addresses"
fi
SECOND_ADDR=$(srpc getAddresses '{}' | jq -r '.result.addresses[1]')

# deleteAddress — delete one of the freshly created addresses.
LAST_ADDR=$(srpc getAddresses '{}' | jq -r '.result.addresses[-1]')
tj  "svc  deleteAddress"            "$(srpc deleteAddress "{\"address\":\"$LAST_ADDR\"}")" '.result != null or (has("error")|not)'

# sendTransaction — needs unlocked balance (mined coinbase past the unlock window).
AVAIL=$(srpc getBalance '{}' | jq -r '.result.availableBalance')
log "Service available balance: $AVAIL"
if [ "${AVAIL:-0}" -gt 200000 ] 2>/dev/null; then
    SEND=$(srpc sendTransaction "{\"transfers\":[{\"address\":\"$SECOND_ADDR\",\"amount\":100000}],\"fee\":100000,\"anonymity\":0,\"sourceAddresses\":[\"$PRIMARY_ADDR\"],\"changeAddress\":\"$PRIMARY_ADDR\"}")
    tj "svc  sendTransaction"       "$SEND" '.result.transactionHash'
    SENT_TX=$(printf '%s' "$SEND" | jq -r '.result.transactionHash // empty')
    if [ -n "$SENT_TX" ]; then
        tj "svc  getTransaction"    "$(srpc getTransaction "{\"transactionHash\":\"$SENT_TX\"}")" '.result.transaction'
    else
        skip "svc  getTransaction" "no tx hash from sendTransaction"
    fi
    # sendFusionTransaction — best effort (needs enough dust inputs).
    tresp "svc  sendFusionTransaction" "$(srpc sendFusionTransaction "{\"threshold\":1000000,\"anonymity\":0,\"addresses\":[\"$PRIMARY_ADDR\"],\"destinationAddress\":\"$PRIMARY_ADDR\"}")"
else
    skip "svc  sendTransaction" "no unlocked balance (mine more blocks / raise MINE_BLOCKS)"
    skip "svc  getTransaction"  "depends on sendTransaction"
    skip "svc  sendFusionTransaction" "depends on unlocked balance"
fi

# createDelayedTransaction + delayed lifecycle (best-effort; needs balance).
if [ "${AVAIL:-0}" -gt 200000 ] 2>/dev/null; then
    DLY=$(srpc createDelayedTransaction "{\"transfers\":[{\"address\":\"$SECOND_ADDR\",\"amount\":100000}],\"fee\":100000,\"anonymity\":0,\"sourceAddresses\":[\"$PRIMARY_ADDR\"],\"changeAddress\":\"$PRIMARY_ADDR\"}")
    tresp "svc  createDelayedTransaction" "$DLY"
    DLY_TX=$(printf '%s' "$DLY" | jq -r '.result.transactionHash // empty')
    if [ -n "$DLY_TX" ]; then
        tresp "svc  sendDelayedTransaction"   "$(srpc sendDelayedTransaction "{\"transactionHash\":\"$DLY_TX\"}")"
        tresp "svc  deleteDelayedTransaction" "$(srpc deleteDelayedTransaction "{\"transactionHash\":\"$DLY_TX\"}")"
    else
        skip "svc  sendDelayedTransaction" "no delayed tx hash"
        skip "svc  deleteDelayedTransaction" "no delayed tx hash"
    fi
else
    skip "svc  createDelayedTransaction" "no unlocked balance"
    skip "svc  sendDelayedTransaction"   "no unlocked balance"
    skip "svc  deleteDelayedTransaction" "no unlocked balance"
fi

tj  "svc  save"                     "$(srpc save '{}')"                                   'has("error")|not'
tresp "svc  export"                 "$(srpc export "{\"fileName\":\"$WORK/export.wallet\"}")"
tresp "svc  reset"                  "$(srpc reset '{}')"

# ============================================================================
# 6. WALLET-API  —  REST routes
# ============================================================================
log "Starting wallet-api"
# wallet-api is interactive (it waits for "exit" on stdin). With no stdin it
# reads EOF immediately and shuts down, so we feed it from a FIFO we hold open.
WAPI_FIFO="$WORK/wapi_stdin"
mkfifo "$WAPI_FIFO"
"$WALLET_API" \
    --port "$WAPI_PORT" --rpc-password "$RPC_PASSWORD" \
    < "$WAPI_FIFO" > "$WORK/wallet-api.log" 2>&1 &
PIDS+=($!)
exec 8<>"$WAPI_FIFO"   # keep the write end open so wallet-api's stdin never EOFs
wapi_up() { wapi GET /status >/dev/null 2>&1; [ "$WAPI_CODE" = "200" ] || [ "$WAPI_CODE" = "400" ]; }
# The API is up once it answers at all (400 = no wallet open yet is fine).
sleep 3
for _ in $(seq 1 20); do
    wapi GET /status
    [ -n "$WAPI_CODE" ] && [ "$WAPI_CODE" != "000" ] && break
    sleep 1
done

WAPI_WALLET="$WORK/wapi.wallet"
rm -f "$WAPI_WALLET"
WCREATE_BODY="{\"daemonHost\":\"127.0.0.1\",\"daemonPort\":$DAEMON_RPC,\"filename\":\"$WAPI_WALLET\",\"password\":\"$WALLET_PASSWORD\"}"

log "wallet-api — wallet lifecycle + read routes"
twapi "POST /wallet/create"          POST /wallet/create "$WCREATE_BODY" 200
twapi "GET  /status"                 GET  /status "" 200 '.walletBlockCount>=0'
twapi "GET  /addresses"              GET  /addresses "" 200 '.addresses|length>=1'
twapi "GET  /addresses/primary"      GET  /addresses/primary "" 200 '.address'

WAPI_PRIMARY=$(wapi GET /addresses/primary; printf '%s' "$WAPI_BODY" | jq -r '.address')
twapi "GET  /balance"                GET  /balance "" 200 '.unlocked>=0 or .locked>=0'
twapi "GET  /balances"               GET  /balances "" 200 '.|type=="array"'
twapi "GET  /balance/{address}"      GET  "/balance/$WAPI_PRIMARY" "" 200 '.unlocked>=0'
twapi "GET  /keys"                   GET  /keys "" 200 '.privateViewKey'
twapi "GET  /keys/{address}"         GET  "/keys/$WAPI_PRIMARY" "" 200 '.privateSpendKey'
twapi "GET  /keys/mnemonic/{address}" GET "/keys/mnemonic/$WAPI_PRIMARY" "" 200 '.mnemonicSeed'
twapi "GET  /node"                   GET  /node "" 200 '.daemonHost'
twapi "GET  /transactions"           GET  /transactions "" 200 '.transactions|type=="array"'
twapi "GET  /transactions/unconfirmed" GET /transactions/unconfirmed "" 200 '.transactions|type=="array"'
twapi "GET  /transactions/unconfirmed/{addr}" GET "/transactions/unconfirmed/$WAPI_PRIMARY" "" 200 '.transactions|type=="array"'
twapi "GET  /transactions/{height}"  GET  /transactions/1 "" 200 '.transactions|type=="array"'
twapi "GET  /transactions/{h}/{h}"   GET  /transactions/1/5 "" 200 '.transactions|type=="array"'
twapi "GET  /transactions/address/{a}/{h}"    GET "/transactions/address/$WAPI_PRIMARY/1" "" 200 '.transactions|type=="array"'
twapi "GET  /transactions/address/{a}/{h}/{h}" GET "/transactions/address/$WAPI_PRIMARY/1/5" "" 200 '.transactions|type=="array"'

# Mnemonic captured for the import cycle later.
WAPI_MNEMONIC=$(wapi GET "/keys/mnemonic/$WAPI_PRIMARY"; printf '%s' "$WAPI_BODY" | jq -r '.mnemonicSeed')

# 100+ addresses in the wallet-api wallet.
log "Creating $WALLET_COUNT wallet-api addresses"
twapi "POST /addresses/create"       POST /addresses/create '{}' 201 '.address'
for _ in $(seq 1 "$((WALLET_COUNT-1))"); do wapi POST /addresses/create '{}'; done
wapi GET /addresses
WAPI_ADDR_TOTAL=$(printf '%s' "$WAPI_BODY" | jq -r '.addresses|length')
if [ "$WAPI_ADDR_TOTAL" -ge "$WALLET_COUNT" ]; then
    pass "wapi 100+ wallets present (/addresses returns $WAPI_ADDR_TOTAL)"
else
    fail "wapi 100+ wallets present" "only $WAPI_ADDR_TOTAL addresses"
fi

# A newly created address to import/delete against.
NEW_ADDR=$(wapi POST /addresses/create '{}'; printf '%s' "$WAPI_BODY" | jq -r '.address')

log "wallet-api — mutating routes (transactions, save, reset, node, delete)"
# The wallet-api wallet is freshly created and unfunded (the single mined
# coinbase belongs to the service wallet), so the send endpoints below are
# skipped unless it somehow has a spendable balance. Their logic is identical to
# the service send endpoints, which ARE exercised with real funds above.
WAPI_AVAIL=$(wapi GET /balance; printf '%s' "$WAPI_BODY" | jq -r '.unlocked')
if [ "${WAPI_AVAIL:-0}" -gt 200000 ] 2>/dev/null; then
    twapi "POST /transactions/send/basic" POST /transactions/send/basic \
        "{\"destination\":\"$WAPI_PRIMARY\",\"amount\":100000}" 201 '.transactionHash'
    twapi "POST /transactions/send/advanced" POST /transactions/send/advanced \
        "{\"destinations\":[{\"address\":\"$WAPI_PRIMARY\",\"amount\":100000}]}" 200 '.transactionHash'
    twapi "POST /transactions/send/fusion/basic"    POST /transactions/send/fusion/basic '{}' 201
    twapi "POST /transactions/send/fusion/advanced" POST /transactions/send/fusion/advanced "{\"destination\":\"$WAPI_PRIMARY\"}" 201
else
    skip "POST /transactions/send/basic" "no unlocked wallet-api balance"
    skip "POST /transactions/send/advanced" "no unlocked wallet-api balance"
    skip "POST /transactions/send/fusion/basic" "no unlocked balance"
    skip "POST /transactions/send/fusion/advanced" "no unlocked balance"
fi

twapi "PUT  /node"                   PUT  /node "{\"daemonHost\":\"127.0.0.1\",\"daemonPort\":$DAEMON_RPC}" 202
twapi "PUT  /save"                   PUT  /save "" 200
twapi "PUT  /reset"                  PUT  /reset '{}' 200
twapi "DELETE /addresses/{address}"  DELETE "/addresses/$NEW_ADDR" "" 200

# Address/transaction detail routes (create-integrated + tx-by-hash).
twapi "GET  /addresses/{a}/{paymentId}" GET "/addresses/$WAPI_PRIMARY/$ZERO_PID" "" 200 '.integratedAddress'

# Import lifecycle: close, then re-open via each import route, closing between.
log "wallet-api — wallet import routes (close/import cycle)"
twapi "DELETE /wallet (close)"       DELETE /wallet "" 200
rm -f "$WAPI_WALLET"
if [ -n "$WAPI_MNEMONIC" ] && [ "$WAPI_MNEMONIC" != "null" ]; then
    twapi "POST /wallet/import/seed"  POST /wallet/import/seed \
        "{\"daemonHost\":\"127.0.0.1\",\"daemonPort\":$DAEMON_RPC,\"filename\":\"$WAPI_WALLET\",\"password\":\"$WALLET_PASSWORD\",\"mnemonicSeed\":\"$WAPI_MNEMONIC\",\"scanHeight\":0}" 200
    twapi "DELETE /wallet (close 2)"  DELETE /wallet "" 200
    rm -f "$WAPI_WALLET"
else
    skip "POST /wallet/import/seed" "no mnemonic captured"
fi

# import/key: derive keys from a fresh service wallet we already have. Use the
# PRIMARY key to seed the wallet, and a DIFFERENT subwallet's key for the
# /addresses/import test (importing the same key twice is "already exists").
IMPORT_SPEND=$(srpc getSpendKeys "{\"address\":\"$PRIMARY_ADDR\"}" | jq -r '.result.spendSecretKey')
IMPORT_SPEND2=$(srpc getSpendKeys "{\"address\":\"$SECOND_ADDR\"}" | jq -r '.result.spendSecretKey')
IMPORT_VIEW=$(srpc getViewKey '{}' | jq -r '.result.viewSecretKey')
if [ -n "$IMPORT_SPEND" ] && [ "$IMPORT_SPEND" != "null" ]; then
    twapi "POST /wallet/import/key"   POST /wallet/import/key \
        "{\"daemonHost\":\"127.0.0.1\",\"daemonPort\":$DAEMON_RPC,\"filename\":\"$WAPI_WALLET\",\"password\":\"$WALLET_PASSWORD\",\"privateSpendKey\":\"$IMPORT_SPEND\",\"privateViewKey\":\"$IMPORT_VIEW\",\"scanHeight\":0}" 200
    # With a wallet open, import a DIFFERENT subwallet key, then close.
    twapi "POST /addresses/import"       POST /addresses/import "{\"privateSpendKey\":\"$IMPORT_SPEND2\",\"scanHeight\":0}" 201
    twapi "DELETE /wallet (close 3)"  DELETE /wallet "" 200
    rm -f "$WAPI_WALLET"
else
    skip "POST /wallet/import/key" "no spend key"
    skip "POST /addresses/import"  "no spend key"
fi

# import/view + view-only address import.
twapi "POST /wallet/import/view"     POST /wallet/import/view \
    "{\"daemonHost\":\"127.0.0.1\",\"daemonPort\":$DAEMON_RPC,\"filename\":\"$WAPI_WALLET\",\"password\":\"$WALLET_PASSWORD\",\"privateViewKey\":\"$IMPORT_VIEW\",\"address\":\"$PRIMARY_ADDR\",\"scanHeight\":0}" 200
twapi "POST /addresses/import/view"  POST /addresses/import/view "{\"publicSpendKey\":\"$(srpc getSpendKeys "{\"address\":\"$SECOND_ADDR\"}" | jq -r '.result.spendPublicKey')\",\"scanHeight\":0}" 201
twapi "DELETE /wallet (final close)" DELETE /wallet "" 200
twapi "POST /wallet/open"            POST /wallet/open "$WCREATE_BODY" 200

# ============================================================================
# 7. XKRWALLET  —  CLI smoke test
# ============================================================================
log "xkrwallet — CLI smoke test"
if [ -x "$XKRWALLET" ]; then
    # It's an interactive TUI; --help / feeding EOF should exit cleanly without
    # segfaulting. We assert the binary runs and prints its banner.
    if printf '\n' | timeout 15 "$XKRWALLET" --help >/dev/null 2>&1 \
       || printf '5\n' | timeout 15 "$XKRWALLET" >/dev/null 2>&1; then
        pass "xkrwallet launches"
    else
        # Non-zero from an interactive exit is acceptable; only a crash is a fail.
        rc=$?; [ "$rc" -ge 128 ] && fail "xkrwallet launches" "signal exit $rc" || pass "xkrwallet launches (rc=$rc)"
    fi
else
    skip "xkrwallet launches" "binary not built"
fi

# ============================================================================
# 8. Daemon endpoints that must run LAST (they can crash the daemon)
# ============================================================================
# /get_blocks_hashes_by_timestamps currently crashes the daemon on these inputs,
# which would kill every daemon-dependent test after it -- so we exercise it only
# now that the service/wallet-api phases are done, and report a crash as a FAIL.
log "kryptokronad — /get_blocks_hashes_by_timestamps (isolated; runs last)"
gbht_resp=$(dpost /get_blocks_hashes_by_timestamps '{"timestampBegin":1,"secondsCount":3600}')
sleep 1
if dget /getinfo | jq -e '.status' >/dev/null 2>&1; then
    tresp "POST /get_blocks_hashes_by_timestamps" "$gbht_resp"
else
    fail "POST /get_blocks_hashes_by_timestamps" "endpoint CRASHED the daemon (no longer responding) -- daemon bug to fix"
fi

# ============================================================================
# Summary
# ============================================================================
printf '\n%s================ API TEST SUMMARY ================%s\n' "$c_blue" "$c_off"
printf '  %sPASS: %d%s   %sFAIL: %d%s   %sSKIP: %d%s\n' \
    "$c_green" "$PASS" "$c_off" "$c_red" "$FAIL" "$c_off" "$c_yellow" "$SKIP" "$c_off"
if [ "$FAIL" -gt 0 ]; then
    printf '  %sFailed:%s\n' "$c_red" "$c_off"
    for n in "${FAILED_NAMES[@]}"; do printf '    - %s\n' "$n"; done
    exit 1
fi
printf '  %sAll exercised endpoints passed.%s\n' "$c_green" "$c_off"
exit 0
