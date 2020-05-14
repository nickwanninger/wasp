use proc_macro_hack;

#[proc_macro_hack::proc_macro_hack]
use wasp_proc_macros::{nasm, cprog32};

pub mod machine;

use std::time::{Instant};

extern crate capstone;
use capstone::prelude::*;
use pretty_hex::*;



fn reg_names<T, I>(cs: &Capstone, regs: T) -> String
where
    T: Iterator<Item = I>,
    I: Into<RegId>,
{
    let names: Vec<String> = regs.map(|x| cs.reg_name(x.into()).unwrap()).collect();
    names.join(", ")
}



fn main() {
    /*
    let code = nasm!("
        [org 0x1000]

        mov eax, 0 ; get the value
        out 0xFF, eax

        mov ebx, eax
        shl ebx, 1

        mov eax, 1 ; print
        out 0xFF, eax
    ");
    */

    let code = cprog32!(r#"
        extern long hcall(long number, unsigned long arg);
        void main() {
            int val = hcall(0, 0);
            hcall(1, val * val);
        }
    "#);

    let mut m = machine::Machine::new(4096 * 10, code);

    for i in 0..10 {
        let start = Instant::now();
        m.run(move |vm| {
            let nr = vm.regs.rax;
            let stack = vm.read_bytes(vm.regs.rsp, 32).unwrap();
            match nr {
                // eax <- i
                0 => {
                    vm.regs.rax = i;
                    return machine::ExitType::Okay;
                }
                // print ebx
                1 => {
                    println!("gst: [{:6}ns] {} {}", start.elapsed().as_nanos(), i, vm.regs.rbx);
                    return machine::ExitType::Kill;
                }

                _ => {
                    println!("unknown hypercall 0x{:x}. Killing.", nr);
                    return machine::ExitType::Kill;
                }
            }
        });

        m.reset();
    }
}



