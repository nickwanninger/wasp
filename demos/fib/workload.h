#pragma once

#include <wasp/workload.h>

class fib_workload : public wasp::workload {
public:
  fib_workload();
  ~fib_workload() override;
  int handle_hcall(struct wasp::regs_t &regs, size_t ramsize, void *ram) override;
};

