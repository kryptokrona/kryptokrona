use tarpc::context;

use crate::WalletRPCServer;

#[tarpc::service]
pub trait WalletRPC {
    async fn wallet_open(name: String) -> String;
    async fn wallet_import_key(name: String) -> String;
}

impl WalletRPC for WalletRPCServer {
    // GET

    // POST
    async fn wallet_open(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn wallet_import_key(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    // PUT

    // DELETE

    // OPTIONS
}
