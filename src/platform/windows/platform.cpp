#include <sysinfoapi.h>
#include <winsock2.h>

struct zn_regs {
//    struct kvm_regs regs;
};

void zn_set_affinity(int cpu)
{
  // TODO
}

int zn_get_processors_count()
{
  SYSTEM_INFO system_information;
  GetSystemInfo(&system_information);

  return system_information.dwNumberOfProcessors;
}

void zn_sleep_micros(uint32_t usecs)
{
  usleep(usecs);
}

int zn_close_socket(zn_socket_t socket)
{
  return closesocket(socket);
}