#include <wasp/socket.h>

#include <stdexcept>
#include <thread>

#include <processthreadsapi.h>
#include <winbase.h>

void zn_set_affinity(int cpu)
{
  if (cpu < 0 || cpu > 64) {
    throw std::runtime_error("failed to set thread affinity: `cpu` must be > 0 and <= 64");
  }

  HANDLE current_thread = GetCurrentThread();

  uint64_t mask = 1 << cpu;
  DWORD_PTR prev_affinity = SetThreadAffinityMask(current_thread, mask);
  if (prev_affinity == 0) {
    throw std::runtime_error("failed to set thread affinity: os error");
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
  std::this_thread::sleep_for(std::chrono::microseconds(usecs));
}
