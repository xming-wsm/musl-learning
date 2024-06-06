#include "syscall.h"
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

static void dummy(void) {}
weak_alias(dummy, __vm_wait);

#define UNIT SYSCALL_MMAP2_UNIT

// 定义掩码OFF_MASK，用于检查偏移量是否合法
#define OFF_MASK ((-0x2000ULL << (8 * sizeof(syscall_arg_t) - 1)) | (UNIT - 1))

void *__mmap(void *start, size_t len, int prot, int flags, int fd, off_t off) {
  long ret;
  if (off & OFF_MASK) {
    errno = EINVAL;
    return MAP_FAILED;
  }

  // 超出内存限制
  if (len >= PTRDIFF_MAX) {
    errno = ENOMEM;
    return MAP_FAILED;
  }
  if (flags & MAP_FIXED) {
    __vm_wait();
  }
#ifdef SYS_mmap2
  ret = __syscall(SYS_mmap2, start, len, prot, flags, fd, off / UNIT);
#else
  ret = __syscall(SYS_mmap, start, len, prot, flags, fd, off);
#endif
  /* Fixup incorrect EPERM from kernel. */
  if (ret == -EPERM && !start && (flags & MAP_ANON) && !(flags & MAP_FIXED))
    ret = -ENOMEM;
  return (void *)__syscall_ret(ret);
}

weak_alias(__mmap, mmap);

weak_alias(mmap, mmap64);
