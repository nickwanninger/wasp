#include <malloc.h>
#include <wasp_c/workload.h>
#include <wasp_c/panic.h>
#include <stdlib.h>
#include "workload.h"

struct double_workload_ctx_t {
  int val;
};

void double_workload_init(wasp_workload_t *self, void *config) {
  struct double_workload_ctx_t *ctx = (struct double_workload_ctx_t *) malloc(sizeof(struct double_workload_ctx_t));
  ctx->val = 20;
  self->ctx = ctx;
}

int double_workload_handle_hcall(
    wasp_workload_t *self,
    wasp_regs_t *regs,
    size_t ramsize,
    void *ram)
{
  struct double_workload_ctx_t *ctx = (struct double_workload_ctx_t *) self->ctx;
  int val = ctx->val;

  if (regs->rax == 0) {
    regs->rbx = val;
    return WORKLOAD_RES_OKAY;
  }

  if (regs->rax == 1) {
    if (regs->rbx != val * 2) {
      PANIC("double test failed. got wrong result.");
    }
    return WORKLOAD_RES_OKAY;
  }

  return WORKLOAD_RES_KILL;
}

void double_workload_handle_exit(wasp_workload_t *self) {
  free(self->ctx);
}

wasp_workload_t g_double_workload = {
    .init = double_workload_init,
    .handle_hcall = double_workload_handle_hcall,
    .handle_exit = double_workload_handle_exit,
};
