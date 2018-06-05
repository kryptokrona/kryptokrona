#!/bin/bash
 
# The host your rpc is running on
host=127.0.0.1
 
# The port your rpc is running on
port=31898
 
# If you only triggered the diff algo at a certain point
startHeight=1
 
# Get the current block height
height=$(curl --silent -X POST -H "Accept:application/json" -d '{"jsonrpc":"2.0", "id":"test", "method":"getblockcount", "params":{}}' $host:$port/json_rpc | jq ".result.count")
 
# Loop from start height to current height
for i in $(seq $startHeight $(expr $height - 1));
do
    # Get height, timestamp, and diff for given block height
    result=$(curl --silent -X POST -H "Accept:application/json" -d '{"jsonrpc":"2.0", "id":"test", "method":"getblockheaderbyheight", "params":{"height" : '"$i"'}}' $host:$port/json_rpc | jq ".result.block_header | .height, .timestamp, .difficulty")
    # Tab separate and print to stdout
    echo $result | awk -v OFS="\t" '$1=$1'
done
