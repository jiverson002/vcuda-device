// SPDX-License-Identifier: MIT
#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
void vcuda::Device::term_handler(int signum) {
  on = false;

  (void)signum;
}
