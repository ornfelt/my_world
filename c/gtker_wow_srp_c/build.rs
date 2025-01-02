fn main() {
    println!("cargo:rustc-cdylib-link-arg=-Wl,-soname,libwow_srp.so");
}
