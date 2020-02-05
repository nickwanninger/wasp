#pragma once

#ifndef __MOBO_MACHINE_
#define __MOBO_MACHINE_

#include <stdlib.h>
#include <string>
#include "platform/support.h"
#include "vcpu.h"

namespace mobo {

class driver;

class machine {
  char *memory = nullptr;
  size_t mem_size = 0;

  driver &m_driver;

 public:
  typedef std::shared_ptr<machine> ptr;

  explicit machine(driver &);
  ~machine();
  void load_elf(std::string file);

  // give the machine nbytes bytes of ram, rounded up to nearest
  // page boundary (4096 bytes)
  void allocate_ram(size_t);
  void run(void);
  void reset();

  uint32_t num_cpus();
  mobo::vcpu::ptr cpus(uint32_t);
};

// a driver is an interface which acts as a generic boundary API
// to various vm implementations
class driver {
 private:
  machine *m_machine = nullptr;

 public:
  virtual ~driver() = default;
  void set_machine(machine *);
  virtual void attach_memory(size_t, void *) = 0;
  virtual void run() = 0;
  virtual void reset() = 0;

  virtual uint32_t num_cpus() = 0;
  virtual mobo::vcpu::ptr cpu(uint32_t) = 0;
};

enum driver_kind : uint32_t {
  driver_kind_any = 0b11,
  driver_kind_kvm = 0b01,
  driver_kind_hyperv = 0b10,
};

class context {
private:
  int kvmfd = -1;

public:
  context();

  static bool is_supported(driver_kind kind);
  machine::ptr create(driver_kind kind, uint32_t num_cpus);
};

}  // namespace mobo

#endif
