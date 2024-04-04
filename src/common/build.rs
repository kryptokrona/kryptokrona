extern crate cc;

fn main() {
    let mut build = cc::Build::new();

    build.warnings(false);
    build.cpp(true);
    build.flag("-std=c++17");
    build.include("../crypto/c");

    build.file("cpp/util.cpp").include("cpp");

    build.compile("cppcommon");
}
