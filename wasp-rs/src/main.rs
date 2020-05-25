
#![feature(proc_macro_hygiene, decl_macro)]

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

#[macro_use] extern crate rocket;

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

fn fib_eval(n: i32, deps: Vec<i32>) -> i32 {
    if n < 2 {
        return n;
    }


    if true {
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

    return std::iter::Sum::sum(deps.into_iter());
}


#[get("/fib/<n>")]
fn get_fib(n: i32) -> String {
    let mut pool = dataflow::WorkPool::new(15);

    // let mut g = dataflow::Graph::load(&graph);
    let mut g = dataflow::Graph::construct(n, fib_recurse);


    // let start = Instant::now();
    let result = g.evaluate(fib_eval, &mut pool);
    // let test_time = start.elapsed().as_secs_f64();

    // println!("|{:5}|  fib({}) = {} in {:.6}s", i, n, result, test_time);
    // println!("{},{:.6}", i, test_time);
    pool.join();
    serde_json::to_string(&result).unwrap()
}

fn main() {

    let n = 20;

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

        println!("{},{:.8},{:.8},{:.8}", i, vm_time, rust_time, c_time);
    }

    /*

    rocket::ignite()
        .mount("/", routes![
            get_fib,
        ]).launch();

    */
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




