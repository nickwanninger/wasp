#pragma once

#ifndef __MOBO_MACHINE_
#define __MOBO_MACHINE_

#include <mobo/workload.h>
#include <stdlib.h>

#include <string>

#include "platform/support.h"
#include "vcpu.h"

namespace mobo {

class driver;

class machine {

  std::vector<std::unique_ptr<mobo::vcpu>> m_cpus;

 public:
  typedef std::shared_ptr<machine> ptr;

  explicit machine(void) {};
  virtual ~machine();
  void load_elf(std::string file);

  // give the machine nbytes bytes of ram, rounded up to nearest
  // page boundary (4096 bytes)
  virtual void allocate_ram(size_t) = 0;
  virtual void run(workload &) = 0;
  virtual void reset() = 0;

  virtual void *gpa2hpa(off_t gpa) = 0;

  template <typename T>
  inline auto val_at(off_t gpa) -> T& {
    return *(T *)gpa2hpa(gpa);
  }


  uint32_t num_cpus();
  mobo::vcpu &cpu(uint32_t);
};


#define PLATFORM_ANY (-1)
#define PLATFORM_LINUX (0b001)
#define PLATFORM_WINDOWS (0b010)
#define PLATFORM_DARWIN (0b100)

namespace platform {
machine::ptr create(int platform);

struct registration {
  const char *name;
  int flags;
  machine::ptr (*allocate)(void);
};
};  // namespace platform

#define __register_platform                \
  __attribute__((__used__)) __attribute__( \
      (unused, __section__("vm_platforms"), aligned(sizeof(void *))))

}  // namespace mobo

#endif
