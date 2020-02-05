#include <mobo/vcpu.h>

using namespace mobo;

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

static void cpu_dump_seg_cache(
    FILE *out,
    const char *name,
    mobo::segment const &seg)
{
  fprintf(out, "%-3s=%04x %016" PR Ix64 " %08x %d %02x %02x  %02x\n", name,
      seg.selector, (size_t)seg.base, seg.limit, seg.present, seg.db,
      seg.dpl, seg.type);
}

void vcpu::dump_state(FILE *out, char *mem) {
  mobo::regs regs = {};
  read_regs(regs);

  unsigned int eflags = regs.rflags;
#define GET(name) \
  *(uint64_t *)(((char *)&regs) + offsetof(struct kvm_regs, name))

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

  mobo::sregs sregs = {};
  read_sregs(sregs);

  fprintf(out, "    sel  base             limit    p db dpl type\n");
  cpu_dump_seg_cache(out, "ES", sregs.es);
  cpu_dump_seg_cache(out, "CS", sregs.cs);
  cpu_dump_seg_cache(out, "SS", sregs.ss);
  cpu_dump_seg_cache(out, "DS", sregs.ds);
  cpu_dump_seg_cache(out, "FS", sregs.fs);
  cpu_dump_seg_cache(out, "GS", sregs.gs);
  cpu_dump_seg_cache(out, "LDT", sregs.ldt);
  cpu_dump_seg_cache(out, "TR", sregs.tr);

  fprintf(out, "GDT=     %016" PRIx64 " %08x\n", (size_t)sregs.gdt.base,
      (int)sregs.gdt.limit);
  fprintf(out, "IDT=     %016" PRIx64 " %08x\n", (size_t)sregs.idt.base,
      (int)sregs.idt.limit);

  fprintf(out,
          "CR0=%016" PRIx64 " CR2=%016" PRIx64 " CR3=%016" PRIx64 " CR4=%08x\n",
      (size_t)sregs.cr0, (size_t)sregs.cr2, (size_t)sregs.cr3,
      (int)sregs.cr4);

  fprintf(out, "EFER=%016" PRIx64 "\n", (size_t)sregs.efer);

  fpu_regs fpu = {};
  read_fregs(fpu);

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

