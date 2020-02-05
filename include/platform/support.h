#pragma once

#ifdef _WIN32
#define ZN_PLATFORM_HYPERV

#elif __linux__
#define ZN_PLATFORM_KVM

#else
#error "unsupported platform"

#endif