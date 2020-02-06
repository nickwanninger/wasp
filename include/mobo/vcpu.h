#pragma once

#ifndef __MOBO_VCPU_
#define __MOBO_VCPU_

#include <functional>
#include <vector>
#include <memory>
#include <stdexcept>

#include <mobo/types.h>


/* eflags masks */
#define CC_C 0x0001
#define CC_P 0x0004
#define CC_A 0x0010
#define CC_Z 0x0040
#define CC_S 0x0080
#define CC_O 0x0800

#define TF_SHIFT 8
#define IOPL_SHIFT 12
#define VM_SHIFT 17

#define TF_MASK 0x00000100
#define IF_MASK 0x00000200
#define DF_MASK 0x00000400
#define IOPL_MASK 0x00003000
#define NT_MASK 0x00004000
#define RF_MASK 0x00010000
#define VM_MASK 0x00020000
#define AC_MASK 0x00040000
#define VIF_MASK 0x00080000
#define VIP_MASK 0x00100000
#define ID_MASK 0x00200000

namespace mobo {

#define NR_INTERRUPTS 256

// general purpose registers
struct regs {
  u64 rax, rbx, rcx, rdx;
  u64 rsi, rdi, rsp, rbp;
  u64 r8, r9, r10, r11;
  u64 r12, r13, r14, r15;
  u64 rip, rflags;
};

// memory segmentation information
struct segment {
  u64 base;
  u32 limit;
  u16 selector;
  u8 type;
  u8 present, dpl, db, s, l, g, avl;
  u8 unusable;
};

struct dtable {
  u64 base;
  u16 limit;
};

// special purpose registers
struct sregs {
  /* out (KVM_GET_SREGS) / in (KVM_SET_SREGS) */
  struct segment cs, ds, es, fs, gs, ss;
  struct segment tr, ldt;
  struct dtable gdt, idt;
  u64 cr0, cr2, cr3, cr4, cr8;
  u64 efer;
  u64 apic_base;
  u64 interrupt_bitmap[(NR_INTERRUPTS + 63) / 64];
};

// Floating point registers
struct fpu_regs {
  u8 fpr[8][16];
  u16 fcw;
  u16 fsw;
  u8 ftwx; /* in fxsave format */
  u16 last_opcode;
  u64 last_ip;
  u64 last_dp;
  u8 xmm[16][16];
  u32 mxcsr;
};

// the vcpu struct is a general interface to a virtualized x86_64 CPU
class vcpu {
 public:
  typedef std::shared_ptr<vcpu> ptr;

  // GPR
  virtual void read_regs(regs &) = 0;
  virtual void write_regs(regs &) = 0;
  // SPR
  virtual void read_sregs(sregs &) = 0;
  virtual void write_sregs(sregs &) = 0;
  // FPR
  virtual void read_fregs(fpu_regs &) = 0;
  virtual void write_fregs(fpu_regs &) = 0;
  virtual void dump_state(FILE *, char *mem = nullptr);

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
  mobo::vcpu *cpu = nullptr;

 public:
  guest_array(mobo::vcpu *cpu, u64 base) : cpu(cpu), base(base) {}

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

};  // namespace mobo

#endif
