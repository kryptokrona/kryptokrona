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
    init_tracing("Kryptokrona Wallet RPC Server")?;

    let server_addr = (IpAddr::V4(Ipv4Addr::LOCALHOST), flags.port);

    let mut listener = tarpc::serde_transport::tcp::listen(&server_addr, Json::default).await?;
    tracing::info!("Listening on port {}", listener.local_addr().port());

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
