#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>  // PRIx64
#include <linux/kvm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <stdexcept>

#include <wasp/multiboot.h>
#include <wasp/platform/linux/kvm.h>

using namespace wasp;

#define DEFINE_KVM_EXIT_REASON(reason) [reason] = #reason

const char *kvm_exit_reasons[] = {
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_UNKNOWN),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_EXCEPTION),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_IO),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_HYPERCALL),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_DEBUG),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_HLT),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_MMIO),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_IRQ_WINDOW_OPEN),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_SHUTDOWN),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_FAIL_ENTRY),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_INTR),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_SET_TPR),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_TPR_ACCESS),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_S390_SIEIC),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_S390_RESET),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_DCR),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_NMI),
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_INTERNAL_ERROR),
#ifdef CONFIG_PPC64
    DEFINE_KVM_EXIT_REASON(KVM_EXIT_PAPR_HCALL),
#endif
};
#undef DEFINE_KVM_EXIT_REASON

struct cpuid_regs {
  int eax;
  int ebx;
  int ecx;
  int edx;
};
#define MAX_KVM_CPUID_ENTRIES 100
static void filter_cpuid(struct kvm_cpuid2 *);

kvm_machine::kvm_machine(int kvmfd, int ncpus) : kvmfd(kvmfd), ncpus(ncpus) {
  assert(ncpus == 1);  // for now...

  int ret;

  ret = ioctl(kvmfd, KVM_GET_API_VERSION, NULL);
  if (ret == -1) throw std::runtime_error("KVM_GET_API_VERSION");
  if (ret != 12) throw std::runtime_error("KVM_GET_API_VERSION invalid");

  ret = ioctl(kvmfd, KVM_CHECK_EXTENSION, KVM_CAP_USER_MEMORY);
  if (ret == -1) throw std::runtime_error("KVM_CHECK_EXTENSION");
  if (!ret)
    throw std::runtime_error(
        "Required extension KVM_CAP_USER_MEM not available");

  // Next, we need to create a virtual machine (VM), which represents everything
  // associated with one emulated system, including memory and one or more CPUs.
  // KVM gives us a handle to this VM in the form of a file descriptor:
  vmfd = ioctl(kvmfd, KVM_CREATE_VM, (unsigned long)0);

  init_cpus();
}

wasp::kvm_machine::~kvm_machine() {
  // need to close the vmfd, and all cpu fds
  for (auto &cpu : cpus) {
    close(cpu.cpufd);
    munmap(cpu.kvm_run, cpu.run_size);
  }
  close(vmfd);
  munmap(mem, memsize);
}

void kvm_machine::init_cpus(void) {
  int kvm_run_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, nullptr);
  // get cpuid info
  struct kvm_cpuid2 *kvm_cpuid;

  kvm_cpuid = (kvm_cpuid2 *)calloc(
      1,
      sizeof(*kvm_cpuid) + MAX_KVM_CPUID_ENTRIES * sizeof(*kvm_cpuid->entries));
  kvm_cpuid->nent = MAX_KVM_CPUID_ENTRIES;
  if (ioctl(kvmfd, KVM_GET_SUPPORTED_CPUID, kvm_cpuid) < 0)
    throw std::runtime_error("KVM_GET_SUPPORTED_CPUID failed");

  filter_cpuid(kvm_cpuid);

  for (int i = 0; i < ncpus; i++) {
    int vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, i);

    // init the cpuid
    int cpuid_err = ioctl(vcpufd, KVM_SET_CPUID2, kvm_cpuid);
    if (cpuid_err != 0) {
      printf("%d %d\n", cpuid_err, errno);
      throw std::runtime_error("KVM_SET_CPUID2 failed");
    }

    auto *run = (struct kvm_run *)mmap(
        NULL, kvm_run_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpufd, 0);

    struct kvm_regs regs = {
        .rflags = 0x2,
    };
    ioctl(vcpufd, KVM_SET_REGS, &regs);

    struct kvm_sregs sregs;
    ioctl(vcpufd, KVM_GET_SREGS, &sregs);
    sregs.cs.base = 0;
    ioctl(vcpufd, KVM_SET_SREGS, &sregs);

    kvm_vcpu cpu;
    cpu.cpufd = vcpufd;
    cpu.kvm_run = run;
    cpu.run_size = kvm_run_size;

    ioctl(vcpufd, KVM_GET_REGS, &cpu.initial_regs);
    ioctl(vcpufd, KVM_GET_SREGS, &cpu.initial_sregs);
    ioctl(vcpufd, KVM_GET_FPU, &cpu.initial_fpu);

    // cpu.dump_state(stdout);
    cpus.push_back(cpu);
  }

  free(kvm_cpuid);
}

void kvm_machine::attach_bank(ram_bank &&bnk) {
  struct kvm_userspace_memory_region code_region = {
      .slot = (uint32_t)ram.size(),
      .guest_phys_addr = bnk.guest_phys_addr,
      .memory_size = bnk.size,
      .userspace_addr = (uint64_t)bnk.host_addr,
  };
  ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &code_region);
  ram.push_back(std::move(bnk));
}

void kvm_machine::allocate_ram(size_t nbytes) {
  if (this->mem != nullptr) {
    munmap(this->mem, this->memsize);
  }
  this->mem = mmap(NULL, nbytes, PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  this->memsize = nbytes;

  for (auto &cpu : cpus) {
    cpu.mem = (char *)mem;
    cpu.memsize = nbytes;
  }

  ram_bank bnk;
  bnk.guest_phys_addr = 0x0000;
  bnk.size = nbytes;
  bnk.host_addr = this->mem;
  attach_bank(std::move(bnk));
}


void *kvm_machine::gpa2hpa(off_t gpa) {
  return (void*)(gpa + (char*)mem);
}

// #define RECORD_VMEXIT_LIFETIME

void kvm_machine::run(workload &work) {
  int cpufd = cpus[0].cpufd;
  struct kvm_run *run = cpus[0].kvm_run;

  while (true) {
    halted = false;
    int err = ioctl(cpufd, KVM_RUN, NULL);

    if (err < 0 && (errno != EINTR && errno != EAGAIN)) {
      printf("KVM_RUN failed\n");
      return;
    }

    int stat = run->exit_reason;

    if (err < 0) continue;
    if (stat == KVM_EXIT_DEBUG) continue;

    if (stat == KVM_EXIT_SHUTDOWN) {
      shutdown = true;
      fprintf(stderr, "SHUTDOWN (probably a triple fault)\n");

      cpus[0].dump_state(stderr);
      return;
    }

    if (stat == KVM_EXIT_INTR) {
      continue;
    }

    if (stat == KVM_EXIT_HLT) {
      continue;
    }

    if (stat == KVM_EXIT_IO) {
      if (run->io.port == 0xFA) {
        // special exit call
        return;
      }

      // 0xFF is the "hcall" io op
      if (run->io.port == 0xFF) {
        regs_t hcall_regs = {};
        cpus[0].read_regs_into(hcall_regs);
        int res = work.handle_hcall(hcall_regs, memsize, mem);

        if (res != WORKLOAD_RES_OKAY) {
          if (res == WORKLOAD_RES_KILL) {
            return;
          }
        }

        cpus[0].write_regs(hcall_regs);
        continue;
      }

      continue;
    }

    if (stat == KVM_EXIT_INTERNAL_ERROR) {
      if (run->internal.suberror == KVM_INTERNAL_ERROR_EMULATION) {
        fprintf(stderr, "emulation failure\n");
        cpus[0].dump_state(stderr);
        return;
      }
      printf("kvm internal error: suberror %d\n", run->internal.suberror);
      return;
    }

    if (stat == KVM_EXIT_FAIL_ENTRY) {
      shutdown = true;
      halted = true;
      return;
    }

    regs_t regs = {};
    cpus[0].read_regs_into(regs);

    fprintf(stderr, "unhandled exit: %d at rip = %p\n", run->exit_reason,
           (void *)regs.rip);
    return;
  }

  // cpus[0].dump_state(stdout, (char *)this->mem);
}

static inline void host_cpuid(struct cpuid_regs *regs) {
  __asm__ volatile("cpuid"
                   : "=a"(regs->eax), "=b"(regs->ebx), "=c"(regs->ecx),
                     "=d"(regs->edx)
                   : "0"(regs->eax), "2"(regs->ecx));
}

static void filter_cpuid(struct kvm_cpuid2 *kvm_cpuid) {
  unsigned int i;
  struct cpuid_regs regs;

  /*
   * Filter CPUID functions that are not supported by the hypervisor.
   */
  for (i = 0; i < kvm_cpuid->nent; i++) {
    struct kvm_cpuid_entry2 *entry = &kvm_cpuid->entries[i];

    switch (entry->function) {
      case 0:

        regs = (struct cpuid_regs){
            .eax = 0x00,

        };
        host_cpuid(&regs);
        /* Vendor name */
        entry->ebx = regs.ebx;
        entry->ecx = regs.ecx;
        entry->edx = regs.edx;
        break;
      case 1:
        /* Set X86_FEATURE_HYPERVISOR */
        if (entry->index == 0) entry->ecx |= (1 << 31);
        /* Set CPUID_EXT_TSC_DEADLINE_TIMER*/
        if (entry->index == 0) entry->ecx |= (1 << 24);
        break;
      case 6:
        /* Clear X86_FEATURE_EPB */
        entry->ecx = entry->ecx & ~(1 << 3);
        break;
      case 10: { /* Architectural Performance Monitoring */
        union cpuid10_eax {
          struct {
            unsigned int version_id : 8;
            unsigned int num_counters : 8;
            unsigned int bit_width : 8;
            unsigned int mask_length : 8;
          } split;
          unsigned int full;
        } eax;

        /*
         * If the host has perf system running,
         * but no architectural events available
         * through kvm pmu -- disable perf support,
         * thus guest won't even try to access msr
         * registers.
         */
        if (entry->eax) {
          eax.full = entry->eax;
          if (eax.split.version_id != 2 || !eax.split.num_counters)
            entry->eax = 0;
        }
        break;
      }
      default:
        /* Keep the CPUID function as -is */
        break;
    };
  }
}

void kvm_machine::reset(void) {
  for (auto &cpu : cpus) {
    cpu.reset();
  }

  /**
   * TODO: for security and isolation, clearing out ram
   *       should be done.
   */

  /*
  munmap(mem, memsize);
  mem = mmap(NULL, memsize, PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
                   */

  memset(mem, 0, memsize);


  // printf("done cleaning\n");
}

uint32_t kvm_machine::num_cpus(void) {return cpus.size(); }
wasp::vcpu &kvm_machine::cpu(uint32_t i) {return cpus[i]; }

void kvm_vcpu::reset(void) {
  ioctl(cpufd, KVM_SET_REGS, &initial_regs);
  ioctl(cpufd, KVM_SET_SREGS, &initial_sregs);
  ioctl(cpufd, KVM_SET_FPU, &initial_fpu);
}

void kvm_vcpu::read_regs_into(regs_t &r) {
  struct kvm_regs regs;
  ioctl(cpufd, KVM_GET_REGS, &regs);

  r.rax = regs.rax;
  r.rbx = regs.rbx;
  r.rcx = regs.rcx;
  r.rdx = regs.rdx;

  r.rsi = regs.rsi;
  r.rdi = regs.rdi;
  r.rsp = regs.rsp;
  r.rbp = regs.rbp;

  r.r8 = regs.r8;
  r.r9 = regs.r8;
  r.r10 = regs.r10;
  r.r11 = regs.r11;
  r.r12 = regs.r12;
  r.r13 = regs.r13;
  r.r14 = regs.r14;
  r.r15 = regs.r15;

  r.rip = regs.rip;
  r.rflags = regs.rflags;
}

void kvm_vcpu::write_regs(regs_t &regs) {
  struct kvm_regs r;
  r.rax = regs.rax;
  r.rbx = regs.rbx;
  r.rcx = regs.rcx;
  r.rdx = regs.rdx;

  r.rsi = regs.rsi;
  r.rdi = regs.rdi;
  r.rsp = regs.rsp;
  r.rbp = regs.rbp;

  r.r8 = regs.r8;
  r.r9 = regs.r8;
  r.r10 = regs.r10;
  r.r11 = regs.r11;
  r.r12 = regs.r12;
  r.r13 = regs.r13;
  r.r14 = regs.r14;
  r.r15 = regs.r15;

  r.rip = regs.rip;
  r.rflags = regs.rflags;
  ioctl(cpufd, KVM_SET_REGS, &r);
}

void kvm_vcpu::read_regs_special_into(regs_special_t &r) {
  struct kvm_sregs sr;

  ioctl(cpufd, KVM_GET_SREGS, &sr);

  static auto copy_segment = [](segment_t &dst, auto &src) {
    dst.base = src.base;
    dst.limit = src.limit;
    dst.selector = src.selector;
    dst.type = src.type;
    dst.present = src.present;
    dst.dpl = src.dpl;
    dst.db = src.db;
    dst.s = src.s;
    dst.long_mode = src.l;
    dst.granularity = src.g;
    dst.available = src.avl;
    dst.unusable = src.unusable;
    // dont need to copy padding
  };

  copy_segment(r.cs, sr.cs);
  copy_segment(r.ds, sr.ds);
  copy_segment(r.es, sr.es);
  copy_segment(r.fs, sr.fs);
  copy_segment(r.gs, sr.gs);
  copy_segment(r.ss, sr.ss);
  copy_segment(r.tr, sr.tr);

  r.gdt.base = sr.gdt.base;
  r.gdt.limit = sr.gdt.limit;

  r.idt.base = sr.idt.base;
  r.idt.limit = sr.idt.limit;

  r.cr0 = sr.cr0;
  r.cr2 = sr.cr2;
  r.cr3 = sr.cr3;
  r.cr4 = sr.cr4;
  r.cr8 = sr.cr8;

  r.efer = sr.efer;
  r.apic_base = sr.apic_base;
  for (int i = 0; i < (NR_INTERRUPTS + 63) / 64; i++) {
    r.interrupt_bitmap[i] = sr.interrupt_bitmap[i];
  }
}

void kvm_vcpu::write_regs_special(regs_special_t &sr) {
  struct kvm_sregs r;

  static auto copy_segment = [](auto &dst, segment_t &src) {
    dst.base = src.base;
    dst.limit = src.limit;
    dst.selector = src.selector;
    dst.type = src.type;
    dst.present = src.present;
    dst.dpl = src.dpl;
    dst.db = src.db;
    dst.s = src.s;
    dst.l = src.long_mode;
    dst.g = src.granularity;
    dst.avl = src.available;
    dst.unusable = src.unusable;
    // dont need to copy padding
  };

  copy_segment(r.cs, sr.cs);
  copy_segment(r.ds, sr.ds);
  copy_segment(r.es, sr.es);
  copy_segment(r.fs, sr.fs);
  copy_segment(r.gs, sr.gs);
  copy_segment(r.ss, sr.ss);
  copy_segment(r.tr, sr.tr);

  r.gdt.base = sr.gdt.base;
  r.gdt.limit = sr.gdt.limit;

  r.idt.base = sr.idt.base;
  r.idt.limit = sr.idt.limit;

  r.cr0 = sr.cr0;
  r.cr2 = sr.cr2;
  r.cr3 = sr.cr3;
  r.cr4 = sr.cr4;
  r.cr8 = sr.cr8;

  r.efer = sr.efer;
  r.apic_base = sr.apic_base;
  for (int i = 0; i < (NR_INTERRUPTS + 63) / 64; i++) {
    r.interrupt_bitmap[i] = sr.interrupt_bitmap[i];
  }

  ioctl(cpufd, KVM_SET_SREGS, &sr);
}

void kvm_vcpu::read_regs_fpu_into(regs_fpu_t &dst) {
  struct kvm_fpu src;

  ioctl(cpufd, KVM_GET_FPU, &src);

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 16; j++) {
      dst.fpr[i][j] = src.fpr[i][j];
    }
  }

  dst.fcw = src.fcw;
  dst.fsw = src.fsw;
  dst.ftwx = src.ftwx;
  dst.last_opcode = src.last_opcode;
  dst.last_ip = src.last_ip;
  dst.last_dp = src.last_dp;
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      dst.xmm[i][j] = src.xmm[i][j];
    }
  }
  dst.mxcsr = dst.mxcsr;
}
void kvm_vcpu::write_regs_fpu(regs_fpu_t &src) {
  struct kvm_fpu dst;

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 16; j++) {
      dst.fpr[i][j] = src.fpr[i][j];
    }
  }

  dst.fcw = src.fcw;
  dst.fsw = src.fsw;
  dst.ftwx = src.ftwx;
  dst.last_opcode = src.last_opcode;
  dst.last_ip = src.last_ip;
  dst.last_dp = src.last_dp;
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      dst.xmm[i][j] = src.xmm[i][j];
    }
  }
  dst.mxcsr = dst.mxcsr;

  ioctl(cpufd, KVM_SET_FPU, &dst);
}

void *kvm_vcpu::translate_address(u64 gva) {
  struct kvm_translation tr;

  tr.linear_address = gva;

  ioctl(cpufd, KVM_TRANSLATE, &tr);

  if (!tr.valid) return nullptr;

  return mem + tr.physical_address;
}

static int kvmfd = -1;

static machine::unique_ptr kvm_allocate(void) {
  if (kvmfd == -1) kvmfd = open("/dev/kvm", O_RDWR);

  return std::make_unique<kvm_machine>(kvmfd, 1);
}

wasp::platform::registration __kvm__reg__ __register_platform = {
    .name = "KVM",
    .flags = PLATFORM_LINUX,
    .allocate = kvm_allocate,
};
