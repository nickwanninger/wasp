mod dataflow;

use std::convert::TryInto;
use std::io::{Bytes, Write};
use futures::try_join;
use lambda::{INVOCATION_CTX, handler_fn, Handler, LambdaCtx};
use serde::{Deserialize, Serialize};
use serde::export::PhantomData;
use rusoto_core::{Region, RusotoError};
use rusoto_lambda::{InvocationRequest, InvokeError, Lambda, LambdaClient, ListFunctionsRequest};
use std::time::Instant;
use std::fs;
use std::fs::File;
use chrono::Utc;
use std::path::Path;
use std::collections::HashMap;
use std::collections::hash_map::RandomState;

type Error = Box<dyn std::error::Error + Send + Sync + 'static>;

#[derive(Serialize, Deserialize, Clone)]
struct MyInput { }

#[derive(Serialize, Deserialize, Clone)]
struct MyOutput {
    report: HashMap<i32, String>,
}

#[tokio::main]
async fn main() -> Result<(), Error> {
    println!("wasp-dataflow-serverless");
    let func = handler_fn(my_handler);
    lambda::run(func).await?;
    Ok(())
}

async fn my_handler(e: MyInput) -> Result<MyOutput, Error> {
    let report = fib();
    Ok(MyOutput { report })
}

fn fib() -> HashMap<i32, String, RandomState> {
    let mut report = HashMap::<i32, String>::new();

    // one threadpool for the whole thing
    let num_threads = num_cpus::get();
    let mut pool = dataflow::WorkPool::new(num_threads);

    for n in 0..=(15/5) {
        let n = n * 5;

        println!("working on fib({})...", n);

        let mut log = report.entry(n).or_insert("".to_string());
        let line = format!("n,latency,throughput,is_vm\n");
        log.push_str(&line);

        for i in 0..100 {
            println!("working on fib({})... {}", n, i);
            let mut graph_native = dataflow::Graph::construct(n, fib_recurse);
            let ntasks = graph_native.task_count() as f64;

            let start = Instant::now();
            graph_native.evaluate(fib_eval_native, &mut pool);
            let native_time = start.elapsed().as_secs_f64();

            let line = format!("{},{},{},{}\n", n, native_time, ntasks / native_time, false);
            log.push_str(&line);
        }
    }

    pool.join();

    return report;
}

fn fib_eval_native(n: i32, deps: Vec<i32>) -> i32 {
    if n < 2 {
        return n;
    }
    return std::iter::Sum::sum(deps.into_iter());
}

fn fib_recurse(n: i32) -> Option<Vec<i32>> {
    if n < 2 {
        return None;
    }
    Some(vec![n - 1, n - 2])
}
