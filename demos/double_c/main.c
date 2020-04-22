#include <stdlib.h>
#include <wasp_c/compiler_defs.h>
#include <wasp_c/workload.h>
#include <wasp_c/machine.h>

#include "runner.h"
#include "workload.h"

int main()
{
  const char *binary_path = "build/tests/double64.bin";

  wasp_workload_t workload = g_double_workload;
  workload.init(&workload, NULL);

  wasp_loader_t *loader = wasp_flatbin_loader_create(binary_path);
  wasp_run(binary_path, 1, NULL, loader, &workload);

  exit(0);
}
