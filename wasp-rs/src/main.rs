extern crate crossbeam;
extern crate core_affinity;
extern crate num_cpus;

#[allow(unused_imports)]
use proc_macro_hack;
#[proc_macro_hack::proc_macro_hack]
#[allow(unused_imports)]
use wasp_proc_macros::cprog32;

pub mod machine;
pub mod dataflow;

use std::time::{Instant};

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

    return std::iter::Sum::sum(deps.into_iter());
    /*
    let code = cprog32!(r#"
        extern long hcall(long number, unsigned long arg);

        void main() {
            // add them up
            hcall(1, hcall(0, 0) + hcall(0, 1));
        }
    "#);

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
    */
}


fn main() {

    let cores = 7;


    for cores in 0..num_cpus::get() {
        let n = 20;
        let mut g = dataflow::Graph::construct(n, fib_recurse);

        let start = Instant::now();
        let result = g.evaluate(fib_eval, cores);
        let test_time = start.elapsed().as_secs_f64();

        println!("fib({}) = {} on {} cores, {:.3}s", n, result, cores, test_time);
    }
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




