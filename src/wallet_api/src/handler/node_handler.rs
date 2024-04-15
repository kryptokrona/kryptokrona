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

use crate::api::node::{
    node_server::Node, GetNodeDetailsRequest, GetNodeDetailsResponse, GetStatusRequest,
    GetStatusResponse, SetNodeInfoRequest, SetNodeInfoResponse,
};

#[derive(Debug, Default)]
pub struct NodeHandler;

#[tonic::async_trait]
impl Node for NodeHandler {
    async fn get_status(
        &self,
        request: Request<GetStatusRequest>,
    ) -> Result<Response<GetStatusResponse>, Status> {
        println!("Received request from: {:?}", request);

        // let wallet_backend = node::WalletBackend {};

        // let daemon_host = wallet_backend

        //     const auto [daemonHost, daemonPort] = m_walletBackend->getNodeAddress();

        // const auto [nodeFee, nodeAddress] = m_walletBackend->getNodeFee();

        // nlohmann::json j{
        //     {"daemonHost", daemonHost},
        //     {"daemonPort", daemonPort},
        //     {"nodeFee", nodeFee},
        //     {"nodeAddress", nodeAddress}};

        // res.set_content(j.dump(4) + "\n", "application/json");

        // return {SUCCESS, 200};

        let response = GetStatusResponse {};

        Ok(Response::new(response))
    }

    async fn set_node_info(
        &self,
        request: Request<SetNodeInfoRequest>,
    ) -> Result<Response<SetNodeInfoResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = SetNodeInfoResponse {};

        Ok(Response::new(response))
    }

    async fn get_node_details(
        &self,
        request: Request<GetNodeDetailsRequest>,
    ) -> Result<Response<GetNodeDetailsResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetNodeDetailsResponse {
            daemon_host: String::from("1000"),
            daemon_port: 11898,
            node_fee: 10,
            node_address: String::from("2312asdakdopj12p3j1"),
        };

        Ok(Response::new(response))
    }
}
