#include <winerror.h>
#include <sysinfoapi.h>
#include <memoryapi.h>
#include <WinHvPlatform.h>

#include <stdexcept>
#include <atomic>
#include <chrono>
#include <wasp/unistd.h>
#include <wasp/compiler_defs.h>
#include <wasp/machine.h>
#include <wasp/platform.h>

#include <timeit.h>

TIMEIT_EXTERN(g_main);

using namespace wasp;
using namespace wasp::memory;

const uint32_t hyperv_machine::PAGE_SIZE = hyperv_machine::get_page_size();
const uint32_t hyperv_machine::PAGE_ALIGNMENT = hyperv_machine::get_page_alignment();

const char *wasp::hyperv_exit_reason_str(WHV_RUN_VP_EXIT_REASON reason)
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

static int instance_count = 0;

hyperv_machine::hyperv_machine(uint32_t num_cpus)
    : cpu_()
    , mem_(nullptr)
    , mem_size_(0)
{
  ++instance_count;
  TIMEIT_MARK(g_main, __FUNCTION__);
  ensure_capability_or_throw();

  WHV_PARTITION_HANDLE handle = create_partition();
  handle_ = handle;
	
  set_num_cpus(handle, num_cpus);
  set_extended_vm_exits(handle);
  setup_partition(handle);
  // setup_long_paging(handle);

  for (uint32_t i = 0; i < num_cpus; i++) {
    cpu_.emplace_back(handle_, i);
  }
}

hyperv_machine::~hyperv_machine() {
  TIMEIT_FN(g_main);
  for (auto alloc : virtual_allocs_) {
    free_virtual_memory(alloc.addr, alloc.size, MEM_DECOMMIT);
  }
  cpu_.clear(); // run v-cpu destructors first
  WHvDeletePartition(handle_);
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

  code = WHvCapabilityCodeExtendedVmExits;
  hr = WHvGetCapability(code, &capability, sizeof(capability), &bytes_written);
  if (FAILED(hr)) {
    throw std::runtime_error("failed to create Hyper-V driver: feature not enabled");
  }

  auto& extended_exits = *reinterpret_cast<WHV_EXTENDED_VM_EXITS *>(&capability.ExceptionExitBitmap);
  if (extended_exits.X64MsrExit == 0) {
	throw std::runtime_error("failed to create Hyper-V driver: extended exits for msr not available");
  }
}

WHV_PARTITION_HANDLE hyperv_machine::create_partition()
{
  TIMEIT_FN(g_main);
  WHV_PARTITION_HANDLE handle;
  HRESULT hr = WHvCreatePartition(&handle);
  if (FAILED(hr)) {
    throw std::runtime_error("failed to create Hyper-V driver: WHvCreatePartition failed");
  }

  return handle;
}

void hyperv_machine::setup_partition(WHV_PARTITION_HANDLE handle)
{
  TIMEIT_FN(g_main);
  HRESULT hr = WHvSetupPartition(handle);
  if (FAILED(hr)) {
    PANIC("failed to create Hyper-V driver: WHvSetupPartition failed");
  }
}

void hyperv_machine::set_extended_vm_exits(WHV_PARTITION_HANDLE handle)
{
  WHV_PARTITION_PROPERTY_CODE code = WHvPartitionPropertyCodeExtendedVmExits;
  WHV_PARTITION_PROPERTY property = {};
  property.ExtendedVmExits.X64MsrExit = 1;
  property.ExtendedVmExits.X64CpuidExit = 1;
  set_partition_property(handle, code, property);
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
  TIMEIT_FN(g_main);

  while (true) {
    wasp::regs_t snapshot_regs_pre = {};
    cpu_[0].read_regs_into(snapshot_regs_pre);

//    printf("================= (1) PRE-EXECUTE =================== \n");
//    cpu_[0].dump_state(stdout);
//    printf("===================================================== \n");

    TIMEIT_BEGIN(g_main, vcpu, "vcpu run");
    WHV_RUN_VP_EXIT_CONTEXT run = cpu_[0].run();
    TIMEIT_END(g_main, vcpu);

//    printf("================= (2) POST-EXECUTE =================== \n");
//    cpu_[0].dump_state(stdout);
//    printf("===================================================== \n");

    WHV_RUN_VP_EXIT_REASON reason = run.ExitReason;

    wasp::regs_t regs = {};
    cpu_[0].read_regs_into(regs);

    // Handle jump to protected mode by emulating the instruction to set RIP
    if (reason == WHvRunVpExitReasonUnrecoverableException
        && run.VpContext.InstructionLength == 3) {

      auto jmp_instr = static_cast<uint8_t *>(gpa2hpa(run.VpContext.Rip));
      if (jmp_instr[0] == 0xEA) {
        regs_special_t regs_special = {};
        cpu_[0].read_regs_special_into(regs_special);
        uint8_t sel = jmp_instr[3];
        regs_special.cs.selector = sel;

        uint16_t high = ((uint16_t) jmp_instr[2]) << 8u;
        uint16_t low = (uint16_t) jmp_instr[1];
        uint16_t addr = high | low;
        regs.rip = addr;

        cpu_[0].write_regs_special(regs_special);
        cpu_[0].write_regs(regs);
        continue;
      }
    }

    // Handle `rdmsr` (Read Machine Specific Register)
    //    and `wrmsr` (Write Machine Specific Register)
    //
    // Unlike KVM, we have to explicitly handle MSR exits. Right now we only care about handling x68 EFER.
    //
#define MSR_EFER (0xC0000080)

    if (reason == WHvRunVpExitReasonX64MsrAccess
        && regs.rcx == MSR_EFER) {

      regs_special_t regs_special = {};
      cpu_[0].read_regs_special_into(regs_special);

      if (run.MsrAccess.AccessInfo.IsWrite) {
        regs_special.efer = regs.rax;
        cpu_[0].write_regs_special(regs_special);
      } else {
        regs.rax = regs_special.efer;
      }

      // Continue execution past the `rdmsr`/`wrmsr` instruction
      uint8_t skip = run.VpContext.InstructionLength;
      regs.rip += skip;

      cpu_[0].write_regs(regs);
      continue;
    }

	if (reason == WHvRunVpExitReasonX64IoPortAccess) {
		uint16_t port_num = run.IoPortAccess.PortNumber;
		if (port_num == 0xFA) {
			// special exit call
			work.handle_exit();
			return;
		}

		// 0xFF is the "hcall" io op
		if (port_num == 0xFF) {
			int res = work.handle_hcall(regs, mem_size_, mem_);

			if (res == WORKLOAD_RES_KILL) {
				return;
			}

			// Continue execution past the `out` instruction
			uint8_t skip = run.VpContext.InstructionLength;
			regs.rip += skip;

			cpu_[0].write_regs(regs);
			continue;
		}

		fprintf(stderr, "%s: unhandled io port 0x%x\n", __FUNCTION__, port_num);
		cpu_[0].dump_state(stderr);
		return;
	}

    printf("unhandled exit! %d (%s) at rip = 0x%llx\n",
           reason,
           wasp::hyperv_exit_reason_str(reason),
           regs.rip);

    cpu_[0].dump_state(stdout);
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

    regs_t r = {};
    vcpu.read_regs_into(r);
    r.rip = entry();
    vcpu.write_regs(r);
  }
}

void *
hyperv_machine::allocate_guest_phys_memory(
    WHV_PARTITION_HANDLE handle,
    uint64_t guest_addr,
    size_t size)
{
  if (guest_addr % PAGE_ALIGNMENT != 0) {
    PANIC("guest_addr is not page aligned");
  }

  // IMPORTANT: Round up to the page size to prevent the call to
  // `WHvMapGpaRange` failing
  size_t size_over_page = size % PAGE_ALIGNMENT;
  if (size_over_page > 0) {
    size = size + (PAGE_ALIGNMENT - size_over_page);
  }

  void *host_virtual_addr = allocate_virtual_memory(
      size,
      MEM_RESERVE | MEM_COMMIT,
      PAGE_EXECUTE_READWRITE);

  fprintf(stderr, "allocate_virtual_memory -> %p\n", host_virtual_addr);
  if (host_virtual_addr == nullptr) {
    PANIC("failed to allocated virtual memory of size %lld bytes", size);
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
  MEM_ADDRESS_REQUIREMENTS addressReqs = {};
  MEM_EXTENDED_PARAMETER param = {};
  addressReqs.Alignment = PAGE_ALIGNMENT;
  param.Type = MemExtendedParameterAddressRequirements;
  param.Pointer = &addressReqs;

  void *addr = VirtualAlloc2(
      GetCurrentProcess(),
      nullptr,
      size,
      allocation_flags,
      protection_flags,
      &param,
      1);

  if (addr == nullptr) {
    PANIC("failed to allocate virtual memory of size %lld bytes", size);
  }

  virtual_allocs_.emplace_back(addr, size);
  return addr;
}

void hyperv_machine::map_guest_physical_addr_range(
    WHV_PARTITION_HANDLE handle,
    void *host_addr,
    uint64_t guest_addr,
    uint64_t size,
    WHV_MAP_GPA_RANGE_FLAGS flags)
{
  TIMEIT_FN(g_main, __FUNCTION__);
//  printf("%s(handle = 0x%p, host_addr = 0x%p, guest_addr = 0x%llx, size = %lld, flags = %d\n",
//      __FUNCTION__, handle, host_addr, guest_addr, size, flags);

  // IMPORTANT: Fails if `size` is not page aligned
  if (size % PAGE_ALIGNMENT != 0) {
    PANIC("%s", ("size must be page aligned to " + std::to_string(PAGE_ALIGNMENT) + " bytes").data());
  }

  HRESULT hr = WHvMapGpaRange(
      handle,
      host_addr,
      guest_addr,
      size,
      flags);

  if (FAILED(hr)) {
    PANIC("WHvMapGpaRange: failed to map guest physical address range");
  }
}

uint32_t hyperv_machine::num_cpus() {
  return cpu_.size();
}

wasp::vcpu &hyperv_machine::cpu(uint32_t index) {
  return cpu_[index];
}

void hyperv_machine::free_virtual_memory(void *address, size_t size, DWORD free_type)
{
  BOOL result = VirtualFree(address, size, free_type);
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

uint32_t hyperv_machine::get_page_alignment() noexcept
{
  SYSTEM_INFO info;
  GetNativeSystemInfo(&info);
  return info.dwAllocationGranularity;
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

  static uint64_t phys_addr = wasp::memory::PML4_PHYSICAL_ADDRESS;

  uint64_t PTE_SIZE = sizeof(struct wasp::memory::page_entry_t);

  uint64_t USN_PAGE_SIZE = 0x1000;
  uint64_t NUM_LVL3_ENTRIES = 1;
  uint64_t NUM_LVL4_ENTRIES = 512;
  uint64_t PML4_SIZE = NUM_LVL4_ENTRIES * PTE_SIZE;
  uint64_t ALL_PAGE_TABLES_SIZE =
      NUM_LVL3_ENTRIES * PTE_SIZE
      + PML4_SIZE;

  void *pte_ptr = allocate_guest_phys_memory(
      handle,
      phys_addr,
      ALL_PAGE_TABLES_SIZE);
  if (pte_ptr == nullptr) {
    throw std::runtime_error("allocate_guest_phys_memory: failed to allocate page table in guest");
  }

  auto pml4 = static_cast<wasp::memory::page_entry_t *>(pte_ptr);
  auto pml3 = static_cast<wasp::memory::page_entry_t *>((void *)((char *) pte_ptr + PML4_SIZE));

  //
  // Build a valid user-mode PML4E
  //
  pml4[0] = {};
  pml4[0].present = 1;
  pml4[0].write = 1;
  pml4[0].allow_user_mode = 1;
  pml4[0].page_frame_num = (wasp::memory::PML4_PHYSICAL_ADDRESS / USN_PAGE_SIZE) + 1;

  //
  // Build a valid user-mode 1GB PDPTE
  //
  wasp::memory::page_entry_t pte_template = {
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
    pml3[i].page_frame_num = ((i * wasp::memory::_1GB) / USN_PAGE_SIZE);
  }

  return phys_addr;
}

static machine::unique_ptr hyperv_allocate() {
  return std::make_unique<hyperv_machine>(1);
}

__register_platform(__hyperv__reg__)
wasp::platform::registration __hyperv__reg__ = {
    .name = "Microsoft Hyper-V",
    .flags = PLATFORM_WINDOWS,
    .allocate = hyperv_allocate,
};
