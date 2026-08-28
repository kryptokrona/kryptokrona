#!/usr/bin/env bash
#
# Level-3 end-to-end test: a FULL two-party BTC<->XKR atomic swap.
#
# Where swap-rpc-service-test.sh drives the JS wallet service and
# swap-engine-e2e-test.sh drives the engine's XkrWallet adapter, this proves the
# one thing neither does: the whole cross-chain choreography, with both parties
# (Alice/maker and Bob/taker) running the real ported state machines against real
# infrastructure on both chains --
#
#   Bob locks BTC -> Alice confirms it, locks XKR -> Bob detects the XKR lock and
#   sends his encrypted signature -> Alice redeems the BTC, revealing the adaptor
#   -> Bob extracts the key and sweeps the XKR.
#
# Both parties run in-process inside the `xkr_two_party_swap` example (built from
# kryptokrona/xkr-swap-core), on connected libp2p swarms, exactly as swap-asb and
# the swap CLI wire them in production.
#
# Infrastructure this script stands up:
#   * XKR side: a peered 3-node testnet mesh + the xkr-wallet-rpc.cjs service, with
#     the ASB's XKR wallet funded by a normal tx (backend-js ignores coinbase) and
#     a background miner confirming blocks.
#   * BTC side: a regtest bitcoind + esplora electrs (Docker), with the chain
#     matured and a background miner confirming blocks.
#
# The BTC<->XKR exchange rate is the engine's FixedRate (0.01), and the XKR lock
# amount the maker must fund is BTC_AMOUNT_SAT * 1_000_000 atomic units
# (0.01 rate x 1e12 pico scaling). BTC_AMOUNT_SAT defaults to 2500 so the XKR the
# maker must mine stays small (~53 blocks) while the BTC lock still comfortably
# exceeds the (deliberately low) regtest redeem fee.
#
# Build the example first:
#   (cd <xkr-swap-core> && cargo build -p swap --example xkr_two_party_swap)
#
# Requires Docker (Linux CI) for bitcoind + electrs.
#
# Usage:
#   BIN_DIR=build/src \
#   XKR_RPC_SERVICE=/path/to/yggdrasil-wallet/src/backend/swap/xkr-wallet-rpc.cjs \
#   EXAMPLE_BIN=/path/to/xkr-swap-core/target/debug/examples/xkr_two_party_swap \
#   scripts/testnet/swap-two-party-test.sh

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
EXAMPLE_BIN="${EXAMPLE_BIN:-$HOME/Developer/xkr-swap-core/target/debug/examples/xkr_two_party_swap}"

RPC_PASSWORD="ci-test-password"
WALLET_PASSWORD="ci-test-wallet-pass"

# Ports (distinct from the other two swap scripts so they can share a CI job).
P2P_BASE=30500
RPC_BASE=31500
N1_RPC=$((RPC_BASE+1)); N2_RPC=$((RPC_BASE+2)); N3_RPC=$((RPC_BASE+3))
NODE_RPC=$N1_RPC
MINER_PORT=32500
FUNDER_PORT=32501
XKR_RPC_PORT=40002

# Bitcoin (regtest) infra -- see the coblox/vulpemventures images below.
BTC_RPC_USER="admin"
BTC_RPC_PASS="123"
BTC_RPC_PORT=18443
ELECTRUM_PORT=3002
ESPLORA_HTTP_PORT=60401
BTC_WALLET="l3"
BTC_CONTAINER="xkr-l3-bitcoind"
ELECTRS_CONTAINER="xkr-l3-electrs"

# Swap sizing. required XKR = BTC_AMOUNT_SAT * 1e6 (FixedRate 0.01 * pico scale).
BTC_AMOUNT_SAT="${BTC_AMOUNT_SAT:-2500}"
XKR_FEE=10
XKR_LOCK_AMOUNT=$((BTC_AMOUNT_SAT * 1000000))
# Fund the ASB with a comfortable margin over the exact lock amount.
XKR_DEPOSIT_AMOUNT=$((XKR_LOCK_AMOUNT + XKR_LOCK_AMOUNT / 5 + 100000))
SWAP_TIMEOUT_SECS="${SWAP_TIMEOUT_SECS:-900}"
# happy = full redeem-both-sides swap; refund = Alice never locks XKR, so Bob must
# reclaim his BTC via the cancel timelock (the safety path). Passed to the example.
SWAP_MODE="${SWAP_MODE:-happy}"

WORK="$(mktemp -d 2>/dev/null || mktemp -d -t kk-l3)"
BTC_DATADIR="$WORK/btcdata"; mkdir -p "$BTC_DATADIR"
PIDS=()
BG_MINER_PID=""
BTC_BG_MINER_PID=""

log()  { printf '\n\033[1;34m==> %s\033[0m\n' "$*"; }
ok()   { printf '\033[1;32m    OK: %s\033[0m\n' "$*"; }
fail() { printf '\n\033[1;31mFAIL: %s\033[0m\n' "$*" >&2; dump_logs; exit 1; }

dump_logs() {
    echo "----- last log output -----" >&2
    for f in "$WORK"/*.log; do
        [ -f "$f" ] || continue
        echo "===== $f =====" >&2; tail -n 40 "$f" >&2 || true
    done
    echo "===== docker: bitcoind =====" >&2; docker logs --tail 40 "$BTC_CONTAINER" 2>&1 | tail -40 >&2 || true
    echo "===== docker: electrs =====" >&2; docker logs --tail 40 "$ELECTRS_CONTAINER" 2>&1 | tail -40 >&2 || true
}

cleanup() {
    log "Cleaning up"
    [ -n "$BG_MINER_PID" ] && kill "$BG_MINER_PID" 2>/dev/null || true
    [ -n "$BTC_BG_MINER_PID" ] && kill "$BTC_BG_MINER_PID" 2>/dev/null || true
    pkill -P $$ 2>/dev/null || true
    for pid in "${PIDS[@]:-}"; do kill "$pid" 2>/dev/null || true; done
    docker rm -f "$ELECTRS_CONTAINER" "$BTC_CONTAINER" >/dev/null 2>&1 || true
    sleep 2
    rm -rf "$WORK" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

# ---- XKR (kryptokrona) helpers -------------------------------------------------
daemon_get() { curl -s --max-time 5 "http://127.0.0.1:$1$2" || true; }
service_rpc() { curl -s --max-time 20 -X POST "http://127.0.0.1:$1/json_rpc" -H 'Content-Type: application/json' \
    -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"password\":\"$RPC_PASSWORD\",\"method\":\"$2\",\"params\":$3}" || true; }
json_field() { sed -E "s/.*\"$1\"[[:space:]]*:[[:space:]]*\"?([^\",}]+)\"?.*/\1/"; }
node_height() { daemon_get "$NODE_RPC" /getinfo | jq -r '.height // 0' 2>/dev/null; }

wait_for() { local desc="$1" timeout="$2"; shift 2; local waited=0
    while ! "$@" >/dev/null 2>&1; do sleep 2; waited=$((waited+2)); [ "$waited" -ge "$timeout" ] && return 1; done; return 0; }

# Mine ~count XKR blocks, capped by watching node height (the miner's --limit
# doesn't stop its worker thread, so we stop it ourselves once height rose).
mine_blocks() { local addr="$1" count="$2" start_h target h
    start_h="$(node_height)"; [[ "$start_h" =~ ^[0-9]+$ ]] || start_h=0
    target=$((start_h + count))
    "$MINER" --daemon-address "127.0.0.1:$NODE_RPC" --address "$addr" --threads 1 --limit "$count" > "$WORK/miner.log" 2>&1 &
    local mp=$!
    for _ in $(seq 1 240); do h="$(node_height)"; [[ "$h" =~ ^[0-9]+$ ]] || h=0
        [ "$h" -ge "$target" ] && break; kill -0 "$mp" 2>/dev/null || break; sleep 0.5; done
    kill "$mp" 2>/dev/null || true; wait "$mp" 2>/dev/null || true; }

# ---- BTC (regtest) helpers -----------------------------------------------------
# JSON-RPC against bitcoind; $1 = method, $2 = params array, optional $3 = wallet.
btc_rpc() { local method="$1" params="$2" wallet="${3:-}" path=""
    [ -n "$wallet" ] && path="/wallet/$wallet"
    curl -s --max-time 30 --user "$BTC_RPC_USER:$BTC_RPC_PASS" \
        -H 'Content-Type: application/json' \
        -d "{\"jsonrpc\":\"1.0\",\"id\":\"l3\",\"method\":\"$method\",\"params\":$params}" \
        "http://127.0.0.1:$BTC_RPC_PORT$path" || true; }
btc_result() { jq -r '.result // empty' 2>/dev/null; }
btc_ready() { btc_rpc getblockchaininfo '[]' | grep -q '"chain"'; }
esplora_tip() { curl -s --max-time 5 "http://127.0.0.1:$ESPLORA_HTTP_PORT/blocks/tip/height" 2>/dev/null || echo ""; }

# ---- Preflight -----------------------------------------------------------------
for b in "$KRYPTOKRONAD" "$MINER" "$SERVICE" "$SWAP_SPIKE"; do [ -x "$b" ] || fail "binary not found: $b"; done
command -v curl >/dev/null || fail "curl is required"
command -v jq   >/dev/null || fail "jq is required"
command -v docker >/dev/null || fail "docker is required (bitcoind + electrs run as containers)"
command -v "$NODE_BIN" >/dev/null || fail "node is required"
[ -f "$XKR_RPC_SERVICE" ] || fail "XKR RPC service not found: $XKR_RPC_SERVICE"
[ -x "$EXAMPLE_BIN" ] || fail "two-party example not built: $EXAMPLE_BIN (cargo build -p swap --example xkr_two_party_swap)"

log "Swap sizing: BTC ${BTC_AMOUNT_SAT} sat  <->  XKR lock ${XKR_LOCK_AMOUNT} atomic (deposit ${XKR_DEPOSIT_AMOUNT})"

# ---------------------------------------------------------------------------
# 1. BTC side: bitcoind (regtest) + esplora electrs, sharing the datadir.
# ---------------------------------------------------------------------------
log "Starting bitcoind (regtest) + electrs (Docker)"
docker rm -f "$BTC_CONTAINER" "$ELECTRS_CONTAINER" >/dev/null 2>&1 || true

docker run -d --name "$BTC_CONTAINER" --network host \
    -v "$BTC_DATADIR:/home/bdk" \
    --entrypoint /usr/bin/bitcoind \
    coblox/bitcoin-core:0.21.0 \
    -server -regtest -listen=1 -prune=0 \
    -rpcallowip=0.0.0.0/0 -rpcbind=0.0.0.0 \
    -rpcuser="$BTC_RPC_USER" -rpcpassword="$BTC_RPC_PASS" \
    -printtoconsole -fallbackfee=0.00001 -datadir=/home/bdk \
    -rpcport="$BTC_RPC_PORT" -port=18886 -rest > "$WORK/btc-run.log" 2>&1 \
    || fail "could not start bitcoind container"

wait_for "bitcoind rpc" 60 btc_ready || fail "bitcoind RPC did not come up"
ok "bitcoind up"

# esplora electrs reads bitcoind's block files directly, so it shares /home/bdk.
docker run -d --name "$ELECTRS_CONTAINER" --network host \
    -v "$BTC_DATADIR:/home/bdk" \
    --entrypoint /build/electrs \
    vulpemventures/electrs \
    --network=regtest -vvvv \
    --daemon-dir=/home/bdk \
    --daemon-rpc-addr="127.0.0.1:$BTC_RPC_PORT" \
    --cookie="$BTC_RPC_USER:$BTC_RPC_PASS" \
    --http-addr="0.0.0.0:$ESPLORA_HTTP_PORT" \
    --electrum-rpc-addr="0.0.0.0:$ELECTRUM_PORT" \
    --cors='*' > "$WORK/electrs-run.log" 2>&1 \
    || fail "could not start electrs container"

# Create/prepare the bitcoind wallet and mature the chain (101 blocks).
btc_rpc createwallet "[\"$BTC_WALLET\"]" >/dev/null 2>&1 || \
    btc_rpc loadwallet "[\"$BTC_WALLET\"]" >/dev/null 2>&1 || true
BTC_MINE_ADDR="$(btc_rpc getnewaddress '[]' "$BTC_WALLET" | btc_result)"
[ -n "$BTC_MINE_ADDR" ] || fail "could not get a bitcoind wallet address"
log "Maturing regtest chain (101 blocks to $BTC_MINE_ADDR)"
btc_rpc generatetoaddress "[101, \"$BTC_MINE_ADDR\"]" "$BTC_WALLET" >/dev/null || fail "regtest maturity mining failed"

# Wait for electrs to index up to the bitcoind tip.
btc_tip() { btc_rpc getblockcount '[]' | btc_result; }
electrs_synced() { local t e; t="$(btc_tip)"; e="$(esplora_tip)"; [[ "$t" =~ ^[0-9]+$ ]] && [[ "$e" =~ ^[0-9]+$ ]] && [ "$e" -ge "$t" ]; }
wait_for "electrs sync" 180 electrs_synced || fail "electrs did not sync to bitcoind tip"
ok "electrs synced to height $(esplora_tip)"

# Background BTC miner: one block ~every 2s to drive lock/redeem confirmations.
( while :; do btc_rpc generatetoaddress "[1, \"$BTC_MINE_ADDR\"]" "$BTC_WALLET" >/dev/null 2>&1; sleep 2; done ) > "$WORK/btc-bgminer.log" 2>&1 &
BTC_BG_MINER_PID=$!

# ---------------------------------------------------------------------------
# 2. XKR side: peered 3-node mesh
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
# 3. XKR keys (the ASB's XKR wallet = party A of swap_spike keygen)
# ---------------------------------------------------------------------------
log "Deriving ASB XKR keys (swap_spike keygen)"
KEYS="$("$SWAP_SPIKE" keygen)" || fail "keygen failed"
kv() { echo "$KEYS" | grep "^$1=" | cut -d= -f2; }
A_SPEND="$(kv SPEND_SHARE_A)"; A_VIEW="$(kv VIEW_SHARE_A)"
[ -n "$A_SPEND" ] && [ -n "$A_VIEW" ] || fail "could not parse ASB XKR keys"

# ---------------------------------------------------------------------------
# 4. Fund the ASB's XKR wallet via a normal tx (backend-js ignores coinbase)
# ---------------------------------------------------------------------------
log "Mining to a throwaway wallet, then depositing to the ASB wallet"
"$SERVICE" --generate-container --container-file "$WORK/miner.bin" --container-password "$WALLET_PASSWORD" > "$WORK/mgen.log" 2>&1 || fail "miner gen failed"
"$SERVICE" --container-file "$WORK/miner.bin" --container-password "$WALLET_PASSWORD" --daemon-address 127.0.0.1 --daemon-port "$NODE_RPC" \
    --bind-address 127.0.0.1 --bind-port "$MINER_PORT" --rpc-password "$RPC_PASSWORD" --log-level 1 > "$WORK/miner-svc.log" 2>&1 &
PIDS+=($!)
miner_up() { service_rpc "$MINER_PORT" getStatus '{}' | grep -q '"result"'; }
wait_for "miner service" 60 miner_up || fail "miner service down"
MINER_ADDR="$(service_rpc "$MINER_PORT" getAddresses '{}' | sed -E 's/.*"addresses":\["([^"]+)".*/\1/')"

# Mine until the miner can afford the deposit. Reward ~4.77e7 atomic/block, so
# this is roughly XKR_DEPOSIT_AMOUNT / 4.77e7 blocks, in batches.
miner_ok() { a="$(service_rpc "$MINER_PORT" getBalance '{}' | json_field availableBalance)"; [[ "$a" =~ ^[0-9]+$ ]] || a=0; [ "$a" -gt $((XKR_DEPOSIT_AMOUNT + XKR_FEE)) ]; }
for _ in $(seq 1 40); do
    miner_ok && break
    mine_blocks "$MINER_ADDR" 10
done
wait_for "miner spendable" 300 miner_ok || fail "miner never accrued enough to fund the ASB (need >$XKR_DEPOSIT_AMOUNT)"
ok "miner balance: $(service_rpc "$MINER_PORT" getBalance '{}' | json_field availableBalance)"

"$SERVICE" --generate-container --container-file "$WORK/funder.bin" --container-password "$WALLET_PASSWORD" --spend-key "$A_SPEND" --view-key "$A_VIEW" > "$WORK/fgen.log" 2>&1 || fail "ASB wallet gen failed"
"$SERVICE" --container-file "$WORK/funder.bin" --container-password "$WALLET_PASSWORD" --daemon-address 127.0.0.1 --daemon-port "$NODE_RPC" \
    --bind-address 127.0.0.1 --bind-port "$FUNDER_PORT" --rpc-password "$RPC_PASSWORD" --SYNC_FROM_ZERO --log-level 1 > "$WORK/funder-svc.log" 2>&1 &
PIDS+=($!)
funder_up() { service_rpc "$FUNDER_PORT" getStatus '{}' | grep -q '"result"'; }
wait_for "ASB wallet service" 60 funder_up || fail "ASB wallet service down"
FUNDER_ADDR="$(service_rpc "$FUNDER_PORT" getAddresses '{}' | sed -E 's/.*"addresses":\["([^"]+)".*/\1/')"
ok "ASB (party A) XKR address: $FUNDER_ADDR"

dep="{\"anonymity\":0,\"fee\":$XKR_FEE,\"unlockTime\":0,\"changeAddress\":\"$MINER_ADDR\",\"transfers\":[{\"address\":\"$FUNDER_ADDR\",\"amount\":$XKR_DEPOSIT_AMOUNT}]}"
DEP_RESP="$(service_rpc "$MINER_PORT" sendTransaction "$dep")"
[ -n "$(echo "$DEP_RESP" | json_field transactionHash)" ] || fail "ASB deposit failed: $DEP_RESP"
mine_blocks "$MINER_ADDR" 6
funder_ok() { a="$(service_rpc "$FUNDER_PORT" getBalance '{}' | json_field availableBalance)"; [[ "$a" =~ ^[0-9]+$ ]] || a=0; [ "$a" -ge $((XKR_LOCK_AMOUNT + XKR_FEE)) ]; }
wait_for "ASB wallet spendable" 240 funder_ok || fail "ASB wallet never received deposit"
ok "ASB wallet available balance: $(service_rpc "$FUNDER_PORT" getBalance '{}' | json_field availableBalance)"

# ---------------------------------------------------------------------------
# 5. Start the JS wallet RPC service + a background XKR miner
# ---------------------------------------------------------------------------
log "Starting xkr-wallet-rpc.cjs service"
"$NODE_BIN" "$XKR_RPC_SERVICE" --port "$XKR_RPC_PORT" --daemon "127.0.0.1:$NODE_RPC" > "$WORK/xkr-rpc.log" 2>&1 &
PIDS+=($!)
xkr_up() { curl -s --max-time 5 -X POST "http://127.0.0.1:$XKR_RPC_PORT/" -d '{"jsonrpc":"2.0","id":1,"method":"ping","params":{}}' | grep -q pong; }
wait_for "xkr service" 60 xkr_up || fail "xkr service did not come up"
ok "xkr service up"

log "Starting background XKR miner (1 block ~every 2s)"
( while :; do mine_blocks "$MINER_ADDR" 1; sleep 2; done ) > "$WORK/bgminer.log" 2>&1 &
BG_MINER_PID=$!

# ---------------------------------------------------------------------------
# 6. Run the full two-party swap
# ---------------------------------------------------------------------------
log "Running the two-party BTC<->XKR swap (Alice + Bob in-process, mode=$SWAP_MODE)"
set +e
ELECTRUM_RPC_URL="tcp://127.0.0.1:$ELECTRUM_PORT" \
BITCOIND_RPC_URL="http://$BTC_RPC_USER:$BTC_RPC_PASS@127.0.0.1:$BTC_RPC_PORT/wallet/$BTC_WALLET" \
BTC_AMOUNT_SAT="$BTC_AMOUNT_SAT" \
SWAP_TIMEOUT_SECS="$SWAP_TIMEOUT_SECS" \
SWAP_MODE="$SWAP_MODE" \
XKR_WALLET_RPC_URL="http://127.0.0.1:$XKR_RPC_PORT" \
XKR_ASB_SPEND_SECRET="$A_SPEND" XKR_ASB_VIEW_SECRET="$A_VIEW" \
XKR_RECEIVE_ADDRESS="$MINER_ADDR" \
RUST_LOG="${RUST_LOG:-info,swap=debug,swap_p2p=info}" \
    "$EXAMPLE_BIN"
RC=$?
set -e
[ "$RC" -eq 0 ] || fail "two-party swap example failed (exit $RC)"

echo
echo "=============================================================="
if [ "$SWAP_MODE" = refund ]; then
    echo "REFUND SAFETY TEST PASSED"
    echo "  Bob locked BTC, Alice never locked XKR, and once the cancel"
    echo "  timelock expired Bob unilaterally reclaimed his BTC -- funds"
    echo "  are safe when a swap is abandoned."
else
    echo "TWO-PARTY BTC<->XKR SWAP TEST PASSED"
    echo "  Bob locked BTC, Alice locked XKR, Bob revealed his signature,"
    echo "  Alice redeemed the BTC and Bob swept the XKR -- end to end,"
    echo "  on two live chains."
fi
echo "=============================================================="
