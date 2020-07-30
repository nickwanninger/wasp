#include <stdio.h>
#include <stdlib.h>
#include <wasp/loader.h>

#include <wasp/runner.hpp>

struct add_args {
  int a, b, ret;
};

extern "C" void wasp_run_virtine(const char *code, size_t codesz, size_t memsz,
				 void *arg, size_t argsz);

int main() {
  FILE *fp = fopen("/tmp/virtine_add.final.bin", "r");

  fseek(fp, 0L, SEEK_END);
  size_t sz = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  auto *code = malloc(sz);

  fread(code, sz, 1, fp);
  fclose(fp);

	struct add_args args;
	args.a = 1;
	args.b = 2;
	wasp_run_virtine((const char*)code, sz, 0x4000, (void*)&args, sizeof(args));
	printf("ret=%d\n", args.ret);
  exit(0);
}
