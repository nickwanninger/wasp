#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NR_INTERRUPTS 256

// general purpose registers
typedef struct wasp_regs_t {
  u64 rax, rbx, rcx, rdx;
  u64 rsi, rdi, rsp, rbp;
  u64 r8, r9, r10, r11;
  u64 r12, r13, r14, r15;
  u64 rip, rflags;
} wasp_regs_t;

// memory segmentation information
typedef struct wasp_segment_t {
  u64 base;
  u32 limit;
  u16 selector;
  u8 type;
  u8 present, dpl, db, s, long_mode, granularity, available;
  u8 unusable;
} wasp_segment_t;

typedef struct wasp_dtable_t {
  u64 base;
  u16 limit;
} wasp_dtable_t;

// special purpose registers
typedef struct wasp_regs_special_t {
  /* out (KVM_GET_SREGS) / in (KVM_SET_SREGS) */
  struct wasp_segment_t cs, ds, es, fs, gs, ss;
  struct wasp_segment_t tr, ldt;
  struct wasp_dtable_t gdt, idt;
  u64 cr0, cr2, cr3, cr4, cr8;
  u64 efer;
  u64 apic_base;
  u64 interrupt_bitmap[(NR_INTERRUPTS + 63) / 64];
} wasp_regs_special_t;

// Floating point registers
typedef struct wasp_regs_fpu_t {
  u8 fpr[8][16];
  u16 fcw;
  u16 fsw;
  u8 ftwx; /* in fxsave format */
  u16 last_opcode;
  u64 last_ip;
  u64 last_dp;
  u8 xmm[16][16];
  u32 mxcsr;
} wasp_regs_fpu_t;


#ifdef __cplusplus
} // extern "C"
#endif
