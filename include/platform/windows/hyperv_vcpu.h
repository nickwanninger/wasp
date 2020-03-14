#pragma once
#include <mobo/vcpu.h>
#include <mobo/machine.h>
#include <WinHvPlatformDefs.h>

namespace mobo {

class hyperv_vcpu : public mobo::vcpu {

  WHV_PARTITION_HANDLE partition_handle_;
  uint32_t cpu_index_;

  void reset_protected();
  void reset_long();

public:
  hyperv_vcpu(
      WHV_PARTITION_HANDLE partition_handle,
      uint32_t cpu_index);

  ~hyperv_vcpu();

  // GPR
  void read_regs_into(mobo::regs_t &r) override;
  void write_regs(mobo::regs_t &) override;
  // SPR
  void read_regs_special_into(mobo::regs_special_t &r) override;
  void write_regs_special(mobo::regs_special_t &r) override;
  // FPR
  void read_regs_fpu_into(mobo::regs_fpu_t &) override;
  void write_regs_fpu(mobo::regs_fpu_t &) override;

  void *translate_address(u64 gva) override;
  void reset() override;

  WHV_RUN_VP_EXIT_CONTEXT run();
};

}