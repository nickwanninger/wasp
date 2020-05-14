#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));


use std::sync::{Mutex, Arc};
use std::collections::{VecDeque};

pub enum ExitType {
    Okay, Kill
}

pub struct Machine<'a> {
    // reference to the internal machine object.
    machine: *mut wasp_machine_t,
    // ramsize: u64,
    code: &'a [u8],
    clean: bool,
}

impl<'a> Machine<'a> {
    pub fn new(ramsize: u64, code: &'a [u8]) -> Self {
        let machine = unsafe { wasp_machine_create(ramsize) };

        let mut m = Machine {
            machine: machine,
            // ramsize: ramsize,
            code: code,
            clean: true,
        };

        m.reset();

        return m;
    }


    pub fn reset(&mut self) {
        unsafe {
            wasp_machine_reset(self.machine);

            let code = self.code.as_ptr() as *mut std::ffi::c_void;
            wasp_inject_code(self.machine, code, self.code.len() as u64, 0x1000);
        };
        self.clean = true;
    }

    // run the VM with a certain workload
    pub fn run(&mut self, f: impl for<'b> Fn(&mut HyperCall) -> ExitType)
    {
        if !self.clean {
            self.reset();
        }

        let mut ctx = WorkloadContext {
            cb: Box::new(f)
        };
        let mut workload = wasp_workload_t {
            ctx: &mut ctx as *mut _ as *mut std::os::raw::c_void,
            init: Some(workload_init),
            handle_hcall: Some(workload_hcall),
            handle_exit: Some(workload_exit)
        };

        unsafe {
            wasp_machine_run(self.machine, &mut workload as *mut _);
        };

        self.clean = false;
    }
}

unsafe impl Send for Machine<'_> {}
unsafe impl Sync for Machine<'_> {}


impl<'a> Drop for Machine<'a> {
    fn drop(&mut self) {
        // free the machine with the FFI
        unsafe {
            wasp_machine_free(self.machine);
        };
    }
}



pub struct HyperCall<'a> {
    pub regs: &'a mut wasp_regs_t,
    ram: *mut u8,
    ramsize: usize
}


impl<'a> HyperCall<'a> {


    fn read_byte(&self, off: isize) -> u8 {
        if off >= self.ramsize as isize {
            return 0u8;
        }

        unsafe {*self.ram.offset(off)}
    }


    pub fn read_bytes(&self, off: u64, len: isize) -> Option<Vec<u8>> {
        // bounds check
        if off as isize + len >= self.ramsize as isize {
            return None;
        }

        let slice = unsafe { std::slice::from_raw_parts(self.ram.offset(off as isize), len as usize) };
        Some(slice.to_vec())
    }

    pub fn read_string(&self, mut off: u64) -> String {
        let mut s = String::new();

        loop {
            let b = self.read_byte(off as isize);

            if b == 0 {
                break;
            }
            s.push(b as char);
            off += 1;
        }

        return s;
    }
}


// unsafe implementations for the workload callbacks
struct WorkloadContext<'b> {
    pub cb: Box<dyn Fn(&mut HyperCall) -> ExitType + 'b>
}


unsafe extern "C" fn workload_init(_self_: *mut wasp_workload_t, _config: *mut ::std::os::raw::c_void) {
    // nothing
}


unsafe extern "C" fn workload_hcall(
    workload: *mut wasp_workload_t,
    regs: *mut wasp_regs_t,
    ramsize: size_t,
    ram: *mut ::std::os::raw::c_void,
) -> ::std::os::raw::c_int {

    let mut hcall = HyperCall {
        regs: &mut *regs,
        ram: ram as *mut u8,
        ramsize: ramsize as usize
    };

    let ctx = &*((*workload).ctx as *mut WorkloadContext);

    match (ctx.cb)(&mut hcall) {
        ExitType::Okay => 0,
        ExitType::Kill => -1,
    }
}


unsafe extern "C" fn workload_exit(_self_: *mut wasp_workload_t) {
    // nothing
}






/// Implements pooling
pub struct MachinePool<'a, T> {
    avail: Mutex<VecDeque<Machine<'a>>>,
    ramsize: usize,
    code: &'a [u8],
    cb: Arc<dyn Fn(&mut HyperCall, T) -> ExitType + 'static>
}


impl<'a, T: Copy> MachinePool<'a, T> {
    pub fn new(
        ramsize: usize,
        code: &'a [u8],
        f: impl Fn(&mut HyperCall, T) -> ExitType + 'static,
    ) -> Self {
        MachinePool {
            avail: Mutex::new(VecDeque::new()),
            ramsize,
            code,
            cb: Arc::new(f)
        }
    }

    fn get_vm(&mut self) -> Machine<'a> {
        let mut a = self.avail.lock().unwrap();

        if a.is_empty() {
            Machine::new(self.ramsize as u64, self.code)
        } else {
            a.pop_front().expect("hey, std::sync::Mutex lied to us. Not cool!")
        }
    }


    /// run a vm with an "argument value" `val`
    pub fn fire(&mut self, val: T) {
        let mut m = self.get_vm();
        m.run(|hc| {
            (self.cb)(hc, val)
        });
        self.avail.lock().unwrap().push_back(m);
    }
}


impl<'a, T> Drop for MachinePool<'a, T> {
    fn drop(&mut self) {
        // self.tpool.join();
    }
}
