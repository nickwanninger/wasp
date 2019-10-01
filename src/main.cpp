// #include <mobo/jit.h>
#include <mobo/kvm.h>

#include <capstone/capstone.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

static inline uint64_t __attribute__((always_inline)) rdtsc(void) {
  uint32_t lo, hi;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return lo | ((uint64_t)(hi) << 32);
}

using namespace mobo;

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
  //
  kvm *v = new kvm(kvmfd, path, 1);
  v->init_ram(memsize);
  v->reset();
  return v;
}

int run_vm(std::string path) {
  auto t = std::thread([]() {
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
  });

  if (kvmfd == 0) kvmfd = open("/dev/kvm", O_RDWR);

  size_t ramsize = 16 * 1024l * 1024l;

  {
    kvm *init_vms[25];
    for (int i = 0; i < 25; i++) {
      init_vms[i] = get_clean(path, ramsize);
    }


    for (int i = 0; i < 25; i++) {
      clean.push(init_vms[i]);
    }
  }

  try {
    for (int i = 0; i < 1000; i++) {
      auto v = get_clean(path, ramsize);
      auto start = rdtsc();

      // run the vm
      v->run();

      // record the time it took
      auto dur = rdtsc() - start;
      printf("%d,%ld\n", i, dur);
      add_dirty(v);
    }

  } catch (std::exception &ex) {
    fprintf(stderr, "ex: %s\n", ex.what());
    return -1;
  }

  t.detach();

  return 0;
}

int main(int argc, char **argv) {
  if (argc == 1) return -1;

  run_vm(argv[1]);
}
/*
// haskell FFI function
extern "C" int mobo_run_vm(char *binary) { return run_vm(binary); }

*/
