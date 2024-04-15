use std::{
    thread,
    time::Duration,
};

pub struct Node {
    m_timeout: Duration,
    m_daemon_host: String,
    m_daemon_port: u16,
    // m_http_client: Arc<Mutex<Client>>,
    m_should_stop: bool,
    m_local_daemon_block_count: u64,
    m_network_block_count: u64,
    m_peer_count: u64,
    m_last_known_hashrate: u64,
    m_node_fee_address: String,
    m_node_fee_amount: u32,
    m_background_thread: Option<thread::JoinHandle<()>>,
}

impl Node {
    fn new(daemon_host: String, daemon_port: u16) -> Self {
        let timeout = Duration::from_secs(10);
        // TODO: create a new Hyper Client here
        // let http_client = Arc::new(Mutex::new(Client::new(
        //     format!("{}:{}", daemon_host, daemon_port),
        //     timeout,
        // )));

        Node {
            m_timeout: timeout,
            m_daemon_host: daemon_host,
            m_daemon_port: daemon_port,
            // m_http_client: http_client,
            m_should_stop: false,
            m_local_daemon_block_count: 0,
            m_network_block_count: 0,
            m_peer_count: 0,
            m_last_known_hashrate: 0,
            m_node_fee_address: String::new(),
            m_node_fee_amount: 0,
            m_background_thread: None,
        }
    }

    fn swap_node(&mut self, daemon_host: String, daemon_port: u16) {
        self.stop();

        self.m_local_daemon_block_count = 0;
        self.m_network_block_count = 0;
        self.m_peer_count = 0;
        self.m_last_known_hashrate = 0;

        self.m_daemon_host = daemon_host;
        self.m_daemon_port = daemon_port;

        // let http_client = Arc::new(Mutex::new(httplib::Client::new(
        //     format!("{}:{}", daemon_host, daemon_port),
        //     self.m_timeout,
        // )));

        // self.m_http_client = http_client;

        self.init();
    }

    fn stop(&mut self) {
        self.m_should_stop = true;
        if let Some(handle) = thread::current().name() {
            if handle == "background_thread" {
                if let Some(background_thread) = self.m_background_thread.take() {
                    background_thread.join().unwrap();
                }
            }
        }
    }

    fn init(&mut self) {
        self.m_should_stop = false;

        self.get_daemon_info();
        self.get_fee_info();

        // let http_client = Arc::clone(&self.m_http_client);
        let timeout = self.m_timeout;
        // thread::Builder::new()
        //     .name("background_thread".to_string())
        //     .spawn(move || {
        //         while !self.m_should_stop {
        //             self.get_daemon_info();
        //             thread::sleep(timeout);
        //         }
        //     })
        //     .unwrap();
    }

    pub fn get_daemon_info(&mut self) {
        // Implementation for getting daemon info
    }

    pub fn get_fee_info(&mut self) {
        // Implementation for getting fee info
    }

    // Implement other methods of Node struct
}
