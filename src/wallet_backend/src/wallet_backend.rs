use node::Node;
use std::sync::{Arc, Mutex};

pub struct WalletBackend {
    filename: String,
    password: String,
    daemon: Arc<Mutex<Node>>,
    // m_event_handler: Arc<EventHandler>,
    // m_sub_wallets: Arc<SubWallets>,
}

impl WalletBackend {
    fn get_node_fee(&self) -> (u64, String) {
        match self.daemon.lock() {
            Ok(guard) => guard.node_fee(),
            Err(_) => (0, String::new()),
        }
    }

    fn get_node_address(&self) -> (u64, String) {
        match self.daemon.lock() {
            Ok(guard) => guard.node_address(),
            Err(_) => (0, String::new()),
        }
    }

    fn get_node_host(&self) -> (String, u16) {
        match self.daemon.lock() {
            Ok(guard) => guard.node_host(),
            Err(_) => (String::new(), 0),
        }
    }

    fn get_node_port(&self) -> (String, u16) {
        match self.daemon.lock() {
            Ok(guard) => guard.node_port(),
            Err(_) => (String::new(), 0),
        }
    }

    fn get_wallet_filename(&self) -> String {
        self.filename.clone()
    }
}
