

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

fn main() {
    let code: [u8; 32] = [0; 32];
    let mut myvm = VM::new(&code);

    let val = myvm.run::<i32, i32>(0).expect("ope");

    println!("val: {}", val);

}
