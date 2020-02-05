#pragma once

#include "vcpu.h"

// results for the workloads
//
// kill the vm. Bad hcall
#define WORKLOAD_RES_KILL -1
// OKAY means the hcall was good and flush the regs
#define WORKLOAD_RES_OKAY 0

class workload {
  public:
    virtual int handle_hcall(
            struct mobo::regs &regs,
            size_t ramsize,
            void *ram) = 0;
};
