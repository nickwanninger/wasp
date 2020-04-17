#include <string>

#include "compiler_defs.h"
#include "platform/loader.h"

namespace mobo::loader {

flatbin_loader::flatbin_loader(std::string path) : path(std::move(path)) {}

bool flatbin_loader::inject(mobo::machine &vm) {
  auto entry = 0x1000;
  vm.set_entry(entry);

  void *addr = vm.gpa2hpa(entry);
  if (addr == nullptr) {
    PANIC("failed to convert GPA -> HPA, got nullptr");
  }

  FILE *fp = fopen(path.data(), "r");
  if (fp == nullptr) {
    PANIC("failed to open binary\n");
  }

  fseek(fp, 0L, SEEK_END);
  size_t sz = ftell(fp);
  fseek(fp, 0L, SEEK_SET);

  fread(addr, sz, 1, fp);
  fclose(fp);

  struct regs_t r = {};
  vm.cpu(0).read_regs_into(r);
  r.rip = entry;
  r.rsp = 0x1000;
  r.rbp = 0x1000;
  vm.cpu(0).write_regs(r);

  // XXX: should we do this?
  //    struct regs_special_t sr;
  //    vm.cpu(0).read_sregs(sr);
  //    sr.cs.base = 0;
  //    sr.ds.base = 0;
  //    vm.cpu(0).write_regs_special(sr);

  return true;
}

}