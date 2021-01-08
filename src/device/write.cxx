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
vcuda::Device::write(const void *vbuf, size_t num) const {
  const char *buf = static_cast<const char *>(vbuf);
  while (num > 0) {
    ssize_t nr = ::write(pipe_wr, buf, num);
    if (-1 == nr) {
      std::fprintf(stderr, "device: write error: %s\n", std::strerror(errno));
      std::fflush(stdout);
      return -1;
    }
    num -= nr;
    buf += nr;
  }

  return 0;
}
