fn main() {
    // Make sure to have the proto compiler installed on your system
    // we will add it here to install depending on OS later
    tonic_build::configure()
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
