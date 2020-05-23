use serde::{Serialize, Deserialize};
use crossbeam::channel;
use bincode;
use core_affinity;
use std::sync::{Arc, Mutex};



type TaskID = i32;
type Evaluator<T> = fn(T, Vec<T>) -> T;
type Recursor<T> = fn(T) -> Option<Vec<T>>;

enum GraphCommand<T: Sync + Send> {
    // Evaluate the task with id TaskID with the state T and args Vec<T>
    Evaluate(TaskID, TaskID /* feeds */, T, Vec<T>, Evaluator<T>),
    Die,
}



pub struct Graph<T: Sync + Send> {
    tasks: Arc<Vec<Mutex<Node<T>>>>,
}


impl<'de, T: 'static + Copy + Sync + Send + Serialize + Deserialize<'de>> Graph<T> {

    pub fn oneshot(
        initial_state: T,
        recur: Recursor<T>,
        eval: Evaluator<T>,
        pool: &mut WorkPool
    ) -> T {
        let mut g = Graph::construct(initial_state, recur);
        return g.evaluate(eval, pool);
    }

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
            node.values.reserve(deps.len());
            self.push_task(node);
            for st in deps {
                self.add_task(st, id, recur);
            }
        } else {
            self.push_task(node);
        }
    }


    pub fn evaluate(&mut self, eval: Evaluator<T>, pool: &mut WorkPool) -> T {

        let (response_send, response_recv) = channel::unbounded();
        let (commands_send, commands_recv) = channel::unbounded();

        // spawn all the threads to do work
        for core_id in 0..pool.size() {
            let commands_recv = commands_recv.clone();
            let commands_send = commands_send.clone();
            let response_send = response_send.clone();
            let tasks = self.tasks.clone();

            pool.exec(move || {
                // set the core affinity of this thread
                core_affinity::set_for_current(core_affinity::CoreId { id: core_id as usize });

                for cmd in commands_recv.into_iter() {
                    match cmd {
                        GraphCommand::Evaluate(_id, feeds, state, vals, eval) => {
                            let res = eval(state, vals);

                            if feeds == -1 {
                                response_send.send(res).unwrap();
                                return;
                            }

                            let mut dependant = tasks[feeds as usize].lock().unwrap();
                            dependant.values.push(res);
                            dependant.waiting_on -= 1;
                            if dependant.waiting_on == 0 {
                                let cmd = GraphCommand::Evaluate(dependant.id, dependant.feeds, dependant.state, dependant.values.clone(), eval);
                                commands_send.send(cmd).unwrap();
                            }
                        },
                        GraphCommand::Die => {
                            break;
                        }
                    }
                }
            });
        }

        // push the initial state
        for t in self.tasks.iter() {
            let t = t.lock().unwrap();
            if t.waiting_on == 0 {
                let cmd = GraphCommand::Evaluate(t.id, t.feeds, t.state, vec![], eval);
                commands_send.send(cmd).unwrap();
            }
        }


        // and wait on the result from the threads :)
        let result = response_recv.recv().unwrap();

        // kill all the threads and join them
        for _core_id in 0..pool.size() {
            commands_send.send(GraphCommand::Die).unwrap();
        }

        return result;
    }



    pub fn load(data: &'de Vec<u8>) -> Self {
        let mut g = Self { tasks: Arc::new(vec![]) };
        let temp: Vec<Node<T>> = bincode::deserialize(&data).unwrap();

        let mut v = vec![];
        for t in temp {
            v.push(Mutex::new(t));
        }
        g.tasks = Arc::new(v);
        return g;
    }


    // serialize the graph
    pub fn serialize(mut self) -> Vec<u8> {
        let temp: Vec<Node<T>> = Arc::get_mut(&mut self.tasks).unwrap().into_iter().map(|v| {
            v.get_mut().unwrap().clone()
        }).collect();

        bincode::serialize(&temp).unwrap()
    }
}

#[derive(Serialize, Deserialize, Debug, Clone)]
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




use threadpool;


pub struct WorkPool {
    size: usize,
    pool: threadpool::ThreadPool
}

impl WorkPool {
    pub fn new(nthreads: usize) -> Self {
        Self {
            size: nthreads,
            pool: threadpool::ThreadPool::new(nthreads)
        }
    }

    pub fn size(&self) -> usize {
        self.size
    }


    pub fn exec<F>(&self, job: F)
        where F: FnOnce() + Send + 'static {
        self.pool.execute(job);
    }

    pub fn join(&self) {
        self.pool.join();
    }
}
