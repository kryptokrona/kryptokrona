// Copyright (c) 2019-2024, The Kryptokrona Project
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

mod api;
mod rpc;

use api::address::address_server;

// use transaction::TransactionRPCServer;
// use wallet::WalletRPCServer;
// use node::NodeRPCServer;
// use address::AddressRPCServer;
// use misc::MiscRPCServer;

const PBKDF2_ITERATIONS: i64 = 10000;
// const ADDRESS_BODY_LENGTH: i16 =
// const ADDRESS_REGEX: &str =
const HASH_REGEX: &str = "[a-fA-F0-9]{64}";

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Create server instances for each service
    let transaction_server = TransactionServer::default();
    let wallet_server = WalletServer::default();
    let node_server = NodeServer::default();
    let address_server = address_server::default();
    let misc_server = MiscServer::default();

    // Serve the gRPC servers
    let transaction_server_addr = "[::1]:50053".parse().unwrap();
    tokio::spawn(async move {
        println!(
            "Transaction server listening on {}",
            transaction_server_addr
        );
        tonic::transport::Server::builder()
            .add_service(transaction_server)
            .serve(transaction_server_addr)
            .await
            .unwrap();
    });

    let wallet_server_addr = "[::1]:50051".parse().unwrap();
    println!("Wallet server listening on {}", wallet_server_addr);
    tokio::spawn(async move {
        tonic::transport::Server::builder()
            .add_service(wallet_server)
            .serve(wallet_server_addr)
            .await
            .unwrap();
    });

    let node_server_addr = "[::1]:50052".parse().unwrap();
    println!("Node server listening on {}", node_server_addr);
    tokio::spawn(async move {
        tonic::transport::Server::builder()
            .add_service(node_server)
            .serve(node_server_addr)
            .await
            .unwrap();
    });

    let address_server_addr = "[::1]:50054".parse().unwrap();
    println!("Address server listening on {}", address_server_addr);
    tokio::spawn(async move {
        tonic::transport::Server::builder()
            .add_service(address_server)
            .serve(address_server_addr)
            .await
            .unwrap();
    });

    let misc_server_addr = "[::1]:50055".parse().unwrap();
    println!("Misc server listening on {}", misc_server_addr);
    tonic::transport::Server::builder()
        .add_service(misc_server)
        .serve(misc_server_addr)
        .await?;

    Ok(())
}
