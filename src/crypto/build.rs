extern crate cc;

fn main() {
    let mut build = cc::Build::new();
    let tool = build.get_compiler();

    if tool.is_like_gnu() || tool.is_like_clang() {
        build.flag_if_supported("-std=c99");
        build
            .flag_if_supported("-msse4.1")
            .flag_if_supported("-maes");
    }

    build.warnings(false);
    build.include("../common/cpp");

    build
        .file("c/aesb.c")
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
