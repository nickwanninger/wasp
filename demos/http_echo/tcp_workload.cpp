#include <chrono>

#include <timeit.h>
#include <wasp/platform.h>
#include <wasp/workload.h>

#include "tcp_workload.h"

TIMEIT_EXTERN(g_main);

static bool send_all(int socket, void *buffer, size_t length) {
  char *ptr = (char *) buffer;
//  printf("%s(len = %lld):\n", __FUNCTION__, length);
//  for (int i = 0; i < length; i++) {
//    putchar(ptr[i]);
//  }

  while (length > 0) {
    int i = send(socket, ptr, length, 0);
    if (i < 1) return false;
    ptr += i;
    length -= i;
  }
  return true;
}

tcp_workload::tcp_workload(zn_socket_t sock) : socket(sock) {}

int tcp_workload::handle_hcall(struct wasp::regs_t &regs, size_t ramsize,
                               void *ram) {
  TIMEIT_FN(g_main);

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
    return WORKLOAD_RES_OKAY;
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
    return WORKLOAD_RES_OKAY;
  }

  fprintf(stderr, "%s: bad hypercall (%lld)\n", __FUNCTION__, regs.rax);
  return WORKLOAD_RES_KILL;
}

void tcp_workload::handle_exit() {
  TIMEIT_FN(g_main);
  zn_socket_close(socket);
}
