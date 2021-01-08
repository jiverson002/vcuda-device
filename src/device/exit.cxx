// SPDX-License-Identifier: MIT
#include <iostream>
#include <string>

#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
void vcuda::Device::exit_handler(void) {
  cleanup(pfx + "  |  ");
}
