#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <wasp_c/loader.h>
#include <wasp_c/workload.h>

bool wasp_run(
    const char *path,
    int run_count,
    const char *stdout_path,
    wasp_loader_t *loader,
    wasp_workload_t *workload);

#ifdef __cplusplus
}; // extern "C"
#endif