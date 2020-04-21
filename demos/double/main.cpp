#include <wasp/runner.hpp>
#include <wasp/loader.h>

#include "./workload.h"

int main()
{
  wasp::run<double_workload, wasp::loader::flatbin_loader>("build/tests/double64.bin");
  exit(0);
}