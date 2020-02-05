#pragma once

#include "support.h"

#ifdef _WIN32
#include "platform/windows/hyperv_vcpu.h"
#include "platform/windows/hyperv_driver.h"

#elif __linux__
#include "platform/linux/kvm_driver.h"

#else
#error "unsupported platform"

#endif