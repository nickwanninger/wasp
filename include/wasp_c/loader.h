#pragma once

#include "stdbool.h"
#include "machine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wasp_loader_t wasp_loader_t;

bool wasp_loader_inject(wasp_loader_t *self, wasp_machine_t *vm);
void wasp_loader_free(wasp_loader_t *self);

typedef wasp_loader_t *(*wasp_loader_create_fn_t)(const char *path);

wasp_loader_t *wasp_elf_loader_create(const char *path);
wasp_loader_t *wasp_flatbin_loader_create(const char *path);

// just load some binary code from ram
void wasp_inject_code(wasp_machine_t *vmc, void *code, u64 size, u64 entry);

#ifdef __cplusplus
}; // extern "C"
#endif
