#pragma once

#include "./vcpu.h"

// results for the workloads
//
// kill the vm. Bad hcall
#define WORKLOAD_RES_KILL -1
// OKAY means the hcall was good and flush the regs
#define WORKLOAD_RES_OKAY 0

namespace wasp {

class workload {
  public:

    workload() = default;
    virtual int handle_hcall(
            struct wasp::regs_t &regs,
            size_t ramsize,
            void *ram) = 0;
    virtual void handle_exit() {};
    virtual ~workload() = default;
};

}