
// use std::{env, fs, path::Path, process::Command};
extern crate proc_macro;
extern crate rand;

use proc_macro::TokenStream;
use proc_macro_hack::proc_macro_hack;
use syn::{parse_macro_input};
use std::process::{ Command, Stdio };
use std::fs;
use rand::Rng;



/// Turns a string of NASM assembly to a binary blob
#[proc_macro_hack]
pub fn nasm(tokens: TokenStream) -> TokenStream {

    let input = parse_macro_input!(tokens as syn::LitStr);
    let input = input.value();

    let mut rng = rand::thread_rng();


    // let rs_file_dir = Path::new(file!()).parent().expect("rust file has no parent?");
    let tmpfile_name = format!("{}-{:016}.asm", file!(), rng.gen::<u64>());

    fs::write(tmpfile_name.clone(), input).expect("failed to write code to tmpfile");

    let child = Command::new("nasm")
                    .arg("-f").arg("bin")
                    .arg("-o").arg("/dev/stdout")
                    .arg(tmpfile_name.clone())
                    .stdout(Stdio::piped())
                    .spawn()
                    .expect("failed to execute nasm");

    let output = child.wait_with_output().unwrap();

    fs::remove_file(tmpfile_name.clone()).expect("failed to remove tmpfile");

    if !output.status.success() {
        panic!("failed to compile nasm");
    }


    let rust_out = format!(
        "{{
            const BINARY_DATA: [u8; {}] = [{}];
            &BINARY_DATA[..]
        }}",
        output.stdout.len(),
        output.stdout.iter()
            .map(|c| format!("0x{:x}", c))
            .collect::<Vec<String>>()
            .join(", "),
    );


    rust_out.parse().unwrap()

}
