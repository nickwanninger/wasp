#pragma once

#include "../compiler_defs.h"

#ifdef _WIN32
#include "windows/socket.h"

#elif __linux__
#include "linux/socket.h"

#else
#error "unsupported platform"

#endif

int
zn_socket_close(zn_socket_t socket);

/**
 * Runs any platform specific initialization to setup support for sockets
 */
void
zn_socket_init();

