#pragma once
#include <wasp/vcpu.h>
#include <wasp/machine.h>
#include <WinHvPlatformDefs.h>

namespace wasp {

class WASP_API hyperv_vcpu : public wasp::vcpu {

  WHV_PARTITION_HANDLE partition_handle_;
  uint32_t cpu_index_;

  void reset_real();
  void reset_protected();
  void reset_long();

public:
  hyperv_vcpu(
      WHV_PARTITION_HANDLE partition_handle,
      uint32_t cpu_index);

  ~hyperv_vcpu();

  // GPR
  void read_regs_into(wasp::regs_t &r) override;
  void write_regs(wasp::regs_t &) override;
  // SPR
  void read_regs_special_into(wasp::regs_special_t &r) override;
  void write_regs_special(wasp::regs_special_t &r) override;
  // FPR
  void read_regs_fpu_into(wasp::regs_fpu_t &) override;
  void write_regs_fpu(wasp::regs_fpu_t &) override;

  void *translate_address(u64 gva) override;
  void reset() override;

  WHV_RUN_VP_EXIT_CONTEXT run();
};

}