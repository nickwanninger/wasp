#pragma once

#ifndef __MOBO_PLATFORM__
#define __MOBO_PLATFORM__

#include <cstdint>
#include "./memory.h"
#include "./socket.h"
#include "./loader.h"

WASP_API
void zn_set_affinity(int cpu);

WASP_API
int zn_get_processors_count();

WASP_API
void zn_sleep_micros(uint32_t usecs);

#endif
