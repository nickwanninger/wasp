#include "platform/platform.h"

#include <cstdint>

#include <sched.h>
#include <sys/sysinfo.h>
#include <unistd.h>

void zn_set_affinity(int cpu) {
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu, &mask);
  sched_setaffinity(0, sizeof(mask), &mask);
}

int zn_get_processors_count() {
  return get_nprocs();
}

void zn_sleep_micros(uint32_t usecs) {
  usleep(usecs);
}

int zn_close_socket(zn_socket_t socket) {
  return close(socket);
}
