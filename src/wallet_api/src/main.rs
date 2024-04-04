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

mod rpc;
mod api {
    pub mod address;
    pub mod misc;
    pub mod node;
    pub mod transaction;
    pub mod wallet;
}

use tonic::server;

// Use the definitions from node.rs
// use crate::api::address::address_server::AddressServer;
// use crate::api::misc::miscellaneous_server::MiscellaneousServer;
use crate::api::node::node_server::NodeServer;
use crate::api::transaction::transaction_server::TransactionServer;
// use crate::api::wallet::wallet_server::WalletServer;
use crate::rpc::{MyNode, MyTransaction};

const PBKDF2_ITERATIONS: i64 = 10000;
// const ADDRESS_BODY_LENGTH: i16 =
// const ADDRESS_REGEX: &str =
const HASH_REGEX: &str = "[a-fA-F0-9]{64}";

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let transaction = MyTransaction::default();
    // let wallet = Wallet::default();
    let node = MyNode::default();
    // let address = Address::default();
    // let misc = Misc::default();

    // Create server instances for each service
    let transaction_server = TransactionServer::new(transaction);
    // let wallet_server = WalletServer::new(inner);
    let node_server = NodeServer::new(node);
    // let address_server = AddressServer::new(inner);
    // let misc_server = MiscellaneousServer::new(inner);

    let server_addr = "[::1]:50055".parse().unwrap();
    println!("RPC server listening on {}", server_addr);
    tonic::transport::Server::builder()
        .add_service(transaction_server)
        .add_service(node_server)
        .serve(server_addr)
        .await?;

    Ok(())
}
