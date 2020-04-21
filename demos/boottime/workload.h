#pragma once

#include <wasp/workload.h>

class boottime_workload : public wasp::workload {
public:
  boottime_workload();
  ~boottime_workload() override;
  int handle_hcall(struct wasp::regs_t &regs, size_t ramsize, void *ram) override;
};
