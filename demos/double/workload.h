#pragma once

#include <mobo/workload.h>

class double_workload : public mobo::workload {
  int val;

public:
  double_workload();
  ~double_workload() override;
  int handle_hcall(struct mobo::regs_t &regs, size_t ramsize, void *ram) override;
};
