#include "workload.h"

fib_workload::fib_workload() = default;
fib_workload::~fib_workload() = default;

int fib_workload::handle_hcall(wasp::regs_t &regs, size_t ramsize,
                               void *ram) {
  printf("rax=%ld\n", regs.rax);
  return WORKLOAD_RES_KILL;
}
