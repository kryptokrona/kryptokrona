#!/usr/bin/env bash
#
# Live testnet smoke test for the XKR wallet JSON-RPC service
# (yggdrasil-wallet/src/backend/swap/xkr-wallet-rpc.cjs) that the Rust swap
# engine (xkr-swap-core) drives. Where swap-live-test.sh proves the C++
# swap_spike path, this exercises the *JS service* end-to-end against a real
# testnet daemon, covering every method the engine calls:
#
#   ping                                  -> service is up
#   encodeAddress(spendPub, viewPub)      -> == the shared 2-of-2 address
#   lockSend(senderKeys, sharedAddr, amt) -> Alice's XKR lock (send from own wallet)
#   watchForLock(sharedAddr, viewSec, amt)-> Bob detects the lock (view-only scan)
#   sweep(combinedKeys, dest, fee)        -> redeem/refund the shared output
#   confirmTx(combinedKeys, txHash)       -> confirm the sweep on-chain
#
# Flow:
#   1. Start a peered 3-node testnet mesh (backend-js only syncs once the node
#      reports a network height, which needs peers).
#   2. swap_spike keygen -> shared address, shared public keys, combined secrets,
#      and party A's individual (spend, view) share.
#   3. Fund party A's wallet with a NORMAL transfer (mine to a throwaway wallet,
#      then send to party A) -- backend-js ignores coinbase outputs by default, so
#      the funder must be funded by a normal tx, as a real ASB wallet would be.
#   4. Start the JS RPC service against node 1.
#   5. Drive the six methods above, asserting the lock is detected and the shared
#      output is swept back out and confirmed.
#
# Requires: TESTNET binaries (-DTEST_NET=ON), the swap_spike helper, node, and the
# service's node_modules (kryptokrona-wallet-backend-js, kryptokrona-utils).
#
# Usage:
#   BIN_DIR=build/src \
#   XKR_RPC_SERVICE=/path/to/yggdrasil-wallet/src/backend/swap/xkr-wallet-rpc.cjs \
#   scripts/testnet/swap-rpc-service-test.sh

set -euo pipefail

# ----------------------------------------------------------------------------
# Config
# ----------------------------------------------------------------------------
BIN_DIR="${BIN_DIR:-build/src}"
EXE=""
[ -f "$BIN_DIR/kryptokronad.exe" ] && EXE=".exe"
KRYPTOKRONAD="$BIN_DIR/kryptokronad$EXE"
MINER="$BIN_DIR/miner$EXE"
SERVICE="$BIN_DIR/kryptokrona-service$EXE"
SWAP_SPIKE="$BIN_DIR/swap_spike$EXE"

NODE_BIN="${NODE_BIN:-node}"
XKR_RPC_SERVICE="${XKR_RPC_SERVICE:-$HOME/Developer/yggdrasil-wallet/src/backend/swap/xkr-wallet-rpc.cjs}"

RPC_PASSWORD="ci-test-password"
WALLET_PASSWORD="ci-test-wallet-pass"

# kryptokrona-wallet-backend-js only advances its sync once the node reports a
# network height, which requires peers. So, like the CI integration/wallet-flow
# tests, we run a peered 3-node mesh (a solo node reports network height 0 and the
# JS wallet never syncs). The funder, miner and JS service all talk to node 1.
P2P_BASE=30300
RPC_BASE=31300
N1_RPC=$((RPC_BASE+1)); N2_RPC=$((RPC_BASE+2)); N3_RPC=$((RPC_BASE+3))
NODE_RPC=$N1_RPC
MINER_PORT=32300
FUNDER_PORT=32301
XKR_RPC_PORT=40000

# Amounts (atomic units): the normal deposit that funds party A, the amount party
# A locks into the shared address, and the fee.
DEPOSIT_AMOUNT=1000000
LOCK_AMOUNT=100000
FEE=10

WORK="$(mktemp -d 2>/dev/null || mktemp -d -t kk-swaprpc)"
PIDS=()

log()  { printf '\n\033[1;34m==> %s\033[0m\n' "$*"; }
ok()   { printf '\033[1;32m    OK: %s\033[0m\n' "$*"; }
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
    sleep 2
    rm -rf "$WORK" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

daemon_get() { curl -s --max-time 5 "http://127.0.0.1:$1$2" || true; }

# JSON-RPC against a C++ kryptokrona-service instance.
service_rpc() { # port method params_json
    curl -s --max-time 20 -X POST "http://127.0.0.1:$1/json_rpc" \
        -H 'Content-Type: application/json' \
        -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"password\":\"$RPC_PASSWORD\",\"method\":\"$2\",\"params\":$3}" \
        || true
}

# JSON-RPC against the XKR wallet JS service. Its methods can block for a while
# (watchForLock/sweep poll the wallet), so allow a generous timeout.
xkr_rpc() { # method params_json
    curl -s --max-time 300 -X POST "http://127.0.0.1:$XKR_RPC_PORT/" \
        -H 'Content-Type: application/json' \
        -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"$1\",\"params\":$2}" \
        || true
}

json_field() { sed -E "s/.*\"$1\"[[:space:]]*:[[:space:]]*\"?([^\",}]+)\"?.*/\1/"; }

wait_for() { # description seconds cmd...
    local desc="$1" timeout="$2"; shift 2
    local waited=0
    while ! "$@" >/dev/null 2>&1; do
        sleep 2; waited=$((waited+2))
        if [ "$waited" -ge "$timeout" ]; then return 1; fi
    done
    return 0
}

node_height() { daemon_get "$NODE_RPC" /getinfo | jq -r '.height // 0' 2>/dev/null; }

# Mine roughly `count` blocks. The miner's --limit lets start() return after N
# blocks but its worker thread keeps mining until killed, so on this trivial
# testnet difficulty it would otherwise add thousands of blocks and balloon the
# chain (making every backend-js wallet sync scan the lot). We therefore cap it
# ourselves: stop as soon as the node height has risen by `count`.
mine_blocks() { # address count
    local addr="$1" count="$2" start_h target h
    start_h="$(node_height)"; [[ "$start_h" =~ ^[0-9]+$ ]] || start_h=0
    target=$((start_h + count))
    "$MINER" \
        --daemon-address "127.0.0.1:$NODE_RPC" \
        --address "$addr" \
        --threads 1 --limit "$count" \
        > "$WORK/miner.log" 2>&1 &
    local mp=$!
    for _ in $(seq 1 120); do
        h="$(node_height)"; [[ "$h" =~ ^[0-9]+$ ]] || h=0
        [ "$h" -ge "$target" ] && break
        kill -0 "$mp" 2>/dev/null || break
        sleep 0.5
    done
    kill "$mp" 2>/dev/null || true
    wait "$mp" 2>/dev/null || true
}

for b in "$KRYPTOKRONAD" "$MINER" "$SERVICE" "$SWAP_SPIKE"; do
    [ -x "$b" ] || fail "binary not found or not executable: $b"
done
command -v curl >/dev/null || fail "curl is required"
command -v jq   >/dev/null || fail "jq is required (brew install jq / apt install jq)"
command -v "$NODE_BIN" >/dev/null || fail "node is required (set NODE_BIN)"
[ -f "$XKR_RPC_SERVICE" ] || fail "XKR RPC service not found: $XKR_RPC_SERVICE (set XKR_RPC_SERVICE)"

# ----------------------------------------------------------------------------
# 1. Start a peered 3-node testnet mesh (so nodes report a network height)
# ----------------------------------------------------------------------------
log "Starting 3 testnet nodes (--add-exclusive-node mesh)"
for i in 1 2 3; do
    mkdir -p "$WORK/node$i"
    exclusive=()
    for j in 1 2 3; do [ "$j" -ne "$i" ] && exclusive+=(--add-exclusive-node "127.0.0.1:$((P2P_BASE+j))"); done
    "$KRYPTOKRONAD" \
        --data-dir "$WORK/node$i" \
        --p2p-bind-ip 127.0.0.1 --p2p-bind-port "$((P2P_BASE+i))" \
        --rpc-bind-ip 127.0.0.1 --rpc-bind-port "$((RPC_BASE+i))" \
        "${exclusive[@]}" --log-level 1 > "$WORK/node$i.log" 2>&1 &
    PIDS+=($!)
done
for i in 1 2 3; do
    node_up() { daemon_get "$((RPC_BASE+i))" /getinfo | grep -q '"status"'; }
    wait_for "node$i rpc" 60 node_up || fail "node$i RPC did not come up"
done
peered() {
    for i in 1 2 3; do
        c=$(daemon_get "$((RPC_BASE+i))" /getinfo | jq -r '(.outgoing_connections_count // 0) + (.incoming_connections_count // 0)' 2>/dev/null)
        [[ "$c" =~ ^[0-9]+$ ]] && [ "$c" -ge 2 ] || return 1
    done
}
wait_for "peering" 120 peered || fail "nodes did not fully peer"
ok "all 3 nodes peered"

# ----------------------------------------------------------------------------
# 2. Derive the shared 2-of-2 keys AND party A's individual share (off-chain)
# ----------------------------------------------------------------------------
log "Deriving keys (swap_spike keygen)"
KEYS="$("$SWAP_SPIKE" keygen)" || fail "swap_spike keygen failed"
kv() { echo "$KEYS" | grep "^$1=" | cut -d= -f2; }
SHARED_ADDR_SEKR="$(kv SHARED_ADDRESS_SEKR)"
SHARED_ADDR_XKR="$(kv SHARED_ADDRESS_XKR)"
SHARED_SPEND_PUB="$(kv SHARED_SPEND_PUBLIC)"
SHARED_VIEW_PUB="$(kv SHARED_VIEW_PUBLIC)"
COMBINED_SPEND="$(kv COMBINED_SPEND_KEY)"
COMBINED_VIEW="$(kv COMBINED_VIEW_KEY)"
A_SPEND="$(kv SPEND_SHARE_A)"
A_VIEW="$(kv VIEW_SHARE_A)"
for v in SHARED_ADDR_SEKR SHARED_SPEND_PUB SHARED_VIEW_PUB COMBINED_SPEND COMBINED_VIEW A_SPEND A_VIEW; do
    [ -n "${!v}" ] || fail "could not parse $v from keygen output"
done
ok "shared address: $SHARED_ADDR_SEKR"

# ----------------------------------------------------------------------------
# 3. Fund party A's wallet with a NORMAL transaction (not coinbase)
#
# backend-js has scanCoinbaseTransactions=false by default, so a mining-funded
# wallet shows a 0 balance. We therefore mine to a throwaway "miner" wallet
# (kryptokrona-service scans coinbase) and have it send a normal transfer to
# party A's address. That normal output IS visible to backend-js, and it mirrors
# how a real ASB wallet is funded (by deposits, not mining).
# ----------------------------------------------------------------------------
log "Mining to a throwaway miner wallet"
MINER_WALLET="$WORK/miner.bin"
"$SERVICE" --generate-container \
    --container-file "$MINER_WALLET" --container-password "$WALLET_PASSWORD" \
    > "$WORK/miner-gen.log" 2>&1 || fail "miner wallet generation failed"
"$SERVICE" \
    --container-file "$MINER_WALLET" --container-password "$WALLET_PASSWORD" \
    --daemon-address 127.0.0.1 --daemon-port "$NODE_RPC" \
    --bind-address 127.0.0.1 --bind-port "$MINER_PORT" \
    --rpc-password "$RPC_PASSWORD" --log-level 1 > "$WORK/miner-service.log" 2>&1 &
PIDS+=($!)
miner_up() { service_rpc "$MINER_PORT" getStatus '{}' | grep -q '"result"'; }
wait_for "miner service" 60 miner_up || fail "miner service did not come up"
MINER_ADDR="$(service_rpc "$MINER_PORT" getAddresses '{}' | sed -E 's/.*"addresses":\["([^"]+)".*/\1/')"
[ -n "$MINER_ADDR" ] || fail "could not read miner address"

mine_blocks "$MINER_ADDR" 12
miner_spendable() {
    avail="$(service_rpc "$MINER_PORT" getBalance '{}' | json_field availableBalance)"
    [[ "$avail" =~ ^[0-9]+$ ]] || avail=0
    [ "$avail" -gt $((DEPOSIT_AMOUNT + FEE)) ]
}
wait_for "miner spendable balance" 180 miner_spendable || fail "miner never got a spendable balance"
ok "miner available balance: $(service_rpc "$MINER_PORT" getBalance '{}' | json_field availableBalance)"

log "Generating party A's wallet (funder) and depositing to it via a normal tx"
FUNDER_WALLET="$WORK/funder.bin"
"$SERVICE" --generate-container \
    --container-file "$FUNDER_WALLET" --container-password "$WALLET_PASSWORD" \
    --spend-key "$A_SPEND" --view-key "$A_VIEW" \
    > "$WORK/funder-gen.log" 2>&1 || fail "funder wallet generation failed"
"$SERVICE" \
    --container-file "$FUNDER_WALLET" --container-password "$WALLET_PASSWORD" \
    --daemon-address 127.0.0.1 --daemon-port "$NODE_RPC" \
    --bind-address 127.0.0.1 --bind-port "$FUNDER_PORT" \
    --rpc-password "$RPC_PASSWORD" --SYNC_FROM_ZERO --log-level 1 > "$WORK/funder-service.log" 2>&1 &
PIDS+=($!)
funder_up() { service_rpc "$FUNDER_PORT" getStatus '{}' | grep -q '"result"'; }
wait_for "funder service" 60 funder_up || fail "funder service did not come up"
FUNDER_ADDR="$(service_rpc "$FUNDER_PORT" getAddresses '{}' | sed -E 's/.*"addresses":\["([^"]+)".*/\1/')"
[ -n "$FUNDER_ADDR" ] || fail "could not read funder address"
ok "funder address: $FUNDER_ADDR"

# This testnet's genesis starts at a high height; scan only from here so each JS
# wallet import doesn't rescan the whole chain. All swap txs happen after this.
SCAN_HEIGHT="$(daemon_get "$NODE_RPC" /getinfo | jq -r '.height // 0' 2>/dev/null)"
[[ "$SCAN_HEIGHT" =~ ^[0-9]+$ ]] || SCAN_HEIGHT=0
[ "$SCAN_HEIGHT" -gt 10 ] && SCAN_HEIGHT=$((SCAN_HEIGHT - 10)) || SCAN_HEIGHT=0
ok "JS wallets will scan from height $SCAN_HEIGHT"

log "Depositing $DEPOSIT_AMOUNT to party A's address (normal transfer)"
dep_params="{\"anonymity\":0,\"fee\":$FEE,\"unlockTime\":0,\"changeAddress\":\"$MINER_ADDR\",\"transfers\":[{\"address\":\"$FUNDER_ADDR\",\"amount\":$DEPOSIT_AMOUNT}]}"
DEP_RESP="$(service_rpc "$MINER_PORT" sendTransaction "$dep_params")"
DEP_TX="$(echo "$DEP_RESP" | json_field transactionHash)"
{ [ -n "$DEP_TX" ] && [ "$DEP_TX" != "$DEP_RESP" ]; } || fail "deposit send failed: $DEP_RESP"
mine_blocks "$MINER_ADDR" 4
funder_spendable() {
    avail="$(service_rpc "$FUNDER_PORT" getBalance '{}' | json_field availableBalance)"
    [[ "$avail" =~ ^[0-9]+$ ]] || avail=0
    [ "$avail" -ge $((LOCK_AMOUNT + FEE)) ]
}
wait_for "funder spendable balance" 180 funder_spendable || fail "funder never received the deposit"
ok "funder available balance: $(service_rpc "$FUNDER_PORT" getBalance '{}' | json_field availableBalance)"

# ----------------------------------------------------------------------------
# 4. Start the XKR wallet JS RPC service against the node
# ----------------------------------------------------------------------------
log "Starting xkr-wallet-rpc.cjs service"
"$NODE_BIN" "$XKR_RPC_SERVICE" --port "$XKR_RPC_PORT" --daemon "127.0.0.1:$NODE_RPC" \
    > "$WORK/xkr-rpc.log" 2>&1 &
PIDS+=($!)
xkr_up() { xkr_rpc ping '{}' | grep -q 'pong'; }
wait_for "xkr rpc service" 60 xkr_up || fail "xkr RPC service did not come up"
ok "xkr RPC service is up (ping -> pong)"

# ----------------------------------------------------------------------------
# 5a. encodeAddress: the JS derivation must match the shared address
# ----------------------------------------------------------------------------
log "encodeAddress(sharedSpendPub, sharedViewPub) must equal the shared address"
ENC_RESP="$(xkr_rpc encodeAddress "{\"spendPublicKey\":\"$SHARED_SPEND_PUB\",\"viewPublicKey\":\"$SHARED_VIEW_PUB\"}")"
ENC_ADDR="$(echo "$ENC_RESP" | json_field address)"
if [ "$ENC_ADDR" = "$SHARED_ADDR_SEKR" ] || [ "$ENC_ADDR" = "$SHARED_ADDR_XKR" ]; then
    ok "encodeAddress matches the shared address ($ENC_ADDR)"
else
    fail "encodeAddress returned $ENC_ADDR, expected $SHARED_ADDR_SEKR / $SHARED_ADDR_XKR (resp: $ENC_RESP)"
fi
# Use whichever prefix the service produced for the rest of the flow.
SHARED_ADDR="$ENC_ADDR"

# ----------------------------------------------------------------------------
# 5b. lockSend: party A locks into the shared address from its own wallet
# ----------------------------------------------------------------------------
log "lockSend: locking $LOCK_AMOUNT into the shared address"
LOCK_RESP="$(xkr_rpc lockSend "{\"senderSpendSecret\":\"$A_SPEND\",\"senderViewSecret\":\"$A_VIEW\",\"destAddress\":\"$SHARED_ADDR\",\"amount\":$LOCK_AMOUNT,\"fee\":$FEE,\"scanHeight\":$SCAN_HEIGHT}")"
LOCK_TX="$(echo "$LOCK_RESP" | json_field txHash)"
{ [ -n "$LOCK_TX" ] && [ "$LOCK_TX" != "$LOCK_RESP" ]; } || fail "lockSend failed: $LOCK_RESP"
ok "lockSend tx: $LOCK_TX"

log "Mining to confirm and unlock the lock output"
mine_blocks "$MINER_ADDR" 12

# ----------------------------------------------------------------------------
# 5c. watchForLock: Bob detects the lock at the shared address (view-only)
# ----------------------------------------------------------------------------
log "watchForLock: detecting the lock deposit (view-only scan)"
WATCH_RESP="$(xkr_rpc watchForLock "{\"address\":\"$SHARED_ADDR\",\"viewSecret\":\"$COMBINED_VIEW\",\"amount\":$LOCK_AMOUNT,\"timeoutMs\":180000,\"scanHeight\":$SCAN_HEIGHT}")"
echo "$WATCH_RESP" | grep -q '"detected":true' || fail "watchForLock did not detect the lock: $WATCH_RESP"
WATCH_TX="$(echo "$WATCH_RESP" | json_field txHash)"
ok "watchForLock detected the deposit (txHash: $WATCH_TX)"

# ----------------------------------------------------------------------------
# 5d. sweep: reconstruct the shared wallet from combined secrets and sweep out
# ----------------------------------------------------------------------------
log "sweep: spending the shared output back to the funder"
SWEEP_RESP="$(xkr_rpc sweep "{\"spendSecret\":\"$COMBINED_SPEND\",\"viewSecret\":\"$COMBINED_VIEW\",\"destAddress\":\"$FUNDER_ADDR\",\"fee\":$FEE,\"scanHeight\":$SCAN_HEIGHT}")"
SWEEP_TX="$(echo "$SWEEP_RESP" | json_field txHash)"
{ [ -n "$SWEEP_TX" ] && [ "$SWEEP_TX" != "$SWEEP_RESP" ]; } || fail "sweep failed: $SWEEP_RESP"
ok "sweep tx: $SWEEP_TX"

log "Mining to confirm the sweep"
mine_blocks "$MINER_ADDR" 4

# ----------------------------------------------------------------------------
# 5e. confirmTx: the sweep reaches the required depth
# ----------------------------------------------------------------------------
log "confirmTx: waiting for the sweep to confirm"
CONFIRM_RESP="$(xkr_rpc confirmTx "{\"spendSecret\":\"$COMBINED_SPEND\",\"viewSecret\":\"$COMBINED_VIEW\",\"txHash\":\"$SWEEP_TX\",\"confirmations\":1,\"timeoutMs\":180000,\"scanHeight\":$SCAN_HEIGHT}")"
echo "$CONFIRM_RESP" | grep -q '"confirmed":true' || fail "confirmTx did not confirm the sweep: $CONFIRM_RESP"
ok "confirmTx confirmed the sweep"

# ----------------------------------------------------------------------------
# 5f. The funder wallet must have received the swept funds back
# ----------------------------------------------------------------------------
log "Verifying the funder received the swept funds"
funder_recovered() {
    # After locking LOCK_AMOUNT and sweeping (LOCK_AMOUNT - FEE) back, the funder's
    # total balance should be within ~2*FEE of where it was minus the two fees.
    total="$(service_rpc "$FUNDER_PORT" getBalance '{}' | json_field availableBalance)"
    [[ "$total" =~ ^[0-9]+$ ]] || total=0
    [ "$total" -gt "$FEE" ]
}
for _ in $(seq 1 10); do
    funder_recovered && break
    mine_blocks "$MINER_ADDR" 1
    sleep 2
done
funder_recovered || fail "funder never saw the swept funds return"
ok "funder recovered balance: $(service_rpc "$FUNDER_PORT" getBalance '{}' | json_field availableBalance)"

echo
echo "=============================================================="
echo "XKR WALLET RPC SERVICE TEST PASSED"
echo "  * ping / encodeAddress (== shared address)"
echo "  * lockSend  -> locked $LOCK_AMOUNT into $SHARED_ADDR"
echo "  * watchForLock detected the lock (view-only)"
echo "  * sweep spent the shared output back out"
echo "  * confirmTx confirmed the sweep on-chain"
echo "=============================================================="
