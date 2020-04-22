#pragma once

// results for the workloads
//
// kill the vm. Bad hcall
#define WORKLOAD_RES_KILL -1
// OKAY means the hcall was good and flush the regs
#define WORKLOAD_RES_OKAY 0
