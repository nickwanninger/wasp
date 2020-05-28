#include "workload.h"
static int boot_runc = 0;

boottime_workload::boottime_workload(void) = default;
boottime_workload::~boottime_workload(void) = default;

int boottime_workload::handle_hcall(wasp::regs_t &regs, size_t ramsize, void *ram) {
  return WORKLOAD_RES_KILL;
}
