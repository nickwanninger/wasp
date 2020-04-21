#pragma once

#include <wasp/machine.h>
#include <wasp/loader.h>

#define PANIC_IF_NULL(_VAR) \
  do { \
    if (_VAR == NULL) { PANIC("`%s` cannot be null", "_VAR"); } \
  } while (0)

extern "C" {

typedef struct wasp_machine_t {
  wasp::machine::ptr instance;
} wasp_machine_t;


typedef struct wasp_loader_t {
  wasp::loader::binary_loader *instance;
} wasp_loader_t;


} // extern "C"