
// use std::{env, fs, path::Path, process::Command};
extern crate proc_macro;
extern crate rand;

use proc_macro::TokenStream;
use proc_macro_hack::proc_macro_hack;
use syn::{parse_macro_input};
use std::process::{ Command, Stdio };
use std::fs;
use std::path::Path;
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


fn relative_file(name: &str) -> String {
    Path::new(file!()).parent().unwrap().join(name).display().to_string()
}


fn compile_cprog(bootprog: &str, cprog: String, linker: &str, gcc_args: &str) -> TokenStream {
    let script = relative_file("compile_cprog.sh");
    let bootprog = relative_file(bootprog);
    let linker = relative_file(linker);


    let mut rng = rand::thread_rng();
    let tmpfile_name = format!("/tmp/{:016}.c", rng.gen::<u64>());
    fs::write(tmpfile_name.clone(), cprog).expect("failed to write code to tmpfile");


    let child = Command::new(script)
                    .arg(bootprog)
                    .arg(tmpfile_name)
                    .arg(linker)
                    .arg(gcc_args)
                    .stdout(Stdio::piped())
                    .spawn()
                    .expect("failed to execute nasm");

    let output = child.wait_with_output().unwrap();


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

/// Turns a string of NASM assembly to a binary blob
#[proc_macro_hack]
pub fn cprog32(tokens: TokenStream) -> TokenStream {
    let input = parse_macro_input!(tokens as syn::LitStr);
    let input = input.value();

    compile_cprog("boot32.asm", input, "link32.ld", "-m32")
}
