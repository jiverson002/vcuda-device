// SPDX-License-Identifier: MIT
#include <cerrno>
#include <cstdlib>

#include <semaphore.h>

#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DEVICE_EXPORT void vcuda::Device::poweron(void) {
  while (on) {
    if (-1 == sem_wait(regs->work)) {
      if (EINTR == errno)
        continue;
      CUDEVICEPANIC();
    }

    /* check again to see if the device is stilled powered on, in case it
     * acquired regs->work only due to it being terminated. */
    if (!on)
      break;

    // dispatch the cmd on this device
    (this->*(regs->cmd))();

    if (-1 == sem_post(regs->done))
      CUDEVICEPANIC();
  }

  exit(CUDA_SUCCESS);
}
