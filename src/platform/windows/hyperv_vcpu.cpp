#include <WinHvPlatform.h>
#include <winnt.h>
#include <winerror.h>
#include <stdexcept>

#include "platform/windows/hyperv_vcpu.h"
#include "compiler_defs.h"

using namespace mobo;

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

enum struct hyperv_mapping_reg_index : uint32_t {
  rax = 0, rbx, rcx, rdx,
  rsi, rdi, rsp, rbp,
  r8, r9, r10, r11, r12, r13, r14, r15,
  rip, rflags
};

const WHV_REGISTER_NAME k_hyperv_mapping_sregnames[] = {
    WHvX64RegisterCs,
    WHvX64RegisterDs,
    WHvX64RegisterEs,
    WHvX64RegisterFs,
    WHvX64RegisterGs,
    WHvX64RegisterSs,
    WHvX64RegisterTr,

    WHvX64RegisterGdtr,
    WHvX64RegisterIdtr,

    WHvX64RegisterCr0,
    WHvX64RegisterCr2,
    WHvX64RegisterCr3,
    WHvX64RegisterCr4,
    WHvX64RegisterCr8,

    WHvX64RegisterEfer,
    WHvX64RegisterApicBase,
};

enum struct hyperv_mapping_sreg_index : uint32_t {
  cs = 0, ds, es, fs, gs, ss, tr,
  gdt, idt,
  cr0, cr2, cr3, cr4, cr8,
  efer, apic_base,
};

constexpr uint32_t k_hyperv_mapping_regs_count = std::extent<decltype(k_hyperv_mapping_regnames)>::value;
constexpr uint32_t k_hyperv_mapping_sregs_count = std::extent<decltype(k_hyperv_mapping_sregnames)>::value;

typedef WHV_REGISTER_VALUE hyperv_reg_values_t[k_hyperv_mapping_regs_count];
typedef WHV_REGISTER_VALUE hyperv_sreg_values_t[k_hyperv_mapping_sregs_count];

void hyperv_vcpu::read_regs(mobo::regs &r)
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

  r.rax = values[hyperv_mapping_reg_index::rax];
  r.rbx = values[hyperv_mapping_reg_index::rbx];
  r.rcx = values[hyperv_mapping_reg_index::rcx];
  r.rdx = values[hyperv_mapping_reg_index::rdx];

  r.rsi = values[hyperv_mapping_reg_index::rsi];
  r.rdi = values[hyperv_mapping_reg_index::rdi];
  r.rsp = values[hyperv_mapping_reg_index::rsp];
  r.rbp = values[hyperv_mapping_reg_index::rbp];

  r.r8 = values[hyperv_mapping_reg_index::r8];
  r.r9 = values[hyperv_mapping_reg_index::r9];
  r.r10 = values[hyperv_mapping_reg_index::r10];
  r.r11 = values[hyperv_mapping_reg_index::r11];
  r.r12 = values[hyperv_mapping_reg_index::r12];
  r.r13 = values[hyperv_mapping_reg_index::r13];
  r.r14 = values[hyperv_mapping_reg_index::r14];
  r.r15 = values[hyperv_mapping_reg_index::r15];

  r.rip = values[hyperv_mapping_reg_index::rip];
  r.rflags = values[hyperv_mapping_reg_index::rflags];
}

void hyperv_vcpu::write_regs(mobo::regs &r)
{
  hyperv_reg_values_t values = {};
  values[hyperv_mapping_reg_index::rax] = r.rax;
  values[hyperv_mapping_reg_index::rbx] = r.rbx;
  values[hyperv_mapping_reg_index::rcx] = r.rcx;
  values[hyperv_mapping_reg_index::rdx] = r.rdx;

  values[hyperv_mapping_reg_index::rsi] = r.rsi;
  values[hyperv_mapping_reg_index::rdi] = r.rdi;
  values[hyperv_mapping_reg_index::rsp] = r.rsp;
  values[hyperv_mapping_reg_index::rbp] = r.rbp;

  values[hyperv_mapping_reg_index::r8] = r.r8;
  values[hyperv_mapping_reg_index::r9] = r.r9;
  values[hyperv_mapping_reg_index::r10] = r.r10;
  values[hyperv_mapping_reg_index::r11] = r.r11;
  values[hyperv_mapping_reg_index::r12] = r.r12;
  values[hyperv_mapping_reg_index::r13] = r.r13;
  values[hyperv_mapping_reg_index::r14] = r.r14;
  values[hyperv_mapping_reg_index::r15] = r.r15;

  values[hyperv_mapping_reg_index::rip] = r.rip;
  values[hyperv_mapping_reg_index::rflags] = r.rflags;

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

void hyperv_vcpu::read_sregs(mobo::sregs &r) {
  HRESULT result;

  hyperv_reg_values_t values = {};
  result = WHvGetVirtualProcessorRegisters(
      partition_handle_,
      cpu_index_,
      k_hyperv_mapping_sregnames,
      k_hyperv_mapping_sregs_count,
      values);

  if (FAILED(result)) {
    throw std::runtime_error("failed to read Hyper-V special registers");
  }

  static auto copy_segment = [](mobo::segment &dst, WHV_REGISTER_VALUE &src_value) {
    WHV_X64_SEGMENT_REGISTER src = src_value.Segment;

    dst.base = src.Base;
    dst.limit = src.Limit;
    dst.selector = src.Selector;

    dst.type = src.SegmentType;
    dst.present = src.Present;
    dst.dpl = src.DescriptorPrivilegeLevel;
    dst.db = src.Default;
    dst.s = src.NonSystemSegment;
    dst.l = src.Long;
    dst.g = src.Granularity;
    dst.avl = src.Available;
    dst.unusable = src.Reserved;
  };

  copy_segment(r.cs, values[hyperv_mapping_sreg_index::cs]);
  copy_segment(r.ds, values[hyperv_mapping_sreg_index::ds]);
  copy_segment(r.es, values[hyperv_mapping_sreg_index::es]);
  copy_segment(r.fs, values[hyperv_mapping_sreg_index::fs]);
  copy_segment(r.gs, values[hyperv_mapping_sreg_index::gs]);
  copy_segment(r.ss, values[hyperv_mapping_sreg_index::ss]);
  copy_segment(r.tr, values[hyperv_mapping_sreg_index::tr]);

  WHV_X64_TABLE_REGISTER gdt = values[hyperv_mapping_sreg_index::gdt].Table;
  r.gdt.base = gdt.Base;
  r.gdt.limit = gdt.Limit;

  WHV_X64_TABLE_REGISTER idt = values[hyperv_mapping_sreg_index::idt].Table;
  r.idt.base = idt.Base;
  r.idt.limit = idt.Limit;

  r.cr0 = values[hyperv_mapping_sreg_index::cr0];
  r.cr2 = values[hyperv_mapping_sreg_index::cr2];
  r.cr3 = values[hyperv_mapping_sreg_index::cr3];
  r.cr4 = values[hyperv_mapping_sreg_index::cr4];
  r.cr8 = values[hyperv_mapping_sreg_index::cr8];

  r.efer = values[hyperv_mapping_sreg_index::efer];
  r.apic_base = values[hyperv_mapping_sreg_index::apic_base];

  // TODO
//  for (int i = 0; i < (NR_INTERRUPTS + 63) / 64; i++) {
//    r.interrupt_bitmap[i] = sr.interrupt_bitmap[i];
//  }
}

void hyperv_vcpu::write_sregs(mobo::sregs &r)
{
  hyperv_reg_values_t values = {};

  static auto copy_segment = [](mobo::segment &src, WHV_REGISTER_VALUE &dst_value) {
    WHV_X64_SEGMENT_REGISTER dst = dst_value.Segment;

    dst.Base = src.base;
    dst.Limit = src.limit;
    dst.Selector = src.selector;

    dst.SegmentType = src.type;
    dst.Present = src.present;
    dst.DescriptorPrivilegeLevel = src.dpl;
    dst.Default = src.db;
    dst.NonSystemSegment = src.s;
    dst.Long = src.l;
    dst.Granularity = src.g;
    dst.Available = src.avl;
    dst.Reserved = src.unusable;
  };

  copy_segment(r.cs, values[hyperv_mapping_sreg_index::cs]);
  copy_segment(r.ds, values[hyperv_mapping_sreg_index::ds]);
  copy_segment(r.es, values[hyperv_mapping_sreg_index::es]);
  copy_segment(r.fs, values[hyperv_mapping_sreg_index::fs]);
  copy_segment(r.gs, values[hyperv_mapping_sreg_index::gs]);
  copy_segment(r.ss, values[hyperv_mapping_sreg_index::ss]);
  copy_segment(r.tr, values[hyperv_mapping_sreg_index::tr]);

  WHV_REGISTER_VALUE gdt_value = {};
  WHV_X64_SEGMENT_REGISTER &gdt = &gdt_value.Table;
  gdt.Base = r.gdt.base;
  gdt.Limit = r.gdt.limit;
  values[hyperv_mapping_sreg_index::gdt] = gdt_value;

  WHV_REGISTER_VALUE idt_value = {};
  WHV_X64_TABLE_REGISTER &idt = &idt_value.Table;
  idt.Base = r.idt.base;
  idt.Limit = r.idt.limit;

  values[hyperv_mapping_sreg_index::cr0] = r.cr0;
  values[hyperv_mapping_sreg_index::cr2] = r.cr2;
  values[hyperv_mapping_sreg_index::cr3] = r.cr3;
  values[hyperv_mapping_sreg_index::cr4] = r.cr4;
  values[hyperv_mapping_sreg_index::cr8] = r.cr8;

  values[hyperv_mapping_sreg_index::efer] = r.efer;
  values[hyperv_mapping_sreg_index::apic_base] = r.apic_base;

  // TODO
//  for (int i = 0; i < (NR_INTERRUPTS + 63) / 64; i++) {
//    r.interrupt_bitmap[i] = sr.interrupt_bitmap[i];
//  }

  HRESULT result;
  result = WHvSetVirtualProcessorRegisters(
      partition_handle_,
      cpu_index_,
      k_hyperv_mapping_sregnames,
      k_hyperv_mapping_sregs_count,
      values);

  if (FAILED(result)) {
    throw std::runtime_error("failed to write Hyper-V special registers");
  }
}

void hyperv_vcpu::read_fregs(mobo::fpu_regs &regs) {
  // TODO
}

void hyperv_vcpu::write_fregs(mobo::fpu_regs &regs) {
  // TODO
}

void *hyperv_vcpu::translate_address(u64 gva)
{
  // TODO
  return nullptr;
}

void hyperv_vcpu::reset()
{
  // TODO
}

