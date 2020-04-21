#pragma once

#include "stdbool.h"
#include "machine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wasp_loader_t wasp_loader_t;

bool wasp_loader_inject(wasp_loader_t *self, wasp_machine_t *vm);
void wasp_loader_free(wasp_loader_t *self);

wasp_loader_t *wasp_elf_loader_create(const char *path);
wasp_loader_t *wasp_flatbin_loader_create(const char *path);

//

#ifdef __cplusplus
}; // extern "C"
#endif