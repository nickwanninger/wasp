#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "workload.h"

typedef struct wasp_machine_t wasp_machine_t;

WASP_API wasp_machine_t *wasp_machine_create(size_t memsize);
WASP_API void wasp_machine_free(wasp_machine_t *self);

WASP_API void wasp_machine_run(wasp_machine_t *self, wasp_workload_t *workload);
WASP_API void wasp_machine_reset(wasp_machine_t *self);

#ifdef __cplusplus
}; // extern "C"
#endif