// SPDX-License-Identifier: MIT
#include "vcuda/core.h"
#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DEVICE_EXPORT CUresult
vcuda::Device::memFree(void) {
  CUdeviceptr dptr;

  // read arguments (dptr)
  argget(regs->args, dptr);

  if (-1 == shm_del(dptr))
    goto ERROR;

  // populate result
  return regs->res = CUDA_SUCCESS;

  ERROR:
  // populate result
  return regs->res = CUDA_ERROR;
}
