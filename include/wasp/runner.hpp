#pragma once

#include <string>
#include <chrono>
#include <cstdio>
#include <wasp/unistd.h>

#include "machine.h"

namespace wasp {

template <class W, class L>
bool run(
    std::string path,
    int run_count = 1,
    const char *stdout_path = nullptr)
{
  printf("test [%s]\n", path.data());

  L loader(path);

  int ofd = -1;

  if (stdout_path != nullptr) {
    ofd = dup(1);
    // int new_fd = open(stdout_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
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

  auto start = std::chrono::high_resolution_clock::now();

  // TODO: Use the RAM size from what you're loading or throw if the loader
  // requested memory size is greater than the limit
  machine::ptr vm = machine::create(1 * 1024l * 1024l);
  if (vm == nullptr) { return false; }
  for (int i = 0; i < run_count; i++) {
    W work;
    vm->reset();
    loader.inject(*vm);
    vm->run(work);
  }
  auto end = std::chrono::high_resolution_clock::now();

  fflush(stdout);

  if (ofd != -1) {
    dup2(ofd, 1);
    close(ofd);
  }

  printf(
      "    DONE %fms\n",
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start) *
      1000.0);

  return true;
}

}
