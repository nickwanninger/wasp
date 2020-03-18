#pragma once

#include <WinHvPlatformDefs.h>
#include "mobo/machine.h"
#include "./hyperv_vcpu.h"

namespace mobo {

const char *hyperv_exit_reason_str(WHV_RUN_VP_EXIT_REASON reason);

class hyperv_machine : public mobo::machine {

private:
  WHV_PARTITION_HANDLE handle_;
  std::vector<hyperv_vcpu> cpu_;
  void *mem_;
  size_t mem_size_;

  struct virtual_alloc_t {
    void *addr; size_t size;
    virtual_alloc_t(void* addr, size_t size) : addr(addr), size(size) { }
  };
  std::vector<virtual_alloc_t> virtual_allocs_;

  static const uint32_t PAGE_SIZE;
  static const uint32_t PAGE_ALIGNMENT;

  static void ensure_capability_or_throw();
  static WHV_PARTITION_HANDLE create_partition();
  static void setup_partition(WHV_PARTITION_HANDLE handle);
  static void set_num_cpus(WHV_PARTITION_HANDLE handle, uint32_t num_cpus);
  static void set_partition_property(WHV_PARTITION_HANDLE handle, WHV_PARTITION_PROPERTY_CODE code, WHV_PARTITION_PROPERTY &property);
  void *allocate_guest_phys_memory(WHV_PARTITION_HANDLE handle, uint64_t guest_addr, size_t size);
  void *allocate_virtual_memory(size_t size, DWORD allocation_flags, DWORD protection_flags);
  static uint32_t get_page_size() noexcept;
  static uint32_t get_page_alignment() noexcept;
  static void free_virtual_memory(void *address, size_t size, DWORD free_type);
  static void map_guest_physical_addr_range(
      WHV_PARTITION_HANDLE handle,
      void *host_addr,
      uint64_t guest_addr,
      uint64_t size,
      WHV_MAP_GPA_RANGE_FLAGS flags);

public:

  explicit hyperv_machine(uint32_t num_cpus);
  ~hyperv_machine();

  void allocate_ram(size_t) override;

  void run(workload &) override;
  void *gpa2hpa(off_t gpa) override;
  void reset() override;
  uint32_t num_cpus() override;
  mobo::vcpu &cpu(uint32_t) override;

  uint64_t setup_long_paging(WHV_PARTITION_HANDLE handle);
};

}