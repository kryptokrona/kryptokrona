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

use tonic::{Request, Response, Status};

use crate::api::wallet::{
    wallet_server::Wallet, CloseWalletRequest, CloseWalletResponse, GetBalanceForAddressRequest,
    GetBalanceForAddressResponse, GetBalanceRequest, GetBalanceResponse, GetBalancesRequest,
    GetBalancesResponse, GetMnemonicSeedRequest, GetMnemonicSeedResponse, GetPrivateViewKeyRequest,
    GetPrivateViewKeyResponse, GetSpendKeysRequest, GetSpendKeysResponse, GetTxPrivateKeyRequest,
    GetTxPrivateKeyResponse, OpenWalletRequest, OpenWalletResponse, ResetWalletRequest,
    ResetWalletResponse, SaveWalletRequest, SaveWalletResponse, WalletCreateRequest,
    WalletCreateResponse, WalletImportKeyRequest, WalletImportKeyResponse, WalletImportSeedRequest,
    WalletImportSeedResponse, WalletImportViewRequest, WalletImportViewResponse,
};

#[derive(Debug, Default)]
pub struct WalletHandler;

#[tonic::async_trait]
impl Wallet for WalletHandler {
    async fn open_wallet(
        &self,
        request: Request<OpenWalletRequest>,
    ) -> Result<Response<OpenWalletResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = OpenWalletResponse {
            status: 200,
            http_status_code: 200,
        };

        Ok(Response::new(response))
    }

    async fn wallet_import_key(
        &self,
        request: Request<WalletImportKeyRequest>,
    ) -> Result<Response<WalletImportKeyResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = WalletImportKeyResponse {};

        Ok(Response::new(response))
    }

    async fn wallet_import_seed(
        &self,
        request: Request<WalletImportSeedRequest>,
    ) -> Result<Response<WalletImportSeedResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = WalletImportSeedResponse {};
        Ok(Response::new(response))
    }

    async fn wallet_import_view(
        &self,
        request: Request<WalletImportViewRequest>,
    ) -> Result<Response<WalletImportViewResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = WalletImportViewResponse {};
        Ok(Response::new(response))
    }

    async fn wallet_create(
        &self,
        request: Request<WalletCreateRequest>,
    ) -> Result<Response<WalletCreateResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = WalletCreateResponse {};
        Ok(Response::new(response))
    }

    async fn save_wallet(
        &self,
        request: Request<SaveWalletRequest>,
    ) -> Result<Response<SaveWalletResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = SaveWalletResponse {};
        Ok(Response::new(response))
    }

    async fn reset_wallet(
        &self,
        request: Request<ResetWalletRequest>,
    ) -> Result<Response<ResetWalletResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = ResetWalletResponse {};
        Ok(Response::new(response))
    }

    async fn close_wallet(
        &self,
        request: Request<CloseWalletRequest>,
    ) -> Result<Response<CloseWalletResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = CloseWalletResponse {};
        Ok(Response::new(response))
    }

    async fn get_balance(
        &self,
        request: Request<GetBalanceRequest>,
    ) -> Result<Response<GetBalanceResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetBalanceResponse {};
        Ok(Response::new(response))
    }

    async fn get_balance_for_address(
        &self,
        request: Request<GetBalanceForAddressRequest>,
    ) -> Result<Response<GetBalanceForAddressResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetBalanceForAddressResponse {};
        Ok(Response::new(response))
    }

    async fn get_balances(
        &self,
        request: Request<GetBalancesRequest>,
    ) -> Result<Response<GetBalancesResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetBalancesResponse {};
        Ok(Response::new(response))
    }

    async fn get_private_view_key(
        &self,
        request: Request<GetPrivateViewKeyRequest>,
    ) -> Result<Response<GetPrivateViewKeyResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetPrivateViewKeyResponse {};
        Ok(Response::new(response))
    }

    async fn get_spend_keys(
        &self,
        request: Request<GetSpendKeysRequest>,
    ) -> Result<Response<GetSpendKeysResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetSpendKeysResponse {};
        Ok(Response::new(response))
    }

    async fn get_mnemonic_seed(
        &self,
        request: Request<GetMnemonicSeedRequest>,
    ) -> Result<Response<GetMnemonicSeedResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetMnemonicSeedResponse {};
        Ok(Response::new(response))
    }

    async fn get_tx_private_key(
        &self,
        request: Request<GetTxPrivateKeyRequest>,
    ) -> Result<Response<GetTxPrivateKeyResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetTxPrivateKeyResponse {};
        Ok(Response::new(response))
    }
}
