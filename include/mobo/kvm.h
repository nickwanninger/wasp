#pragma once

#ifndef __MOBO_KVMDRIVER_
#define __MOBO_KVMDRIVER_

#ifdef __LINUX__
#include <linux/kvm.h>
#endif

#include <mobo/vcpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <mobo/workload.h>
#include <vector>

/*
 * The hole includes VESA framebuffer and PCI memory.
 */
#define KVM_32BIT_MAX_MEM_SIZE (1ULL << 32)
#define KVM_32BIT_GAP_SIZE (768 << 20)
#define KVM_32BIT_GAP_START (KVM_32BIT_MAX_MEM_SIZE - KVM_32BIT_GAP_SIZE)

#define KVM_MMIO_START KVM_32BIT_GAP_START

namespace mobo {

struct kvm_vcpu : public mobo::vcpu {
  int cpufd = -1;
  int index = 0;

  char *mem;
  size_t memsize;

  struct kvm_run *kvm_run;
  size_t run_size = 0;

  struct kvm_regs initial_regs;
  struct kvm_sregs initial_sregs;
  struct kvm_fpu initial_fpu;


  // GPR
  virtual void read_regs(regs &);
  virtual void write_regs(regs &);
  // SPR
  virtual void read_sregs(sregs &);
  virtual void write_sregs(sregs &);
  // FPR
  virtual void read_fregs(fpu_regs &);
  virtual void write_fregs(fpu_regs &);

  virtual void dump_state(FILE *, char *mem = nullptr);

  virtual void *translate_address(u64 gva);
  virtual void reset(void);
};

// a memory bank represents a segment of memory in the kvm CPU
struct ram_bank {
  uint64_t guest_phys_addr;
  void *host_addr;
  size_t size;
};

class kvm {
 private:
  void *mem;
  size_t memsize;

  int kvmfd;
  int vmfd;
  int ncpus;
  std::vector<kvm_vcpu> cpus;
  void init_cpus(void);

  // ram is made up of a series of banks, typically the region
  // before the PCI gap, and the region after the gap
  std::vector<ram_bank> ram;
  void attach_bank(ram_bank &&);

  std::string path;

 public:

  inline void *mem_addr(off_t o) {
    return (void*)((char*)mem + o);
  }

  bool halted = false;
  bool shutdown = false;

  kvm(int kvmfd, std::string path, int ncpus);
  ~kvm(void);
  void load_elf(std::string);

  void init_ram(size_t);
  void run(workload &);

  void reset();
};

}  // namespace mobo

#endif
