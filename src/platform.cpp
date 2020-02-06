#include <mobo/machine.h>

extern mobo::platform::registration __start_vm_platforms[];
extern mobo::platform::registration __stop_vm_platforms[];

mobo::machine::ptr mobo::platform::create(int flags) {

  for (auto *plat = __start_vm_platforms; plat != __stop_vm_platforms; plat++) {
    if ((plat->flags & flags) != 0) {
      return plat->allocate();
    }
  }

  return nullptr;
};
