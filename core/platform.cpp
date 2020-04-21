#include <mobo/machine.h>

#ifdef _MSC_VER

struct __phantom {};

__declspec(allocate("vm_platforms$a"))
__declspec(align(sizeof(void *)))
static struct __phantom __msvc_start_vm_platforms;

__declspec(allocate("vm_platforms$z"))
__declspec(align(sizeof(void *)))
static struct __phantom __msvc_stop_vm_platforms;

const mobo::platform::registration *const __start_vm_platforms = reinterpret_cast<const mobo::platform::registration *const>(&__msvc_start_vm_platforms);
const mobo::platform::registration *const __stop_vm_platforms = reinterpret_cast<const mobo::platform::registration *const>(&__msvc_stop_vm_platforms);

#pragma init_seg("vm_platforms")

#endif

mobo::machine::ptr mobo::platform::create(int flags) {

  for (auto *plat = __start_vm_platforms; plat < __stop_vm_platforms; plat++) {
    if ((plat->flags & flags) != 0) {
      auto allocate = plat->allocate;
      if (allocate == nullptr) continue;
      return allocate();
    }
  }

  return nullptr;
};

