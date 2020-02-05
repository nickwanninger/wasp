#pragma once

#include <WinHvPlatformDefs.h>

namespace mobo {

class hyperv_driver : public mobo::driver {

private:
  WHV_PARTITION_HANDLE handle_;
  uint32_t num_cpus_;
  std::vector<hyperv_vcpu> cpus_;

  static void ensure_capability_or_throw();
  static WHV_PARTITION_HANDLE create_partition();
  static void setup_partition(WHV_PARTITION_HANDLE handle);
  static void set_num_cpus(WHV_PARTITION_HANDLE handle, uint32_t num_cpus);
  static void set_partition_property(WHV_PARTITION_HANDLE handle, WHV_PARTITION_PROPERTY_CODE code, WHV_PARTITION_PROPERTY &property);

public:
  explicit hyperv_driver(uint32_t num_cpus);

  uint32_t num_cpus() override;

  mobo::vcpu::ptr cpu(uint32_t uint_32) override;

  void attach_memory(size_t size, void *pVoid) override;

  void run(void) override;

  void reset() override;
};

}