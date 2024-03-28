use std::process::Command;
use std::env;
use std::path::{Path, PathBuf};

fn main() {
    let cmake_lists_path = PathBuf::from(env::current_dir().unwrap())
        .join("../../CMakeLists.txt")
        .canonicalize()
        .expect("Failed to calculate path to CMakeLists.txt");

    let build_dir = env::var("OUT_DIR").unwrap();

    Command::new("cmake")
        .arg("-S")
        .arg(&cmake_lists_path)
        .arg("-B")
        .arg(&build_dir)
        .status()
        .expect("Failed to run CMake");

    Command::new("make")
        .status()
        .expect("Failed to build.");
}