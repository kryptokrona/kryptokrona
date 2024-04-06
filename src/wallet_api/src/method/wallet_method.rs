use tonic::{Request, Response, Status};

use crate::api::wallet::{wallet_server::Wallet, OpenWalletRequest, OpenWalletResponse};

#[derive(Debug, Default)]
pub struct MyWallet;

#[tonic::async_trait]
impl Wallet for MyWallet {
    async fn open_wallet(
        &self,
        request: Request<OpenWalletRequest>,
    ) -> Result<Response<OpenWalletResponse>, Status> {
        println!("Received request from: {:?}", request);

        let response = OpenWalletResponse {
            status: todo!(),
            http_status_code: todo!(),
        };

        Ok(Response::new(response))
    }
}
