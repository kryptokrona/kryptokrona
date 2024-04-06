use tonic::{Request, Response, Status};

use crate::api::address::{
    address_server::Address, GetPrimaryAddressRequest, GetPrimaryAddressResponse,
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

        let response = GetPrimaryAddressResponse { address: todo!() };

        Ok(Response::new(response))
    }
}
