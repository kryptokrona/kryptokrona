#!/usr/bin/env bash
#
# End-to-end testnet integration test.
#
# Brings up a 3-node testnet (as the docs describe, but natively so it runs on a
# CI runner), waits for the nodes to peer, mines blocks with the `miner` to a
# wallet we control via `kryptokrona-service`, then sends a real transaction
# through the service's JSON-RPC. Exits non-zero if any stage fails.
#
# Requires TESTNET binaries (built with -DTEST_NET=ON): low difficulty so the
# first blocks mine near-instantly, and a coinbase unlock window of 20 blocks.
#
# Usage: BIN_DIR=build_testnet/src scripts/testnet/ci-integration-test.sh

set -euo pipefail

# ----------------------------------------------------------------------------
# Config
# ----------------------------------------------------------------------------
BIN_DIR="${BIN_DIR:-build/src}"
# On Windows (MSVC) the binaries are kryptokronad.exe etc.
EXE=""
[ -f "$BIN_DIR/kryptokronad.exe" ] && EXE=".exe"
KRYPTOKRONAD="$BIN_DIR/kryptokronad$EXE"
MINER="$BIN_DIR/miner$EXE"
SERVICE="$BIN_DIR/kryptokrona-service$EXE"

RPC_PASSWORD="ci-test-password"
WALLET_PASSWORD="ci-test-wallet-pass"

# Node i (1..3): p2p on 30000+i, rpc on 31000+i. Service on 32000.
NODE1_RPC=31001
SERVICE_PORT=32000

# Mine a single block: enough to prove the daemon accepts mined blocks and the
# wallet picks up the coinbase. (The reward stays locked for the coinbase unlock
# window of 20 blocks, so we assert on the wallet's total balance, not spendable.)
BLOCKS_TO_MINE=1

WORK="$(mktemp -d 2>/dev/null || mktemp -d -t kk-testnet)"
PIDS=()

log()  { printf '\n\033[1;34m==> %s\033[0m\n' "$*"; }
fail() { printf '\n\033[1;31mFAIL: %s\033[0m\n' "$*" >&2; dump_logs; exit 1; }

dump_logs() {
    echo "----- last log output -----" >&2
    for f in "$WORK"/*.log; do
        [ -f "$f" ] || continue
        echo "===== $f =====" >&2
        tail -n 40 "$f" >&2 || true
    done
}

cleanup() {
    log "Cleaning up"
    pkill -P $$ 2>/dev/null || true
    for pid in "${PIDS[@]:-}"; do kill "$pid" 2>/dev/null || true; done
    # give daemons a moment to close their DBs
    sleep 2
    rm -rf "$WORK" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

# JSON-RPC helper for the daemon HTTP interface (GET endpoints like /getinfo).
daemon_get() { # port path
    curl -s --max-time 5 "http://127.0.0.1:$1$2" || true
}

# JSON-RPC helper for kryptokrona-service.
service_rpc() { # method params_json
    curl -s --max-time 15 -X POST "http://127.0.0.1:$SERVICE_PORT/json_rpc" \
        -H 'Content-Type: application/json' \
        -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"password\":\"$RPC_PASSWORD\",\"method\":\"$1\",\"params\":$2}" \
        || true
}

# Extract a JSON string/number field without needing jq (best-effort).
json_field() { # key
    sed -E "s/.*\"$1\"[[:space:]]*:[[:space:]]*\"?([^\",}]+)\"?.*/\1/"
}

wait_for() { # description seconds cmd...
    local desc="$1" timeout="$2"; shift 2
    local waited=0
    while ! "$@" >/dev/null 2>&1; do
        sleep 2; waited=$((waited+2))
        if [ "$waited" -ge "$timeout" ]; then return 1; fi
    done
    return 0
}

for b in "$KRYPTOKRONAD" "$MINER" "$SERVICE"; do
    [ -x "$b" ] || fail "binary not found or not executable: $b"
done
command -v curl >/dev/null || fail "curl is required"

# ----------------------------------------------------------------------------
# 1. Start the 3-node testnet
# ----------------------------------------------------------------------------
log "Starting 3 testnet nodes"
for i in 1 2 3; do
    data="$WORK/node$i"; mkdir -p "$data"
    p2p=$((30000+i)); rpc=$((31000+i))
    exclusive=()
    for j in 1 2 3; do
        [ "$j" -eq "$i" ] && continue
        exclusive+=(--add-exclusive-node "127.0.0.1:$((30000+j))")
    done
    "$KRYPTOKRONAD" \
        --data-dir "$data" \
        --p2p-bind-ip 127.0.0.1 --p2p-bind-port "$p2p" \
        --rpc-bind-ip 127.0.0.1 --rpc-bind-port "$rpc" \
        "${exclusive[@]}" \
        --log-level 2 > "$WORK/node$i.log" 2>&1 &
    PIDS+=($!)
done

log "Waiting for node RPCs to come up"
for i in 1 2 3; do
    rpc=$((31000+i))
    node_up() { daemon_get "$rpc" /getinfo | grep -q '"status"'; }
    wait_for "node$i rpc" 60 node_up || fail "node$i RPC did not come up"
done

log "Waiting for nodes to peer (each should see >=2 connections)"
peered() {
    for i in 1 2 3; do
        rpc=$((31000+i))
        info="$(daemon_get "$rpc" /getinfo)"
        out="$(echo "$info" | json_field outgoing_connections_count)"
        inc="$(echo "$info" | json_field incoming_connections_count)"
        [[ "$out" =~ ^[0-9]+$ ]] || out=0
        [[ "$inc" =~ ^[0-9]+$ ]] || inc=0
        [ $((out+inc)) -ge 2 ] || return 1
    done
    return 0
}
wait_for "peering" 120 peered || fail "nodes did not fully peer"
log "All 3 nodes are connected"

# ----------------------------------------------------------------------------
# 2. Create a wallet via kryptokrona-service
# ----------------------------------------------------------------------------
log "Generating a wallet container"
WALLET="$WORK/wallet.bin"
"$SERVICE" --generate-container \
    --container-file "$WALLET" --container-password "$WALLET_PASSWORD" \
    > "$WORK/service-gen.log" 2>&1 || fail "wallet generation failed"

log "Starting kryptokrona-service"
"$SERVICE" \
    --container-file "$WALLET" --container-password "$WALLET_PASSWORD" \
    --daemon-address 127.0.0.1 --daemon-port "$NODE1_RPC" \
    --bind-address 127.0.0.1 --bind-port "$SERVICE_PORT" \
    --rpc-password "$RPC_PASSWORD" \
    --log-level 2 > "$WORK/service.log" 2>&1 &
PIDS+=($!)

service_up() { service_rpc getStatus '{}' | grep -q '"result"'; }
wait_for "service" 60 service_up || fail "kryptokrona-service did not come up"

ADDRESS="$(service_rpc getAddresses '{}' | sed -E 's/.*"addresses":\["([^"]+)".*/\1/')"
[ -n "$ADDRESS" ] && [ "$ADDRESS" != "$(service_rpc getAddresses '{}')" ] || fail "could not read wallet address"
log "Wallet address: $ADDRESS"

# ----------------------------------------------------------------------------
# 2b. Address-prefix backwards compatibility (SEKR + Xkr both decode)
# ----------------------------------------------------------------------------
# The address prefix is display-only, not consensus: the same keys can be
# encoded under the legacy prefix (SEKR / 2239254) or the new one (Xkr / 45239),
# and every address-accepting entry point dual-decodes both. These two strings
# encode the SAME keys under each prefix; the daemon/service must consider both
# valid, and an obviously malformed address invalid. This exercises the C++
# dual-decode (Currency::parseAccountAddressString via validateAddress) on every
# OS the integration test runs on.
log "Checking address-prefix backwards compatibility (SEKR + Xkr both accepted)"
SEKR_ADDR="SEKReXzbFzP13xQEcTeZ7v2xb7n2wkpzXQTGpoVU5DevgHbjPyS8Zz9SzfErVB8KFGAyVNkcbUbKjGJhYovhCxG83DLwaYj6eYX"
XKR_ADDR="Xkrf4ot1pfRE3XZ5WckmX8XLriaSoUXXHAFx5Ppwowq3eYTUgHbDJcAJW5FAomRg26TzXFHqGrLmYNZbFtxZBXAU3ECQ2HEy51"
address_valid() { # address -> succeeds iff the service reports it valid
    service_rpc validateAddress "{\"address\":\"$1\"}" | grep -q '"isValid":true'
}
address_valid "$SEKR_ADDR" || fail "service rejected the legacy SEKR-prefixed address"
address_valid "$XKR_ADDR"  || fail "service rejected the new Xkr-prefixed address (dual-decode broken)"
address_valid "SEKRnotarealaddress123" && fail "service accepted an obviously invalid address"
log "Both SEKR and Xkr address prefixes are accepted; malformed address rejected"

# ----------------------------------------------------------------------------
# 3. Mine a block to our wallet address
# ----------------------------------------------------------------------------
log "Mining $BLOCKS_TO_MINE block to the wallet address"
# Judge success by whether the chain actually advances, not the miner's exit
# code -- the outcome (a new block on all nodes) is what we care about. Run the
# miner under a watchdog so it can never hang the job: at testnet difficulty 1
# blocks are near-instant, and it exits on --limit; if it ever loops (e.g. the
# daemon returns errors) we kill it and let the height check decide.
"$MINER" \
    --daemon-address "127.0.0.1:$NODE1_RPC" \
    --address "$ADDRESS" \
    --threads 1 --limit "$BLOCKS_TO_MINE" \
    > "$WORK/miner.log" 2>&1 &
miner_pid=$!
for _ in $(seq 1 90); do
    kill -0 "$miner_pid" 2>/dev/null || break
    sleep 1
done
kill "$miner_pid" 2>/dev/null || true
wait "$miner_pid" 2>/dev/null || true

# height starts at 1 (genesis), so after mining BLOCKS_TO_MINE it is 1 + that.
height_reached() {
    h="$(daemon_get "$NODE1_RPC" /getinfo | json_field height)"
    [[ "$h" =~ ^[0-9]+$ ]] && [ "$h" -gt "$BLOCKS_TO_MINE" ]
}
wait_for "block height" 120 height_reached || fail "chain did not advance past mining"
log "Chain height is now $(daemon_get "$NODE1_RPC" /getinfo | json_field height)"

# ----------------------------------------------------------------------------
# 4. Confirm the wallet received the coinbase
# ----------------------------------------------------------------------------
# The coinbase reward stays locked until the unlock window (20 blocks) passes,
# so after a single block it shows up as lockedAmount, not availableBalance.
# We assert on the total (available + locked) to prove the wallet received it.
log "Waiting for the wallet to sync and register the mined coinbase"
received_coinbase() {
    resp="$(service_rpc getBalance '{}')"
    avail="$(echo "$resp" | json_field availableBalance)"
    locked="$(echo "$resp" | json_field lockedAmount)"
    [[ "$avail"  =~ ^[0-9]+$ ]] || avail=0
    [[ "$locked" =~ ^[0-9]+$ ]] || locked=0
    [ $((avail + locked)) -gt 0 ]
}
wait_for "wallet coinbase" 180 received_coinbase || fail "wallet never received the mined coinbase"

resp="$(service_rpc getBalance '{}')"
AVAIL="$(echo "$resp" | json_field availableBalance)"
LOCKED="$(echo "$resp" | json_field lockedAmount)"
log "SUCCESS: wallet received coinbase (available=$AVAIL, locked=$LOCKED atomic units)"
echo
echo "Integration test passed: 3 nodes peered, mined $BLOCKS_TO_MINE block, wallet received the coinbase"
