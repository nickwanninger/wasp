// #include <mobo/jit.h>
#include <mobo/kvm.h>

#include <capstone/capstone.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <chrono>
#include <iostream>



#include <fcntl.h>


#include <stdint.h>

static inline uint64_t __attribute__((always_inline)) rdtsc(void) {
  uint32_t lo, hi;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return lo | ((uint64_t)(hi) << 32);
}

using namespace mobo;

int run_vm(std::string path) {
  static int kvmfd = open("/dev/kvm", O_RDWR);

  // int i = 0;

  try {
    // create a vmm
    kvm vmm(kvmfd, 1);


      // give it some ram
      vmm.init_ram(16 * 1024l * 1024l);
    for (int i = 0; i < 1000; i++) {
      auto start = rdtsc();

      // load the kernel elf
      vmm.load_elf(path);

      // run the vm
      vmm.run();

      // record the time it took
      auto dur = rdtsc() - start;
      printf("%d,%ld\n", i, dur);

      // reset the VM
      vmm.reset();
    }
  } catch (std::exception &ex) {
    fprintf(stderr, "ex: %s\n", ex.what());
    return -1;
  }

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
