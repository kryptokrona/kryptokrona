grpcurl -plaintext -d '{}' localhost:50051 address.Address.GetPrimaryAddress
grpcurl -plaintext -d '{}' localhost:50051 node.Node.GetNodeDetails