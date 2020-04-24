#pragma once

#ifndef __MOBO_VCPU_
#define __MOBO_VCPU_

#include <functional>
#include <vector>
#include <memory>
#include <stdexcept>

#include "./types.h"
#include "./vcpu_regs.h"

/* eflags masks */
#define CC_C 0x0001u
#define CC_P 0x0004u
#define CC_A 0x0010u
#define CC_Z 0x0040u
#define CC_S 0x0080u
#define CC_O 0x0800u

#define TF_SHIFT 8
#define IOPL_SHIFT 12
#define VM_SHIFT 17

#define TF_MASK 0x00000100u
#define IF_MASK 0x00000200u
#define DF_MASK 0x00000400u
#define IOPL_MASK 0x00003000u
#define NT_MASK 0x00004000u
#define RF_MASK 0x00010000u
#define VM_MASK 0x00020000u
#define AC_MASK 0x00040000u
#define VIF_MASK 0x00080000u
#define VIP_MASK 0x00100000u
#define ID_MASK 0x00200000u

namespace wasp {

using regs_t = ::wasp_regs_t;
using regs_special_t = ::wasp_regs_special_t;
using regs_fpu_t = ::wasp_regs_fpu_t;
using segment_t = ::wasp_segment_t;
using dtable_t = ::wasp_dtable_t;

// the vcpu struct is a general interface to a virtualized x86_64 CPU
class vcpu {
 public:
  typedef std::shared_ptr<vcpu> ptr;

  // GPR
  virtual void read_regs_into(regs_t &) = 0;
  virtual void write_regs(regs_t &) = 0;
  // SPR
  virtual void read_regs_special_into(regs_special_t &) = 0;
  virtual void write_regs_special(regs_special_t &) = 0;
  // FPR
  virtual void read_regs_fpu_into(regs_fpu_t &) = 0;
  virtual void write_regs_fpu(regs_fpu_t &) = 0;
  virtual void dump_state(FILE *);

  // translate a guest virtual address into the host address
  virtual void *translate_address(u64 gva) = 0;

  // used to reset vm state. CPUS must manage this on their own
  virtual void reset() = 0;

  // read a string starting from gva until a nullptr
  std::string read_string(u64 gva);

  int read_guest(u64 gva, void *buf, size_t len);
};

template <typename T>
class guest_array {
 private:
  u64 base = 0;

  // cached information. if the gpn is the same, then the ppn is valid.
  // if not, then we need to find the new mapping
  u64 gpn = -1;
  u64 ppn = -1;
  wasp::vcpu *cpu = nullptr;

 public:
  guest_array(wasp::vcpu *cpu, u64 base) : cpu(cpu), base(base) {}

  T &operator[](size_t off) {
    u64 gva = base + off;

    // need to update the cached ppn so we avoid translating things too often.
    if (gpn != (gva >> 12)) {
      void *found = cpu->translate_address(gva);
      if (found == nullptr)
        throw std::runtime_error(
            "Failed to translate address. Not mapped in guest address space");
      gpn = gva >> 12;
      ppn = (u64)found >> 12;
    }

    // calculate the host virtual address and return a reference to it.
    auto *ppa = reinterpret_cast<T *>((ppn << 12) + (gva & 0xfff));
    return *ppa;
  }
};

};  // namespace wasp

#endif
