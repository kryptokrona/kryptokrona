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

use tonic::Request;

#[derive(Clone)]
pub struct WalletRPCServer;

/* pub trait WalletRPC {
    // GET
    async fn get_node_info(&self, request: Request<String>) -> String; // TODO: add a serialized serde type for the request instead later
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
} */

pub struct WalletRpC {}

// #[tonic::async_trait]
// impl WalletRPC for WalletRPCServer {
//     // GET
//     async fn get_node_info(&self, request: Request<String>) -> String {
//         format!("Get Node Info")
//     }

//     async fn get_private_view_key(&self, request: Request<String>) -> String {
//         "Get Private View Key".to_string()
//     }

//     async fn get_spend_keys(&self, request: Request<String>) -> String {
//         format!("Get Spend Keys for Address")
//     }

//     async fn get_mnemonic_seed(&self, request: Request<String>) -> String {
//         format!("Get Mnemonic Seed for Address")
//     }

//     async fn get_status(&self, request: Request<String>) -> String {
//         format!("Get Status")
//     }

//     async fn get_addresses(&self, request: Request<String>) -> String {
//         format!("Get Addresses")
//     }

//     async fn get_primary_address(&self, request: Request<String>) -> String {
//         "Get Primary Address".to_string()
//     }

//     async fn create_integrated_address(&self, request: Request<String>) -> String {
//         format!("Create Integrated Address from and Payment ID ")
//     }

//     async fn get_transactions(&self, request: Request<String>) -> String {
//         format!("Get Transactions")
//     }

//     async fn get_unconfirmed_transactions(&self, request: Request<String>) -> String {
//         "Get Unconfirmed Transactions".to_string()
//     }

//     async fn get_unconfirmed_transactions_for_address(&self, request: Request<String>) -> String {
//         format!("Get Unconfirmed Transactions for Address")
//     }

//     async fn get_transactions_from_height(&self, request: Request<String>) -> String {
//         format!("Get Transactions from Height")
//     }

//     async fn get_transactions_from_height_to_height(&self, request: Request<String>) -> String {
//         format!("Get Transactions from Height  to ")
//     }

//     async fn get_transactions_from_height_with_address(&self, request: Request<String>) -> String {
//         format!("Get Transactions from Height  with Address ")
//     }

//     async fn get_transactions_from_height_to_height_with_address(
//         &self,
//         request: Request<String>,
//     ) -> String {
//         format!("Get Transactions from Height  to  with Address ")
//     }

//     async fn get_tx_private_key(&self, request: Request<String>) -> String {
//         format!("Get Balance")
//     }

//     async fn get_transaction_details(&self, request: Request<String>) -> String {
//         format!("Get Balance")
//     }

//     async fn get_balance(&self, request: Request<String>) -> String {
//         format!("Get Balance")
//     }

//     async fn get_balance_for_address(&self, request: Request<String>) -> String {
//         format!("Get Balance")
//     }

//     async fn get_balances(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     // POST
//     async fn wallet_open(&self, request: Request<String>) -> String {
//         println!("YOOOOOOOO");
//         format!("Get Balances")
//     }

//     async fn wallet_import_key(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     async fn wallet_import_seed(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     async fn wallet_import_view(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     async fn wallet_create(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     async fn addresses_create(&self, request: Request<String>) -> String {
//         format!("Hello, ! You are connected from ")
//     }

//     async fn addresses_import(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     async fn addresses_import_view(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     async fn transactions_send_basic(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     async fn transactions_send_advanced(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     async fn transactions_send_fusion_basic(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     async fn transactions_send_fusion_advanced(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     // PUT
//     async fn save_wallet(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     async fn reset_wallet(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     async fn set_node_info(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     // DELETE
//     async fn close_wallet(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     async fn delete_address(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }

//     // OPTIONS
//     async fn handle_options(&self, request: Request<String>) -> String {
//         format!("Get Balances")
//     }
// }
