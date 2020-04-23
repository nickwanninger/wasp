#pragma once

#include <wasp_c/workload_defs.h>
#include "./vcpu.h"

namespace wasp {

class WASP_API workload {
  public:

    workload() = default;
    virtual int handle_hcall(
            wasp::regs_t &regs,
            size_t ramsize,
            void *ram) = 0;
    virtual void handle_exit() {};
    virtual ~workload() = default;
};

}