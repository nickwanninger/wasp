#include <mobo/machine.h>
#include <mobo/types.h>
#include <mobo/vcpu.h>

#include <elfio/elfio.hpp>

using namespace mobo;

mobo::machine::~machine(void) = default;

machine::ptr machine::create(size_t memsize)
{
  machine::ptr v = platform::create(PLATFORM_ANY);
  v->allocate_ram(memsize);
  v->reset();
  return v;
}
