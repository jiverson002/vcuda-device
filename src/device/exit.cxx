// SPDX-License-Identifier: MIT
#include <iostream>

#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
void vcuda::Device::exit_handler(void) {
  log << ('`' != sym ? "|" : " ") << "  |- process cleanup..." << std::endl;
  (void)sem_unlink(done_fname);
  (void)sem_unlink(work_fname);
  log << ('`' != sym ? "|" : " ") << "  |  |- semaphore cleanup...done" << std::endl;
  (void)shm_del(regs);
  log << ('`' != sym ? "|" : " ") << "  |  |- shared memory cleanup...done" << std::endl;
  (void)close(pipe_rd);
  (void)close(pipe_wr);
  log << ('`' != sym ? "|" : " ") << "  |  `- pipe done...done" << std::endl;
}
