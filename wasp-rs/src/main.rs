
use proc_macro_hack;
#[proc_macro_hack::proc_macro_hack]
use wasp_proc_macros::{cprog32};

pub mod machine;

use std::sync::{Arc};
use tokio::sync::{mpsc};
use std::collections::{VecDeque};
use std::time::{Instant};
use std::marker::PhantomData;
use serde;
use serde::{Serialize, Deserialize};



/// A graph that evaluates to a T
struct Graph<T> {
    final_rx: mpsc::Receiver<T>,
}

type TaskRef<T> = Arc<dyn Task<T> + std::marker::Send + std::marker::Sync + 'static>;

impl<T: 'static + std::marker::Send> Graph<T> {

    pub fn new(root: TaskRef<T>) -> Self {
        let (tx, rx) = mpsc::channel(1);

        let mut s = Self {
            final_rx: rx
        };
        s.add_task(root, tx);
        return s;
    }

    pub fn add_task(&mut self, tsk: TaskRef<T>, dst: mpsc::Sender<T>) {
        let mut recvs = VecDeque::<mpsc::Receiver<T>>::new();
        for d in tsk.expand() {
            let (tx, rx) = mpsc::channel(1);
            recvs.push_back(rx);
            self.add_task(d, tx);
        }

        tokio::spawn(async move {
            let mut gn = GraphNode::new(tsk, dst, recvs);
            gn.evaluate().await;
        });
    }

    /// Evaluate the graph starting on a root node
    pub async fn run(&mut self) -> T {
        // Wait on the final mpsc
        self.final_rx.recv().await.expect("Failed to await on Graph::run's result")
    }
}



struct GraphNode<T> {
    task: TaskRef<T>, // the task which handles executing this node

    tx: mpsc::Sender<T>, // where the return value is sent
    rx: VecDeque<mpsc::Receiver<T>>, // values that are generated by the dependencies
}


impl<T: std::marker::Send> GraphNode<T> {
    pub fn new(
        task: TaskRef<T>,
        tx: mpsc::Sender<T>,
        rx: VecDeque<mpsc::Receiver<T>>) -> Self
    {
        Self {
            task, tx, rx
        }
    }
    /// Wait for all the dependencies to resolve, then evaluate the task, placing the value
    /// in the self.tx mpsc
    pub async fn evaluate(&mut self) {
        let mut v = vec![];
        for r in &mut self.rx {
            let rv = r.recv().await.expect("Failed to wait on dependency");
            v.push(rv);
        }
        let value = self.task.evaluate(v);
        match self.tx.send(value).await {
            Err(_e) => {
                panic!("Failed!\n");
            },
            _ => {}
        }
    }
}

trait Task<T>
    where T: std::marker::Send
{
    /// Expand this trait into it's dependencies.
    fn expand(&self) -> Vec<TaskRef<T>>;
    fn evaluate(&self, deps: Vec<T>) -> T;
}

impl<T> Task<T> {
}


/**
 *
 *
 */
struct FibTask {
    n: i32
}

impl FibTask {
    pub fn alloc(n: i32) -> Arc<Self> {
        Arc::new(FibTask::new(n))
    }
    pub fn new(n: i32) -> Self {
        FibTask { n }
    }
}

impl Task<i32> for FibTask {
    fn expand(&self) -> Vec<TaskRef<i32>> {
        if self.n < 2 {
            return vec![];
        }
        return vec![
            Arc::new(FibTask::new(self.n - 1)),
            Arc::new(FibTask::new(self.n - 2))];
    }

    fn evaluate(&self, deps: Vec<i32>) -> i32 {

        if self.n < 2 {
            return self.n;
        }

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
    }
}


fn fib(n: i32) -> i32 {
    if n < 2 {
        return n;
    }

    fib(n - 1) + fib(n - 2)
}


fn sum(n: i64) -> i64 {
    if n == 1 {
        return 1;
    }

    return n + sum(n-1);
}

#[tokio::main]
async fn main() {
    for i in 1..500 {
        let root = SumTask::alloc(i);
        let mut graph = Graph::new(root);


        let start = Instant::now();
        let res = graph.run().await;
        let test_time = start.elapsed().as_secs_f64();

        println!("{},{}", i, test_time*1000.0);
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




struct SumTask {
    n: i64
}

impl SumTask {
    fn alloc(n: i64) -> Arc<Self> {
        Arc::new(SumTask::new(n))
    }
    fn new(n: i64) -> Self {
        SumTask { n }
    }
}

impl Task<i64> for SumTask {
    fn expand(&self) -> Vec<TaskRef<i64>> {
        if self.n == 1 {
            return vec![];
        }
        return vec![Arc::new(SumTask::new(self.n - 1))];
    }

    fn evaluate(&self, deps: Vec<i64>) -> i64 {

        let code = cprog32!(r#"
            extern long hcall(long number, unsigned long arg);

            void main() {
                long n = hcall(0, 0);

                for (int i = 0; 1; i++) {
                    int a = hcall(1, i);
                    if (a == 0) break;
                    n += a;
                }
                hcall(2, n);
            }
        "#);


        let mut m = machine::Machine::new(4096 * 4, code);
        let mut res: i32 = 0;
        m.run(|vm| {
            match vm.regs.rax {
                0 => {
                    vm.regs.rax = self.n as u64;
                    return machine::ExitType::Okay;
                }
                1 => {
                    if (vm.regs.rbx as usize) < deps.len() {
                        vm.regs.rax = deps[vm.regs.rbx as usize] as u64;
                    } else {
                        vm.regs.rax = 0;
                    }
                    return machine::ExitType::Okay;
                }
                2 => {
                    res = vm.regs.rbx as i32;
                    return machine::ExitType::Kill;
                }
                _ => {
                    println!("unknown hypercall. Killing.");
                    return machine::ExitType::Kill;
                }
            }
        });
        return res as i64;

        let mut n = self.n;
        for d in deps {
            n += d;
        }
        return n;
    }
}
