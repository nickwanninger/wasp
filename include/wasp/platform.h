#pragma once

#ifndef __MOBO_PLATFORM__
#define __MOBO_PLATFORM__

#include <cstdint>
#include "./support.h"
#include "./memory.h"
#include "./socket.h"
#include "./backend.h"
#include "./loader.h"

void
zn_set_affinity(int cpu);

int
zn_get_processors_count();

void
zn_sleep_micros(uint32_t usecs);

#endif