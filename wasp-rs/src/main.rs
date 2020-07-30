
extern crate crossbeam;
extern crate core_affinity;
extern crate num_cpus;
extern crate threadpool;
extern crate bincode;


#[allow(unused_imports)]
use proc_macro_hack;
#[proc_macro_hack::proc_macro_hack]
#[allow(unused_imports)]
use wasp_proc_macros::cprog32;

pub mod machine;
pub mod dataflow;

use std::time::{Instant};




extern "C" {
    pub fn wasp_fib(n: i32) -> i32;
}




fn fib(n: i32) -> i32 {
    if n < 2 { return n; }
    return fib(n - 2) + fib(n - 1);
}







fn fib_vm(n: i32) -> i32 {
    let code = cprog32!(r#"
        extern long hcall(long number, unsigned long arg);


        int fib(int n) {
            if (n < 2) return n;
            return fib(n-2) + fib(n-1);
        }

        void main() {
            int n = hcall(0, 0);

            hcall(1, fib(n));
        }
    "#);

    // allocate a vm with 4 pages of ram.
    let mut m = machine::Machine::new(4096 * 4, code);
    let mut res: i32 = 0;

    m.run(|vm| {
        match vm.regs.rax {
            0 => {
                vm.regs.rax = n as u64;
                return machine::ExitType::Okay;
            }
            1 => {
                res = vm.regs.rbx as i32;
                return machine::ExitType::Kill;
            }
            _ => {
                println!("unknown hypercall. Killing.");
                return machine::ExitType::Kill;
            }
            }
    });
    return res;
}







fn fib_recurse(n: i32) -> Option<Vec<i32>> {
    if n < 2 {
        return None;
    }
    Some(vec![n - 1, n - 2])
}


fn fib_eval_vm(n: i32, deps: Vec<i32>) -> i32 {
    if n < 2 {
        return n;
    }


    let code = cprog32!(r#"
        extern long hcall(long number, unsigned long arg);

        void main() {
            // add them up
            hcall(1, hcall(0, 0) + hcall(0, 1));
        }
    "#);

    // allocate a vm with 4 pages of ram.
    let mut m = machine::Machine::new(4096 * 4, code);
    let mut res: i32 = 0;

    m.run(|vm| {
        match vm.regs.rax {
            0 => {
                vm.regs.rax = deps[vm.regs.rbx as usize] as u64;
                return machine::ExitType::Okay;
            }
            1 => {
                res = vm.regs.rbx as i32;
                return machine::ExitType::Kill;
            }
            _ => {
                println!("unknown hypercall. Killing.");
                return machine::ExitType::Kill;
            }
        }
    });
    return res;

}

fn fib_eval_native(n: i32, deps: Vec<i32>) -> i32 {
    if n < 2 {
        return n;
    }

    return std::iter::Sum::sum(deps.into_iter());
}

use std::fs::File;
use std::io::prelude::*;



fn measure_fib() {
    for n in 0..=(30/5) {
        let n = n * 5;

        // let mut file = File::create(format!("data/fib/fib_vm_{}.csv", n)).unwrap();

        for i in 0..1000 {
            let start = Instant::now();
            let result = fib_vm(n);
            let vm_time = start.elapsed().as_secs_f64();

            let start = Instant::now();
            let result = fib(n);
            let rust_time = start.elapsed().as_secs_f64();


            let start = Instant::now();
            let result = fib(n);
            let c_time = start.elapsed().as_secs_f64();


            // file.write(format!("{},{:.8},{:.8},{:.8}\n", n, vm_time, rust_time, c_time).as_bytes());
        }
        println!("{} done", n);
    }
}



fn measure_fib_dataflow() {

    // one threadpool for the whole thing
    let mut pool = dataflow::WorkPool::new(15);

    for n in 0..=(15/5) {

        let n = n * 5;


        println!("working on fib({})...", n);

        let mut native_file = File::create(format!("data/df/nested_native_{}.csv", n)).unwrap();
        let mut vm_file = File::create(format!("data/df/nested_vm_{}.csv", n)).unwrap();

        native_file.write(format!("n,latency,throughput,is_vm\n").as_bytes());
        vm_file.write(format!("n,latency,throughput,is_vm\n").as_bytes());


        for i in 0..100 {
        println!("working on fib({})... {}", n, i);
            let mut graph_vm = dataflow::Graph::construct(n, fib_recurse);
            let mut graph_native = dataflow::Graph::construct(n, fib_recurse);

            let ntasks = graph_vm.task_count() as f64;

            let start = Instant::now();
            graph_vm.evaluate(fib_eval_vm, &mut pool);
            let vm_time = start.elapsed().as_secs_f64();


            let start = Instant::now();
            graph_native.evaluate(fib_eval_native, &mut pool);
            let native_time = start.elapsed().as_secs_f64();


            native_file.write(format!("{},{},{},{}\n", n, native_time, ntasks / native_time, false).as_bytes());
            vm_file.write(format!("{},{},{},{}\n", n, vm_time, ntasks / native_time, true).as_bytes());

            // native_file.write(format!("{},{:.8},{:.8},{:.8}\n", n, vm_time, rust_time, c_time).as_bytes());
            // vm_file.write(format!("{},{:.8},{:.8},{:.8}\n", n, vm_time, rust_time, c_time).as_bytes());
        }
    }

    pool.join();
}


fn measure_concur() {


    let code = cprog32!(r#"
        extern long hcall(long number, unsigned long arg);
        void main() {
            hcall(0, 0);
        }
    "#);
    let mut m = machine::Machine::new(4096 * 3, code);

    for iter in 0..1000 {

        let start = Instant::now();
        for i in 0..1000 {
            m.run(|vm| machine::ExitType::Kill );
        }
        let time = start.elapsed().as_secs_f64();

        println!("{},{}", iter, 1000.0 / time);
    }
}

fn main() {
    // measure_concur();
    // measure_fib();
    measure_fib_dataflow();

}






#[test]
fn vm_multiply() {
    let code = cprog32!(r#"
        extern long hcall(long number, unsigned long arg);

        void main() {
            int val = hcall(0, 0);
            hcall(1, val * val);
        }
    "#);


    for i in 0..3000 {
        let mut m = machine::Machine::new(4096 * 10, code);
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
                    assert_eq!(i * i, vm.regs.rbx);
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




