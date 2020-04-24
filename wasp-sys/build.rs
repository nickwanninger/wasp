extern crate bindgen;
extern crate cfg_if;

use std::env;
use std::path::PathBuf;

fn main() {
    // Tell cargo to link the wasp shared libraries.
    println!("cargo:rustc-link-search=../build/core/");
    println!("cargo:rustc-link-lib=static=wasp");

    cfg_if::cfg_if! {
        if #[cfg(target_os = "linux")] {
            println!("cargo:rustc-link-search=../build/platform/linux");
            println!("cargo:rustc-link-lib=static=wasp_backend_linux");
        }
        else if #[cfg(target_os = "windows")] {
            println!("cargo:rustc-link-search=../build/platform/windows");
            println!("cargo:rustc-flags=link-args='-Wl,-force_load'");
            println!("cargo:rustc-link-lib=static=wasp_backend_windows");
        }
        else {
            panic!("unsupported wasp backend platform");
        }
    }

    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        .header("wrapper.h")

        // Only include our types, ignore any other transitively included libraries
        .whitelist_type("wasp_.*")
        .whitelist_function("wasp_.*")

        // Ignore integer shorthand types, since they conflict with Rust's built-ins
        .blacklist_type("i[0-9]+")
        .blacklist_type("u[0-9]+")

        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))

        // Include directory for wrapper header
        .clang_arg("-I../include/")

        .rustfmt_bindings(true)
        .generate()
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(PathBuf::from("./gen").join("bindings.rs"))
        .expect("Couldn't write bindings!");

    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}
