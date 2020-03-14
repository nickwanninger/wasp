#include "compiler_defs.h"

#include <elfio/elfio.hpp>
#include "platform/platform.h"
#include "platform/loader.h"

namespace mobo::loader {

elf_loader::elf_loader(std::string path) { reader.load(path); }

bool elf_loader::inject(mobo::machine &vm) {
  auto entry = reader.get_entry();

  auto secc = reader.sections.size();
  for (int i = 0; i < secc; i++) {
    auto psec = reader.sections[i];
    auto type = psec->get_type();
    if (psec->get_name() == ".comment") continue;

    if (type == SHT_PROGBITS) {
      auto size = psec->get_size();
      if (size == 0) continue;
      const char *data = psec->get_data();
      auto dst = (char *)vm.gpa2hpa(psec->get_address());
      memcpy(dst, data, size);
    }
  }

  struct regs_t r;
  vm.cpu(0).read_regs_into(r);
  r.rip = entry;
  vm.cpu(0).write_regs(r);

  // XXX: should we do this?
  struct regs_special_t sr;
  vm.cpu(0).read_regs_special_into(sr);
  sr.cs.base = 0;
  sr.ds.base = 0;
  vm.cpu(0).write_regs_special(sr);

  return true;
}

};