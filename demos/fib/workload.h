#pragma once

#include <mobo/workload.h>

class fib_workload : public mobo::workload {
public:
  fib_workload();
  ~fib_workload() override;
  int handle_hcall(struct mobo::regs_t &regs, size_t ramsize, void *ram) override;
};

