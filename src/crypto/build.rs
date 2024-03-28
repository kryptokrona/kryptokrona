extern crate cc;

fn main() {
    let mut build = cc::Build::new();

    build.flag("-std=c99");
    build.warnings(false);
    build.include("../common/cpp");

    build.file("c/aesb.c")
        .file("c/blake256.c")
        .file("c/crypto-ops-data.c")
        .file("c/crypto-ops.c")
        .file("c/groestl.c")
        .file("c/hash-extra-blake.c")
        .file("c/hash-extra-groestl.c")
        .file("c/hash-extra-jh.c")
        .file("c/hash-extra-skein.c")
        .file("c/hash.c")
        .file("c/jh.c")
        .file("c/keccak.c")
        .file("c/oaes_lib.c")
        .file("c/skein.c")
        .file("c/slow-hash.c")
        .include("c");

    build.compile("ccrypto");
}