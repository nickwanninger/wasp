#pragma once

#ifdef _WIN32
#include "windows/socket.h"

#elif __linux__
#include "linux/socket.h"

#else
#error "unsupported platform"

#endif