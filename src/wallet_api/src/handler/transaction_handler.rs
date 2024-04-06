use tonic::{Request, Response, Status};

use crate::transaction::{
    transaction_server::Transaction, SendBasicTransactionRequest, SendBasicTransactionResponse,
};

#[derive(Debug, Default)]
pub struct TransactionHandler;

#[tonic::async_trait]
impl Transaction for TransactionHandler {
    async fn send_basic_transaction(
        &self,
        request: Request<SendBasicTransactionRequest>,
    ) -> Result<Response<SendBasicTransactionResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = SendBasicTransactionResponse {
            status: todo!(),
            http_status_code: todo!(),
        };

        Ok(Response::new(response))
    }
}
