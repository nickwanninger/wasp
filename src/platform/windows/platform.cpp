#include <Windows.h>
#include <WinBase.h>
#include <sysinfoapi.h>
#include <winsock2.h>
#include <processthreadsapi.h>

struct zn_regs {
//    struct kvm_regs regs;
};

void zn_set_affinity(int cpu)
{
  if (cpu < 0 || cpu > 64) {
    throw std::runtime_error("failed to set thread affinity: `cpu` must be > 0 and <= 64")
  }

  HANDLE current_thread = GetCurrentThread();

  uint64_t mask = 1 << cpu;
  DWORD_PTR prev_affinity = SetThreadAffinityMask(current_thread, &mask);
  if (prev_affinity == nullptr) {
    throw std::runtime_error("failed to set thread affinity: os error")
  }
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