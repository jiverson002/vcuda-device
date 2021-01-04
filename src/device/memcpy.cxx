// SPDX-License-Identifier: MIT
#include <cstddef>

#include "vcuda/core.h"
#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DEVICE_EXPORT CUresult
vcuda::Device::memCpyDtoH(void) {
  CUdeviceptr dptr;
  size_t num;

  // read arguments (dptr, num)
  argget(regs->args, dptr, num);

  // write memory to pipe
  if (-1 == write(dptr, num))
    goto ERROR;

  // populate result
  return regs->res = CUDA_SUCCESS;

  ERROR:
  // populate result
  return regs->res = CUDA_ERROR;
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DEVICE_EXPORT CUresult
vcuda::Device::memCpyHtoD(void) {
  CUdeviceptr dptr;
  size_t num;

  // read arguments (dptr, num)
  argget(regs->args, dptr, num);

  // read memory from pipe
  if (-1 == read(dptr, num))
    goto ERROR;

  // populate result
  return regs->res = CUDA_SUCCESS;

  ERROR:
  // populate result
  return regs->res = CUDA_ERROR;
}
