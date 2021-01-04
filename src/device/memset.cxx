// SPDX-License-Identifier: MIT
#include <cstddef>
#include <cstring>

#include "vcuda/core.h"
#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DEVICE_EXPORT CUresult
vcuda::Device::memSet(void) {
  CUdeviceptr dptr;
  int value;
  size_t num;

  // read arguments (dptr, value, num)
  argget(regs->args, dptr, value, num);

  // execute command
  std::memset(dptr, value, num);

  // populate result
  return regs->res = CUDA_SUCCESS;
}
