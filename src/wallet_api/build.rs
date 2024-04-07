use std::{env, path::PathBuf};

fn main() {
    // Make sure to have the protobuf compiler installed on your system

    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());

    tonic_build::configure()
        .file_descriptor_set_path(out_dir.join("wallet_api_descriptor.bin"))
        .build_server(true)
        .out_dir("src/api/")
        .compile(
            &[
                "proto/node.proto",
                "proto/wallet.proto",
                "proto/transaction.proto",
                "proto/address.proto",
            ],
            &["proto"],
        )
        .unwrap_or_else(|e| panic!("Failed to compile protos {:?}", e));
}
