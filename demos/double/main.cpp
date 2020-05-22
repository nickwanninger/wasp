#include <wasp/runner.hpp>
#include <wasp/loader.h>

#include "./workload.h"

int main()
{
  bool did_succeed = wasp::run<double_workload, wasp::loader::flatbin_loader>("build/tests/double64.bin");
  if (!did_succeed) {
    fprintf(stderr, "error: failed to create vm\n");
    exit(1);
  }

  exit(0);
}