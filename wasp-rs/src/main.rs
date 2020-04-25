use proc_macro_hack;

#[proc_macro_hack::proc_macro_hack]
use wasp_proc_macros::nasm;

pub mod machine;

use std::time::{Instant};


fn main() {
    let code = nasm!("
        [org 0x1000]

        mov eax, 0 ; get the value
        out 0xFF, eax

        mov ebx, eax
        shl ebx, 1

        mov eax, 1 ; print
        out 0xFF, eax
    ");


    let mut m = machine::Machine::new(4096 * 10, code);

    for i in 0.. {

        
        let start = Instant::now();

        let res = i * 2;
        println!("hst: [{:6}ns] {} {}", start.elapsed().as_nanos(), i, res);

        let start = Instant::now();
        m.run(move |vm| {
            let nr = vm.regs.rax;
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



