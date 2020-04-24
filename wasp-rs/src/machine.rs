#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));



pub struct Machine {
    // reference to the internal machine object.
    machine: *mut wasp_machine_t,
    ramsize: u64
}

impl Machine {
    pub fn new(ramsize: u64) -> Machine {
        let machine = unsafe { wasp_machine_create(ramsize) };

        Machine {
            machine: machine,
            ramsize: ramsize,
        }
    }

    /// load binary code 
    pub fn load_code() -> bool {
        false
    }


    pub fn reset() {
        
    }
}


impl Drop for Machine {
    fn drop(&mut self) {
        // free the machine with the FFI
        unsafe {
            wasp_machine_free(self.machine);
        };
    }
}

pub fn run_binary_code(code: &[u8], ramsize: u64) {
    let _vm = Machine::new(ramsize);
}
