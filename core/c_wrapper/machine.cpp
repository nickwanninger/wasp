#include <wasp/compiler_defs.h>
#include <wasp/machine.h>
#include <wasp/workload.h>

#include <wasp_c/machine.h>
#include "opaque_types.h"

extern "C" {

wasp_machine_t *wasp_machine_create(size_t memsize) {
  wasp_machine_t *self = (wasp_machine_t *) malloc(sizeof(wasp_machine_t));
  auto vm = wasp::machine::create(memsize);
  self->container = new wasp::wrapper::details::machine_container(vm);
  return self;
}

void wasp_machine_free(wasp_machine_t *self) {
  PANIC_IF_NULL(self);
  delete self->container;
  free(self);
}

void wasp_machine_run(wasp_machine_t *self, wasp_workload_t *workload) {
  PANIC_IF_NULL(self);
  PANIC_IF_NULL(workload);

  wasp::wrapper::details::workload_wrapper wrapper(workload);
  self->container->get().run(wrapper);
}

void wasp_machine_reset(wasp_machine_t *self) {
  PANIC_IF_NULL(self);
  self->container->get().reset();
}

}; // extern "C"
