#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#include <wasp_c/unistd.h>
#include <wasp_c/loader.h>
#include <wasp_c/panic.h>

bool wasp_run(
    const char *path,
    int run_count,
    const char *stdout_path,
    wasp_loader_t *loader,
    wasp_workload_t *workload)
{
  PANIC_IF_NULL(path);
  PANIC_IF_NULL(loader);
  PANIC_IF_NULL(workload);

  printf("test [%s]\n", path);

  int ofd = -1;

  if (stdout_path != NULL) {
    ofd = dup(1);
    // int new_fd = open(stdout_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
    FILE* file = fopen(stdout_path, "w+");
    if (file == NULL) {
      fprintf(stderr, "failed to open file\n");
      return false;
    }

    int new_fd = fileno(file);
    dup2(new_fd, 1);
    // close(new_fd);
    fclose(file);
  }

  struct timespec start = {0};
  timespec_get(&start, TIME_UTC);

  // TODO: Use the RAM size from what you're loading or throw if the loader
  // requested memory size is greater than the limit
  wasp_machine_t *vm = wasp_machine_create(1 * 1024l * 1024l);
  if (vm == NULL) { return false; }
  for (int i = 0; i < run_count; i++) {
    wasp_machine_reset(vm);
    wasp_loader_inject(loader, vm);
    wasp_machine_run(vm, workload);
  }

  struct timespec end = {0};
  timespec_get(&end, TIME_UTC);
  uint64_t elapsed_ms =
      ((end.tv_sec - start.tv_sec) * 1000)
      + (uint64_t) ((end.tv_nsec - start.tv_nsec) / 1e6);

  fflush(stdout);
  wasp_machine_free(vm);

  if (ofd != -1) {
    dup2(ofd, 1);
    close(ofd);
  }

  printf("    DONE %lldms\n", elapsed_ms);

  return true;
}
