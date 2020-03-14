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


int zn_socket_close(zn_socket_t socket)
{
  // see also https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-shutdown

  int result;
  result = shutdown(socket, SD_SEND);
  if (result == FD_CLOSE) {
    int read;
    char buf[4096];
    while ((read = recv(socket, buf, 4096, MSG_WAITALL)) != 0) {
      if (read < 0) {
        fprintf(stderr, "%s: warning: failed to gracefully close socket\n");
        break;
      }
    }
  }

  return closesocket(socket);
}