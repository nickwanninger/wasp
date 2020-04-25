#pragma once

#ifndef __MOBO_MACHINE_
#define __MOBO_MACHINE_

#include <stdlib.h>

#include <string>

#include "./vcpu.h"
#include "./workload.h"

namespace wasp {

namespace memory {

struct page_entry_t {
  union {
    struct {
      uint64_t present : 1;
      uint64_t write : 1;	     // If 0, writes not allowed.
      uint64_t allow_user_mode : 1;  // If 0, user-mode accesses not allowed.
      uint64_t write_through : 1;
      uint64_t cache_disable : 1;
      uint64_t accessed : 1;
      uint64_t dirty : 1;
      uint64_t large_page : 1;	// Must be 0 for PML4E.
      uint64_t available : 4;
      uint64_t page_frame_num : 36;
      uint64_t reserved_hardware : 4;
      uint64_t reserved_software : 11;
      uint64_t no_execute : 1;	// If 1, instruction fetches not allowed.
    };
    uint64_t Value;
  };
};
static_assert(sizeof(struct page_entry_t) == sizeof(uint64_t),
	      "expected 64-bit page entry");

}  // namespace memory

class driver;

class machine {
  uint64_t entry_ = (uint64_t)-1;

 public:
  typedef std::unique_ptr<machine> unique_ptr;
  typedef std::shared_ptr<machine> shared_ptr;

  machine() = default;
  virtual ~machine();
  static machine::unique_ptr create(size_t memsize);

  template <typename T>
  inline auto val_at(off_t gpa) -> T & {
    return *(T *)gpa2hpa(gpa);
  }

  /* Each platform must implement these methods */
  virtual void allocate_ram(size_t) = 0;
  virtual void run(workload &) = 0;
  virtual void *gpa2hpa(off_t gpa) = 0;
  virtual void reset() = 0;

  virtual uint32_t num_cpus() = 0;
  virtual wasp::vcpu &cpu(uint32_t) = 0;
  inline uint64_t entry() { return entry_; }
  inline void set_entry(uint64_t entry) { entry_ = entry; }
};

namespace platform {
// implemented per-platform
machine::unique_ptr create();
}  // namespace platform

}  // namespace wasp

#endif
