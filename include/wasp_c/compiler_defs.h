#pragma once

// see also https://stackoverflow.com/a/2164853/809572
#if defined(_MSC_VER)
  #define WASP_ATTRIBUTE_EXPORT __declspec(dllexport)
  #define WASP_ATTRIBUTE_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
  #define WASP_ATTRIBUTE_EXPORT __attribute__((visibility("default")))
  #define WASP_ATTRIBUTE_IMPORT
#else
  #define WASP_ATTRIBUTE_EXPORT
  #define WASP_ATTRIBUTE_IMPORT
  #pragma warning Unknown dynamic link import/export semantics.
#endif

#ifdef WASP_LIBRARY_EXPORT
  #define WASP_API WASP_ATTRIBUTE_EXPORT
#else
  #define WASP_API WASP_ATTRIBUTE_IMPORT
#endif

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif

#include "panic.h"
