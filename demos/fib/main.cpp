#include <mobo/runner.hpp>
#include <mobo/loader.h>

#include "./workload.h"

int main()
{
  mobo::run<fib_workload, mobo::loader::flatbin_loader>("build/tests/fib20.bin");
  exit(0);
}