// SPDX-License-Identifier: MIT
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>

#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "vcuda/core.h"
#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
static std::function<void(int)> static_term_handler_wrapper;

static void static_term_handler(int value) {
  static_term_handler_wrapper(value);
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
static std::function<void(void)> static_exit_handler_wrapper;

static void static_exit_handler(void) {
  static_exit_handler_wrapper();
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DEVICE_EXPORT
vcuda::Device::Device(int devnum, std::ostream *log, const std::string &pfx)
  : devnum(devnum), pfx(pfx), on(true), uniq(0), log(log)
{
  int htod2[2];
  int dtoh2[2];

  // registers: create shared memory object name
  if (0 >= std::snprintf(regs_fname, sizeof(regs_fname), "/cu-regs-%d-%d", getpid(), devnum))
    GOTO(ERROR);

  // work: create semaphore object name
  if (0 >= std::snprintf(work_fname, sizeof(work_fname), "/cu-work-%d-%d", getpid(), devnum))
    GOTO(ERROR);

  // done: create semaphore object name
  if (0 >= std::snprintf(done_fname, sizeof(done_fname), "/cu-done-%d-%d", getpid(), devnum))
    GOTO(ERROR);

  // pipe: create the device-to-host pipe
  if (-1 == pipe(dtoh2))
    GOTO(ERROR);

  /*--------------------------------------------------------------------------*/
  /* !! DTOH_CLOSE: Device-to-host pipe has been created -- any failure after
   *    this should close both file descriptors. !! */
  /*--------------------------------------------------------------------------*/

  // pipe: create the host-to-device pipe
  if (-1 == pipe(htod2))
    GOTO(DTOH_CLOSE);

  /*--------------------------------------------------------------------------*/
  /* !! HTOD_CLOSE: Host-to-device pipe has been created -- any failure after
   *    this should close both file descriptors. !! */
  /*--------------------------------------------------------------------------*/

  // registers: open shared memory object
  regs = static_cast<decltype(regs)>(shm_new(sizeof(*regs), regs_fname));
  if (NULL == regs)
    GOTO(HTOD_CLOSE);

  /*--------------------------------------------------------------------------*/
  /* !! SHM_DELETE: Shared memory region has been created -- any failure after
   *    this should delete the region. !! */
  /*--------------------------------------------------------------------------*/

  // work: open semaphore
  regs->work = sem_open(work_fname, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR, 0);
  if (SEM_FAILED == regs->work)
    GOTO(SHM_DELETE);

  /*--------------------------------------------------------------------------*/
  /* !! WORK_DESTROY: Work semaphore has been initialized -- any failure after
   *    this should destroy the semaphore. !! */
  /*--------------------------------------------------------------------------*/

  // done: open semaphore
  regs->done = sem_open(done_fname, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR, 0);
  if (SEM_FAILED == regs->done)
    GOTO(WORK_DESTROY);

  /*--------------------------------------------------------------------------*/
  /* !! DONE_DESTROY: Done semaphore has been initialized -- any failure after
   *    this should destroy the semaphore. !! */
  /*--------------------------------------------------------------------------*/

  // process: create the device process
  switch (id = fork()) {
    case -1:  // process creation failed
    GOTO(DONE_DESTROY);

    case 0:   // device process
    id = getpid(); // get the process id
    break;

    default:  // driver process
    break;
  };

  /*--------------------------------------------------------------------------*/
  /* !! POWEROFF: Device process has been forked -- any failure after this
   *    should poweroff the device. !! */
  /*--------------------------------------------------------------------------*/

  // dtoh: close unused side of dtoh pipe
  if (-1 == close(dtoh2[isDev() ? 0 : 1]))
    GOTO(POWEROFF);

  // htod: close unused side of htod pipe
  if (-1 == close(htod2[isDev() ? 1 : 0]))
    GOTO(POWEROFF);

  // record the sides of the pipes to be used
  pipe_rd = isDev() ? htod2[0] : dtoh2[0];
  pipe_wr = isDev() ? dtoh2[1] : htod2[1];

  /* SIGTERM: set the signal handler for poweroff() */
  if (isDev()) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = static_term_handler;

    static_term_handler_wrapper = std::bind(&Device::term_handler, this,
                                            std::placeholders::_1);
    if (-1 == sigaction(SIGTERM, &sa, NULL))
      GOTO(POWEROFF);

    static_exit_handler_wrapper = std::bind(&Device::exit_handler, this);
    if (0 != std::atexit(static_exit_handler))
      GOTO(POWEROFF);
  }

  /*--------------------------------------------------------------------------*/
  /* !! Device initialization has completed successfully. !! */
  /*--------------------------------------------------------------------------*/
  return;

  POWEROFF:
  /* if driver process fails to correctly initialize its side of the device,
   * then it should poweroff the device. */
  if (!isDev())
    poweroff();

  DONE_DESTROY:
  (void)sem_unlink(done_fname);

  WORK_DESTROY:
  (void)sem_unlink(work_fname);

  SHM_DELETE:
  (void)shm_del(regs);

  HTOD_CLOSE:
  (void)close(htod2[0]);
  (void)close(htod2[1]);

  DTOH_CLOSE:
  (void)close(dtoh2[0]);
  (void)close(dtoh2[1]);

  ERROR:
  throw "device: initialization failed";
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DEVICE_EXPORT
vcuda::Device::~Device(void) {
  assert(!isDev());

  pfx.pop_back();
  *log << pfx << "`- deconstructing device#" << devnum << "..." << std::endl;
  pfx.push_back(' ');

  poweroff();

  cleanup(pfx + "  ");
}
