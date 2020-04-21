#pragma once

#include <mobo/workload.h>

class boottime_workload : public mobo::workload {
public:
  boottime_workload();
  ~boottime_workload() override;
  int handle_hcall(struct mobo::regs_t &regs, size_t ramsize, void *ram) override;
};
