#include <winerror.h>
#include <WinHvPlatform.h>
#include <stdexcept>
#include <memoryapi.h>

#include "compiler_defs.h"
#include "mobo/machine.h"
#include "platform/windows/hyperv_vcpu.h"
#include "platform/windows/hyperv_vcpu.h"
#include "platform/windows/hyperv_machine.h"

using namespace mobo;

hyperv_machine::hyperv_machine(uint32_t num_cpus)
    : cpu_()
{
  ensure_capability_or_throw();

  WHV_PARTITION_HANDLE handle = create_partition();
  set_num_cpus(handle, num_cpus);
  setup_partition(handle);

  handle_ = handle;

  for (uint32_t i = 0; i < num_cpus; i++) {
    hyperv_vcpu cpu(handle_, i);
    cpu_.push_back(cpu);
  }
}

void hyperv_machine::ensure_capability_or_throw()
{
  WHV_CAPABILITY capability = {};
  HRESULT hr;
  WHV_CAPABILITY_CODE code;
  uint32_t bytes_written;

  code = WHvCapabilityCodeHypervisorPresent;
  hr = WHvGetCapability(code, &capability, sizeof(capability), &bytes_written);
  if (FAILED(hr)) {
    throw std::runtime_error("failed to create Hyper-V driver: feature not enabled");
  }

  code = WHvCapabilityCodeFeatures;
  hr = WHvGetCapability(code, &capability, sizeof(capability), &bytes_written);
  if (FAILED(hr)) {
    throw std::runtime_error("failed to create Hyper-V driver: feature not enabled");
  }
}

WHV_PARTITION_HANDLE hyperv_machine::create_partition()
{
  WHV_PARTITION_HANDLE handle;
  HRESULT hr = WHvCreatePartition(&handle);
  if (FAILED(hr)) {
    throw std::runtime_error("failed to create Hyper-V driver: WHvCreatePartition failed");
  }

  return handle;
}

void hyperv_machine::setup_partition(WHV_PARTITION_HANDLE handle)
{
  HRESULT hr = WHvSetupPartition(handle);
  if (FAILED(hr)) {
    throw std::runtime_error("failed to create Hyper-V driver: WHvSetupPartition failed");
  }
}

void hyperv_machine::set_num_cpus(WHV_PARTITION_HANDLE handle, uint32_t num_cpus)
{
  WHV_PARTITION_PROPERTY_CODE code = WHvPartitionPropertyCodeProcessorCount;
  WHV_PARTITION_PROPERTY property = { .ProcessorCount = num_cpus };
  set_partition_property(handle, code, property);
}

void hyperv_machine::set_partition_property(
    WHV_PARTITION_HANDLE handle,
    WHV_PARTITION_PROPERTY_CODE code,
    WHV_PARTITION_PROPERTY &property)
{
  HRESULT hr = WHvSetPartitionProperty(handle, code, (const void *) &property, sizeof(property));
  if (FAILED(hr)) {
    throw std::runtime_error("failed to set Hyper-V machine property: WHvSetPartitionProperty failed");
  }
}

void hyperv_machine::allocate_ram(size_t size) {
  mem_size_ = size;
  mem_ = allocate_guest_phys_memory(handle_, 0, size);
}

void hyperv_machine::run(workload &work) {

  bool halted = false;
  while (true) {
    halted = false;

    WHV_RUN_VP_EXIT_CONTEXT run = cpu_[0].run();

//    if (stat == KVM_EXIT_SHUTDOWN) {
//      shutdown = true;
//      printf("SHUTDOWN (probably a triple fault)\n");
//
//      printf("%d\n", run->internal.suberror);
//
//      cpus[0].dump_state(stderr, (char *)this->mem);
//      throw std::runtime_error("triple fault");
//      return;
//    }

    WHV_RUN_VP_EXIT_REASON reason = run.ExitReason;

    if (reason == WHvRunVpExitReasonX64IoPortAccess) {
      uint16_t port_num = run.IoPortAccess.PortNumber;
      if (port_num == 0xFA) {
        // special exit call
        return;
      }

      // 0xFF is the "hcall" io op
      if (port_num == 0xFF) {
        mobo::regs regs = {};
        cpu_[0].read_regs(regs);
        int res = work.handle_hcall(regs, mem_size_, mem_);

        if (res != WORKLOAD_RES_OKAY) {
          if (res == WORKLOAD_RES_KILL) {
            return;
          }
        }

        cpu_[0].write_regs(regs);
        continue;
      }

      continue;
    }

//    if (stat == KVM_EXIT_INTERNAL_ERROR) {
//      if (run->internal.suberror == KVM_INTERNAL_ERROR_EMULATION) {
//        fprintf(stderr, "emulation failure\n");
//        cpus[0].dump_state(stderr);
//        return;
//      }
//      printf("kvm internal error: suberror %d\n", run->internal.suberror);
//      return;
//    }
//
//    if (stat == KVM_EXIT_FAIL_ENTRY) {
//      shutdown = true;
//      halted = true;
//      return;
//    }

    mobo::regs regs = {};
    cpu_[0].read_regs(regs);

    printf("unhandled exit: %d at rip = %p\n", run.ExitReason,
           (void *)regs.rip);
    return;
  }
}

void *hyperv_machine::gpa2hpa(off_t gpa) {
  return nullptr;
}

void hyperv_machine::reset() {

}

void *
hyperv_machine::allocate_guest_phys_memory(
    WHV_PARTITION_HANDLE handle,
    uint64_t guest_addr,
    size_t size)
{
  // Allocate top-down as to not disturb the guest VA
  void *host_virtual_addr = allocate_virtual_memory(
      size,
      MEM_COMMIT | MEM_TOP_DOWN,
      PAGE_READWRITE);

  if (host_virtual_addr == nullptr) {
    throw std::runtime_error("failed to allocated virtual memory of size " + std::to_string(size) + " bytes");
  }

  // Map it into the partition
  map_guest_physical_addr_range(
      handle,
      host_virtual_addr,
      guest_addr,
      size,
      WHvMapGpaRangeFlagRead | WHvMapGpaRangeFlagWrite);

  return host_virtual_addr;
}

void *
hyperv_machine::allocate_virtual_memory(
    size_t size,
    DWORD allocation_flags,
    DWORD protection_flags)
{
  void *addr = VirtualAlloc(
      nullptr,
      size,
      allocation_flags,
      protection_flags);

  if (addr == nullptr) {
    throw std::runtime_error("failed to allocate virtual memory of size " + std::to_string(size) + " bytes: VirtualAlloc");
  }

  return addr;
}

void hyperv_machine::map_guest_physical_addr_range(
    WHV_PARTITION_HANDLE handle,
    void *host_addr,
    uint64_t guest_addr,
    uint64_t size,
    WHV_MAP_GPA_RANGE_FLAGS flags)
{
  HRESULT hr = WHvMapGpaRange(
      handle,
      host_addr,
      guest_addr,
      size,
      flags);

  if (FAILED(hr)) {
    throw std::runtime_error("failed to map guest physical address range: WHvMapGpaRange");
  }
}

uint32_t hyperv_machine::num_cpus() {
  return cpu_.size();
}

mobo::vcpu &hyperv_machine::cpu(uint32_t index) {
  return cpu_[index];
}

static machine::ptr hyperv_allocate(void) {
  return std::make_shared<hyperv_machine>(1);
}

__register_platform
static mobo::platform::registration __hyperv__reg__ = {
    .name = "Microsoft Hyper-V",
    .flags = PLATFORM_WINDOWS,
    .allocate = hyperv_allocate,
};
