// SPDX-License-Identifier: MIT
#include <signal.h>
#include <sys/wait.h>

#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DEVICE_EXPORT void vcuda::Device::poweroff(void) const {
  *log << pfx << "  |- waiting for process#" << id << "..." << std::endl;
  if (0 == kill(id, SIGTERM)) {
    (void)sem_post(regs->work);
    int wstatus;
    (void)waitpid(id, &wstatus, 0);
  }
}
