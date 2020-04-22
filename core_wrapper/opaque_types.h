#pragma once

#include <wasp/machine.h>
#include <wasp/loader.h>

namespace wasp::wrapper::details {

  class machine_container {
  private:
    wasp::machine::unique_ptr instance_;

  public:
    explicit machine_container(machine::unique_ptr &instance)
      : instance_(std::move(instance)) {}

    wasp::machine &get() { return *instance_; }
  };

  class workload_wrapper : public wasp::workload {
  private:
    wasp_workload_t *internal_;

  public:
    explicit workload_wrapper(wasp_workload_t *internal)
        : internal_(internal) {}

    ~workload_wrapper() override = default;

    int handle_hcall(wasp::regs_t &regs, size_t ramsize, void *ram) override {
      auto f = internal_->handle_hcall;
      if (f == nullptr) { PANIC("wasp_workload_t must have a non-null `handle_hcall` handler"); }
      return f(internal_, &regs, ramsize, ram);
    }

    void handle_exit() override {
      auto f = internal_->handle_exit;
      if (f == nullptr) { PANIC("wasp_workload_t must have a non-null `handle_exit` handler"); }
      f(internal_);
    }
  };

}

extern "C" {

typedef struct wasp_machine_t {
  wasp::wrapper::details::machine_container *container;
} wasp_machine_t;


typedef struct wasp_loader_t {
  wasp::loader::binary_loader *instance;
} wasp_loader_t;


} // extern "C"