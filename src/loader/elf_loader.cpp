#include "compiler_defs.h"

#include <elfio/elfio.hpp>
#include "mobo/platform.h"
#include "mobo/loader.h"

namespace mobo::loader {

elf_loader::elf_loader(std::string path) {
  if (!reader.load(path)) {
    PANIC("failed to open ELF binary: '%s'", path.data());
  }
}

bool elf_loader::inject(mobo::machine &vm) {
  fprintf(stderr, "%s\n", __FUNCTION__);
  auto entry = reader.get_entry();
  vm.set_entry(entry);

  uint16_t num_sections = reader.sections.size();
  for (int i = 0; i < num_sections; i++) {
    auto psec = reader.sections[i];
    auto type = psec->get_type();
    if (psec->get_name() == ".comment") continue;

    auto size = psec->get_size();
    ELFIO::Elf64_Addr gpa = psec->get_address();
    std::string name = psec->get_name();

    if (type == SHT_PROGBITS) {
      if (size == 0) {
        fprintf(stderr, "%s: skip '%s' (0x%llx) size 0x%llx\n", __FUNCTION__, name.data(), gpa, size);
        continue;
      }

      // TODO: It's possible that the memcpy will be copying beyond the allocated
      // ram size, perhaps this should force a resize, or there should be another
      // function that attempts to get a region of memory or otherwise will
      // map the range into the guest
      const char *data = psec->get_data();
      auto dst = (char *)vm.gpa2hpa(gpa);
      memcpy(dst, data, size);
      fprintf(stderr, "%s: map  '%s' (0x%llx) size 0x%llx\n", __FUNCTION__, name.data(), gpa, size);
    }
    else {
      fprintf(stderr, "%s: skip '%s' (0x%llx) size 0x%llx\n", __FUNCTION__, name.data(), gpa, size);
      continue;
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