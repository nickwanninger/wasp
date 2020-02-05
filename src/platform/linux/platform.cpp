#include <sched.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <linux/kvm.h>

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

void zn_close_socket(zn_socket_t socket) {
  close(socket);
}