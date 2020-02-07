#pragma once

#ifndef __MOBO_PLATFORM__
#define __MOBO_PLATFORM__

#include <cstdint>
#include "support.h"
#include "./socket.h"
#include "./backend.h"

void
zn_set_affinity(int cpu);

int
zn_get_processors_count();

void
zn_sleep_micros(uint32_t usecs);

int
zn_close_socket(zn_socket_t socket);

/**
 * Runs any platform specific initialization to setup support for sockets
 */
void
zn_socket_init();

#endif