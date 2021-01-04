// SPDX-License-Identifier: MIT
#ifndef VCUDA_DEVICE_H
#define VCUDA_DEVICE_H 1
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <vector>

#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>

#include "vcuda/core.h"
#include "vcuda/device/export.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
#ifndef GOTO
# define GOTO(lbl) do {\
  std::fprintf(stderr, "device#%d: failure in %s @ %s:%d\n", getpid(),\
               __FILE__, __func__, __LINE__);\
  if (0 != errno)\
    std::fprintf(stderr, "  errno: %s\n", std::strerror(errno));\
  std::fflush(stderr);\
  goto lbl;\
} while (0)
#endif

namespace vcuda {
  /*--------------------------------------------------------------------------*/
  /*! */
  /*--------------------------------------------------------------------------*/
  class Device {
    public:
      struct registers {
        public:
          sem_t *work;
          sem_t *done;
          CUresult(Device::* volatile cmd)(void);
          volatile size_t argsbytes;
          volatile CUresult res;
          char args[4096];
      };

      Device(int devnum, std::ostream &log, char sym);
      ~Device(void);

      CUresult launchKernel(void);
      CUresult memAlloc(void);
      CUresult memCpyDtoH(void);
      CUresult memCpyHtoD(void);
      CUresult memFree(void);
      CUresult memSet(void);

      void poweron(void);
      void poweroff(void) const;

      int read(void *, size_t);
      int write(const void *, size_t);

      registers *regs;  /*!< set of registers */

    private:
      pid_t id; /*!< device ID */
      char sym; /*!< symbol used in pretty output */

      volatile bool on; /*!< indicator variable that device has been powered
                             on (true) or off (false) */

      int pipe_wr;      /*!< host-to-device pipe */
      int pipe_rd;      /*!< device-to-host pipe */

      int uniq;             /*!< unique identifier for shared mem */
      char regs_fname[64];  /*!< file name of shared object for registers */
      char work_fname[64];  /*!< file name of work semaphore */
      char done_fname[64];  /*!< file name of done semaphore */

      std::ostream &log;

      void* shm_new(std::size_t bytesize, const char *fname = NULL);
      int   shm_del(void *ptr);

      bool isDev(void) { return id == getpid(); }

      template <typename T> inline
      const char * argget(const char *buf, T& arg) {
        std::memcpy(&arg, buf, sizeof(arg));
        return buf + sizeof(arg);
      }
      template <typename T, typename... Args> inline
      const char * argget(const char *buf, T& arg, Args&... args) {
        std::memcpy(&arg, buf, sizeof(arg));
        return argget(buf + sizeof(arg), args...);
      }
      const char * arggetv(const char *buf, const int argc,
                           std::vector<void*> &args)
      {
        for (int i = 0; i < argc; i++) {
          size_t size;
          buf = argget(buf, size);

          args.push_back(malloc(size));
          if (!args.back())
            throw std::bad_alloc();

          std::memcpy(args.back(), buf, size);
          buf += size;
        }
        return buf;
      }

      template <typename T> inline
      char * argput(char *buf, const T& arg) {
        std::memcpy(buf, &arg, sizeof(arg));
        return buf + sizeof(arg);
      }
      template <typename T, typename... Args> inline
      char * argput(char *buf, const T& arg, const Args&... args) {
        std::memcpy(buf, &arg, sizeof(arg));
        return argput(buf + sizeof(arg), args...);
      }

      void term_handler(int signum);
      void exit_handler(void);

      void panic(const char * const filename, const char * const funcname,
                 const int line);
  };
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
#define CUDEVICEPANIC() panic(__FILE__, __func__, __LINE__)

#endif // VCUDA_DEVICE_H
