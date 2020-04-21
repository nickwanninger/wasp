#include <wasp/compiler_defs.h>
#include <wasp/machine.h>
#include <wasp/workload.h>

#include <wasp_c/machine.h>
#include "opaque_types.h"

extern "C" {

wasp_machine_t *wasp_machine_create(size_t memsize) {
  wasp_machine_t *self = (wasp_machine_t *) malloc(sizeof(wasp_machine_t));
  self->instance = wasp::machine::create(memsize);
  return self;
}

void wasp_machine_free(wasp_machine_t *self) {
  if (self == NULL) {
    PANIC("'self' cannot be null");
  }

  self->instance.reset();
  free(self);
}

class wrapper_workload : public wasp::workload {
private:
  wasp_workload_t *internal_;

public:
  wrapper_workload(wasp_workload_t *internal) : internal_(internal) {}
  ~wrapper_workload() override { }
  int handle_hcall(wasp::regs_t &regs, size_t ramsize, void *ram) override {
    auto f = internal_->handle_hcall;
    if (f == nullptr) { PANIC("wasp_workload_t must have a non-null `handle_hcall` handler"); }
    return f(&regs, ramsize, ram);
  }

  void handle_exit() override {
    auto f = internal_->handle_exit;
    if (f == nullptr) { PANIC("wasp_workload_t must have a non-null `handle_exit` handler"); }
    f();
  }
};

void wasp_machine_run(wasp_machine_t *self, wasp_workload_t *workload) {
  PANIC_IF_NULL(self);
  PANIC_IF_NULL(workload);

  wrapper_workload wrapper(workload);
  self->instance->run(wrapper);
}

void wasp_machine_reset(wasp_machine_t *self) {
  PANIC_IF_NULL(self);

  self->instance.reset();
}

}; // extern "C"
