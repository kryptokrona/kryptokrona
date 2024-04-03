fn main() {
    tonic_build::configure()
        .compile(
            &[
                "proto/node.proto",
                "proto/wallet.proto",
                "proto/transaction.proto",
                "proto/address.proto",
                "proto/misc.proto",
            ],
            &["proto"],
        )
        .unwrap_or_else(|e| panic!("Failed to compile protos {:?}", e));
}
