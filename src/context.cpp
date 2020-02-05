#include <stdexcept>
#include "mobo/machine.h"
#include "platform/support.h"
#include "platform/backend.h"

#ifdef ZN_PLATFORM_KVM
#include <fcntl.h>
#endif

mobo::context::context() = default;

bool mobo::context::is_supported(mobo::driver_kind kind) {
  // TODO
  return true;
}

mobo::machine::ptr mobo::context::create(mobo::driver_kind kind, uint32_t num_cpus)
{
#ifdef ZN_PLATFORM_KVM
  if ((kind & mobo::driver_kind_kvm) != 0) {
    if (kvmfd < 0) {
      kvmfd = open("/dev/kvm", O_RDWR);
      if (kvmfd <= 0) {
        throw std::runtime_error("failed to open /dev/kvm");
      }
    }

    mobo::kvm_driver driver(kvmfd, num_cpus);
    mobo::machine machine(driver);
    return std::make_shared<mobo::machine>(machine);
  }
#endif

#ifdef ZN_PLATFORM_HYPERV
  if ((kind & mobo::driver_kind_hyperv) != 0) {
    mobo::hyperv_driver driver(num_cpus);
    mobo::machine machine(driver);
    return std::make_shared<mobo::machine>(machine);
  }
#endif

  throw std::out_of_range("failed to create machine: no supported platforms available");
}


