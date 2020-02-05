#pragma once
#include <mobo/vcpu.h>
#include <mobo/machine.h>
#include <WinHvPlatformDefs.h>

class hyperv_driver : public mobo::driver {

  void attach_memory(size_t size, void *pVoid) override;

  void load_elf(std::string &string) override;

  void run(void) override;
};

class hyperv_vcpu : public mobo::vcpu {

  WHV_PARTITION_HANDLE partition_handle_;
  uint32_t cpu_index_;

public:
  hyperv_vcpu(
      WHV_PARTITION_HANDLE partition_handle,
      uint32_t cpu_index);

  // GPR
  void read_regs(mobo::regs &) override;
  void write_regs(mobo::regs &) override;
  // SPR
  void read_sregs(mobo::sregs &) override;
  void write_sregs(mobo::sregs &) override;
  // FPR
  void read_fregs(mobo::fpu_regs &) override;
  void write_fregs(mobo::fpu_regs &) override;

  void dump_state(FILE *, char *mem = nullptr) override;

  void *translate_address(u64 gva) override;
  void reset(void) override;
};