#pragma once

#include <cstdint>

#ifdef __GNUC__
  #if __GNUC__ < 8
    #error "require gcc >= 8.x"
  #endif
#endif
