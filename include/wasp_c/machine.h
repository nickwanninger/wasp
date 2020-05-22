#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "workload.h"

typedef struct wasp_machine_t wasp_machine_t;

wasp_machine_t *wasp_machine_create(size_t memsize);
void wasp_machine_free(wasp_machine_t *self, size_t memsize);

void wasp_machine_run(wasp_machine_t *self, wasp_workload_t *workload);
void wasp_machine_reset(wasp_machine_t *self);

#ifdef __cplusplus
}; // extern "C"
#endif
