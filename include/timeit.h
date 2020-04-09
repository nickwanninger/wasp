#pragma once

#include <chrono>
#include <cstdio>
#include <thread>

#define TIMEIT_ENABLED 0

#if TIMEIT_ENABLED

#define TIMEIT_EXTERN(STREAM) \
  extern std::chrono::high_resolution_clock::time_point __timeit_start_##STREAM


#define TIMEIT_START(STEAM) \
  std::chrono::high_resolution_clock::time_point __timeit_start_##STEAM = std::chrono::high_resolution_clock::now()


#define __TIMEIT_PRINT(STREAM, MSG, MSG_SUFFIX, START_TIME, DELTA) \
  do { \
    auto end = std::chrono::high_resolution_clock::now(); \
    typedef std::chrono::microseconds micros; \
    auto dur = std::chrono::duration_cast<micros>(end - __timeit_start_##STREAM); \
    auto dur_micros = dur.count(); \
    auto delta = std::chrono::duration_cast<micros>(end - START_TIME); \
    auto delta_micros = delta.count(); \
    auto tid = std::this_thread::get_id(); \
    printf("[%.4f tid %d] %s %s ", dur.count() / 1000.0, tid, MSG, MSG_SUFFIX); \
    if (DELTA && delta_micros < 1000) { \
      printf("(+%lu us)", delta_micros); \
    } \
    else if (DELTA) { \
      printf("(+%.2f ms)", delta_micros / 1000.0); \
    } \
    printf("\n"); \
  } while (0)


#define TIMEIT_MARK(STREAM, NAME) __TIMEIT_PRINT(STREAM, NAME, "", __timeit_start_##STREAM, false)

#define TIMEIT_GUARD_NAME(STREAM, NAME) __timeit_guard_##STREAM##_##NAME
#define TIMEIT_GUARD_NAME_T(STREAM, NAME) __timeit_guard_##STREAM##_##NAME##_t
#define TIMEIT_GUARD_VAR(STREAM, NAME, MSG) TIMEIT_GUARD_NAME_T(STREAM, NAME) TIMEIT_GUARD_NAME(STREAM, NAME)(MSG)

#define TIMEIT_GUARD_EXIT(MODE, STREAM, NAME) TIMEIT_GUARD_EXIT_##MODE(STREAM, NAME)
#define TIMEIT_GUARD_EXIT_dtor(NUM, NAME) ~TIMEIT_GUARD_NAME_T(NUM, NAME)()
#define TIMEIT_GUARD_EXIT_exit(NUM, NAME) void exit()

#define TIMEIT_GUARD(STREAM, NAME, MODE) \
  struct TIMEIT_GUARD_NAME_T(STREAM, NAME) { \
    const char *msg; \
    std::chrono::high_resolution_clock::time_point start; \
    explicit TIMEIT_GUARD_NAME_T(STREAM, NAME)(const char *msg)\
      : start(std::chrono::high_resolution_clock::now()) \
      , msg(msg) \
    { \
      __TIMEIT_PRINT(STREAM, msg, "begin", start, false); \
    } \
    TIMEIT_GUARD_EXIT(MODE, STREAM, NAME) { __TIMEIT_PRINT(STREAM, msg, "end", start, true); } \
  }

#define TIMEIT_RECORD(STREAM, NAME) TIMEIT_GUARD(STREAM, NAME, exit)

#define TIMEIT_BEGIN(STREAM, NAME, MSG) \
  TIMEIT_RECORD(STREAM, NAME); \
  TIMEIT_GUARD_VAR(STREAM, NAME, MSG)

#define TIMEIT_END(NUM, NAME) \
  TIMEIT_GUARD_NAME(NUM, NAME).exit()

#define TIMEIT_FN(STREAM) \
  TIMEIT_GUARD(STREAM, fn, dtor); \
  TIMEIT_GUARD_VAR(STREAM, fn, __FUNCTION__)

#else

#define TIMEIT_EXTERN(STREAM)
#define TIMEIT_START(STEAM)
#define __TIMEIT_PRINT(STREAM, MSG, MSG_SUFFIX, START_TIME, DELTA)
#define TIMEIT_MARK(STREAM, NAME)
#define TIMEIT_GUARD_NAME(STREAM, NAME)
#define TIMEIT_GUARD_NAME_T(STREAM, NAME)
#define TIMEIT_GUARD_VAR(STREAM, NAME, MSG)
#define TIMEIT_GUARD_EXIT(MODE, STREAM, NAME)
#define TIMEIT_GUARD(STREAM, NAME, MODE)
#define TIMEIT_RECORD(STREAM, NAME)
#define TIMEIT_BEGIN(STREAM, NAME, MSG)
#define TIMEIT_END(NUM, NAME)
#define TIMEIT_FN(STREAM)

#endif