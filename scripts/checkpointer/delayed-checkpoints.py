#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import json
import requests

topbuffer = 3 * 24 * 60 * 2


def lastknownblock():
    try:
        with open(sys.argv[1], 'r') as cp:
            (block, hash) = list(cp)[-1].split(',')
            cp.close()
            return int(block)
    except:
        return 0


def height():
    base_url = 'http://localhost:11898/getheight'
    resp = requests.get(base_url).json()
    if 'height' not in resp:
        print ('Unexpected response, make sure TurtleCoind is running',
               resp)
        sys.exit(-1)
    else:
        return resp['height']


def rpc(method, params={}):
    base_url = 'http://localhost:11898/json_rpc'
    payload = {
        'jsonrpc': '2.0',
        'id': 'block_info',
        'method': method,
        'params': params,
        }
    resp = requests.post(base_url, data=json.dumps(payload)).json()
    if 'result' not in resp:
        print ('Unexpected response, make sure Turtlecoind is running with block explorer enabled'
               , resp)
        sys.exit(-1)
    else:
        return resp['result']


def get_height():
    resp = rpc('getblockcount')
    return resp['count']


def get_block_info(from_height):
    resp = rpc('f_blocks_list_json', {'height': from_height})
    return resp['blocks']


stop_height = lastknownblock() + 1

current_height = height() - topbuffer
all_blocks = []
while current_height > stop_height:
    try:
        blocks = get_block_info(current_height)
        for b in blocks:
            print '%(height)s,%(hash)s' % b
            all_blocks.append('%(height)s,%(hash)s' % b)
            current_height = b['height'] - 1
            if current_height < stop_height:
                break
    except:
        print "Whoops... let's try that again"

all_blocks.reverse()

with open(sys.argv[1], 'a') as f:
    f.write('\n'.join(all_blocks))
    if len(all_blocks) > 0:
        f.write('\n')
