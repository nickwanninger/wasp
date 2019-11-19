// #include <mobo/jit.h>
#include <capstone/capstone.h>
#include <fcntl.h>
#include <mobo/kvm.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <sched.h>
#include <sys/sysinfo.h>



#define RAMSIZE (16 * 1024l * 1024l)

static inline uint64_t __attribute__((always_inline)) rdtsc(void) {
  uint32_t lo, hi;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return lo | ((uint64_t)(hi) << 32);
}

using namespace mobo;

std::atomic<int> nruns = 0;

std::mutex dirty_lock;
std::mutex clean_lock;
std::queue<kvm *> clean;
std::queue<kvm *> dirty;
std::condition_variable dirty_signal;

static void add_dirty(kvm *v) {
  dirty_lock.lock();
  dirty.push(v);
  dirty_lock.unlock();
  dirty_signal.notify_one();
}

int kvmfd = 0;

static kvm *get_clean(std::string &path, size_t memsize) {
  {
    std::scoped_lock lck(clean_lock);
    if (!clean.empty()) {
      auto v = clean.front();
      clean.pop();
      return v;
    }
  }
  // allocate a new one
  kvm *v = new kvm(kvmfd, path, 1);
  v->init_ram(memsize);
  v->reset();
  return v;
}




auto cleaner(void) {
  while (1) {
    std::unique_lock<std::mutex> lk(dirty_lock);

    dirty_signal.wait(lk);
    // grab something to clean
    //
    kvm *v = nullptr;

    if (!dirty.empty()) {
      v = dirty.front();
      dirty.pop();
    }
    lk.unlock();
    if (v != nullptr) {
      v->reset();
      // printf("RESET!\n");
      clean_lock.lock();
      clean.push(v);
      clean_lock.unlock();
    }
  }
}



void print_throughput(float sleep_time = 0.25) {
  float secs = 0;


#define US_IN_S (1000 * 1000)


	long sleep_us = sleep_time * US_IN_S;


	long last_ran = nruns.load();
  while (1) {
		usleep(sleep_us);
    secs += sleep_time;

    auto ran = nruns.load(); // atomic load of global variable
    printf("%ld per second\n", (long)((ran - last_ran) / sleep_time));
		last_ran = ran;
  }
}



auto runner_1(int i, std::string path) {
  while (1) {
    auto v = get_clean(path, RAMSIZE);

    // run the vm
    v->run();

    nruns++;
    // printf("thd=%d  run=%-8d  cycles=%-8ld  us=%-6zu\n", i, nruns++, cycles,
    // microseconds);

    add_dirty(v);
  }
  return 0;
}




int test_throughput_1(std::string path, int nrunners, int ncleaners) {
  std::vector<std::thread> cleaners;
  for (int i = 0; i < ncleaners; i++) cleaners.emplace_back(cleaner);


  for (int i = 0; i < 25; i++) {
    clean.push(get_clean(path, RAMSIZE));
  }


  std::vector<std::thread> runners;
  for (int i = 0; i < nrunners; i++)
    runners.emplace_back(runner_1, i, path);

  print_throughput();


  // join all the threads
  for (auto &t : runners) t.join();
  for (auto &t : cleaners) t.join();

  return 0;
}




void set_affinity(int cpu) {
  cpu_set_t  mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu, &mask);
  sched_setaffinity(0, sizeof(mask), &mask);
}

void runner_2(kvm *vm, int id) {

  set_affinity(id);
  while (1) {
    // run the vm
    vm->run();

    nruns++;

    vm->reset();

  }
}

int test_throughput_2(std::string path, int nrunners) {

  size_t ramsize = 16 * 1024l * 1024l;
  std::vector<std::thread> runners;

  for (int i = 0; i < nrunners; i++) {
    runners.emplace_back(runner_2, get_clean(path, ramsize), i);
  }

  set_affinity(nrunners+1);
  print_throughput(0.25);

  for (auto &t : runners) t.join();
  return -1;
}

int main(int argc, char **argv) {
  if (argc == 1) return -1;
  if (kvmfd == 0) kvmfd = open("/dev/kvm", O_RDWR);

  int nprocs = get_nprocs();
  printf("nprocs=%d\n", nprocs);

  // test_throughput_1(argv[1], 1, 8);
  test_throughput_2(argv[1], nprocs);

}
/*
// haskell FFI function
extern "C" int mobo_run_vm(char *binary) { return run_vm(binary); }

*/
