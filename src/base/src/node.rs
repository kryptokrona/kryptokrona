use std::{
    thread,
    time::Duration,
};

pub struct Node {
    timeout: Duration,
    daemon_host: String,
    daemon_port: i8,
    // http_client: Arc<Mutex<Client>>,
    should_stop: bool,
    local_daemon_block_count: u64,
    network_block_count: u64,
    peer_count: u64,
    last_known_hashrate: u64,
    node_fee_address: String,
    node_fee_amount: u32,
    background_thread: Option<thread::JoinHandle<()>>,
}

impl Node {
    fn new(daemon_host: String, daemon_port: u16) -> Self {
        let timeout = Duration::frosecs(10);
        // TODO: create a new Hyper Client here
        // let http_client = Arc::new(Mutex::new(Client::new(
        //     format!("{}:{}", daemon_host, daemon_port),
        //     timeout,
        // )));

        Node {
            timeout: timeout,
            daemon_host: daemon_host,
            daemon_port: daemon_port,
            // http_client: http_client,
            should_stop: false,
            local_daemon_block_count: 0,
            network_block_count: 0,
            peer_count: 0,
            last_known_hashrate: 0,
            node_fee_address: String::new(),
            node_fee_amount: 0,
            background_thread: None,
        }
    }

    fn swap_node(&mut self, daemon_host: String, daemon_port: u16) {
        self.stop();

        self.local_daemon_block_count = 0;
        self.network_block_count = 0;
        self.peer_count = 0;
        self.last_known_hashrate = 0;

        self.daemon_host = daemon_host;
        self.daemon_port = daemon_port;

        // let http_client = Arc::new(Mutex::new(httplib::Client::new(
        //     format!("{}:{}", daemon_host, daemon_port),
        //     self.timeout,
        // )));

        // self.http_client = http_client;

        self.init();
    }

    fn stop(&mut self) {
        self.should_stop = true;
        if let Some(handle) = thread::current().name() {
            if handle == "background_thread" {
                if let Some(background_thread) = self.background_thread.take() {
                    background_thread.join().unwrap();
                }
            }
        }
    }

    fn init(&mut self) {
        self.should_stop = false;

        self.get_daemon_info();
        self.get_fee_info();

        // let http_client = Arc::clone(&self.http_client);
        let timeout = self.timeout;
        // thread::Builder::new()
        //     .name("background_thread".to_string())
        //     .spawn(move || {
        //         while !self.should_stop {
        //             self.get_daemon_info();
        //             thread::sleep(timeout);
        //         }
        //     })
        //     .unwrap();
    }

    pub fn get_fee(&mut self) {
        // Implementation for getting daemon info
    }

    pub fn get_address(&mut self) -> String {
        // Implementation for getting fee info
    }

    pub fn get_host(&mut self) -> String {
        self.daemon_host.clone()
    }

    pub fn get_port(&mut self) -> i8 {
        self.daemon_port.clone()
    }
}
