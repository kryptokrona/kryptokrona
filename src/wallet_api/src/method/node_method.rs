use tonic::{Request, Response, Status};

use crate::api::{
    node::{node_server::Node, GetNodeDetailsRequest, GetNodeDetailsResponse},
    transaction::{
        transaction_server::Transaction, SendBasicTransactionRequest, SendBasicTransactionResponse,
    },
    wallet::{wallet_server::Wallet, OpenWalletRequest, OpenWalletResponse},
};

#[derive(Debug, Default)]
pub struct MyNode;

#[tonic::async_trait]
impl Node for MyNode {
    async fn get_node_details(
        &self,
        request: Request<GetNodeDetailsRequest>,
    ) -> Result<Response<GetNodeDetailsResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = GetNodeDetailsResponse {
            daemon_host: todo!(),
            daemon_port: todo!(),
            node_fee: todo!(),
            node_address: todo!(),
        };

        Ok(Response::new(response))
    }
}
