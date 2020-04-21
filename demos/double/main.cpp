#include <mobo/runner.hpp>
#include <mobo/loader.h>

#include "./workload.h"

int main()
{
  mobo::run<double_workload, mobo::loader::flatbin_loader>("build/tests/double64.bin");
  exit(0);
}