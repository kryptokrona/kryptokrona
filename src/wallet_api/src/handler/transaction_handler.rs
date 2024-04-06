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

use crate::{
    api::transaction::{
        GetTransactionDetailsRequest, GetTransactionDetailsResponse,
        GetTransactionsFromHeightRequest, GetTransactionsFromHeightResponse,
        GetTransactionsFromHeightToHeightRequest, GetTransactionsFromHeightToHeightResponse,
        GetTransactionsFromHeightToHeightWithAddressRequest,
        GetTransactionsFromHeightToHeightWithAddressResponse,
        GetTransactionsFromHeightWithAddressRequest, GetTransactionsFromHeightWithAddressResponse,
        GetTransactionsRequest, GetTransactionsResponse,
        GetUnconfirmedTransactionsForAddressRequest, GetUnconfirmedTransactionsForAddressResponse,
        GetUnconfirmedTransactionsRequest, GetUnconfirmedTransactionsResponse,
        SendAdvancedTransactionRequest, SendAdvancedTransactionResponse,
        SendFusionAdvancedTransactionRequest, SendFusionAdvancedTransactionResponse,
        SendFusionBasicTransactionRequest, SendFusionBasicTransactionResponse,
    },
    transaction::{
        transaction_server::Transaction, SendBasicTransactionRequest, SendBasicTransactionResponse,
    },
};

#[derive(Debug, Default)]
pub struct TransactionHandler;

#[tonic::async_trait]
impl Transaction for TransactionHandler {
    async fn get_transactions(
        &self,
        request: Request<GetTransactionsRequest>,
    ) -> Result<Response<GetTransactionsResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetTransactionsResponse {};

        Ok(Response::new(response))
    }

    async fn get_unconfirmed_transactions(
        &self,
        request: Request<GetUnconfirmedTransactionsRequest>,
    ) -> Result<Response<GetUnconfirmedTransactionsResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetUnconfirmedTransactionsResponse {};

        Ok(Response::new(response))
    }

    async fn get_unconfirmed_transactions_for_address(
        &self,
        request: Request<GetUnconfirmedTransactionsForAddressRequest>,
    ) -> Result<Response<GetUnconfirmedTransactionsForAddressResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetUnconfirmedTransactionsForAddressResponse {};

        Ok(Response::new(response))
    }

    async fn get_transactions_from_height(
        &self,
        request: Request<GetTransactionsFromHeightRequest>,
    ) -> Result<Response<GetTransactionsFromHeightResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetTransactionsFromHeightResponse {};

        Ok(Response::new(response))
    }

    async fn get_transactions_from_height_to_height(
        &self,
        request: Request<GetTransactionsFromHeightToHeightRequest>,
    ) -> Result<Response<GetTransactionsFromHeightToHeightResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetTransactionsFromHeightToHeightResponse {};

        Ok(Response::new(response))
    }

    async fn get_transactions_from_height_with_address(
        &self,
        request: Request<GetTransactionsFromHeightWithAddressRequest>,
    ) -> Result<Response<GetTransactionsFromHeightWithAddressResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetTransactionsFromHeightWithAddressResponse {};

        Ok(Response::new(response))
    }

    async fn get_transactions_from_height_to_height_with_address(
        &self,
        request: Request<GetTransactionsFromHeightToHeightWithAddressRequest>,
    ) -> Result<Response<GetTransactionsFromHeightToHeightWithAddressResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetTransactionsFromHeightToHeightResponse {};

        Ok(Response::new(response))
    }

    async fn get_transaction_details(
        &self,
        request: Request<GetTransactionDetailsRequest>,
    ) -> Result<Response<GetTransactionDetailsResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetTransactionDetailsResponse {};

        Ok(Response::new(response))
    }

    async fn send_basic_transaction(
        &self,
        request: Request<SendBasicTransactionRequest>,
    ) -> Result<Response<SendBasicTransactionResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = SendBasicTransactionResponse {
            status: 200,
            http_status_code: 200,
        };

        Ok(Response::new(response))
    }

    async fn send_advanced_transaction(
        &self,
        request: Request<SendAdvancedTransactionRequest>,
    ) -> Result<Response<SendAdvancedTransactionResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = SendAdvancedTransactionResponse {};

        Ok(Response::new(response))
    }

    async fn send_fusion_basic_transaction(
        &self,
        request: Request<SendFusionBasicTransactionRequest>,
    ) -> Result<Response<SendFusionBasicTransactionResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = SendFusionBasicTransactionResponse {};

        Ok(Response::new(response))
    }

    async fn send_fusion_advanced_transaction(
        &self,
        request: Request<SendFusionAdvancedTransactionRequest>,
    ) -> Result<Response<SendFusionAdvancedTransactionResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = SendFusionAdvancedTransactionResponse {};

        Ok(Response::new(response))
    }
}
