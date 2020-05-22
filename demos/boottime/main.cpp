#include <wasp/runner.hpp>
#include <wasp/loader.h>

#include "./workload.h"

int main()
{
  bool did_succeed = wasp::run<boottime_workload, wasp::loader::elf_loader>(
      "build/tests/boottime.elf",
      1000,
      "data/hyperv-release.csv");

  if (!did_succeed) {
    fprintf(stderr, "error: failed to create vm\n");
    exit(1);
  }

  exit(0);
}