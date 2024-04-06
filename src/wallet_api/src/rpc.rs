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

// #[tonic::async_trait]
// impl WalletRPC for WalletRPCServer {
//     // GET
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
