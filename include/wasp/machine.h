#pragma once

#ifndef __MOBO_MACHINE_
#define __MOBO_MACHINE_

#include <stdlib.h>
#include <string>

#include "./workload.h"
#include "./vcpu.h"

namespace wasp {

namespace memory {

struct page_entry_t
{
  union
  {
    struct
    {
      uint64_t present : 1;
      uint64_t write : 1; // If 0, writes not allowed.
      uint64_t allow_user_mode : 1; // If 0, user-mode accesses not allowed.
      uint64_t write_through : 1;
      uint64_t cache_disable : 1;
      uint64_t accessed : 1;
      uint64_t dirty : 1;
      uint64_t large_page : 1; // Must be 0 for PML4E.
      uint64_t available : 4;
      uint64_t page_frame_num : 36;
      uint64_t reserved_hardware : 4;
      uint64_t reserved_software : 11;
      uint64_t no_execute : 1; // If 1, instruction fetches not allowed.
    };
    uint64_t Value;
  };
};
static_assert(sizeof(struct page_entry_t) == sizeof(uint64_t), "expected 64-bit page entry");

} // namespace memory


class driver;

class machine {
  uint64_t entry_ = (uint64_t) -1;

public:
  typedef std::unique_ptr<machine> unique_ptr;
  typedef std::shared_ptr<machine> shared_ptr;

  machine() = default;
  virtual ~machine();
  static machine::unique_ptr create(size_t memsize);

  template <typename T>
  inline auto val_at(off_t gpa) -> T& {
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


#define PLATFORM_ANY (-1)
#define PLATFORM_LINUX (0b001)
#define PLATFORM_WINDOWS (0b010)
#define PLATFORM_DARWIN (0b100)

namespace platform {

machine::unique_ptr create(int platform);

struct registration {
  const char *name;
  uint32_t flags;
  machine::unique_ptr (*allocate)(void);
};

}  // namespace platform

}  // namespace wasp

#ifdef _MSC_VER

#define IGNORE_UNUSED __pragma(warning(suppress:4100))

// see https://stackoverflow.com/a/14783759/809572
// for additional explination

/*
 * Declare `vm_platforms` section for MSVC linker.
 * We need to declare the start and end using `$a` and `$z` which will be merged
 * into the one section itself before the dollar sign.
 */
#pragma section("vm_platforms$a", read)
#pragma section("vm_platforms$u", read)
#pragma section("vm_platforms$z", read)

/*
 * Force the linker to include the definition
 * see also https://stackoverflow.com/a/2993476/809572
 */
#ifdef _WIN64
  #define FORCE_EXPORT(x) __pragma(comment (linker, "/export:" #x))
#else
  #define FORCE_EXPORT(x) __pragma(comment (linker, "/export:_" #x))
#endif

#define __register_platform(NAME) \
   FORCE_EXPORT(NAME) extern "C" \
   __declspec(allocate("vm_platforms$u")) \
   IGNORE_UNUSED __declspec(align(sizeof(void *)))

#else

#define IGNORE_UNUSED __attribute__((__used__))
#define FORCE_EXPORT __attribute__((externally_visible))

extern wasp::platform::registration __start_vm_platforms[];
extern wasp::platform::registration __stop_vm_platforms[];

#define __register_platform \
  IGNORE_UNUSED FORCE_EXPORT __attribute__( \
      (unused, __section__("vm_platforms"), aligned(sizeof(void *))))

#endif

#endif