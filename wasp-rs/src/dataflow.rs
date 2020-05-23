use serde::{Serialize, Deserialize};
use crossbeam::channel;
// use crossbeam::scope;
use core_affinity;
use std::sync::{Arc, Mutex};


type TaskID = i32;
type Evaluator<T> = fn(T, Vec<T>) -> T;
type Recursor<T> = fn(T) -> Option<Vec<T>>;

enum GraphCommand<T: Sync + Send> {
    // Evaluate the task with id TaskID with the state T and args Vec<T>
    Evaluate(TaskID, T, Vec<T>, Evaluator<T>),
}

enum GraphResponse<T: Sync + Send> {
    // task with id TaskID evalutated to a final value of type T
    Evaluated(TaskID, T)
}

pub struct Graph<T: Sync + Send> {
    tasks: Arc<Vec<Mutex<Node<T>>>>,
}


impl<T: 'static + Copy + Sync + Send> Graph<T> {

    /// Construct a new graph with some initial state and functions to expand it
    pub fn construct(
        initial_state: T,
        recur: Recursor<T>
    ) -> Self {
        // construct the graph
        let mut g = Self { tasks: Arc::new(vec![]) };
        g.add_task(initial_state, -1, recur);


        return g;
    }

    fn push_task(&mut self, node: Node<T>) {
        Arc::get_mut(&mut self.tasks).unwrap().push(Mutex::new(node));
    }

    /// Add a task to the graph
    fn add_task(
        &mut self,
        initial_state: T,
        feeds: TaskID,
        recur: Recursor<T>,
    ) {
        let id = self.tasks.len() as i32;
        let mut node = Node::new(id, initial_state, feeds);
        let deps = recur(node.state);


        if let Some(deps) = deps {
            node.waiting_on = deps.len() as i32;
            self.push_task(node);
            for st in deps {
                self.add_task(st, id, recur);
            }
        } else {
            self.push_task(node);
        }
    }


    pub fn evaluate(&mut self, eval: Evaluator<T>, mut parallelism: usize) -> T {
        if parallelism < 1 {
            parallelism = 1;
        }

        let cpus = num_cpus::get();

        let (commands_send, commands_recv) = channel::unbounded();
        let (response_send, response_recv) = channel::unbounded();


        // spawn all the threads to do work
        let mut threads = Vec::with_capacity(parallelism);
        for core_id in 0..parallelism {
            let commands_recv = commands_recv.clone();
            let commands_send = commands_send.clone();
            let response_send = response_send.clone();
            let tasks = self.tasks.clone();

            threads.push(std::thread::spawn(move || {
                // set the core affinity of this thread
                core_affinity::set_for_current(core_affinity::CoreId { id: (core_id % cpus) as usize });

                for cmd in commands_recv.into_iter() {
                    match cmd {
                        GraphCommand::Evaluate(id, state, vals, eval) => {
                            let res = eval(state, vals);


                            // Handle the fact that the task has been evaluated
                            let feeds = tasks[id as usize].lock().unwrap().feeds;


                            if feeds == -1 {
                                response_send.send(res).unwrap();
                                return;
                            }

                            let mut dependant = tasks[feeds as usize].lock().unwrap();

                            dependant.values.push(res);
                            dependant.waiting_on -= 1;
                            if dependant.waiting_on == 0 {
                                let cmd = GraphCommand::Evaluate(dependant.id, dependant.state, dependant.values.clone(), eval);
                                commands_send.send(cmd).unwrap();
                            }
                        },
                    }
                }
            }));
        }

        // push the initial state
        for t in self.tasks.iter() {
            let t = t.lock().unwrap();
            if t.waiting_on == 0 {
                let cmd = GraphCommand::Evaluate(t.id, t.state, vec![], eval);
                commands_send.send(cmd).unwrap();
            }
        }

        // and wait on the result from the threads :)
        return response_recv.recv().unwrap();


        /*
        */

        /*
        for event in response_recv {
            match event {
                GraphResponse::Evaluated(id, res) => {
                    // Handle the fact that the task has been evaluated
                    let feeds = self.tasks[id as usize].feeds;

                    // Is this node the root node?
                    if feeds == -1 {
                        return Some(res);
                    }

                    let dependant = &mut self.tasks[feeds as usize];
                    dependant.values.push(res);
                    dependant.waiting_on -= 1;
                    if dependant.waiting_on == 0 {
                        let cmd = GraphCommand::Evaluate(dependant.id, dependant.state, dependant.values.clone(), eval);
                        commands_send.send(cmd).unwrap();
                    }
                }
            }
        }
        */


    }
}

#[derive(Serialize, Deserialize, Debug)]
pub struct Node<T: Sync + Send> {
    pub id: TaskID,
    pub state: T,
    pub waiting_on: i32, // how many nodes are we waiting on now?

    // feeds: Which task to notify of partial completion
    // If this value is -1 it feeds the "root's parent" which is the graph's mpsc
    pub feeds: TaskID,
    pub values: Vec<T>,
}


impl<T: Sync + Send> Node<T> {
    pub fn new(
        id: TaskID,
        state: T,
        feeds: TaskID,
    ) -> Self {
        Self {
            id, state,
            waiting_on: 0, // waits on nothing. Graph::add_task sets this
            feeds, values: vec![],
        }
    }
}
