#include <wasp/compiler_defs.h>
#include <wasp/machine.h>

#ifdef _MSC_VER

struct __phantom {};

__declspec(allocate("vm_plat$a"))
__declspec(align(sizeof(void *)))
static struct __phantom __msvc_start_vm_platforms;

__declspec(allocate("vm_plat$z"))
__declspec(align(sizeof(void *)))
static struct __phantom __msvc_stop_vm_platforms;

const wasp::platform::registration *const __start_vm_platforms = reinterpret_cast<const wasp::platform::registration *const>(&__msvc_start_vm_platforms);
const wasp::platform::registration *const __stop_vm_platforms = reinterpret_cast<const wasp::platform::registration *const>(&__msvc_stop_vm_platforms);

#endif

wasp::machine::unique_ptr wasp::platform::create(int flags) {
  const static char magic_buf[WASP_REGISTRATION_MAGIC_LEN] = WASP_REGISTRATION_MAGIC;

  // HACK: MSVC will pad the linker section (likely word aligned),
  //       but just check each by for the magic
  //
  for (char *ptr = (char *) __start_vm_platforms; ptr < (char *) __stop_vm_platforms; ptr++) {
    const auto *plat = reinterpret_cast<const registration *>(ptr);

    if (memcmp(plat->magic, magic_buf, WASP_REGISTRATION_MAGIC_LEN) == 0
        && (plat->flags & ((uint32_t) flags)) != 0)
    {
      auto allocate = plat->allocate;
      if (allocate == nullptr) {
        PANIC("platform registration allocate function ptr was null");
      }

      return allocate();
    }
  }

  return nullptr;
}

