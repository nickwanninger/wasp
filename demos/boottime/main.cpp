#include <mobo/runner.hpp>
#include <mobo/loader.h>

#include "./workload.h"

int main()
{
  bool did_succeed = mobo::run<boottime_workload, mobo::loader::elf_loader>(
      "build/tests/boottime.elf",
      1);

  if (!did_succeed) {
    fprintf(stderr, "error: failed to create vm\n");
    exit(1);
  }

  exit(0);
}