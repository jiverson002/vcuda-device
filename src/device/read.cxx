// SPDX-License-Identifier: MIT
#include <cerrno>
#include <cstdio>
#include <cstring>

#include <unistd.h>

#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DEVICE_EXPORT int
vcuda::Device::read(void *vbuf, size_t num) const {
  char *buf = static_cast<char *>(vbuf);
  while (num > 0) {
    ssize_t nr = ::read(pipe_rd, buf, num);
    if (-1 == nr) {
      std::fprintf(stderr, "device: read error: %s\n", std::strerror(errno));
      std::fflush(stdout);
      return -1;
    }
    num -= nr;
    buf += nr;
  }

  return 0;
}
