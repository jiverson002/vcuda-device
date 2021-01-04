// SPDX-License-Identifier: MIT
#include <cerrno>
#include <cstdlib>

#include <semaphore.h>

#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DEVICE_EXPORT void vcuda::Device::poweron(void) {
  on = true;

  while (on) {
    if (-1 == sem_wait(regs->work)) {
      if (EINTR == errno)
        continue;
      CUDEVICEPANIC();
    }

    // dispatch the cmd on this device
    (this->*(regs->cmd))();

    if (-1 == sem_post(regs->done))
      CUDEVICEPANIC();
  }

  exit(CUDA_SUCCESS);
}
