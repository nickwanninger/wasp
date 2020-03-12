#include "compiler_defs.h"
#include <WinHvPlatform.h>
#include <winerror.h>

#include <stdexcept>
#include "platform/memory.h"
#include "platform/windows/hyperv_vcpu.h"

using namespace mobo;
using namespace mobo::memory;

hyperv_vcpu::hyperv_vcpu(
    WHV_PARTITION_HANDLE partition_handle,
    uint32_t cpu_index)
    : partition_handle_(partition_handle)
    , cpu_index_(cpu_index)
{
  HRESULT result = WHvCreateVirtualProcessor(partition_handle, cpu_index, 0);
  if (FAILED(result)) {
    throw std::runtime_error("failed to create Hyper-V virtual processor");
  }
}

const WHV_REGISTER_NAME k_hyperv_mapping_regnames[] = {
    WHvX64RegisterRax,
    WHvX64RegisterRbx,
    WHvX64RegisterRcx,
    WHvX64RegisterRdx,

    WHvX64RegisterRsi,
    WHvX64RegisterRdi,
    WHvX64RegisterRsp,
    WHvX64RegisterRbp,

    WHvX64RegisterR8,
    WHvX64RegisterR9,
    WHvX64RegisterR10,
    WHvX64RegisterR11,
    WHvX64RegisterR12,
    WHvX64RegisterR13,
    WHvX64RegisterR14,
    WHvX64RegisterR15,

    WHvX64RegisterRip,
    WHvX64RegisterRflags,
};

enum hyperv_mapping_reg_index : uint32_t {
  rax = 0,
  rbx,
  rcx,
  rdx,

  rsi,
  rdi,
  rsp,
  rbp,

  r8,
  r9,
  r10,
  r11,
  r12,
  r13,
  r14,
  r15,

  rip,
  rflags
};

const WHV_REGISTER_NAME k_hyperv_mapping_reg_special_names[] = {
    WHvX64RegisterCs,
    WHvX64RegisterDs,
    WHvX64RegisterEs,
    WHvX64RegisterFs,
    WHvX64RegisterGs,
    WHvX64RegisterSs,
    WHvX64RegisterTr,

    WHvX64RegisterGdtr,
    WHvX64RegisterIdtr,
    WHvX64RegisterLdtr,

    WHvX64RegisterCr0,
    WHvX64RegisterCr2,
    WHvX64RegisterCr3,
    WHvX64RegisterCr4,
    WHvX64RegisterCr8,

    WHvX64RegisterEfer,
    WHvX64RegisterApicBase,
};

enum hyperv_mapping_sreg_index : uint32_t {
  cs = 0, ds, es, fs, gs, ss, tr,
  gdt, idt, ldt,
  cr0, cr2, cr3, cr4, cr8,
  efer, apic_base,
};

constexpr uint32_t k_hyperv_mapping_regs_count = std::extent<decltype(k_hyperv_mapping_regnames)>::value;
constexpr uint32_t k_hyperv_mapping_regs_special_count = std::extent<decltype(k_hyperv_mapping_reg_special_names)>::value;

typedef WHV_REGISTER_VALUE hyperv_reg_values_t[k_hyperv_mapping_regs_count];
typedef WHV_REGISTER_VALUE hyperv_reg_special_values_t[k_hyperv_mapping_regs_special_count];

void hyperv_vcpu::read_regs(mobo::regs_t &r)
{
  HRESULT result;

  hyperv_reg_values_t values = {};
  result = WHvGetVirtualProcessorRegisters(
      partition_handle_,
      cpu_index_,
      k_hyperv_mapping_regnames,
      k_hyperv_mapping_regs_count,
      values);

  if (FAILED(result)) {
    throw std::runtime_error("failed to read Hyper-V registers");
  }

  r.rax = values[hyperv_mapping_reg_index::rax].Reg64;
  r.rbx = values[hyperv_mapping_reg_index::rbx].Reg64;
  r.rcx = values[hyperv_mapping_reg_index::rcx].Reg64;
  r.rdx = values[hyperv_mapping_reg_index::rdx].Reg64;

  r.rsi = values[hyperv_mapping_reg_index::rsi].Reg64;
  r.rdi = values[hyperv_mapping_reg_index::rdi].Reg64;
  r.rsp = values[hyperv_mapping_reg_index::rsp].Reg64;
  r.rbp = values[hyperv_mapping_reg_index::rbp].Reg64;

  r.r8 = values[hyperv_mapping_reg_index::r8].Reg64;
  r.r9 = values[hyperv_mapping_reg_index::r9].Reg64;
  r.r10 = values[hyperv_mapping_reg_index::r10].Reg64;
  r.r11 = values[hyperv_mapping_reg_index::r11].Reg64;
  r.r12 = values[hyperv_mapping_reg_index::r12].Reg64;
  r.r13 = values[hyperv_mapping_reg_index::r13].Reg64;
  r.r14 = values[hyperv_mapping_reg_index::r14].Reg64;
  r.r15 = values[hyperv_mapping_reg_index::r15].Reg64;

  r.rip = values[hyperv_mapping_reg_index::rip].Reg64;
  r.rflags = values[hyperv_mapping_reg_index::rflags].Reg64;
}

void hyperv_vcpu::write_regs(mobo::regs_t &r)
{
  hyperv_reg_values_t values = {};
  values[hyperv_mapping_reg_index::rax].Reg64 = r.rax;
  values[hyperv_mapping_reg_index::rbx].Reg64 = r.rbx;
  values[hyperv_mapping_reg_index::rcx].Reg64 = r.rcx;
  values[hyperv_mapping_reg_index::rdx].Reg64 = r.rdx;

  values[hyperv_mapping_reg_index::rsi].Reg64 = r.rsi;
  values[hyperv_mapping_reg_index::rdi].Reg64 = r.rdi;
  values[hyperv_mapping_reg_index::rsp].Reg64 = r.rsp;
  values[hyperv_mapping_reg_index::rbp].Reg64 = r.rbp;

  values[hyperv_mapping_reg_index::r8].Reg64 = r.r8;
  values[hyperv_mapping_reg_index::r9].Reg64 = r.r9;
  values[hyperv_mapping_reg_index::r10].Reg64 = r.r10;
  values[hyperv_mapping_reg_index::r11].Reg64 = r.r11;
  values[hyperv_mapping_reg_index::r12].Reg64 = r.r12;
  values[hyperv_mapping_reg_index::r13].Reg64 = r.r13;
  values[hyperv_mapping_reg_index::r14].Reg64 = r.r14;
  values[hyperv_mapping_reg_index::r15].Reg64 = r.r15;

  values[hyperv_mapping_reg_index::rip].Reg64 = r.rip;
  values[hyperv_mapping_reg_index::rflags].Reg64 = r.rflags;

  HRESULT result;
  result = WHvSetVirtualProcessorRegisters(
      partition_handle_,
      cpu_index_,
      k_hyperv_mapping_regnames,
      k_hyperv_mapping_regs_count,
      values);

  if (FAILED(result)) {
    throw std::runtime_error("failed to write Hyper-V registers");
  }
}

void hyperv_vcpu::read_sregs(mobo::regs_special_t &r) {
  HRESULT result;

  hyperv_reg_special_values_t values = {};
  result = WHvGetVirtualProcessorRegisters(
      partition_handle_,
      cpu_index_,
      k_hyperv_mapping_reg_special_names,
      k_hyperv_mapping_regs_special_count,
      values);

  if (FAILED(result)) {
    throw std::runtime_error("failed to read Hyper-V special registers");
  }

  static auto copy_segment = [](mobo::segment_t &dst, WHV_REGISTER_VALUE &src_value) {
    WHV_X64_SEGMENT_REGISTER &src = src_value.Segment;

    dst.base = src.Base;
    dst.limit = src.Limit;
    dst.selector = src.Selector;

    dst.type = src.SegmentType;
    dst.present = src.Present;
    dst.dpl = src.DescriptorPrivilegeLevel;
    dst.db = src.Default;
    dst.s = src.NonSystemSegment;
    dst.long_mode = src.Long;
    dst.granularity = src.Granularity;
    dst.available = src.Available;
  };

  copy_segment(r.cs, values[hyperv_mapping_sreg_index::cs]);
  copy_segment(r.ds, values[hyperv_mapping_sreg_index::ds]);
  copy_segment(r.es, values[hyperv_mapping_sreg_index::es]);
  copy_segment(r.fs, values[hyperv_mapping_sreg_index::fs]);
  copy_segment(r.gs, values[hyperv_mapping_sreg_index::gs]);
  copy_segment(r.ss, values[hyperv_mapping_sreg_index::ss]);
  copy_segment(r.tr, values[hyperv_mapping_sreg_index::tr]);
  copy_segment(r.ldt, values[hyperv_mapping_sreg_index::ldt]);

  WHV_X64_TABLE_REGISTER gdt = values[hyperv_mapping_sreg_index::gdt].Table;
  r.gdt.base = gdt.Base;
  r.gdt.limit = gdt.Limit;

  WHV_X64_TABLE_REGISTER idt = values[hyperv_mapping_sreg_index::idt].Table;
  r.idt.base = idt.Base;
  r.idt.limit = idt.Limit;
	
  r.cr0 = values[hyperv_mapping_sreg_index::cr0].Reg64;
  r.cr2 = values[hyperv_mapping_sreg_index::cr2].Reg64;
  r.cr3 = values[hyperv_mapping_sreg_index::cr3].Reg64;
  r.cr4 = values[hyperv_mapping_sreg_index::cr4].Reg64;
  r.cr8 = values[hyperv_mapping_sreg_index::cr8].Reg64;

  r.efer = values[hyperv_mapping_sreg_index::efer].Reg64;
  r.apic_base = values[hyperv_mapping_sreg_index::apic_base].Reg64;

  // TODO
//  for (int i = 0; i < (NR_INTERRUPTS + 63) / 64; i++) {
//    r.interrupt_bitmap[i] = sr.interrupt_bitmap[i];
//  }
}

void hyperv_vcpu::write_regs_special(mobo::regs_special_t &r)
{
  hyperv_reg_special_values_t values = {};

  static auto copy_segment = [](mobo::segment_t &src, WHV_REGISTER_VALUE &dst_value) {
    WHV_X64_SEGMENT_REGISTER &dst = dst_value.Segment;

    dst.Base = src.base;
    dst.Limit = src.limit;
    dst.Selector = src.selector;

    dst.SegmentType = src.type;
    dst.Present = src.present;
    dst.DescriptorPrivilegeLevel = src.dpl;
    dst.Default = src.db;
    dst.NonSystemSegment = src.s;
    dst.Long = src.long_mode;
    dst.Granularity = src.granularity;
    dst.Available = src.available;
  };

  copy_segment(r.cs, values[cs]);
  copy_segment(r.ds, values[ds]);
  copy_segment(r.es, values[es]);
  copy_segment(r.fs, values[fs]);
  copy_segment(r.gs, values[gs]);
  copy_segment(r.ss, values[ss]);
  copy_segment(r.tr, values[tr]);
  copy_segment(r.ldt, values[ldt]);

  WHV_REGISTER_VALUE gdt_value = {};
  WHV_X64_TABLE_REGISTER &gdt_reg = gdt_value.Table;
  gdt_reg.Base = r.gdt.base;
  gdt_reg.Limit = r.gdt.limit;
  values[gdt] = gdt_value;

  WHV_REGISTER_VALUE idt_value = {};
  WHV_X64_TABLE_REGISTER &idt_reg = idt_value.Table;
  idt_reg.Base = r.idt.base;
  idt_reg.Limit = r.idt.limit;
  values[idt] = idt_value;

  values[cr0].Reg64 = r.cr0;
  values[cr2].Reg64 = r.cr2;
  values[cr3].Reg64 = r.cr3;
  values[cr4].Reg64 = r.cr4;
  values[cr8].Reg64 = r.cr8;

  values[efer].Reg64 = r.efer;
  values[apic_base].Reg64 = r.apic_base;

  // TODO
//  for (int i = 0; i < (NR_INTERRUPTS + 63) / 64; i++) {
//    r.interrupt_bitmap[i] = sr.interrupt_bitmap[i];
//  }

  HRESULT result;
  result = WHvSetVirtualProcessorRegisters(
      partition_handle_,
      cpu_index_,
      k_hyperv_mapping_reg_special_names,
      k_hyperv_mapping_regs_special_count,
      values);

  if (FAILED(result)) {
    throw std::runtime_error("failed to write Hyper-V special registers");
  }
}

void hyperv_vcpu::read_fregs(mobo::regs_fpu_t &regs) {
  // TODO
}

void hyperv_vcpu::write_fregs(mobo::regs_fpu_t &regs) {
  // TODO
}

void *hyperv_vcpu::translate_address(u64 gva)
{
  // TODO
  return nullptr;
}

void hyperv_vcpu::reset()
{
  // TODO: deal with protected mode, and figure out why it's broken...
  reset_long();
}

WHV_RUN_VP_EXIT_CONTEXT hyperv_vcpu::run() {
  WHV_RUN_VP_EXIT_CONTEXT exit_context = {};
  HRESULT hr = WHvRunVirtualProcessor(
      partition_handle_,
      cpu_index_,
      &exit_context,
      sizeof(exit_context));

  if (FAILED(hr)) {
    throw std::runtime_error("failed to run virtual processor: WHvRunVirtualProcessor");
  }

  return exit_context;
}

void hyperv_vcpu::reset_protected()
{
  struct mobo::regs_t r = {0};
  struct mobo::regs_special_t s = {0};

  struct mobo::segment_t clone = {
      .limit = 0xffff,
      .type = 0x03,
      .present = 1,
  };

  s.cs = {
      .limit = 0xffff,
      .selector = 0x1000,
      .type = 0x0b,
      .present = 1,
  };

  s.tr = {
      .limit = 0xffff,
      .type = 0x0b,
      .present = 1,
  };

  s.ldt = {0};
  s.es = clone;
  s.ss = clone;
  s.ds = clone;
  s.es = clone;
  s.fs = clone;
  s.gdt.limit = 0xffff;
  s.idt.limit = 0xffff;

  r.rflags = 0x2;
  s.cr0 = 0x60000010;

  write_regs(r);
  write_regs_special(s);
}

void hyperv_vcpu::reset_long()
{
  struct mobo::regs_t r = {};
  struct mobo::regs_special_t s = {};
  read_regs(r);
  read_sregs(s);

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
  s.cr3 = PML4_PHYSICAL_ADDRESS;
  s.cr4 = CR4_PAE | CR4_OSFXSR | CR4_OSXMMEXCPT;
  s.cr0 = CR0_PE | CR0_MP | CR0_ET | CR0_NE | CR0_WP | CR0_AM | CR0_PG;
  s.efer = EFER_LME | EFER_LMA;

  // Set segment registers with long mode flag
  s.cs = {
      .base = 0,
      .limit = 0xffffffff,
      .selector = 1u << 3u,
      .type = 11,
      .present = 1,
      .dpl = 0,
      .long_mode = 1,
      .granularity = 1,
  };

  s.ds = {
      .base = 0,
      .limit = 0xffffffff,
      .selector = 2u << 3u,
      .type = 3,
      .present = 1,
      .dpl = 0,
      .long_mode = 1,
      .granularity = 1,
  };

  // TODO: Assumes this wasn't touched during execution
  struct segment_t segment = s.ss;
  s.ds = segment;
  s.es = segment;
  s.gs = segment;

  // Disable interrupts
  r.rflags = 0x002;

  // Set the actual PC and stack pointer
  r.rip = 0x0;
  r.rsp = 0x1000;

  write_regs(r);
  write_regs_special(s);
}

