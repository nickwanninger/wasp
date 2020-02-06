#pragma once

#include <WinHvPlatformDefs.h>

namespace mobo {

class hyperv_machine : public mobo::machine {

private:
  WHV_PARTITION_HANDLE handle_;
  std::vector<hyperv_vcpu> cpu_;
  void *mem_;
  size_t mem_size_;

  static void ensure_capability_or_throw();
  static WHV_PARTITION_HANDLE create_partition();
  static void setup_partition(WHV_PARTITION_HANDLE handle);
  static void set_num_cpus(WHV_PARTITION_HANDLE handle, uint32_t num_cpus);
  static void set_partition_property(WHV_PARTITION_HANDLE handle, WHV_PARTITION_PROPERTY_CODE code, WHV_PARTITION_PROPERTY &property);
  static void *allocate_guest_phys_memory(WHV_PARTITION_HANDLE handle, uint64_t guest_addr, size_t size);
  static void *allocate_virtual_memory(size_t size, DWORD allocation_flags, DWORD protection_flags);
  static void map_guest_physical_addr_range(
      WHV_PARTITION_HANDLE handle,
      void *host_addr,
      uint64_t guest_addr,
      uint64_t size,
      WHV_MAP_GPA_RANGE_FLAGS flags);

public:
  explicit hyperv_machine(uint32_t num_cpus);

  void allocate_ram(size_t) override;
  void run(workload &) override;
  void *gpa2hpa(off_t gpa) override;
  void reset() override;

  uint32_t num_cpus() override;
  mobo::vcpu &cpu(uint32_t) override;
};

}