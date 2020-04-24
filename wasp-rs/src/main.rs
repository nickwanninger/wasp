use proc_macro_hack;

#[proc_macro_hack::proc_macro_hack]
use wasp_proc_macros::nasm;

extern crate capstone;
use capstone::prelude::*;

pub mod machine;


macro_rules! nasm_vm {
    ($e:expr) => (VM {
        code: nasm!($e)
    });
}


fn main() {
    let x = nasm!("
        [bits 64]
        mov rax, 1
        syscall
        ret
    ");

    let vm = nasm_vm!("
        [bits 64]
        mov rax, 2
    ");

    println!("vm: {:#?}", vm);



    let cs = Capstone::new()
        .x86()
        .mode(arch::x86::ArchMode::Mode64)
        .syntax(arch::x86::ArchSyntax::Intel)
        .detail(true)
        .build().expect("failed to build capstone");

    let insns = cs.disasm_all(x, 0x1000).expect("failed to diasm");


    let myvm = machine::Machine::new(4096 * 10);


    println!("code: {:02x?}", x);

    println!("asm:");
    for i in insns.iter() {
        println!("{}", i);

    }

}









#[derive(Debug)]
pub struct VM<'a> {
    code: &'a [u8]
}


unsafe fn runvmwithcode(
    _code: *const u8, _codesize: usize,
    _arg: *const u8, _argsize: usize,
    _out: *const u8, _outsize: usize,
    ) -> i32 {
    0
}


impl<'a> VM<'a> {
    pub fn new(code: &'a [u8]) -> VM<'a> {
        VM {
            code: code
        }
    }

    pub fn run<I: Sized, O: Sized>(&mut self, _arg: I) -> Result<O, i32> {

        let output_buffer: O = unsafe { std::mem::zeroed() };

        let result: i32 = unsafe {

            // run the vm in C function
            runvmwithcode(
                self.code.as_ptr(), // code pointer
                self.code.len(), // codesize

                &_arg as *const I as *const _,
                std::mem::size_of::<I>(), // argsize


                &output_buffer as *const O as *const _,
                std::mem::size_of::<O>() // argsize
                )
        };

        if result < 0 {
            return Err(-result);
        }


        Ok(output_buffer)
    }

}
