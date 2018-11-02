# USAGE: Start turtle-service as you usually would
# then run `python optimize.py --address $ADDRESS --port $PORT --password $RPCPASS`
# replacing $ADDRESS, $PORT AND $RPCPASS with your values
from turtlecoin import Walletd as Wallet
import time
import argparse
parser = argparse.ArgumentParser()
parser.add_argument("--address", help="Turtlecoin address to optimize", required=True)
parser.add_argument("--port", type=int, help="Wallet port", required=False, default=8070)
parser.add_argument("--password", help="Wallet RPC Password", required=True)
parser.add_argument("--wait", type=int, help="Time to wait between fusions", required=False, default=5)
args = parser.parse_args()


def main():

    wallet = Wallet(password=args.password, port=args.port)
    while True:
        fuseable = wallet.estimate_fusion(100000, [args.address])
        print("Outputs fuseable: {}".format(fuseable))
        try:
            print("Sending Fusion")
            fusion_result = wallet.send_fusion_transaction(100000, 3, [args.address], args.address)
            print("Fusion result: {}".format(y))
            time.sleep(args.wait)
        except ValueError:
            print("Node threw an error, continuing...")

if __name__ == "__main__":
    main()
