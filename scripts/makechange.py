#!/usr/bin/env python3

'''
Usage: python makechange.py
This is python3, so you might need to launch it with python3 makechange.py

Make two wallets and fill one or both with some funds, or start mining to it.
Open the wallets with walletd like so:

./walletd -w walletA.wallet -p yourpass --rpc-password test --daemon-port 11898 --bind-port 8070
./walletd -w walletB.wallet -p yourpass --rpc-password test --daemon-port 11898 --bind-port 8071

Feel free to change these parameters if needed of course.

Next, fill in the address variables with your two address, and change the walletd ports and hosts if needed.

Make sure the address variables are correct, because if it isn't, you will
rapidly be sending your funds to this address.

This script rapidly sends random amount of funds from two wallets to each other,
hopefully generating change on a new network.
'''

import requests
import json
import random
import time

# The address of your first wallet
addressA = "Fill me in!"

# The address of your second wallet
addressB = "Fill me in!"

if len(addressA) != 99 or len(addressB) != 99:
    print("Please fill in your addresses and re-run the script.")
    quit()

walletdPortA = "8070"
walletdAddressA = "127.0.0.1"

walletdPortB = "8071"
walletdAddressB = "127.0.0.1"

rpcPasswordA = "test"
rpcPasswordB = "test"

def sendTransaction(host, port, rpcPassword, **kwargs):
    payload = {
        'jsonrpc': '2.0',
        'method': "sendTransaction",
        'password': rpcPassword,
        'id': 'test',
        'params': kwargs
    }

    url = 'http://' + host + ':' + port + '/json_rpc'

    response = requests.post(url, data=json.dumps(payload),
                             headers={'content-type': 'application/json'}).json()

    if 'error' in response:
        print(response['error'])
        return False
    else:
        print(response['result'])
        return True

def sendTXs(host, port, rpcPassword, sender, receiver):
    def loop():
        n = 1000
        while(n < 100000000000):
            yield n
            n *= 10

    sleepAmount = 0.001

    while True:
        for i in loop():
            # give it a bit more randomness, maybe this helps
            amount = random.randint(i, i+10000)

            params = {'transfers': [{'address': receiver, 'amount': amount}],
                      'fee': 10,
                      'anonymity': 5,
                      'changeAddress': sender}

            if not sendTransaction(host, port, rpcPassword, **params):
                time.sleep(sleepAmount)
                print("Sleeping for " + str(sleepAmount) + " seconds...")
                sleepAmount *= 2
                break
            else:
                sleepAmount = 0.001

while True:
    sendTXs(walletdAddressA, walletdPortA, rpcPasswordA, sender=addressA, receiver=addressB)
    sendTXs(walletdAddressB, walletdPortB, rpcPasswordB, sender=addressB, receiver=addressA)
