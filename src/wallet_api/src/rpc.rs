use tarpc::context;

use crate::WalletRPCServer;

#[tarpc::service]
pub trait WalletRPC {
    // GET
    async fn get_node_info() -> String;
    async fn get_private_view_key() -> String;
    async fn get_spend_keys(address: String) -> String;
    async fn get_mnemonic_seed(address: String) -> String;
    async fn get_status() -> String;
    async fn get_addresses() -> String;
    async fn get_primary_address() -> String;
    async fn create_integrated_address(address: String, payment_id: String) -> String;
    async fn get_transactions() -> String;
    async fn get_unconfirmed_transactions() -> String;
    async fn get_unconfirmed_transactions_for_address(address: String) -> String;
    async fn get_transactions_from_height(block_height: u64) -> String;
    async fn get_transactions_from_height_to_height(start_height: u64, end_height: u64) -> String;
    async fn get_transactions_from_height_with_address(
        address: String,
        block_height: u64,
    ) -> String;
    async fn get_transactions_from_height_to_height_with_address(
        address: String,
        start_height: u64,
        end_height: u64,
    ) -> String;
    async fn get_tx_private_key(hash: String) -> String;
    async fn get_transaction_details(hash: String) -> String;
    async fn get_balance() -> String;
    async fn get_balance_for_address(address: String) -> String;
    async fn get_balances() -> String;

    // POST
    async fn wallet_open(name: String) -> String;
    async fn wallet_import_key(name: String) -> String;
    async fn wallet_import_seed(name: String) -> String;
    async fn wallet_import_view(name: String) -> String;
    async fn wallet_create(name: String) -> String;
    async fn addresses_create(name: String) -> String;
    async fn addresses_import(name: String) -> String;
    async fn addresses_import_view(name: String) -> String;
    async fn transactions_send_basic(name: String) -> String;
    async fn transactions_send_advanced(name: String) -> String;
    async fn transactions_send_fusion_basic(name: String) -> String;
    async fn transactions_send_fusion_advanced(name: String) -> String;

    // PUT
    async fn save_wallet(name: String) -> String;
    async fn reset_wallet(name: String) -> String;
    async fn set_node_info(name: String) -> String;

    // DELETE
    async fn close_wallet(name: String) -> String;
    async fn delete_address(name: String) -> String;

    // OPTIONS
    async fn handle_options(name: String) -> String;
}

impl WalletRPC for WalletRPCServer {
    // GET
    async fn get_node_info(self, _: context::Context) -> String {
        format!("Get Node Info")
    }

    async fn get_private_view_key(self, _: context::Context) -> String {
        format!("Get Private View Key")
    }

    async fn get_spend_keys(self, _: context::Context, address: String) -> String {
        format!("Get Spend Keys for Address {}", address)
    }

    async fn get_mnemonic_seed(self, _: context::Context, address: String) -> String {
        format!("Get Mnemonic Seed for Address {}", address)
    }

    async fn get_status(self, _: context::Context) -> String {
        format!("Get Status")
    }

    async fn get_addresses(self, _: context::Context) -> String {
        format!("Get Addresses")
    }

    async fn get_primary_address(self, _: context::Context) -> String {
        format!("Get Primary Address")
    }

    async fn create_integrated_address(
        self,
        _: context::Context,
        address: String,
        payment_id: String,
    ) -> String {
        format!(
            "Create Integrated Address from {} and Payment ID {}",
            address, payment_id
        )
    }

    async fn get_transactions(self, _: context::Context) -> String {
        format!("Get Transactions")
    }

    async fn get_unconfirmed_transactions(self, _: context::Context) -> String {
        format!("Get Unconfirmed Transactions")
    }

    async fn get_unconfirmed_transactions_for_address(
        self,
        _: context::Context,
        address: String,
    ) -> String {
        format!("Get Unconfirmed Transactions for Address {}", address)
    }

    async fn get_transactions_from_height(self, _: context::Context, block_height: u64) -> String {
        format!("Get Transactions from Height {}", block_height)
    }

    async fn get_transactions_from_height_to_height(
        self,
        _: context::Context,
        start_height: u64,
        end_height: u64,
    ) -> String {
        format!(
            "Get Transactions from Height {} to {}",
            start_height, end_height
        )
    }

    async fn get_transactions_from_height_with_address(
        self,
        _: context::Context,
        address: String,
        block_height: u64,
    ) -> String {
        format!(
            "Get Transactions from Height {} with Address {}",
            block_height, address
        )
    }

    async fn get_transactions_from_height_to_height_with_address(
        self,
        _: context::Context,
        address: String,
        start_height: u64,
        end_height: u64,
    ) -> String {
        format!(
            "Get Transactions from Height {} to {} with Address {}",
            start_height, end_height, address
        )
    }

    async fn get_tx_private_key(self, _: context::Context, hash: String) -> String {
        format!("Get Transaction Private Key for Hash {}", hash)
    }

    async fn get_transaction_details(self, _: context::Context, hash: String) -> String {
        format!("Get Transaction Details for Hash {}", hash)
    }

    async fn get_balance(self, _: context::Context) -> String {
        format!("Get Balance")
    }

    async fn get_balance_for_address(self, _: context::Context, address: String) -> String {
        format!("Get Balance for Address {}", address)
    }

    async fn get_balances(self, _: context::Context) -> String {
        format!("Get Balances")
    }

    // POST
    async fn wallet_open(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn wallet_import_key(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn wallet_import_seed(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn wallet_import_view(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn wallet_create(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn addresses_create(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn addresses_import(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn addresses_import_view(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn transactions_send_basic(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn transactions_send_advanced(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn transactions_send_fusion_basic(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn transactions_send_fusion_advanced(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    // PUT
    async fn save_wallet(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn reset_wallet(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn set_node_info(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    // DELETE
    async fn close_wallet(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    async fn delete_address(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }

    // OPTIONS
    async fn handle_options(self, _: context::Context, name: String) -> String {
        format!("Hello, {name}! You are connected from {}", self.0)
    }
}
