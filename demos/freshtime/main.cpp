#include <intrin.h>
#include <wasp/compiler_defs.h>
#include <WinHvPlatform.h>
#include <wasp/loader.h>
#include <wasp/platform.h>
#include <wasp/unistd.h>

#include "./workload.h"

using namespace wasp;

void *load(char *flatbin_path, size_t *size)
{
  void *addr = malloc(1024);
  if (addr == nullptr) {
    PANIC("failed to convert GPA -> HPA, got nullptr");
  }

  FILE *fp = fopen(flatbin_path, "r");
  if (fp == nullptr) {
    PANIC("failed to open binary\n");
  }

  fseek(fp, 0L, SEEK_END);
  size_t sz = ftell(fp);
  fseek(fp, 0L, SEEK_SET);

  if (sz > 1024) { PANIC(""); }
  *size = sz;

  fread(addr, sz, 1, fp);
  fclose(fp);

  return addr;
}

template <class W>
bool run(
        std::string path,
        int run_count = 1,
        const char *stdout_path = nullptr)
{
  printf("test [%s]\n", path.data());

  int ofd = -1;

  if (stdout_path != nullptr) {
    ofd = dup(1);
    FILE* file = fopen(stdout_path, "w+");
    if (file == nullptr)
    {
      fprintf(stderr, "failed to open file\n");
      return false;
    }

    int new_fd = fileno(file);
    dup2(new_fd, 1);
    // close(new_fd);
    fclose(file);
  }

  size_t size;
  void *bin = load(path.data(), &size);

  for (int i = 0; i < run_count; i++) {

    W work;

    uint64_t start, end;
    {
      HRESULT hr;
      start = __rdtsc();

      WHV_PARTITION_HANDLE handle;
      WHvCreatePartition(&handle);

      hyperv_machine::set_num_cpus(handle, 1);
      WHvSetupPartition(handle);
      WHvCreateVirtualProcessor(handle, 0, 0);

      void *mem = hyperv_machine::allocate_guest_phys_memory(handle, 0, 1024L);
      memcpy(mem, bin, size);

      WHV_RUN_VP_EXIT_CONTEXT exit_context = {};
      hr = WHvRunVirtualProcessor(
              handle,
              0,
              &exit_context,
              sizeof(exit_context));

      end = __rdtsc();

      WHvDeletePartition(handle);
      if (FAILED(hr)) {
        PANIC("run failed");
      }

    }

    uint64_t cycles = end - start;
    printf("%lld\n", cycles);
  }

  fflush(stdout);

  if (ofd != -1) {
    dup2(ofd, 1);
    close(ofd);
  }

  return true;
}


int main()
{
  bool did_succeed = run<boottime_workload>(
      "build/tests/freshtime.bin",
      1000,
      "data/hyperv-freshtime-release.csv");

  if (!did_succeed) {
    fprintf(stderr, "error: failed to create vm\n");
    exit(1);
  }

  exit(0);
}