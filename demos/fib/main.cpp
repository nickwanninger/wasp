#include <wasp/runner.hpp>
#include <wasp/loader.h>

#include "./workload.h"

int main()
{
  wasp::run<fib_workload, wasp::loader::flatbin_loader>("build/tests/fib20.bin");
  exit(0);
}