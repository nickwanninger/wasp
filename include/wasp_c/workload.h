#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "vcpu_regs.h"
#include "workload_defs.h"

struct wasp_workload_t;

typedef void (*wasp_workload_init_fn)(struct wasp_workload_t *self, void *config);
typedef int (*wasp_workload_handle_hcall_fn)(struct wasp_workload_t *self, struct wasp_regs_t *regs, size_t ramsize, void *ram);
typedef void (*wasp_workload_handle_exit_fn)(struct wasp_workload_t *self);

typedef struct wasp_workload_t {
  void *ctx;
  wasp_workload_init_fn init;
  wasp_workload_handle_hcall_fn handle_hcall;
  wasp_workload_handle_exit_fn handle_exit;
} wasp_workload_t;

#ifdef __cplusplus
}; // extern "C"
#endif