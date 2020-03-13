#include <winerror.h>
#include <WinHvPlatform.h>
#include <processthreadsapi.h>
#include <sysinfoapi.h>

#include <stdexcept>
#include <memoryapi.h>
#include <atomic>

#include "compiler_defs.h"
#include "mobo/machine.h"
#include "platform/memory.h"
#include "platform/windows/hyperv_vcpu.h"
#include "platform/windows/hyperv_machine.h"

using namespace mobo;
using namespace mobo::memory;

const uint32_t hyperv_machine::PAGE_SIZE = hyperv_machine::get_page_size();

const char *mobo::hyperv_exit_reason_str(WHV_RUN_VP_EXIT_REASON reason)
{
  switch (reason) {
    case WHvRunVpExitReasonNone: return "WHvRunVpExitReasonNone";
    case WHvRunVpExitReasonMemoryAccess: return "WHvRunVpExitReasonMemoryAccess";
    case WHvRunVpExitReasonX64IoPortAccess: return "WHvRunVpExitReasonX64IoPortAccess";
    case WHvRunVpExitReasonUnrecoverableException: return "WHvRunVpExitReasonUnrecoverableException";
    case WHvRunVpExitReasonInvalidVpRegisterValue: return "WHvRunVpExitReasonInvalidVpRegisterValue";
    case WHvRunVpExitReasonUnsupportedFeature: return "WHvRunVpExitReasonUnsupportedFeature";
    case WHvRunVpExitReasonX64InterruptWindow: return "WHvRunVpExitReasonX64InterruptWindow";
    case WHvRunVpExitReasonX64Halt: return "WHvRunVpExitReasonX64Halt";
    case WHvRunVpExitReasonX64ApicEoi: return "WHvRunVpExitReasonX64ApicEoi";
    case WHvRunVpExitReasonX64MsrAccess: return "WHvRunVpExitReasonX64MsrAccess";
    case WHvRunVpExitReasonX64Cpuid: return "WHvRunVpExitReasonX64Cpuid";
    case WHvRunVpExitReasonException: return "WHvRunVpExitReasonException";
    case WHvRunVpExitReasonCanceled: return "WHvRunVpExitReasonCanceled";
    default: return nullptr;
  }
}

hyperv_machine::hyperv_machine(uint32_t num_cpus)
    : cpu_()
    , mem_(nullptr)
    , mem_size_(0)
{
  ensure_capability_or_throw();

  WHV_PARTITION_HANDLE handle = create_partition();
  handle_ = handle;
	
  set_num_cpus(handle, num_cpus);
  setup_partition(handle);
  setup_long_paging(handle);

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

//    mobo::regs_t snapshot_regs_pre = {};
//    cpu_[0].read_regs_into(snapshot_regs_pre);
//
//    mobo::regs_special_t snapshot_sregs_pre = {};
//    cpu_[0].read_regs_special_into(snapshot_sregs_pre);

//    printf("================= (1) PRE-EXECUTE =================== \n");
//    cpu_[0].dump_state(stdout);
//    printf("===================================================== \n");

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

//    mobo::regs_t snapshot_regs = {};
//    cpu_[0].read_regs_into(snapshot_regs);
//
//    mobo::regs_special_t snapshot_sregs = {};
//    cpu_[0].read_regs_special_into(snapshot_sregs);

//    printf("================= (2) POST-EXECUTE =================== \n");
//    cpu_[0].dump_state(stdout);
//    printf("===================================================== \n");

    WHV_RUN_VP_EXIT_REASON reason = run.ExitReason;

    mobo::regs_t regs = {};
    cpu_[0].read_regs_into(regs);

    if (reason == WHvRunVpExitReasonX64IoPortAccess) {
      uint16_t port_num = run.IoPortAccess.PortNumber;
      if (port_num == 0xFA) {
        // special exit call
        return;
      }

      // 0xFF is the "hcall" io op
      if (port_num == 0xFF) {
        mobo::regs_t hcall_regs = {};
        cpu_[0].read_regs_into(hcall_regs);
        int res = work.handle_hcall(hcall_regs, mem_size_, mem_);

        if (res == WORKLOAD_RES_KILL) {
          return;
        }

        // Continue execution past the `out` instruction
        uint8_t skip = run.VpContext.InstructionLength;
        hcall_regs.rip += skip;
//        printf("%s: rip += %d\n", __FUNCTION__, skip);
        
        cpu_[0].write_regs(hcall_regs);
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

    printf("unhandled exit: %d (%s) at rip = 0x%llx\n",
           reason,
           mobo::hyperv_exit_reason_str(reason),
           regs.rip);

    return;
  }
}

void *hyperv_machine::gpa2hpa(off_t gpa) {
  if (mem_ != nullptr) {
    return (void*)((char*)mem_ + gpa);
  }
  return nullptr;
}

void hyperv_machine::reset() {
  for (hyperv_vcpu &vcpu : cpu_) {
    vcpu.reset();
  }
}

void *
hyperv_machine::allocate_guest_phys_memory(
    WHV_PARTITION_HANDLE handle,
    uint64_t guest_addr,
    size_t size)
{
  // IMPORTANT: Round up to the page size to prevent the call to
  // `WHvMapGpaRange` failing
  size_t size_over_page = size % PAGE_SIZE;
  if (size_over_page > 0) {
    size = size + (PAGE_SIZE - size_over_page);
  }

  // Allocate top-down as to not disturb the guest VA
  void *host_virtual_addr = allocate_virtual_memory(
      size,
      MEM_COMMIT | MEM_TOP_DOWN,
      PAGE_EXECUTE_READWRITE);

  if (host_virtual_addr == nullptr) {
    throw std::runtime_error("failed to allocated virtual memory of size " + std::to_string(size) + " bytes");
  }

  // Map it into the partition
  // TODO: Configure protection flags from function
  map_guest_physical_addr_range(
      handle,
      host_virtual_addr,
      guest_addr,
      size,
      WHvMapGpaRangeFlagRead | WHvMapGpaRangeFlagWrite | WHvMapGpaRangeFlagExecute);

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
  // IMPORTANT: Fails if `size` is not page aligned
  if (size % PAGE_SIZE != 0) {
    throw std::runtime_error("size must be page aligned to " + std::to_string(PAGE_SIZE) + " bytes");
  }

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

void hyperv_machine::free_virtual_memory(void *address, size_t size, DWORD free_type)
{
  BOOL result = VirtualFreeEx(GetCurrentProcess(), address, size, free_type);
  if (result == 0) {
    throw std::runtime_error("failed to free virtual memory");
  }
}

uint32_t hyperv_machine::get_page_size() noexcept
{
  SYSTEM_INFO info;
  GetNativeSystemInfo(&info);
  return info.dwPageSize;
}

//

uint64_t hyperv_machine::setup_long_paging(WHV_PARTITION_HANDLE handle)
{
  /* Recall logical memory address field format

      63             48|47        39|38       30|29      21|20      12|11          0|
      +----------------+------------+-----------+----------+----------+-------------+
      |     unused     | PML4 index | PDP index | PD index | PT index | page offset |
      +----------------+------------+-----------+----------+----------+-------------+

       - PML4 - page mapping table level 4
       - PDP index - page directory pointer index (level 3 index)
           `-> PDPTE - page directory pointer table entry (level 3 entry)
       - PD index - page directory index (level 2)
    */

  uint64_t PTE_SIZE = sizeof(struct mobo::memory::page_entry_t);

  uint64_t USN_PAGE_SIZE = 0x1000;
  uint64_t NUM_LVL3_ENTRIES = 1;
  uint64_t NUM_LVL4_ENTRIES = 512;
  uint64_t PML4_SIZE = NUM_LVL4_ENTRIES * PTE_SIZE;
  uint64_t ALL_PAGE_TABLES_SIZE =
      NUM_LVL3_ENTRIES * PTE_SIZE
      + PML4_SIZE;

  void *pte_ptr = allocate_guest_phys_memory(
      handle,
      mobo::memory::PML4_PHYSICAL_ADDRESS,
      ALL_PAGE_TABLES_SIZE);
  if (pte_ptr == nullptr) {
    throw std::runtime_error("allocate_guest_phys_memory: failed to allocate page table in guest");
  }

  auto pml4 = static_cast<mobo::memory::page_entry_t *>(pte_ptr);
  auto pml3 = static_cast<mobo::memory::page_entry_t *>((void *)((char *) pte_ptr + PML4_SIZE));

  //
  // Build a valid user-mode PML4E
  //
  pml4[0] = {};
  pml4[0].present = 1;
  pml4[0].write = 1;
  pml4[0].allow_user_mode = 1;
  pml4[0].page_frame_num = (mobo::memory::PML4_PHYSICAL_ADDRESS / USN_PAGE_SIZE) + 1;

  //
  // Build a valid user-mode 1GB PDPTE
  //
  mobo::memory::page_entry_t pte_template = {
      .present = 1,
      .write = 1,
      .allow_user_mode = 1,
      .large_page = 1,
  };

  //
  // Loop over the PDPT (PML3) minus the last 1GB
  //
  uint64_t actual_pml3 = max(1, NUM_LVL3_ENTRIES - 1);
  for (uint64_t i = 0; i < actual_pml3; i++) {
    //
    // Set the PDPTE to the next valid 1GB of RAM, creating a 1:1 map
    //
    pml3[i] = pte_template;
    pml3[i].page_frame_num = ((i * mobo::memory::_1GB) / USN_PAGE_SIZE);
  }

  return mobo::memory::PML4_PHYSICAL_ADDRESS;
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
