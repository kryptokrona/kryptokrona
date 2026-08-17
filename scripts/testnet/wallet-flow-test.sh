#!/usr/bin/env bash
#
# End-to-end wallet money-flow test across a real 3-node testnet.
#
# This is the "can this release actually move money" smoke test. Unlike
# ci-integration-test.sh (which only proves a mined coinbase lands in one
# wallet), this brings up three peered nodes and drives BOTH wallet backends --
# kryptokrona-service (JSON-RPC) and wallet-api (REST) -- through the flows that
# actually break in a bad release:
#
#   1. create a mining wallet, mine to it            (no imports, no fixtures)
#   2. create more wallets on OTHER nodes
#   3. send funds between them, ACROSS the p2p network
#   4. mine confirmations and assert every wallet sees the exact amount
#   5. send again from the receiving wallet (exercises both send paths)
#   6. restart a wallet and assert its balance/history persist
#
# Topology (one wallet per node; both backends covered):
#   node1 (rpc 31001)  <-  W1 = kryptokrona-service (32001)  [miner]
#   node2 (rpc 31002)  <-  W2 = wallet-api           (33002)
#   node3 (rpc 31003)  <-  W3 = kryptokrona-service  (32003)
#
# Requires TESTNET binaries (-DTEST_NET=ON): difficulty is pinned to 1 so mining
# is instant, and the coinbase unlock window is 1 block so mined coins spend
# almost immediately. Requires jq + curl.
#
# Usage: BIN_DIR=build/src scripts/testnet/wallet-flow-test.sh

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
WALLET_API="$BIN_DIR/wallet-api$EXE"

RPC_PASSWORD="ci-test-password"
WALLET_PASSWORD="ci-test-wallet-pass"

# node i: p2p 30000+i, rpc 31000+i. W1 svc 32001, W3 svc 32003, W2 wallet-api 33002.
W1_PORT=32001; W3_PORT=32003; W2_PORT=33002

MINE_INITIAL=25     # blocks mined to W1 up front (instant at difficulty 1)
MINE_CONFIRM=3      # blocks mined to confirm each round of sends
FEE=1000            # atomic units; comfortably above MINIMUM_FEE
# Distinct, recognisable amounts so an assertion failure says exactly which leg broke.
SEND_1_2=111111     # W1 -> W2  (service    -> wallet-api,  node1 -> node2)
SEND_1_3=222222     # W1 -> W3  (service    -> service,     node1 -> node3)
SEND_2_3=50000      # W2 -> W3  (wallet-api -> service,      node2 -> node3)

WORK="$(mktemp -d 2>/dev/null || mktemp -d -t kk-flow)"
PIDS=()
WAPI_FD_OPEN=0

# ----------------------------------------------------------------------------
# Output helpers
# ----------------------------------------------------------------------------
log()  { printf '\n\033[1;34m==> %s\033[0m\n' "$*"; }
ok()   { printf '  \033[1;32m✔\033[0m %s\n' "$*"; }
dump_logs() {
    echo "----- last log output -----" >&2
    for f in "$WORK"/*.log; do
        [ -f "$f" ] || continue
        echo "===== $f =====" >&2; tail -n 40 "$f" >&2 || true
    done
}
fail() { printf '\n\033[1;31mFAIL: %s\033[0m\n' "$*" >&2; dump_logs; exit 1; }

cleanup() {
    log "Cleaning up"
    # kill -9: a wallet that saves-on-exit can otherwise linger and squat a port,
    # breaking a subsequent run.
    for pid in "${PIDS[@]:-}"; do kill -9 "$pid" 2>/dev/null || true; done
    pkill -9 -P $$ 2>/dev/null || true
    [ "$WAPI_FD_OPEN" = 1 ] && exec 8>&- 2>/dev/null || true
    sleep 1
    rm -rf "$WORK" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

# ----------------------------------------------------------------------------
# HTTP helpers
# ----------------------------------------------------------------------------
dget()  { curl -s --max-time 8 "http://127.0.0.1:$1/getinfo"; }                 # $1=rpcport
# kryptokrona-service JSON-RPC:  svc PORT METHOD [PARAMS]
svc() {
    local port="$1" method="$2" params="${3:-}"
    [ -z "$params" ] && params="{}"
    curl -s --max-time 60 -X POST "http://127.0.0.1:$port/json_rpc" \
        -H 'Content-Type: application/json' \
        -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"password\":\"$RPC_PASSWORD\",\"method\":\"$method\",\"params\":$params}"
}
# wallet-api REST: sets WAPI_CODE/WAPI_BODY.  wapi METHOD PATH [BODY]
WAPI_CODE=""; WAPI_BODY=""
wapi() {
    local method="$1" path="$2" body="${3:-}" out
    out=$(curl -s --max-time 60 -w $'\n%{http_code}' -X "$method" \
        -H "X-API-KEY: $RPC_PASSWORD" -H 'Content-Type: application/json' \
        ${body:+-d "$body"} "http://127.0.0.1:$W2_PORT$path")
    WAPI_CODE=$(printf '%s' "$out" | tail -n1)
    WAPI_BODY=$(printf '%s' "$out" | sed '$d')
}

wait_for() { # description seconds cmd...
    local desc="$1" timeout="$2"; shift 2; local waited=0
    while ! "$@" >/dev/null 2>&1; do
        sleep 1; waited=$((waited+1))
        [ "$waited" -ge "$timeout" ] && return 1
    done
    return 0
}

# ----------------------------------------------------------------------------
# Preflight
# ----------------------------------------------------------------------------
command -v curl >/dev/null || fail "curl is required"
command -v jq   >/dev/null || fail "jq is required (brew install jq / apt install jq)"
for b in "$KRYPTOKRONAD" "$MINER" "$SERVICE" "$WALLET_API"; do
    [ -x "$b" ] || fail "binary not found or not executable: $b (build with -DTEST_NET=ON)"
done

# ----------------------------------------------------------------------------
# 1. Start the 3-node testnet and wait for it to peer
# ----------------------------------------------------------------------------
log "Starting 3 testnet nodes (--add-exclusive-node mesh)"
for i in 1 2 3; do
    mkdir -p "$WORK/node$i"
    exclusive=()
    for j in 1 2 3; do [ "$j" -ne "$i" ] && exclusive+=(--add-exclusive-node "127.0.0.1:$((30000+j))"); done
    "$KRYPTOKRONAD" \
        --data-dir "$WORK/node$i" \
        --p2p-bind-ip 127.0.0.1 --p2p-bind-port "$((30000+i))" \
        --rpc-bind-ip 127.0.0.1 --rpc-bind-port "$((31000+i))" \
        "${exclusive[@]}" --log-level 1 > "$WORK/node$i.log" 2>&1 &
    PIDS+=($!)
done
for i in 1 2 3; do
    node_up() { dget "$((31000+i))" | jq -e '.status' >/dev/null 2>&1; }
    wait_for "node$i rpc" 60 node_up || fail "node$i RPC did not come up"
done
peered() {
    for i in 1 2 3; do
        c=$(dget "$((31000+i))" | jq -r '(.outgoing_connections_count // 0) + (.incoming_connections_count // 0)' 2>/dev/null)
        [[ "$c" =~ ^[0-9]+$ ]] && [ "$c" -ge 2 ] || return 1
    done
}
wait_for "peering" 120 peered || fail "nodes did not fully peer"
ok "all 3 nodes peered ($(for i in 1 2 3; do printf 'node%s=%s ' "$i" "$(dget $((31000+i)) | jq -r '(.outgoing_connections_count//0)+(.incoming_connections_count//0)')"; done))"

# ----------------------------------------------------------------------------
# 2. Start one wallet per node -- both backends
# ----------------------------------------------------------------------------
start_service_wallet() { # name port daemon_rpc container
    local name="$1" port="$2" rpc="$3" file="$4"
    "$SERVICE" --generate-container --container-file "$file" --container-password "$WALLET_PASSWORD" >"$WORK/gen.log" 2>&1 \
        || fail "wallet generation failed for $file"
    "$SERVICE" --container-file "$file" --container-password "$WALLET_PASSWORD" \
        --daemon-address 127.0.0.1 --daemon-port "$rpc" \
        --bind-address 127.0.0.1 --bind-port "$port" --rpc-password "$RPC_PASSWORD" \
        --log-level 1 > "$WORK/$name.log" 2>&1 &
    PIDS+=($!)
    # $port is visible to svc_up via bash's dynamic scoping (wait_for calls it
    # while this frame is still on the stack).
    svc_up() { svc "$port" getStatus | jq -e '.result' >/dev/null 2>&1; }
    wait_for "$name" 60 svc_up || fail "$name (kryptokrona-service) did not come up"
}

log "Starting W1 = kryptokrona-service @ node1 (the miner's wallet)"
start_service_wallet w1 "$W1_PORT" 31001 "$WORK/w1.container"
W1_ADDR=$(svc "$W1_PORT" getAddresses | jq -r '.result.addresses[0]')
[ -n "$W1_ADDR" ] && [ "$W1_ADDR" != null ] || fail "could not read W1 address"
ok "W1 (service)    $W1_ADDR"

log "Starting W3 = kryptokrona-service @ node3"
start_service_wallet w3 "$W3_PORT" 31003 "$WORK/w3.container"
W3_ADDR=$(svc "$W3_PORT" getAddresses | jq -r '.result.addresses[0]')
[ -n "$W3_ADDR" ] && [ "$W3_ADDR" != null ] || fail "could not read W3 address"
ok "W3 (service)    $W3_ADDR"

log "Starting W2 = wallet-api @ node2"
# wallet-api is interactive (waits for "exit" on stdin); feed it from a FIFO we
# hold open so its stdin never EOFs and it stays running.
WAPI_FIFO="$WORK/wapi_stdin"; mkfifo "$WAPI_FIFO"
"$WALLET_API" --port "$W2_PORT" --rpc-password "$RPC_PASSWORD" < "$WAPI_FIFO" > "$WORK/w2.log" 2>&1 &
PIDS+=($!)
exec 8<>"$WAPI_FIFO"; WAPI_FD_OPEN=1
# Before a wallet is created/opened, /status returns 403 ("no wallet open"); any
# numeric HTTP code just means the REST server itself is accepting connections.
wapi_up() { wapi GET /status; [[ "$WAPI_CODE" =~ ^[0-9]+$ ]]; }
wait_for "w2" 60 wapi_up || fail "W2 (wallet-api) did not come up"
wapi POST /wallet/create "{\"daemonHost\":\"127.0.0.1\",\"daemonPort\":31002,\"filename\":\"$WORK/w2.container\",\"password\":\"$WALLET_PASSWORD\"}"
[ "$WAPI_CODE" = 200 ] || fail "wallet-api /wallet/create failed (HTTP $WAPI_CODE): $WAPI_BODY"
wapi GET /addresses/primary; W2_ADDR=$(printf '%s' "$WAPI_BODY" | jq -r '.address')
[ -n "$W2_ADDR" ] && [ "$W2_ADDR" != null ] || fail "could not read W2 address"
ok "W2 (wallet-api) $W2_ADDR"

# ----------------------------------------------------------------------------
# Sync + balance helpers
# ----------------------------------------------------------------------------
# A freshly-attached wallet reports knownBlockCount=1 until its node proxy polls
# the daemon's real height. Correct barrier: wait for the proxy to learn the
# height, THEN for the wallet to scan up to it. Getting this wrong is what makes
# these tests look like "mining produced no funds".
svc_synced() { # port height
    local s; s=$(svc "$1" getStatus)
    local wb nb; wb=$(printf '%s' "$s" | jq -r '.result.blockCount'); nb=$(printf '%s' "$s" | jq -r '.result.knownBlockCount')
    [[ "$nb" =~ ^[0-9]+$ ]] && [ "$nb" -ge "$2" ] && [[ "$wb" =~ ^[0-9]+$ ]] && [ "$wb" -ge "$(($2-1))" ]
}
wapi_synced() { # height
    wapi GET /status; local wb; wb=$(printf '%s' "$WAPI_BODY" | jq -r '.walletBlockCount')
    [[ "$wb" =~ ^[0-9]+$ ]] && [ "$wb" -ge "$(($1-1))" ]
}
svc_avail()  { svc "$1" getBalance | jq -r '.result.availableBalance'; }         # $1=port
wapi_unlocked() { wapi GET /balance; printf '%s' "$WAPI_BODY" | jq -r '.unlocked'; }
daemon_height() { dget "$1" | jq -r '.height'; }

# Mine COUNT blocks to ADDR via node1, then wait every node to reach that height.
mine_and_propagate() { # count addr
    "$MINER" --daemon-address "127.0.0.1:31001" --address "$2" --threads 1 --limit "$1" > "$WORK/miner.log" 2>&1 &
    local mp=$!
    for _ in $(seq 1 200); do kill -0 "$mp" 2>/dev/null || break; local h; h=$(daemon_height 31001); [[ "$h" =~ ^[0-9]+$ ]] && [ "$h" -ge "$TARGET" ] && break; sleep 0.2; done
    kill "$mp" 2>/dev/null || true; wait "$mp" 2>/dev/null || true
    local H; H=$(daemon_height 31001)
    all_at_height() { for i in 1 2 3; do [ "$(daemon_height $((31000+i)))" = "$H" ] || return 1; done; }
    wait_for "propagate to $H" 60 all_at_height || fail "block $H did not propagate to all nodes"
    echo "$H"
}

# ----------------------------------------------------------------------------
# 3. Mine to W1 and confirm every node + W1 see it
# ----------------------------------------------------------------------------
log "Mining $MINE_INITIAL blocks to W1 and checking propagation to all nodes"
TARGET=$((MINE_INITIAL+1))
H=$(mine_and_propagate "$MINE_INITIAL" "$W1_ADDR")
ok "chain at height $H on node1/node2/node3"
wait_for "W1 sync" 120 svc_synced "$W1_PORT" "$H" || fail "W1 did not sync to $H"
[ "$(svc_avail "$W1_PORT")" -gt $((SEND_1_2+SEND_1_3+3*FEE)) ] || fail "W1 has insufficient spendable balance"
ok "W1 spendable balance = $(svc_avail "$W1_PORT")"

# ----------------------------------------------------------------------------
# 4. Cross-node sends: W1 -> W2 (wallet-api) and W1 -> W3 (service)
# ----------------------------------------------------------------------------
log "Sending across the network: W1->W2 ($SEND_1_2) and W1->W3 ($SEND_1_3), anonymity 0"
R=$(svc "$W1_PORT" sendTransaction "{\"anonymity\":0,\"fee\":$FEE,\"sourceAddresses\":[\"$W1_ADDR\"],\"transfers\":[{\"address\":\"$W2_ADDR\",\"amount\":$SEND_1_2}],\"changeAddress\":\"$W1_ADDR\"}")
TX_1_2=$(printf '%s' "$R" | jq -r '.result.transactionHash // empty'); [ -n "$TX_1_2" ] || fail "W1->W2 send failed: $(printf '%s' "$R" | jq -c '.error')"
ok "W1->W2 tx $TX_1_2"
R=$(svc "$W1_PORT" sendTransaction "{\"anonymity\":0,\"fee\":$FEE,\"sourceAddresses\":[\"$W1_ADDR\"],\"transfers\":[{\"address\":\"$W3_ADDR\",\"amount\":$SEND_1_3}],\"changeAddress\":\"$W1_ADDR\"}")
TX_1_3=$(printf '%s' "$R" | jq -r '.result.transactionHash // empty'); [ -n "$TX_1_3" ] || fail "W1->W3 send failed: $(printf '%s' "$R" | jq -c '.error')"
ok "W1->W3 tx $TX_1_3"

# Mempool relay: both txs should gossip to EVERY node's pool before they are
# mined. /getinfo reports tx_pool_size, so once all three nodes report >=2
# pooled txs, the two sends have propagated across the p2p network.
log "Checking mempool relay (both txs reach all 3 nodes' pools)"
pool_count() { dget "$1" | jq -r '.tx_pool_size // 0'; }   # $1=rpcport
for i in 1 2 3; do
    relayed() { local c; c=$(pool_count "$((31000+i))"); [[ "$c" =~ ^[0-9]+$ ]] && [ "$c" -ge 2 ]; }
    wait_for "relay to node$i" 30 relayed || fail "txs did not relay to node$i's mempool (pool size $(pool_count "$((31000+i))"))"
    ok "node$i mempool holds $(pool_count "$((31000+i))") txs"
done

# ----------------------------------------------------------------------------
# 5. Confirm and assert exact receipt on the other nodes
# ----------------------------------------------------------------------------
log "Mining $MINE_CONFIRM confirmations"
TARGET=$((H+MINE_CONFIRM)); H=$(mine_and_propagate "$MINE_CONFIRM" "$W1_ADDR")
wait_for "W2 sync" 120 wapi_synced "$H" || fail "W2 (wallet-api) did not sync to $H"
wait_for "W3 sync" 120 svc_synced "$W3_PORT" "$H" || fail "W3 did not sync to $H"

W2_BAL=$(wapi_unlocked); W3_BAL=$(svc_avail "$W3_PORT")
[ "$W2_BAL" = "$SEND_1_2" ] || fail "W2 (wallet-api) balance $W2_BAL != expected $SEND_1_2"
ok "W2 (wallet-api @ node2) received exactly $SEND_1_2"
[ "$W3_BAL" = "$SEND_1_3" ] || fail "W3 (service) balance $W3_BAL != expected $SEND_1_3"
ok "W3 (service @ node3) received exactly $SEND_1_3"

# ----------------------------------------------------------------------------
# 6. Exercise the wallet-api SEND path: W2 -> W3
# ----------------------------------------------------------------------------
log "wallet-api send: W2 -> W3 ($SEND_2_3), mixin 0"
wapi POST /transactions/send/advanced "{\"destinations\":[{\"address\":\"$W3_ADDR\",\"amount\":$SEND_2_3}],\"mixin\":0,\"sourceAddresses\":[\"$W2_ADDR\"],\"changeAddress\":\"$W2_ADDR\"}"
[ "$WAPI_CODE" = 201 ] || [ "$WAPI_CODE" = 200 ] || fail "wallet-api send failed (HTTP $WAPI_CODE): $WAPI_BODY"
TX_2_3=$(printf '%s' "$WAPI_BODY" | jq -r '.transactionHash // empty'); [ -n "$TX_2_3" ] || fail "wallet-api send returned no tx hash: $WAPI_BODY"
ok "W2->W3 tx $TX_2_3"

TARGET=$((H+MINE_CONFIRM)); H=$(mine_and_propagate "$MINE_CONFIRM" "$W1_ADDR")
wait_for "W3 resync" 120 svc_synced "$W3_PORT" "$H" || fail "W3 did not resync to $H"
W3_BAL2=$(svc_avail "$W3_PORT")
[ "$W3_BAL2" = "$((SEND_1_3+SEND_2_3))" ] || fail "W3 balance $W3_BAL2 != expected $((SEND_1_3+SEND_2_3)) after wallet-api send"
ok "W3 now holds exactly $((SEND_1_3+SEND_2_3)) (received from a wallet-api send)"
wait_for "W2 resync" 120 wapi_synced "$H" || true
W2_BAL2=$(wapi_unlocked)
[ "$W2_BAL2" -lt "$SEND_1_2" ] || fail "W2 balance did not decrease after sending (was $SEND_1_2, now $W2_BAL2)"
ok "W2 balance decreased to $W2_BAL2 after its send"

# ----------------------------------------------------------------------------
# 7. Persistence: restart W3's service, balance/history must survive
# ----------------------------------------------------------------------------
log "Restarting W3's service to verify the container persists balance + history"
svc "$W3_PORT" save >/dev/null 2>&1 || true
# Stop just W3 (last-started service on W3_PORT), then reopen the same container.
W3_PID=$(pgrep -f "kryptokrona-service.*--bind-port $W3_PORT" | head -1 || true)
[ -n "$W3_PID" ] && { kill -9 "$W3_PID" 2>/dev/null || true; }
sleep 2
"$SERVICE" --container-file "$WORK/w3.container" --container-password "$WALLET_PASSWORD" \
    --daemon-address 127.0.0.1 --daemon-port 31003 \
    --bind-address 127.0.0.1 --bind-port "$W3_PORT" --rpc-password "$RPC_PASSWORD" \
    --log-level 1 > "$WORK/w3-restart.log" 2>&1 &
PIDS+=($!)
up3() { svc "$W3_PORT" getStatus | jq -e '.result' >/dev/null 2>&1; }
wait_for "W3 restart" 60 up3 || fail "W3 did not come back up after restart"
wait_for "W3 resync" 120 svc_synced "$W3_PORT" "$H" || fail "W3 did not resync after restart"
W3_BAL3=$(svc_avail "$W3_PORT")
[ "$W3_BAL3" = "$((SEND_1_3+SEND_2_3))" ] || fail "W3 balance $W3_BAL3 did not persist across restart (expected $((SEND_1_3+SEND_2_3)))"
TX_COUNT=$(svc "$W3_PORT" getTransactionHashes "{\"firstBlockIndex\":1,\"blockCount\":$((H+1))}" | jq -r '[.result.items[].transactionHashes[]] | length' 2>/dev/null || echo 0)
[ "$TX_COUNT" -ge 2 ] || fail "W3 transaction history did not persist (found $TX_COUNT tx, expected >=2)"
ok "W3 balance ($W3_BAL3) and $TX_COUNT-tx history persisted across restart"

# ----------------------------------------------------------------------------
log "SUCCESS"
echo
echo "Wallet flow test passed:"
echo "  - 3 nodes peered; blocks + txs propagated across the network"
echo "  - kryptokrona-service AND wallet-api both sent and received real funds"
echo "  - every wallet observed the exact expected amounts, on a different node"
echo "  - balances + tx history survived a wallet restart"
