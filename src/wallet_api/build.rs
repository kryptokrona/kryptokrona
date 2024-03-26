// TODO: move to more appropriate place later since we should be able to build without wallet_api

use std::process::Command;
use std::env;

fn main() {
    let build_dir = env::var("OUT_DIR").unwrap();
    Command::new("cmake")
        .arg("-S")
        .arg("../../CMakeLists.txt")
        .arg("-B")
        .arg(&build_dir)
        .status()
        .expect("Failed to run CMake");

    Command::new("make")
        .status()
        .expect("Failed to build.");
}