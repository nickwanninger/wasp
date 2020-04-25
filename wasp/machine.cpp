#include <wasp/machine.h>
#include <wasp/types.h>
#include <wasp/vcpu.h>

#include <elfio/elfio.hpp>

using namespace wasp;

wasp::machine::~machine(void) = default;

machine::unique_ptr machine::create(size_t memsize)
{
  machine::unique_ptr v = platform::create();
  if (v == nullptr) { return v; }
  v->allocate_ram(memsize);
  v->reset();
  return v;
}
