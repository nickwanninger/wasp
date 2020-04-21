#pragma once

#define _INC_WINDOWS
#define WIN32_MEAN_AND_LEAN
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
typedef SOCKET zn_socket_t;