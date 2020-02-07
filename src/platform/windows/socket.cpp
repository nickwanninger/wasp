#include "platform/socket.h"
#include <stdexcept>
#include <string>

#define MIN_WINSOCK_MAJOR_VERSION (2)
#define MIN_WINSOCK_MINOR_VERSION (2)
#define MIN_WINSOCK_VERSION (MAKEWORD(MIN_WINSOCK_MAJOR_VERSION, MIN_WINSOCK_MINOR_VERSION))

void zn_socket_init()
{
  // see https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup
  uint16_t version_requested = MIN_WINSOCK_VERSION;

  WSADATA wsaData;
  int err = WSAStartup(version_requested, &wsaData);
  if (err != 0) {
    throw std::runtime_error("WSAStartup failed with error: " + std::to_string(err));
  }

  if (wsaData.wVersion != version_requested) {
    WSACleanup();
    throw std::runtime_error("WSAStartup: failed to find WinSocket >= version 2.2");
  }
}
