#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <WinHvPlatform.h>
#include <strsafe.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <inttypes.h>

#include "platform/windows/hyperv_machine.h"
#include "platform/memory.h"

void get_registers(
    WHV_PARTITION_HANDLE partition_handle,
    uint32_t cpu_index,
    WHV_REGISTER_VALUE *regs);

void exit_last_error(char *function_name)
{
  // Retrieve the system error message for the last-error code

  void *msg_buffer;
  void *display_buf;
  DWORD dw = GetLastError();

  FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      dw,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &msg_buffer,
      0,
      nullptr);

  // Display the error message and exit the process

  display_buf = (void *)LocalAlloc(LMEM_ZEROINIT, (lstrlen((char *)msg_buffer) + lstrlen((char *)function_name) + 40) * sizeof(char));
  StringCchPrintf((char *)display_buf,
                  LocalSize(display_buf) / sizeof(TCHAR),
                  TEXT("%s failed with error %d: %s"),
                  function_name, dw, msg_buffer);
  fprintf(stderr, "%s", (char *) display_buf);

  LocalFree(msg_buffer);
  LocalFree(display_buf);
  exit(1);
}

uint32_t get_page_size()
{
  SYSTEM_INFO info = {};
  GetSystemInfo(&info);
  const uint32_t page_size = info.dwPageSize;
  return page_size;
}

uint32_t PAGE_SIZE;

constexpr uint32_t NUM_REGISTERS = 18 + 6 + 2 + 2 + 5 + 3;

const WHV_REGISTER_NAME HVM_REGISTERS_MAPPING[NUM_REGISTERS] = {
  WHvX64RegisterRax, WHvX64RegisterRcx, WHvX64RegisterRdx,
  WHvX64RegisterRbx, WHvX64RegisterRsp, WHvX64RegisterRbp,
  WHvX64RegisterRsi, WHvX64RegisterRdi, WHvX64RegisterR8,
  WHvX64RegisterR9,  WHvX64RegisterR10, WHvX64RegisterR11,
  WHvX64RegisterR12, WHvX64RegisterR13, WHvX64RegisterR14,
  WHvX64RegisterR15, WHvX64RegisterRip, WHvX64RegisterRflags,

  WHvX64RegisterEs,  WHvX64RegisterCs,  WHvX64RegisterSs,
  WHvX64RegisterDs,  WHvX64RegisterFs,  WHvX64RegisterGs,

  WHvX64RegisterLdtr, WHvX64RegisterTr,
  
  WHvX64RegisterGdtr, WHvX64RegisterIdtr,
  
  WHvX64RegisterCr0, WHvX64RegisterCr2, WHvX64RegisterCr3,
  WHvX64RegisterCr4, WHvX64RegisterCr8,

  WHvX64RegisterEfer, WHvX64RegisterLstar, WHvRegisterPendingInterruption,
};

const WHV_REGISTER_NAME *HVM_REGISTERS_MAPPING_INPUT =
    reinterpret_cast<const WHV_REGISTER_NAME *>(&HVM_REGISTERS_MAPPING);

enum hyperv_register_index {
  rax, rcx, rdx,
  rbx, rsp, rbp,
  rsi, rdi, r8,
  r9, r10, r11,
  r12, r13, r14,
  r15, rip, rflags,

  es, cs, ss,
  ds, fs, gs,

  ldtr, tr,

  gdtr, idtr,

  cr0, cr2, cr3,
  cr4, cr8,

  efer, lstar, pending_interrupts,
};

static void cpu_dump_seg_cache(FILE *out, const char *name,
                               WHV_REGISTER_VALUE const &reg) {
  WHV_X64_SEGMENT_REGISTER const &seg = reg.Segment;
  fprintf(out, "%-3s=%04x %016" PRIx64 " %08x %d %02x %02x  %02x\n", name,
      seg.Selector, (size_t)seg.Base, seg.Limit, seg.Present, seg.Default,
      seg.DescriptorPrivilegeLevel, seg.SegmentType);
}

void dump_state(FILE *out, WHV_PARTITION_HANDLE handle, uint32_t cpu_index) {
  WHV_REGISTER_VALUE regs[NUM_REGISTERS] = {};
  get_registers(handle, cpu_index, regs);

  unsigned int eflags = regs[rflags].Reg64;
#define GET(name) (regs[name].Reg64)

#define REGFMT "%016" PRIx64
  fprintf(out,
          "RAX=" REGFMT " RBX=" REGFMT " RCX=" REGFMT " RDX=" REGFMT
  "\n"
  "RSI=" REGFMT " RDI=" REGFMT " RBP=" REGFMT " RSP=" REGFMT
  "\n"
  "R8 =" REGFMT " R9 =" REGFMT " R10=" REGFMT " R11=" REGFMT
  "\n"
  "R12=" REGFMT " R13=" REGFMT " R14=" REGFMT " R15=" REGFMT
  "\n"
  "RIP=" REGFMT " RFL=%08x [%c%c%c%c%c%c%c]\n",

      GET(rax), GET(rbx), GET(rcx), GET(rdx), GET(rsi), GET(rdi), GET(rbp),
      GET(rsp), GET(r8), GET(r9), GET(r10), GET(r11), GET(r12), GET(r13),
      GET(r14), GET(r15), GET(rip), eflags, eflags & DF_MASK ? 'D' : '-',
      eflags & CC_O ? 'O' : '-', eflags & CC_S ? 'S' : '-',
      eflags & CC_Z ? 'Z' : '-', eflags & CC_A ? 'A' : '-',
      eflags & CC_P ? 'P' : '-', eflags & CC_C ? 'C' : '-');

  fprintf(out, "    sel  base             limit    p db dpl type\n");
  cpu_dump_seg_cache(out, "ES", regs[es]);
  cpu_dump_seg_cache(out, "CS", regs[cs]);
  cpu_dump_seg_cache(out, "SS", regs[ss]);
  cpu_dump_seg_cache(out, "DS", regs[ds]);
  cpu_dump_seg_cache(out, "FS", regs[fs]);
  cpu_dump_seg_cache(out, "GS", regs[gs]);
  cpu_dump_seg_cache(out, "LDT", regs[ldtr]);
  cpu_dump_seg_cache(out, "TR", regs[tr]);

  WHV_X64_TABLE_REGISTER &gdt_register = regs[gdtr].Table;
  fprintf(out, "GDT=     %016" PRIx64 " %08x\n", (size_t)gdt_register.Base,
          (int) gdt_register.Limit);
  
  WHV_X64_TABLE_REGISTER &idt_register = regs[idtr].Table;
  fprintf(out, "IDT=     %016" PRIx64 " %08x\n", (size_t)idt_register.Base,
          (int) idt_register.Limit);

  fprintf(out,
          "CR0=%016" PRIx64 " CR2=%016" PRIx64 " CR3=%016" PRIx64 " CR4=%08x\n",
      (size_t)regs[cr0].Reg64, (size_t)regs[cr2].Reg64, (size_t)regs[cr3].Reg64,
      (int)regs[cr4].Reg64);

  fprintf(out, "EFER=%016" PRIx64 "\n", (size_t)regs[efer].Reg64);

#undef GET
}

void get_registers(
    WHV_PARTITION_HANDLE partition_handle,
    uint32_t cpu_index,
    WHV_REGISTER_VALUE *regs)
{
  HRESULT hr = WHvGetVirtualProcessorRegisters(
      partition_handle,
      cpu_index,
      HVM_REGISTERS_MAPPING_INPUT,
      NUM_REGISTERS,
      regs);

  if (FAILED(hr)) {
    throw std::runtime_error("WHvGetVirtualProcessorRegisters: failed to get virtual processors");
  }
}

void set_registers(
    WHV_PARTITION_HANDLE partition_handle,
    uint32_t cpu_index,
    WHV_REGISTER_VALUE *regs)
{
  HRESULT hr = WHvSetVirtualProcessorRegisters(
      partition_handle,
      cpu_index,
      HVM_REGISTERS_MAPPING_INPUT,
      NUM_REGISTERS,
      regs);

  if (FAILED(hr)) {
    throw std::runtime_error("WHvSetVirtualProcessorRegisters: failed to set virtual processors");
  }
}


void *allocate_guest_phys_memory(
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
  void *host_virtual_addr = VirtualAlloc(
      nullptr,
      size,
      MEM_COMMIT | MEM_TOP_DOWN,
      PAGE_EXECUTE_READWRITE);

  if (host_virtual_addr == nullptr) {
    fprintf(stderr, "%s", ("VirtualAlloc: failed to allocated virtual memory of size " + std::to_string(size) + " bytes").data());
    exit(1);
  }

  // Map it into the partition
  HRESULT hr = WHvMapGpaRange(
      handle,
      host_virtual_addr,
      guest_addr,
      size,
      WHvMapGpaRangeFlagRead | WHvMapGpaRangeFlagWrite);
  if (FAILED(hr)) {
    fprintf(stderr, "WHvMapGpaRange: failed to map memory into partition\n");
    exit(1);
  }

  return host_virtual_addr;
}

void reset_long(WHV_PARTITION_HANDLE partition_handle, uint32_t cpu_index, uint64_t pml4_address)
{
  WHV_REGISTER_VALUE r[NUM_REGISTERS] = {};
  get_registers(partition_handle, cpu_index, r);

  // see https://wiki.osdev.org/CPU_Registers_x86#CR0
  uint64_t CR0_PE = 1u << 0u; // protected mode enable
  uint64_t CR0_MP = 1u << 1u; // monitor co-processor
  uint64_t CR0_ET = 1u << 4u; // extension type
  uint64_t CR0_NE = 1u << 5u; // numeric error
  uint64_t CR0_WP = 1u << 16u; // write protect
  uint64_t CR0_AM = 1u << 18u; // alighnment mask
  uint64_t CR0_PG = 1u << 31u; // paging

  uint64_t CR4_PAE = 1u << 5u; // physical address extension
  uint64_t CR4_OSFXSR = 1u << 9u; //	os support for fxsave and fxrstor instructions
  uint64_t CR4_OSXMMEXCPT = 1u << 10u; // os support for unmasked simd floating point exceptions

  // see https://en.wikipedia.org/wiki/Control_register#EFER
  // EFER (Extended Feature Enable Register)
  uint64_t EFER_LME = 1u << 8u;  // long mode enable
  uint64_t EFER_LMA = 1u << 10u; // long mode active

  // Enable paging
  r[cr0].Reg64 = CR0_PE | CR0_MP | CR0_ET | CR0_NE | CR0_WP | CR0_AM | CR0_PG;
  r[cr3].Reg64 = pml4_address;
  r[cr4].Reg64 = CR4_PAE | CR4_OSFXSR | CR4_OSXMMEXCPT;
  r[efer].Reg64 = EFER_LME | EFER_LMA;

//  r[cr0].Reg64 = 0x80050033;
//  r[cr4].Reg64 = 0x6e8;
//  r[efer].Reg64 = 0xd01;

  // Set segment registers with long mode flag
  WHV_X64_SEGMENT_REGISTER &cs_segment = r[cs].Segment;
//  cs_segment.Selector = 0xffff;
  cs_segment.DescriptorPrivilegeLevel = 0;
  cs_segment.Long = 1;

  WHV_X64_SEGMENT_REGISTER &ss_segment = r[ss].Segment;
//  ss_segment.Selector = 0xffff;
  ss_segment.DescriptorPrivilegeLevel = 0;

  r[ds] = r[ss];
  r[es] = r[ss];
  r[gs] = r[ss];

  // Disable interrupts
  r[rflags].Reg64 = 0x002;

  // Set the actual PC and stack pointer
  r[rip].Reg64 = 0x0;
  r[rsp].Reg64 = 0x1000;

  set_registers(partition_handle, cpu_index, r);
}


uint64_t setup_long_paging(WHV_PARTITION_HANDLE handle)
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
  uint64_t NUM_LVL3_ENTRIES = 512;
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
  for (uint64_t i = 0; i < NUM_LVL3_ENTRIES - 1; i++) {
    //
    // Set the PDPTE to the next valid 1GB of RAM, creating a 1:1 map
    //
    pml3[i] = pte_template;
    pml3[i].page_frame_num = ((i * mobo::memory::_1GB) / USN_PAGE_SIZE);
  }

  //
  // We mark the last GB of RAM as off-limits
  // This corresponds to 0x0000`007FC0000000->0x0000`007FFFFFFFFF
  //
  pml3[511].present = 0;

  return mobo::memory::PML4_PHYSICAL_ADDRESS;
}

int main() {
  HRESULT hr;

  PAGE_SIZE = get_page_size();

  //
  // Create a Hyper-V External Partition
  //
  WHV_PARTITION_HANDLE handle = nullptr;
  hr = WHvCreatePartition(&handle);
  if (FAILED(hr)) {
    fprintf(stderr, "WHvCreatePartition: failed to create partition\n");
    exit(1);
  }

  //
  // Allow a single processor
  //
  WHV_PARTITION_PROPERTY prop = {.ProcessorCount = 1};

  hr = WHvSetPartitionProperty(
      handle,
      WHvPartitionPropertyCodeProcessorCount,
      &prop,
      sizeof(prop));

  if (FAILED(hr)) {
    fprintf(stderr, "WHvSetPartitionProperty: failed to set processor count\n");
    exit(1);
  }

  //
  // Activate the partition
  //
  hr = WHvSetupPartition(handle);
  if (FAILED(hr)) {
    fprintf(stderr, "WHvSetupPartition: failed to setup partition\n");
    exit(1);
  }

  //
  // Setup processor
  //
  hr = WHvCreateVirtualProcessor(handle, 0, 0);
  if (FAILED(hr)) {
    fprintf(stderr, "WHvCreateVirtualProcessor: failed to setup processor 0\n");
    exit(1);
  }

  //
  // Map file into guest
  //
  auto bin_path = "build/tests/double64.bin";
  HANDLE bin_file = CreateFile(
      bin_path,
      GENERIC_READ | GENERIC_EXECUTE,
      FILE_SHARE_READ,
      nullptr,
      OPEN_ALWAYS,
      0,
      nullptr);
  if (bin_file == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "CreateFile: failed to open executable file\n");
    exit(1);
  }

  LARGE_INTEGER file_size_raw = {};
  bool did_succeed = GetFileSizeEx(bin_file, &file_size_raw);
  if (!did_succeed) {
    fprintf(stderr, "GetFileSizeEx: failed to get size of bin file\n");
    exit(1);
  }
  int64_t file_size = file_size_raw.QuadPart;

  HANDLE bin_mapping = CreateFileMapping(
      bin_file,
      nullptr,
      PAGE_EXECUTE_READ,
      0,
      0,
      nullptr);
  if (bin_mapping == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "CreateFileMapping: failed to create file mapping executable file\n");
    exit(1);
  }

  //
  // Reserve region of virtual memory to map the file into
  //

  void *host_bin_addr = MapViewOfFile(
      bin_mapping,
      FILE_MAP_READ | FILE_MAP_EXECUTE,
      0,
      0,
      file_size);

  if (host_bin_addr == nullptr) {
    exit_last_error("MapViewOfFileEx");
  }

  //
  // Map file into guest memory
  //
  hr = WHvMapGpaRange(
      handle,
      host_bin_addr,
      0x0,
      file_size + (file_size % PAGE_SIZE == 0 ? 0 : PAGE_SIZE - (file_size % PAGE_SIZE)),
      WHvMapGpaRangeFlagRead | WHvMapGpaRangeFlagWrite | WHvMapGpaRangeFlagExecute);

  if (FAILED(hr)) {
    fprintf(stderr, "WHvMapGpaRange: failed to create mapping to binary file\n");
    exit(1);
  }

  printf("================= (0) INITIAL STATE ================= \n");
  dump_state(stdout, handle, 0);
  printf("===================================================== \n");

  //
  // Setup paging and long mode
  //
  uint64_t guest_paging_addr = setup_long_paging(handle);
  printf("paging addr -> 0x%llx\n", guest_paging_addr);
  reset_long(handle, 0, guest_paging_addr);

  printf("================= (1) SETUP LONG MODE =============== \n");
  dump_state(stdout, handle, 0);
  printf("===================================================== \n");

  //
  // Set initial state
  //
  WHV_REGISTER_VALUE r[NUM_REGISTERS] = {};
  get_registers(handle, 0, r);
  r[rax].Reg64 = 5;
  set_registers(handle, 0, r);

  printf("rax = 0x%llx\n", r[rax].Reg64);

  //
  // Run the VP
  //
  WHV_RUN_VP_EXIT_CONTEXT context = {};
  hr = WHvRunVirtualProcessor(handle, 0, &context, sizeof(context));
  if (FAILED(hr)) {
    fprintf(stderr, "WHvMapGpaRange: failed to create mapping to binary file\n");
    exit(1);
  }

  get_registers(handle, 0, r);

  printf("exit reason -> %d; rip = 0x%llx; rax = 0x%llx\n",
      context.ExitReason, context.VpContext.Rip, r[rax].Reg64);

  printf("================= (2) EXIT CONTEXT =================== \n");
  dump_state(stdout, handle, 0);
  printf("===================================================== \n");

  if (context.ExitReason == WHvRunVpExitReasonInvalidVpRegisterValue) {
    fprintf(stderr, "ERROR: Failed to run v-cpu: invalid register state\n");
    exit(1);
  }

  return 0;
}