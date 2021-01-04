// SPDX-License-Identifier: MIT
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "vcuda/core.h"
#include "vcuda/device.h"

// compute a header size that is align suitably for any type
static inline std::size_t hdrsize(void) {
  std::size_t size = 0;
  const std::size_t align = alignof(std::max_align_t);

  size += 64;
  size += sizeof(unsigned int);
  if (size % align)
    size += align - size % align;

  return size;
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
void* vcuda::Device::shm_new(std::size_t bytesize, const char *ufname) {
  int fd;
  char *dptr = NULL;
  char fname[64];

  if (!ufname) {
    // generate shared memory object name
    if (0 >= std::snprintf(fname, sizeof(fname), "/cu%d-shm%d", id, uniq))
      GOTO(ERROR);
    uniq++;
  } else {
    assert(std::strlen(ufname) < sizeof(fname));
    std::strncpy(fname, ufname, sizeof(fname) - 1);
  }

  // adjust allocation size -- make room for storage of header
  bytesize += hdrsize();

  // open shared memory object
  fd = shm_open(fname, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
#if 1
  if (-1 == fd)
    GOTO(ERROR);

  if (-1 == ftruncate(fd, bytesize))
    GOTO(ERROR);
#else
  if (-1 == fd) {
    if (EEXIST == errno) {
      fd = shm_open(fname, O_RDWR, S_IRUSR|S_IWUSR);
      if (-1 == fd)
        GOTO(ERROR);
    } else {
      GOTO(ERROR);
    }
  } else {
    if (-1 == ftruncate(fd, bytesize))
      GOTO(ERROR);
  }
#endif

  /*--------------------------------------------------------------------------*/
  /* !! Shared memory object has been opened -- any failure after this should
   *    close the file and unlink the shared memory object. !! */
  /*--------------------------------------------------------------------------*/

  // map shared memory region
  dptr = static_cast<decltype(dptr)>(mmap(NULL, bytesize, PROT_READ|PROT_WRITE,
                                          MAP_SHARED, fd, 0));
  if (MAP_FAILED == dptr)
    GOTO(UNLINK);

  /*--------------------------------------------------------------------------*/
  /* !! Shared memory region has been mapped -- any failure after this should
   *    unmap the region. !! */
  /*--------------------------------------------------------------------------*/

  // close file descriptor
  if (-1 == close(fd))
    GOTO(UNMAP);

  /*--------------------------------------------------------------------------*/
  /* !! Allocation has completed successfully -- store some book-keeping
   *    information in the allocation and return. !! */
  /*--------------------------------------------------------------------------*/

  /* store bytesize in the allocation */
  std::memcpy(dptr, &bytesize, sizeof(bytesize));

  /* store shared memory object name in the allocation */
  std::memcpy(dptr + sizeof(bytesize), fname, sizeof(fname));

  return dptr + hdrsize();

  UNMAP:
  (void)munmap(dptr, bytesize);

  UNLINK:
  (void)shm_unlink(fname);

  ERROR:
  return NULL;
}

int vcuda::Device::shm_del(void *ptr) {
  int res = 0;
  char * const cptr = static_cast<char*>(ptr) - hdrsize();
  std::size_t bytesize;
  char fname[64];

  // retrieve book-keeping info
  std::memcpy(&bytesize, cptr, sizeof(bytesize));
  std::memcpy(fname, cptr + sizeof(bytesize), sizeof(fname));

  if (-1 == munmap(cptr, bytesize))
    res = -1;

  if (-1 == shm_unlink(fname))
    res = -1;

  return res;
}
