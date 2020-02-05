#pragma once

#ifdef _WIN32
#include "windows/hyperv.h"

#elif __LINUX__
#include "linux/kvm.h"

#else
#error "unsupported platform"

#endif