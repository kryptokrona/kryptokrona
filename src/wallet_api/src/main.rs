use sled::Error;

use clap::Arg;

const PBKDF2_ITERATIONS: i64 = 10000;
// const ADDRESS_BODY_LENGTH: i16 =
// const ADDRESS_REGEX: &str =
const HASH_REGEX: &str = "[a-fA-F0-9]{64}";

struct Config {
    port: u16,
    rpc_bind_ip: String,
    enable_cors: Option<String>,
    rpc_password: Option<String>,
}

#[tokio::main]
async fn main() -> Result<(), Error> {
    Ok(())
}