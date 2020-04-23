#pragma once

#include "stdbool.h"
#include "machine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wasp_loader_t wasp_loader_t;

bool WASP_API wasp_loader_inject(wasp_loader_t *self, wasp_machine_t *vm);
void WASP_API wasp_loader_free(wasp_loader_t *self);

typedef wasp_loader_t *(*wasp_loader_create_fn_t)(const char *path);

WASP_API wasp_loader_t *wasp_elf_loader_create(const char *path);
WASP_API wasp_loader_t *wasp_flatbin_loader_create(const char *path);

//

#ifdef __cplusplus
}; // extern "C"
#endif