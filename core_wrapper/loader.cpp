#include <wasp/compiler_defs.h>
#include <wasp/loader.h>

#include <wasp_c/loader.h>
#include <wasp_c/machine.h>

#include "./opaque_types.h"

namespace wasp::wrapper::details {

  template<class L>
  inline wasp_loader_t *create_loader(const char *path) {
    PANIC_IF_NULL(path);
    wasp_loader_t *self = (wasp_loader_t *) malloc(sizeof(wasp_loader_t));
    self->instance = new L(path);
    return self;
  }

}

extern "C" {

using namespace wasp::wrapper;

bool wasp_loader_inject(wasp_loader_t *self, wasp_machine_t *vm) {
  PANIC_IF_NULL(self);
  PANIC_IF_NULL(vm);

  return self->instance->inject(vm->container->get());
}

void wasp_loader_free(wasp_loader_t *self) {
  PANIC_IF_NULL(self);
  delete self->instance;
  free(self);
}

wasp_loader_t *wasp_elf_loader_create(const char *path) {
  return details::create_loader<wasp::loader::elf_loader>(path);
}

wasp_loader_t *wasp_flatbin_loader_create(const char *path) {
  return details::create_loader<wasp::loader::flatbin_loader>(path);
}


} // extern "C"