// SPDX-License-Identifier: MIT
#include <iostream>
#include <string>

#include <semaphore.h>
#include <unistd.h>

#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
void vcuda::Device::cleanup(const std::string &pfx) {
  *log << pfx << "|- pipe cleanup..." << std::endl;
  (void)close(pipe_rd);
  *log << pfx << "|  |- " << pipe_rd << "...done" << std::endl;
  (void)close(pipe_wr);
  *log << pfx << "|  `- " << pipe_wr << "...done" << std::endl;

  *log << pfx << "|- shared memory cleanup..." << std::endl;
  (void)shm_del(regs);
  *log << pfx << "|  `- " << regs_fname << "...done" << std::endl;

  *log << pfx << "`- semaphore cleanup..." << std::endl;
  (void)sem_unlink(work_fname);
  *log << pfx << "   |- " << work_fname << "...done" << std::endl;
  (void)sem_unlink(done_fname);
  *log << pfx << "   `- " << done_fname << "...done" << std::endl;
}
