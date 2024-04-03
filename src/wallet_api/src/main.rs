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
mod trace;

const PBKDF2_ITERATIONS: i64 = 10000;
// const ADDRESS_BODY_LENGTH: i16 =
// const ADDRESS_REGEX: &str =
const HASH_REGEX: &str = "[a-fA-F0-9]{64}";

use clap::Parser;
use futures::{future, prelude::*};
use rpc::WalletRPC;
use std::net::{IpAddr, Ipv4Addr, SocketAddr};
use tarpc::{
    server::{self, incoming::Incoming, Channel},
    tokio_serde::formats::Json,
};
use trace::init_tracing;

#[derive(Parser)]
struct Flags {
    #[clap(long)]
    port: u16,
    rpc_bind_ip: String,
    enable_cors: Option<String>,
    rpc_password: Option<String>,
}

#[derive(Clone)]
pub struct WalletRPCServer(SocketAddr);

async fn spawn(fut: impl Future<Output = ()> + Send + 'static) {
    tokio::spawn(fut);
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    // TODO: build CLI with Clap here to take in arguments instead
    let flags = Flags::parse();
    // init_tracing("Kryptokrona Wallet RPC Server")?;
    println!("Kryptokrona Wallet RPC Server");

    let server_addr = (IpAddr::V4(Ipv4Addr::LOCALHOST), flags.port);

    let mut listener = tarpc::serde_transport::tcp::listen(&server_addr, Json::default).await?;
    // tracing::info!("Listening on port {}", listener.local_addr().port());
    println!("Listening on {}", listener.local_addr());

    listener.config_mut().max_frame_length(usize::MAX);
    listener
        .filter_map(|r| future::ready(r.ok()))
        .map(server::BaseChannel::with_defaults)
        .max_channels_per_key(1, |t| t.transport().peer_addr().unwrap().ip())
        .map(|channel| {
            let server = WalletRPCServer(channel.transport().peer_addr().unwrap());
            channel.execute(WalletRPC::serve(server)).for_each(spawn)
        })
        .buffer_unordered(10)
        .for_each(|_| async {})
        .await;

    Ok(())
}
