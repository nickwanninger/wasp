#include "mobo/workload.h"
#include "mobo/workload_impl.h"

namespace mobo::workload_impl {

static int boot_runc = 0;

boottime_workload::boottime_workload(void) {
  if (boot_runc == 0) {
    printf("Platform, cli, lgdt, prot, in 32, id map, long, in 64\n");
  }
  boot_runc++;
}

boottime_workload::~boottime_workload(void) { printf("\n"); }

int boottime_workload::handle_hcall(struct mobo::regs_t &regs, size_t ramsize,
                                    void *ram) {
  if (regs.rax == 1) {
    auto *tsc = (uint64_t *) ram;

#ifdef __linux__
    printf("KVM Noserial, ");
#endif

#ifdef _WIN32
    printf("Hyper-V, ");
#endif

    uint64_t baseline = tsc[0];
    // uint64_t overhead = tsc[1] - baseline;

    for (int i = 2; tsc[i] != 0; i++) {
      auto prev = tsc[i - 1];
      auto curr = tsc[i];

      printf("%4ld", curr - prev);
      if (tsc[i + 1] != 0) printf(",");
    }
    /*
    for (int i = 2; tsc[i] != 0; i++) {
      printf("%4ld", tsc[i] - baseline);
      if (tsc[i + 1] != 0) printf(",");
    }
    */
    return WORKLOAD_RES_KILL;
  }

  return WORKLOAD_RES_OKAY;
}

}
