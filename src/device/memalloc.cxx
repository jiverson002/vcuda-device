// SPDX-License-Identifier: MIT
#include <cstddef>

#include "vcuda/core.h"
#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DEVICE_EXPORT CUresult
vcuda::Device::memAlloc(void) {
  std::size_t bytesize;

  // read arguments (bytesize)
  argget(regs->args, bytesize);

  void * const dptr = shm_new(bytesize);
  if (NULL == dptr)
    GOTO(ERROR);

  // populate output params (dptr)
  argput(regs->args, dptr);

  // record size of output
  regs->argsbytes = sizeof(dptr);

  // populate result
  return regs->res = CUDA_SUCCESS;

  ERROR:
  // populate result
  return regs->res = CUDA_ERROR;
}
