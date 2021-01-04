// SPDX-License-Identifier: MIT
#include <cstdlib>
#include <cstring>
#include <vector>

#include <sys/wait.h>

#include "vcuda/core.h"
#include "vcuda/device.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

VCUDA_DEVICE_EXPORT CUresult
vcuda::Device::launchKernel(void) {
  CUfunction f;
  dim3 gridDim;
  dim3 blockDim;
  unsigned int sharedMemBytes;
  int argc;
  std::vector<void*> kernelParams;
  void ** extra = NULL;

  const char *rargs = regs->args;

  // get fixed length args
  rargs = argget(rargs, gridDim, blockDim, sharedMemBytes, f.fn, argc);

  // get variable length args
  arggetv(rargs, argc, kernelParams);

  /*--------------------------------------------------------------------------*/
  /* !! All parameters have been copied, prepared for kernel launch. !! */
  /*--------------------------------------------------------------------------*/

  for (unsigned blockIdxX = 0; blockIdxX < gridDim.x; blockIdxX++) {
    for (unsigned blockIdxY = 0; blockIdxY < gridDim.y; blockIdxY++) {
      for (unsigned blockIdxZ = 0; blockIdxZ < gridDim.z; blockIdxZ++) {
        if (0 != fork()) // TODO: handle fork failure
          continue;

#       pragma omp parallel
#       pragma omp single
#       pragma omp taskloop collapse(3)
        for (unsigned threadIdxX = 0; threadIdxX < blockDim.x; threadIdxX++) {
          for (unsigned threadIdxY = 0; threadIdxY < blockDim.y; threadIdxY++) {
            for (unsigned threadIdxZ = 0; threadIdxZ < blockDim.z; threadIdxZ++) {
              f.fn(
                0,
                gridDim,
                blockDim,
                dim3(blockIdxX, blockIdxY, blockIdxZ),
                dim3(threadIdxX, threadIdxY, threadIdxZ),
                kernelParams.data(),
                extra
              );
            }
          }
        }

        _exit(CUDA_SUCCESS);
      }
    }
  }

  for (unsigned blockIdxX = 0; blockIdxX < gridDim.x; blockIdxX++) {
    for (unsigned blockIdxY = 0; blockIdxY < gridDim.y; blockIdxY++) {
      for (unsigned blockIdxZ = 0; blockIdxZ < gridDim.z; blockIdxZ++) {
        int wstatus;
        (void)wait(&wstatus);
        /* TODO: check return status of blocks -- they could have segfaulted */
      }
    }
  }

  for (auto& kp : kernelParams)
    free(kp);

  regs->res = CUDA_SUCCESS;

  return CUDA_SUCCESS;
}

#pragma GCC diagnostic pop
