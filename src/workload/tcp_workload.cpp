#include <atomic>
#include <io.h>
#include "platform/socket.h"
#include "mobo/workload.h"
#include "mobo/workload_impl.h"

namespace mobo::workload_impl {

static bool send_all(int socket, void *buffer, size_t length) {
  char *ptr = (char *) buffer;
  while (length > 0) {
    int i = write(socket, ptr, length);
    if (i < 1) return false;
    ptr += i;
    length -= i;
  }
  return true;
}

tcp_workload::tcp_workload(zn_socket_t sock) : socket(sock) {}

int tcp_workload::handle_hcall(struct mobo::regs_t &regs, size_t ramsize,
                               void *ram) {
  // send
  if (regs.rax == 2) {
    off_t buf_off = regs.rdi;
    size_t len = regs.rsi;

    if (buf_off + len >= ramsize) {
      regs.rax = -1;
      return WORKLOAD_RES_OKAY;
    }

    char *buf = (char *) ram + buf_off;
    regs.rax = send_all(socket, buf, len);
    return 0;
  }

  // recv
  if (regs.rax == 3) {
    off_t buf_off = regs.rdi;
    size_t len = regs.rsi;

    if (buf_off + len >= ramsize) {
      regs.rax = -1;
      return WORKLOAD_RES_OKAY;
    }

    char *buf = (char *) ram + buf_off;
    regs.rax = recv(socket, buf, len, 0);
  }
  return WORKLOAD_RES_OKAY;
}

}