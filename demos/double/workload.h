#pragma once

#include <wasp/workload.h>

class double_workload : public wasp::workload {
  int val;

public:
  double_workload();
  ~double_workload() override;
  int handle_hcall(wasp::regs_t &regs, size_t ramsize, void *ram) override;
};
