#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wasp_regs_t wasp_regs_t;

typedef void (*wasp_workload_init_fn_t)();
typedef int (*wasp_workload_handle_hcall_fn_t)(wasp_regs_t *regs, size_t ramsize, void *ram);
typedef void (*wasp_workload_handle_exit_fn_t)();

typedef struct wasp_workload_t {
  wasp_workload_init_fn_t init;
  wasp_workload_handle_hcall_fn_t handle_hcall;
  wasp_workload_handle_exit_fn_t handle_exit;
} wasp_workload_t;

#ifdef __cplusplus
}; // extern "C"
#endif