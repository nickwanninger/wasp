#pragma once

#ifndef __MOBO_KVMDRIVER_
#define __MOBO_KVMDRIVER_

#include <linux/kvm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include <wasp/workload.h>
#include <wasp/vcpu.h>
#include <wasp/machine.h>

/*
 * The hole includes VESA framebuffer and PCI memory.
 */
#define KVM_32BIT_MAX_MEM_SIZE (1ULL << 32)
#define KVM_32BIT_GAP_SIZE (768 << 20)
#define KVM_32BIT_GAP_START (KVM_32BIT_MAX_MEM_SIZE - KVM_32BIT_GAP_SIZE)

#define KVM_MMIO_START KVM_32BIT_GAP_START

namespace wasp {

struct kvm_vcpu : public wasp::vcpu {
  int cpufd = -1;
  int index;

  char *mem;
  size_t memsize;

  struct kvm_run *kvm_run;
  size_t run_size;

  struct kvm_regs initial_regs;
  struct kvm_sregs initial_sregs;
  struct kvm_fpu initial_fpu;


  // GPR
  void read_regs_into(regs_t &) override;
  void write_regs(regs_t &) override;
  // SPR
  void read_regs_special_into(regs_special_t &r) override;
  void write_regs_special(regs_special_t &) override;
  // FPR
  void read_regs_fpu_into(regs_fpu_t &dst) override;
  void write_regs_fpu(regs_fpu_t &src) override;

  // void dump_state(FILE *, char *mem = nullptr) override;

  void *translate_address(u64 gva) override;
  void reset(void) override;
};

// a memory bank represents a segment of memory in the kvm CPU
struct ram_bank {
  uint64_t guest_phys_addr;
  void *host_addr;
  size_t size;
};

class kvm_machine : public wasp::machine {
 private:
  void *mem;
  size_t memsize;

  int kvmfd;
  int vmfd;
  int ncpus;
  std::vector<kvm_vcpu> cpus;
  void init_cpus();

  // ram is made up of a series of banks, typically the region
  // before the PCI gap, and the region after the gap
  std::vector<ram_bank> ram;
  void attach_bank(ram_bank &&);

  std::string path;

  bool halted = false;
  bool shutdown = false;


 public:

  inline void *mem_addr(off_t o) {
    return (void*)((char*)mem + o);
  }

  kvm_machine(int kvmfd, int ncpus);
  ~kvm_machine() override;


  void allocate_ram(size_t) override;
  void run(workload &) override;
  void *gpa2hpa(off_t gpa) override;
  void reset() override;


  uint32_t num_cpus() override;
  wasp::vcpu &cpu(uint32_t) override;
};

}  // namespace wasp

#endif
