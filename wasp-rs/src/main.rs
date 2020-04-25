use proc_macro_hack;

#[proc_macro_hack::proc_macro_hack]
use wasp_proc_macros::nasm;

pub mod machine;



fn main() {
    let code = nasm!("
        [org 0x1000]
        mov eax, 1 ; print
        mov ebx, msg
        out 0xFF, eax

        msg: db \"hello, world\", 0
    ");

    let mut mp = machine::MachinePool::new(10, 4096 * 10, code, |vm, value| {

        println!("value: {}", value);
        let nr = vm.regs.rax;
        match nr {
            // print
            1 => {
                let address = vm.regs.rbx;
                println!("{:08x} {}", address, vm.read_string(address));
                return machine::ExitType::Kill;
            }

            _ => {
                println!("unknown hypercall 0x{:x}. Killing.", nr);
                return machine::ExitType::Kill;
            }
        }
    });


    for i in 0..30 {
        mp.fire(i);
    }
}







