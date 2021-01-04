// SPDX-License-Identifier: MIT
#include <cstdio>
#include <cstdlib>

#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
void vcuda::Device::panic(const char * const filename,
                          const char * const funcname,
                          const int line)
{
  if (on) {
    std::fprintf(stderr, "PANIC: device#%d: %s:%s@%d\n", id, filename, funcname,
                 line);
    std::fflush(stderr);
    std::abort();
  }
}
