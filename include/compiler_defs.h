#pragma once

#include <cstdint>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

#define PANIC(msg, ...) \
    do { \
        fprintf(stderr, "panic! [%s] %s:L%d: " msg "\n", \
                __FILE__,  \
                __FUNCTION__, \
                __LINE__, \
                __VA_ARGS__); \
        fflush(stderr); \
        exit(1); \
    } while (0)

#else

#define PANIC(msg, vargs...) \
    do { \
        fprintf(stderr, "panic! [%s] %s:L%d: " msg "\n", \
                __FILE__,  \
                __FUNCTION__, \
                __LINE__, \
                ##vargs); \
        fflush(stderr); \
        exit(1); \
    } while (0)

#endif
