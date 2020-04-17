#include "mobo/workload.h"
#include "mobo/workload_impl.h"

namespace mobo::workload_impl {

double_workload::double_workload() : val(20) { }

double_workload::~double_workload() = default;

int double_workload::handle_hcall(struct mobo::regs_t &regs, size_t ramsize,
                                  void *ram) {
  if (regs.rax == 0) {
//      printf("%s: rax = 0\n", __FUNCTION__);
    regs.rbx = val;
    return WORKLOAD_RES_OKAY;
  }

  if (regs.rax == 1) {
//      printf("%s: rax = 1\n", __FUNCTION__);
    if (regs.rbx != val * 2) {
      throw std::runtime_error("double test failed\n");
    }
    return WORKLOAD_RES_OKAY;
  }

//    printf("%s: rax = %lld (kill)\n", __FUNCTION__, regs.rax);
  return WORKLOAD_RES_KILL;
}

}
