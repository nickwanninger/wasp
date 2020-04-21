#include <inttypes.h>  // PRIx64
#include <wasp/machine.h>
#include <wasp/vcpu.h>

using namespace wasp;

std::string vcpu::read_string(u64 gva) {
  std::string str;
  guest_array<char> mem(this, gva);

  try {
    for (int i = 0; true; i++) {
      char c = mem[i];
      if (c == '\0') break;
      str.push_back(c);
    }
  } catch (std::exception &e) {
    return str;
  }
  return str;
}

int vcpu::read_guest(u64 gva, void *buf, size_t len) {
  auto mem = guest_array<char>(this, gva);
  int i = 0;
  try {
    for (i = 0; i < len; i++) {
      ((char *)buf)[i] = mem[i];
    }
  } catch (std::exception &e) {
    // catch an exception that occurs when the virtual address isn't mapped
    return i;
  }
  return i;
}

static void cpu_dump_seg_cache(FILE *out, const char *name,
                               wasp::segment_t const &seg) {
  fprintf(out,        "%-3s=%04x %016" PRIx64 " %08x %d %02x   %1x   %d   %d     %d     %d    %d\n",
          name,
          seg.selector,
          (size_t) seg.base,
          seg.limit,
          seg.present,
          seg.type,
          seg.dpl,
          seg.s,
          seg.granularity,
          seg.db,
          seg.long_mode,
          seg.available);
}

void vcpu::dump_state(FILE *out) {
  wasp::regs_t regs = {};
  wasp::regs_special_t s = {};
  read_regs_into(regs);
  read_regs_special_into(s);

  unsigned int eflags = regs.rflags;
#define GET(name) (regs.name)

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

          GET(rax), GET(rbx), GET(rcx), GET(rdx),
          GET(rsi), GET(rdi), GET(rbp), GET(rsp),
          GET(r8), GET(r9), GET(r10), GET(r11),
          GET(r12), GET(r13), GET(r14), GET(r15),
          GET(rip), eflags,
          eflags & DF_MASK ? 'D' : '-',
          eflags & CC_O ? 'O' : '-', eflags & CC_S ? 'S' : '-',
          eflags & CC_Z ? 'Z' : '-', eflags & CC_A ? 'A' : '-',
          eflags & CC_P ? 'P' : '-', eflags & CC_C ? 'C' : '-');

  fprintf(out, "                                             (C/E) (G)   (D/B) (L) (AVL)\n");
  fprintf(out, "    sel  base             limit    p type dpl u/s 8b/4k 16/32 long avl\n");
  cpu_dump_seg_cache(out, "ES", s.es);
  cpu_dump_seg_cache(out, "CS", s.cs);
  cpu_dump_seg_cache(out, "SS", s.ss);
  cpu_dump_seg_cache(out, "DS", s.ds);
  cpu_dump_seg_cache(out, "FS", s.fs);
  cpu_dump_seg_cache(out, "GS", s.gs);
  cpu_dump_seg_cache(out, "LDT", s.ldt);
  cpu_dump_seg_cache(out, "TR", s.tr);

  wasp::dtable_t &gdt_register = s.gdt;
  fprintf(out, "GDT=     %016" PRIx64 " %08x\n", (size_t)gdt_register.base,
          (int) gdt_register.limit);

  wasp::dtable_t &idt_register = s.idt;
  fprintf(out, "IDT=     %016" PRIx64 " %08x\n", (size_t)idt_register.base,
          (int) idt_register.limit);

  fprintf(out,
          "CR0=%016" PRIx64 " CR2=%016" PRIx64 " CR3=%016" PRIx64 " CR4=%08x\n",
          (size_t)s.cr0, (size_t)s.cr2, (size_t)s.cr3,
          (int)s.cr4);

  fprintf(out, "EFER=%016" PRIx64 "\n", (size_t)s.efer);

  regs_fpu_t fpu = {};
  read_regs_fpu_into(fpu);
  {
    int written = 0;
    for (int i = 0; i < 16; i++) {
      fprintf(out, "XMM%-2d=", i);

      for (int j = 0; j < 16; j++) {
        fprintf(out, "%02x", fpu.xmm[i][j]);
      }
      written++;

      if (written == 2) {
        fprintf(out, "\n");
        written = 0;
      } else {
        fprintf(out, " ");
      }
    }
  }

#undef GET
}

