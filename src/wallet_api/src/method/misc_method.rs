use tonic::{Request, Response, Status};

use crate::api::{
    node::{node_server::Node, GetNodeDetailsRequest, GetNodeDetailsResponse},
    transaction::{
        transaction_server::Transaction, SendBasicTransactionRequest, SendBasicTransactionResponse,
    },
    wallet::{wallet_server::Wallet, OpenWalletRequest, OpenWalletResponse},
};
