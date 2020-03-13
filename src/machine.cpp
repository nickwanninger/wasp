#include <inttypes.h>
#include <mobo/machine.h>
#include <stdio.h>
//#include <sys/mman.h>
#include <mobo/memwriter.h>
#include <mobo/types.h>
#include <mobo/vcpu.h>

#include <elfio/elfio.hpp>

using namespace mobo;

mobo::machine::~machine(void) {}

/*
void machine::allocate_ram(size_t nbytes) {
// round up to the nearest page boundary
if ((nbytes & 0xfff) != 0) {
  nbytes &= ~0xfff;
  nbytes += 0x1000;
}

memory = (char *)mmap(NULL, nbytes, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
mem_size = nbytes;

// attach to the driver
m_driver.attach_memory(mem_size, memory);
}
*/

// TODO: abstract the loading of an elf file to a general function that takes
//       some memory and a starting vcpu
void machine::load_elf(std::string file) {

  char *memory = (char *)gpa2hpa(0);  // grab a char buffer reference to the mem

  ELFIO::elfio reader;
  reader.load(file);
  auto entry = reader.get_entry();

  auto secc = reader.sections.size();
  for (int i = 0; i < secc; i++) {
    auto psec = reader.sections[i];
    auto type = psec->get_type();
    if (psec->get_name() == ".comment") continue;

    if (type == SHT_PROGBITS) {
      // printf("%p\n", (void*)psec->get_address());
      auto size = psec->get_size();
      if (size == 0) continue;
      const char *data = psec->get_data();
      char *dst = memory + (psec->get_address() & 0xFFFFFFFF);
      memcpy(dst, data, size);
    }
  }

  // arbitrarially on the 5th page
  u64 hypertable = 0x5000;

  {
    /*
     * TODO: memory regions like mmap() has in a kernel.
    {
      // create a memory writer to the part of memory that represents the mbd
      memwriter hdr(memory + hypertable);

      hdr.write<u64>(1);                 // entry is a memory map (type 0)
      hdr.write<u64>(this->ram.size());  // how many memory regions are there

      for (auto bank : this->ram) {
        // write the physical address and the extent
        hdr.write<u64>(bank.guest_phys_addr);
        // and write how long it is
        hdr.write<u64>(bank.size);
      }
    }
    */

    // setup general purpose registers
    {
      struct regs_t r;
      cpu(0).read_regs_into(r);
      r.rdi = 0x4242424242424242L;
      r.rsi = hypertable;  // TODO: The physical address of the hypertable
      // information
      r.rip = entry;
      r.rflags &= ~(1 << 17);
      r.rflags &= ~(1 << 9);
      cpu(0).write_regs(r);
    }

    // setup the special registers
    {
      regs_special_t sr;
      cpu(0).read_regs_special_into(sr);

      auto init_seg = [](mobo::segment_t &seg) {
        seg.present = 1;
        seg.base = 0;
        seg.db = 1;
        seg.granularity = 1;
        seg.selector = 0x10;
        seg.limit = 0xFFFFFFF;
        seg.type = 0x3;
      };

      init_seg(sr.cs);
      sr.cs.selector = 0x08;
      sr.cs.type = 0x0a;
      sr.cs.s = 1;
      init_seg(sr.ds);
      init_seg(sr.es);
      init_seg(sr.fs);
      init_seg(sr.gs);
      init_seg(sr.ss);

      // clear bit 31, set bit 0
      sr.cr0 &= ~(1 << 31);
      sr.cr0 |= (1 << 0);

      cpu(0).write_regs_special(sr);
    }
  }
}

