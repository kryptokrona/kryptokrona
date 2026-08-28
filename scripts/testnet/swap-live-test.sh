#!/usr/bin/env bash
#
# Live testnet proof of the XKR side of a BTC<->XKR atomic swap: the on-chain
# "lock and sweep" of a shared 2-of-2 output.
#
# Flow (all against a real testnet daemon, no protocol changes):
#   1. Start a testnet node.
#   2. Fund a normal wallet ("funder") by mining a coinbase to it.
#   3. Off-chain, derive a SHARED address whose spend pubkey is B_A+B_B and view
#      secret is v_A+v_B (swap_spike keygen). Neither party alone can spend it.
#   4. Funder sends XKR to the shared address  ->  this is the swap LOCK.
#   5. Reconstruct the shared wallet by importing the COMBINED secrets
#      (b_A+b_B, v_A+v_B) into a second service instance -- as would happen when
#      one party learns the other's share at the end of a swap.
#   6. Assert the reconstructed wallet's address == the shared address (proves
#      additivity end-to-end through the real key-import path), that it sees the
#      locked funds, and that it can SWEEP them back out (proves spendability).
#
# Requires TESTNET binaries (-DTEST_NET=ON) and the swap_spike helper.
#
# Usage: BIN_DIR=build/src scripts/testnet/swap-live-test.sh

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

RPC_PASSWORD="ci-test-password"
WALLET_PASSWORD="ci-test-wallet-pass"

NODE_RPC=31001
NODE_P2P=30001
FUNDER_PORT=32000
SWEPT_PORT=32001

# Amount (atomic units) the funder locks into the shared address, and the fee.
LOCK_AMOUNT=100000
FEE=10

WORK="$(mktemp -d 2>/dev/null || mktemp -d -t kk-swap)"
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

# JSON-RPC against a kryptokrona-service instance on the given port.
service_rpc() { # port method params_json
    curl -s --max-time 20 -X POST "http://127.0.0.1:$1/json_rpc" \
        -H 'Content-Type: application/json' \
        -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"password\":\"$RPC_PASSWORD\",\"method\":\"$2\",\"params\":$3}" \
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

# Mine N blocks to the given address (watchdog so it can never hang the job).
mine_blocks() { # address count
    local addr="$1" count="$2"
    "$MINER" \
        --daemon-address "127.0.0.1:$NODE_RPC" \
        --address "$addr" \
        --threads 1 --limit "$count" \
        > "$WORK/miner.log" 2>&1 &
    local mp=$!
    for _ in $(seq 1 120); do kill -0 "$mp" 2>/dev/null || break; sleep 1; done
    kill "$mp" 2>/dev/null || true
    wait "$mp" 2>/dev/null || true
}

for b in "$KRYPTOKRONAD" "$MINER" "$SERVICE" "$SWAP_SPIKE"; do
    [ -x "$b" ] || fail "binary not found or not executable: $b"
done
command -v curl >/dev/null || fail "curl is required"

# ----------------------------------------------------------------------------
# 1. Start a testnet node
# ----------------------------------------------------------------------------
log "Starting testnet node"
mkdir -p "$WORK/node"
"$KRYPTOKRONAD" \
    --data-dir "$WORK/node" \
    --p2p-bind-ip 127.0.0.1 --p2p-bind-port "$NODE_P2P" \
    --rpc-bind-ip 127.0.0.1 --rpc-bind-port "$NODE_RPC" \
    --log-level 2 > "$WORK/node.log" 2>&1 &
PIDS+=($!)
node_up() { daemon_get "$NODE_RPC" /getinfo | grep -q '"status"'; }
wait_for "node rpc" 60 node_up || fail "node RPC did not come up"
ok "node is up"

# ----------------------------------------------------------------------------
# 2. Fund a normal wallet by mining a coinbase to it
# ----------------------------------------------------------------------------
log "Generating funder wallet"
FUNDER_WALLET="$WORK/funder.bin"
"$SERVICE" --generate-container \
    --container-file "$FUNDER_WALLET" --container-password "$WALLET_PASSWORD" \
    > "$WORK/funder-gen.log" 2>&1 || fail "funder wallet generation failed"

"$SERVICE" \
    --container-file "$FUNDER_WALLET" --container-password "$WALLET_PASSWORD" \
    --daemon-address 127.0.0.1 --daemon-port "$NODE_RPC" \
    --bind-address 127.0.0.1 --bind-port "$FUNDER_PORT" \
    --rpc-password "$RPC_PASSWORD" \
    --log-level 2 > "$WORK/funder-service.log" 2>&1 &
PIDS+=($!)
funder_up() { service_rpc "$FUNDER_PORT" getStatus '{}' | grep -q '"result"'; }
wait_for "funder service" 60 funder_up || fail "funder service did not come up"

FUNDER_ADDR="$(service_rpc "$FUNDER_PORT" getAddresses '{}' | sed -E 's/.*"addresses":\["([^"]+)".*/\1/')"
[ -n "$FUNDER_ADDR" ] || fail "could not read funder address"
ok "funder address: $FUNDER_ADDR"

log "Mining coinbase to funder (enough to unlock a spendable output)"
# Mine several blocks so at least one coinbase clears the unlock window and the
# funder has a spendable balance well above LOCK_AMOUNT+FEE.
mine_blocks "$FUNDER_ADDR" 12
funder_spendable() {
    resp="$(service_rpc "$FUNDER_PORT" getBalance '{}')"
    avail="$(echo "$resp" | json_field availableBalance)"
    [[ "$avail" =~ ^[0-9]+$ ]] || avail=0
    [ "$avail" -gt $((LOCK_AMOUNT + FEE)) ]
}
wait_for "funder spendable balance" 180 funder_spendable || fail "funder never got a spendable balance"
ok "funder available balance: $(service_rpc "$FUNDER_PORT" getBalance '{}' | json_field availableBalance)"

# ----------------------------------------------------------------------------
# 3. Derive the shared 2-of-2 address + combined secrets (off-chain)
# ----------------------------------------------------------------------------
log "Deriving shared 2-of-2 address (swap_spike keygen)"
KEYS="$("$SWAP_SPIKE" keygen)" || fail "swap_spike keygen failed"
SHARED_ADDR="$(echo "$KEYS" | grep '^SHARED_ADDRESS_SEKR=' | cut -d= -f2)"
COMBINED_SPEND="$(echo "$KEYS" | grep '^COMBINED_SPEND_KEY=' | cut -d= -f2)"
COMBINED_VIEW="$(echo "$KEYS" | grep '^COMBINED_VIEW_KEY=' | cut -d= -f2)"
[ -n "$SHARED_ADDR" ] && [ -n "$COMBINED_SPEND" ] && [ -n "$COMBINED_VIEW" ] || fail "could not parse keygen output"
ok "shared address: $SHARED_ADDR"

# The node/service must accept the shared address as valid.
service_rpc "$FUNDER_PORT" validateAddress "{\"address\":\"$SHARED_ADDR\"}" | grep -q '"isValid":true' \
    || fail "service rejected the shared address"
ok "shared address validates"

# ----------------------------------------------------------------------------
# 4. LOCK: funder sends XKR to the shared address
# ----------------------------------------------------------------------------
log "Locking $LOCK_AMOUNT atomic units into the shared address"
send_params="{\"anonymity\":0,\"fee\":$FEE,\"unlockTime\":0,\"changeAddress\":\"$FUNDER_ADDR\",\"transfers\":[{\"address\":\"$SHARED_ADDR\",\"amount\":$LOCK_AMOUNT}]}"
SEND_RESP="$(service_rpc "$FUNDER_PORT" sendTransaction "$send_params")"
LOCK_TXHASH="$(echo "$SEND_RESP" | json_field transactionHash)"
{ [ -n "$LOCK_TXHASH" ] && [ "$LOCK_TXHASH" != "$SEND_RESP" ]; } || fail "lock send failed: $SEND_RESP"
ok "lock tx: $LOCK_TXHASH"

log "Mining to confirm the lock transaction"
mine_blocks "$FUNDER_ADDR" 3

# ----------------------------------------------------------------------------
# 5. Reconstruct the shared wallet from the COMBINED secrets
# ----------------------------------------------------------------------------
log "Reconstructing shared wallet from combined secrets (b_A+b_B, v_A+v_B)"
SWEPT_WALLET="$WORK/swept.bin"
"$SERVICE" --generate-container \
    --container-file "$SWEPT_WALLET" --container-password "$WALLET_PASSWORD" \
    --spend-key "$COMBINED_SPEND" --view-key "$COMBINED_VIEW" \
    > "$WORK/swept-gen.log" 2>&1 || fail "shared wallet import failed"

"$SERVICE" \
    --container-file "$SWEPT_WALLET" --container-password "$WALLET_PASSWORD" \
    --daemon-address 127.0.0.1 --daemon-port "$NODE_RPC" \
    --bind-address 127.0.0.1 --bind-port "$SWEPT_PORT" \
    --rpc-password "$RPC_PASSWORD" \
    --SYNC_FROM_ZERO \
    --log-level 2 > "$WORK/swept-service.log" 2>&1 &
PIDS+=($!)
swept_up() { service_rpc "$SWEPT_PORT" getStatus '{}' | grep -q '"result"'; }
wait_for "swept service" 60 swept_up || fail "reconstructed service did not come up"

# 6a. The reconstructed address MUST equal the shared address (additivity proof).
SWEPT_ADDR="$(service_rpc "$SWEPT_PORT" getAddresses '{}' | sed -E 's/.*"addresses":\["([^"]+)".*/\1/')"
[ "$SWEPT_ADDR" = "$SHARED_ADDR" ] \
    || fail "reconstructed address ($SWEPT_ADDR) != shared address ($SHARED_ADDR)"
ok "reconstructed wallet address matches the shared address"

# 6b. It must SEE the locked funds.
log "Waiting for the reconstructed wallet to see the locked funds"
swept_sees_lock() {
    resp="$(service_rpc "$SWEPT_PORT" getBalance '{}')"
    avail="$(echo "$resp" | json_field availableBalance)"
    locked="$(echo "$resp" | json_field lockedAmount)"
    [[ "$avail"  =~ ^[0-9]+$ ]] || avail=0
    [[ "$locked" =~ ^[0-9]+$ ]] || locked=0
    [ $((avail + locked)) -ge "$LOCK_AMOUNT" ]
}
wait_for "swept sees lock" 180 swept_sees_lock || fail "reconstructed wallet never saw the locked funds"
ok "reconstructed wallet balance: $(service_rpc "$SWEPT_PORT" getBalance '{}')"

# 6c. It must be able to SWEEP the funds out (proves spendability).
log "Waiting for the locked output to become spendable, then sweeping it"
swept_spendable() {
    avail="$(service_rpc "$SWEPT_PORT" getBalance '{}' | json_field availableBalance)"
    [[ "$avail" =~ ^[0-9]+$ ]] || avail=0
    [ "$avail" -gt "$FEE" ]
}
# Keep mining (in case the output needs more confirmations) while we wait.
for _ in $(seq 1 10); do
    swept_spendable && break
    mine_blocks "$FUNDER_ADDR" 1
    sleep 2
done
swept_spendable || fail "locked output never became spendable"

SWEEP_AVAIL="$(service_rpc "$SWEPT_PORT" getBalance '{}' | json_field availableBalance)"
SWEEP_AMOUNT=$((SWEEP_AVAIL - FEE))
sweep_params="{\"anonymity\":0,\"fee\":$FEE,\"unlockTime\":0,\"changeAddress\":\"$SHARED_ADDR\",\"transfers\":[{\"address\":\"$FUNDER_ADDR\",\"amount\":$SWEEP_AMOUNT}]}"
SWEEP_RESP="$(service_rpc "$SWEPT_PORT" sendTransaction "$sweep_params")"
SWEEP_TXHASH="$(echo "$SWEEP_RESP" | json_field transactionHash)"
{ [ -n "$SWEEP_TXHASH" ] && [ "$SWEEP_TXHASH" != "$SWEEP_RESP" ]; } || fail "sweep send failed: $SWEEP_RESP"
ok "sweep tx: $SWEEP_TXHASH"

log "Mining to confirm the sweep, then verifying the funds left the shared wallet"
mine_blocks "$FUNDER_ADDR" 3
swept_drained() {
    total="$(service_rpc "$SWEPT_PORT" getBalance '{}' | json_field availableBalance)"
    [[ "$total" =~ ^[0-9]+$ ]] || total=0
    [ "$total" -lt "$SWEEP_AMOUNT" ]
}
wait_for "swept drained" 120 swept_drained || fail "swept wallet balance did not drop after sweep"
ok "shared wallet swept: final available=$(service_rpc "$SWEPT_PORT" getBalance '{}' | json_field availableBalance)"

echo
echo "=============================================================="
echo "LIVE SWAP TEST PASSED"
echo "  * funder locked $LOCK_AMOUNT into shared address $SHARED_ADDR"
echo "  * shared wallet reconstructed from (b_A+b_B, v_A+v_B)"
echo "  * reconstructed address matched the shared address"
echo "  * shared output detected AND swept back out on-chain"
echo "=============================================================="
