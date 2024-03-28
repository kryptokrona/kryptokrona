use std::ffi::{c_int, c_void};

extern "C" {
    fn cn_fast_hash(data: c_void, length: c_int, hash: c_ch);
}

#[tokio::main]
async fn main() {
    unsafe {
        cn_fast_hash();
    }
    println!("hello");
}