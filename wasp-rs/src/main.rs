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


    if false {
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


fn main() {
    let mut pool = dataflow::WorkPool::new(32);

    let n = 20;
    let graph = dataflow::Graph::construct(n, fib_recurse).serialize();

    println!("{} bytes of serialization", graph.len());

    for i in 0.. {

        let mut g = dataflow::Graph::load(&graph);

        let start = Instant::now();
        let result = g.evaluate(fib_eval, &mut pool);
        let test_time = start.elapsed().as_secs_f64();

        println!("{:4}  fib({}) = {} in {:.6}s", i, n, result, test_time);
        pool.join();
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




