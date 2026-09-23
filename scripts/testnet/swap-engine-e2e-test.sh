#!/usr/bin/env bash
#
# Level-2 end-to-end test: the Rust swap engine's OWN XKR adapter code
# (swap::xkr::XkrWallet -> xkr-wallet client -> xkr-wallet-rpc.cjs -> node),
# exercised against a live testnet. Where swap-rpc-service-test.sh drives the JS
# service with curl, this runs the compiled `xkr_adapter_e2e` example, so the
# exact calls the ported Bob/Alice state-machine arms make actually execute:
# asb_keys_from_env + lock_send, shared_address + watch_for_lock, redeem,
# wait_until_confirmed.
#
# Flow: peered 3-node mesh -> fund party A by a normal tx -> start the JS service
# -> start a background miner (so the adapter's blocking polls resolve) -> run the
# engine example -> assert it passes.
#
# Build the example first:
#   (cd <xkr-swap-core> && ACLOCAL_PATH=/usr/local/share/aclocal \
#      cargo build -p swap --example xkr_adapter_e2e)
#
# Usage:
#   BIN_DIR=build/src \
#   XKR_RPC_SERVICE=/path/to/yggdrasil-wallet/src/backend/swap/xkr-wallet-rpc.cjs \
#   EXAMPLE_BIN=/path/to/xkr-swap-core/target/debug/examples/xkr_adapter_e2e \
#   scripts/testnet/swap-engine-e2e-test.sh

set -euo pipefail

BIN_DIR="${BIN_DIR:-build/src}"
EXE=""
[ -f "$BIN_DIR/kryptokronad.exe" ] && EXE=".exe"
KRYPTOKRONAD="$BIN_DIR/kryptokronad$EXE"
MINER="$BIN_DIR/miner$EXE"
SERVICE="$BIN_DIR/kryptokrona-service$EXE"
SWAP_SPIKE="$BIN_DIR/swap_spike$EXE"

NODE_BIN="${NODE_BIN:-node}"
XKR_RPC_SERVICE="${XKR_RPC_SERVICE:-$HOME/Developer/yggdrasil-wallet/src/backend/swap/xkr-wallet-rpc.cjs}"
EXAMPLE_BIN="${EXAMPLE_BIN:-$HOME/Developer/xkr-swap-core/target/debug/examples/xkr_adapter_e2e}"

RPC_PASSWORD="ci-test-password"
WALLET_PASSWORD="ci-test-wallet-pass"

# Distinct ports from swap-rpc-service-test.sh so both can run in one CI job.
P2P_BASE=30400
RPC_BASE=31400
N1_RPC=$((RPC_BASE+1)); N2_RPC=$((RPC_BASE+2)); N3_RPC=$((RPC_BASE+3))
NODE_RPC=$N1_RPC
MINER_PORT=32400
FUNDER_PORT=32401
XKR_RPC_PORT=40001

DEPOSIT_AMOUNT=1000000
LOCK_AMOUNT=100000
FEE=10

WORK="$(mktemp -d 2>/dev/null || mktemp -d -t kk-engine)"
PIDS=()
BG_MINER_PID=""

log()  { printf '\n\033[1;34m==> %s\033[0m\n' "$*"; }
ok()   { printf '\033[1;32m    OK: %s\033[0m\n' "$*"; }
fail() { printf '\n\033[1;31mFAIL: %s\033[0m\n' "$*" >&2; dump_logs; exit 1; }

dump_logs() {
    echo "----- last log output -----" >&2
    for f in "$WORK"/*.log; do
        [ -f "$f" ] || continue
        echo "===== $f =====" >&2; tail -n 40 "$f" >&2 || true
    done
}

cleanup() {
    log "Cleaning up"
    [ -n "$BG_MINER_PID" ] && kill "$BG_MINER_PID" 2>/dev/null || true
    pkill -P $$ 2>/dev/null || true
    for pid in "${PIDS[@]:-}"; do kill "$pid" 2>/dev/null || true; done
    sleep 2
    rm -rf "$WORK" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

daemon_get() { curl -s --max-time 5 "http://127.0.0.1:$1$2" || true; }
service_rpc() { curl -s --max-time 20 -X POST "http://127.0.0.1:$1/json_rpc" -H 'Content-Type: application/json' \
    -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"password\":\"$RPC_PASSWORD\",\"method\":\"$2\",\"params\":$3}" || true; }
json_field() { sed -E "s/.*\"$1\"[[:space:]]*:[[:space:]]*\"?([^\",}]+)\"?.*/\1/"; }
node_height() { daemon_get "$NODE_RPC" /getinfo | jq -r '.height // 0' 2>/dev/null; }

wait_for() { local desc="$1" timeout="$2"; shift 2; local waited=0
    while ! "$@" >/dev/null 2>&1; do sleep 2; waited=$((waited+2)); [ "$waited" -ge "$timeout" ] && return 1; done; return 0; }

# Mine ~count blocks, capped by watching node height (the miner's --limit doesn't
# stop its worker thread, so we stop it ourselves once the height rose by count).
mine_blocks() { local addr="$1" count="$2" start_h target h
    start_h="$(node_height)"; [[ "$start_h" =~ ^[0-9]+$ ]] || start_h=0
    target=$((start_h + count))
    "$MINER" --daemon-address "127.0.0.1:$NODE_RPC" --address "$addr" --threads 1 --limit "$count" > "$WORK/miner.log" 2>&1 &
    local mp=$!
    for _ in $(seq 1 120); do h="$(node_height)"; [[ "$h" =~ ^[0-9]+$ ]] || h=0
        [ "$h" -ge "$target" ] && break; kill -0 "$mp" 2>/dev/null || break; sleep 0.5; done
    kill "$mp" 2>/dev/null || true; wait "$mp" 2>/dev/null || true; }

for b in "$KRYPTOKRONAD" "$MINER" "$SERVICE" "$SWAP_SPIKE"; do [ -x "$b" ] || fail "binary not found: $b"; done
command -v curl >/dev/null || fail "curl is required"
command -v jq   >/dev/null || fail "jq is required"
command -v "$NODE_BIN" >/dev/null || fail "node is required"
[ -f "$XKR_RPC_SERVICE" ] || fail "XKR RPC service not found: $XKR_RPC_SERVICE"
[ -x "$EXAMPLE_BIN" ] || fail "engine example not built: $EXAMPLE_BIN (cargo build -p swap --example xkr_adapter_e2e)"

# ---------------------------------------------------------------------------
# 1. Peered 3-node mesh
# ---------------------------------------------------------------------------
log "Starting 3 testnet nodes (mesh)"
for i in 1 2 3; do
    mkdir -p "$WORK/node$i"; exclusive=()
    for j in 1 2 3; do [ "$j" -ne "$i" ] && exclusive+=(--add-exclusive-node "127.0.0.1:$((P2P_BASE+j))"); done
    "$KRYPTOKRONAD" --data-dir "$WORK/node$i" --p2p-bind-ip 127.0.0.1 --p2p-bind-port "$((P2P_BASE+i))" \
        --rpc-bind-ip 127.0.0.1 --rpc-bind-port "$((RPC_BASE+i))" "${exclusive[@]}" --log-level 1 > "$WORK/node$i.log" 2>&1 &
    PIDS+=($!)
done
for i in 1 2 3; do node_up() { daemon_get "$((RPC_BASE+i))" /getinfo | grep -q '"status"'; }
    wait_for "node$i rpc" 60 node_up || fail "node$i RPC did not come up"; done
peered() { for i in 1 2 3; do
    c=$(daemon_get "$((RPC_BASE+i))" /getinfo | jq -r '(.outgoing_connections_count // 0) + (.incoming_connections_count // 0)' 2>/dev/null)
    [[ "$c" =~ ^[0-9]+$ ]] && [ "$c" -ge 2 ] || return 1; done; }
wait_for "peering" 120 peered || fail "nodes did not peer"
ok "all 3 nodes peered"

# ---------------------------------------------------------------------------
# 2. Keys
# ---------------------------------------------------------------------------
log "Deriving keys (swap_spike keygen)"
KEYS="$("$SWAP_SPIKE" keygen)" || fail "keygen failed"
kv() { echo "$KEYS" | grep "^$1=" | cut -d= -f2; }
SHARED_ADDR_SEKR="$(kv SHARED_ADDRESS_SEKR)"; SHARED_SPEND_PUB="$(kv SHARED_SPEND_PUBLIC)"
SHARED_VIEW_PUB="$(kv SHARED_VIEW_PUBLIC)"; COMBINED_SPEND="$(kv COMBINED_SPEND_KEY)"
COMBINED_VIEW="$(kv COMBINED_VIEW_KEY)"; A_SPEND="$(kv SPEND_SHARE_A)"; A_VIEW="$(kv VIEW_SHARE_A)"
for v in SHARED_ADDR_SEKR SHARED_SPEND_PUB SHARED_VIEW_PUB COMBINED_SPEND COMBINED_VIEW A_SPEND A_VIEW; do
    [ -n "${!v}" ] || fail "could not parse $v"; done
ok "shared address: $SHARED_ADDR_SEKR"

# ---------------------------------------------------------------------------
# 3. Fund party A via a normal tx (backend-js ignores coinbase)
# ---------------------------------------------------------------------------
log "Mining to a throwaway wallet, then depositing to party A"
"$SERVICE" --generate-container --container-file "$WORK/miner.bin" --container-password "$WALLET_PASSWORD" > "$WORK/mgen.log" 2>&1 || fail "miner gen failed"
"$SERVICE" --container-file "$WORK/miner.bin" --container-password "$WALLET_PASSWORD" --daemon-address 127.0.0.1 --daemon-port "$NODE_RPC" \
    --bind-address 127.0.0.1 --bind-port "$MINER_PORT" --rpc-password "$RPC_PASSWORD" --log-level 1 > "$WORK/miner-svc.log" 2>&1 &
PIDS+=($!)
miner_up() { service_rpc "$MINER_PORT" getStatus '{}' | grep -q '"result"'; }
wait_for "miner service" 60 miner_up || fail "miner service down"
MINER_ADDR="$(service_rpc "$MINER_PORT" getAddresses '{}' | sed -E 's/.*"addresses":\["([^"]+)".*/\1/')"
mine_blocks "$MINER_ADDR" 12
miner_ok() { a="$(service_rpc "$MINER_PORT" getBalance '{}' | json_field availableBalance)"; [[ "$a" =~ ^[0-9]+$ ]] || a=0; [ "$a" -gt $((DEPOSIT_AMOUNT+FEE)) ]; }
wait_for "miner spendable" 180 miner_ok || fail "miner not spendable"

"$SERVICE" --generate-container --container-file "$WORK/funder.bin" --container-password "$WALLET_PASSWORD" --spend-key "$A_SPEND" --view-key "$A_VIEW" > "$WORK/fgen.log" 2>&1 || fail "funder gen failed"
"$SERVICE" --container-file "$WORK/funder.bin" --container-password "$WALLET_PASSWORD" --daemon-address 127.0.0.1 --daemon-port "$NODE_RPC" \
    --bind-address 127.0.0.1 --bind-port "$FUNDER_PORT" --rpc-password "$RPC_PASSWORD" --SYNC_FROM_ZERO --log-level 1 > "$WORK/funder-svc.log" 2>&1 &
PIDS+=($!)
funder_up() { service_rpc "$FUNDER_PORT" getStatus '{}' | grep -q '"result"'; }
wait_for "funder service" 60 funder_up || fail "funder service down"
FUNDER_ADDR="$(service_rpc "$FUNDER_PORT" getAddresses '{}' | sed -E 's/.*"addresses":\["([^"]+)".*/\1/')"
ok "funder (party A) address: $FUNDER_ADDR"

dep="{\"anonymity\":0,\"fee\":$FEE,\"unlockTime\":0,\"changeAddress\":\"$MINER_ADDR\",\"transfers\":[{\"address\":\"$FUNDER_ADDR\",\"amount\":$DEPOSIT_AMOUNT}]}"
DEP_RESP="$(service_rpc "$MINER_PORT" sendTransaction "$dep")"
[ -n "$(echo "$DEP_RESP" | json_field transactionHash)" ] || fail "deposit failed: $DEP_RESP"
mine_blocks "$MINER_ADDR" 4
funder_ok() { a="$(service_rpc "$FUNDER_PORT" getBalance '{}' | json_field availableBalance)"; [[ "$a" =~ ^[0-9]+$ ]] || a=0; [ "$a" -ge $((LOCK_AMOUNT+FEE)) ]; }
wait_for "funder spendable" 180 funder_ok || fail "funder never received deposit"
ok "funder available balance: $(service_rpc "$FUNDER_PORT" getBalance '{}' | json_field availableBalance)"

# ---------------------------------------------------------------------------
# 4. Start the JS service + a background miner (keeps confirming blocks)
# ---------------------------------------------------------------------------
log "Starting xkr-wallet-rpc.cjs service"
"$NODE_BIN" "$XKR_RPC_SERVICE" --port "$XKR_RPC_PORT" --daemon "127.0.0.1:$NODE_RPC" > "$WORK/xkr-rpc.log" 2>&1 &
PIDS+=($!)
xkr_up() { curl -s --max-time 5 -X POST "http://127.0.0.1:$XKR_RPC_PORT/" -d '{"jsonrpc":"2.0","id":1,"method":"ping","params":{}}' | grep -q pong; }
wait_for "xkr service" 60 xkr_up || fail "xkr service did not come up"
ok "xkr service up"

log "Starting background miner (1 block ~every 2s) so blocking polls resolve"
( while :; do mine_blocks "$MINER_ADDR" 1; sleep 2; done ) > "$WORK/bgminer.log" 2>&1 &
BG_MINER_PID=$!

# ---------------------------------------------------------------------------
# 5. Run the ENGINE adapter end-to-end
# ---------------------------------------------------------------------------
log "Running the engine adapter example (swap::xkr::XkrWallet)"
set +e
XKR_WALLET_RPC_URL="http://127.0.0.1:$XKR_RPC_PORT" \
XKR_ASB_SPEND_SECRET="$A_SPEND" XKR_ASB_VIEW_SECRET="$A_VIEW" \
SHARED_ADDR="$SHARED_ADDR_SEKR" SHARED_SPEND_PUB="$SHARED_SPEND_PUB" SHARED_VIEW_PUB="$SHARED_VIEW_PUB" \
COMBINED_SPEND="$COMBINED_SPEND" COMBINED_VIEW="$COMBINED_VIEW" \
FUNDER_ADDR="$FUNDER_ADDR" LOCK_AMOUNT="$LOCK_AMOUNT" FEE="$FEE" \
    "$EXAMPLE_BIN"
RC=$?
set -e
[ "$RC" -eq 0 ] || fail "engine adapter example failed (exit $RC)"

echo
echo "=============================================================="
echo "ENGINE <-> SERVICE E2E TEST PASSED"
echo "  The engine's own XkrWallet adapter drove lock -> watch ->"
echo "  redeem -> confirm through the live service, on-chain."
echo "=============================================================="
