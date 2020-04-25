#include <wasp/loader.h>

#include <wasp/runner.hpp>

class double_workload : public wasp::workload {
  int val = 20;

 public:
  double_workload() {}
  ~double_workload() = default;
  int handle_hcall(wasp::regs_t &regs, size_t ramsize, void *ram) override {
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
  };
};

int main() {
  wasp::run<double_workload, wasp::loader::flatbin_loader>(
      "build/tests/double64.bin");
  exit(0);
}
