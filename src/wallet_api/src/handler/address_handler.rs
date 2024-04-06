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

use crate::api::address::{
    address_server::Address, AddressesCreateRequest, AddressesCreateResponse,
    AddressesImportRequest, AddressesImportResponse, AddressesImportViewRequest,
    AddressesImportViewResponse, CreateIntegratedAddressRequest, CreateIntegratedAddressResponse,
    DeleteAddressRequest, DeleteAddressResponse, GetAddressesRequest, GetAddressesResponse,
    GetPrimaryAddressRequest, GetPrimaryAddressResponse,
};

#[derive(Debug, Default)]
pub struct AddressHandler;

#[tonic::async_trait]
impl Address for AddressHandler {
    async fn get_primary_address(
        &self,
        request: Request<GetPrimaryAddressRequest>,
    ) -> Result<Response<GetPrimaryAddressResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetPrimaryAddressResponse {
            address: "Hello!".to_string(),
        };

        Ok(Response::new(response))
    }

    async fn create_integrated_address(
        &self,
        request: Request<CreateIntegratedAddressRequest>,
    ) -> Result<Response<CreateIntegratedAddressResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = CreateIntegratedAddressResponse {};

        Ok(Response::new(response))
    }

    async fn addresses_create(
        &self,
        request: Request<AddressesCreateRequest>,
    ) -> Result<Response<AddressesCreateResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = AddressesCreateResponse {};

        Ok(Response::new(response))
    }

    async fn addresses_import(
        &self,
        request: Request<AddressesImportRequest>,
    ) -> Result<Response<AddressesImportResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = AddressesImportResponse {};

        Ok(Response::new(response))
    }

    async fn addresses_import_view(
        &self,
        request: Request<AddressesImportViewRequest>,
    ) -> Result<Response<AddressesImportViewResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = AddressesImportViewResponse {};

        Ok(Response::new(response))
    }

    async fn get_addresses(
        &self,
        request: Request<GetAddressesRequest>,
    ) -> Result<Response<GetAddressesResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetAddressesResponse {};

        Ok(Response::new(response))
    }

    async fn delete_address(
        &self,
        request: Request<DeleteAddressRequest>,
    ) -> Result<Response<DeleteAddressResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = DeleteAddressResponse {};

        Ok(Response::new(response))
    }
}
