# curl -X POST http://localhost:8000 -H "Content-Type: application/json" -d '{
#     "method": "wallet_open",
#     "params": {
#         "name": "wallet_name"
#     },
# }'

curl -X POST http://localhost:8000 -H "Content-Type: application/json" -d '{
    "method": "get_primary_address"
}'