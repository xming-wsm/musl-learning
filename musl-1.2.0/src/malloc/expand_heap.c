#include "libc.h"
#include "malloc_impl.h"
#include "syscall.h"
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <sys/mman.h>

/* This function returns true if the interval [old,new]
 * intersects the 'len'-sized interval below &libc.auxv
 * (interpreted as the main-thread stack) or below &b
 * (the current stack). It is used to defend against
 * buggy brk implementations that can cross the stack. */

static int traverses_stack_p(uintptr_t old, uintptr_t new) {
  const uintptr_t len = 8 << 20;
  uintptr_t a, b;

  b = (uintptr_t)libc.auxv;
  a = b > len ? b - len : 0;
  if (new > a && old < b)
    return 1;

  b = (uintptr_t)&b;
  a = b > len ? b - len : 0;
  if (new > a && old < b)
    return 1;

  return 0;
}

/* Expand the heap in-place if brk can be used, or otherwise via mmap,
 * using an exponential lower bound on growth by mmap to make
 * fragmentation asymptotically irrelevant. The size argument is both
 * an input and an output, since the caller needs to know the size
 * allocated, which will be larger than requested due to page alignment
 * and mmap minimum size rules. The caller is responsible for locking
 * to prevent concurrent calls. */

void *__expand_heap(size_t *pn) {
  static uintptr_t brk;
  static unsigned mmap_step;
  size_t n = *pn;

  if (n > SIZE_MAX / 2 - PAGE_SIZE) {
    errno = ENOMEM;
    return 0;
  }

  // 对传入的大侠进行调整，确保其为页面的整数倍
  n += -n & PAGE_SIZE - 1;

  // 如果brk为0，表示为第一次分配堆内存。则获取当前brk的位置，将其调整为页面大小的
  // 整数倍
  if (!brk) {
    brk = __syscall(SYS_brk, 0);
    brk += -brk & PAGE_SIZE - 1;
  }

  // 如果新的大小小于当前堆的剩余空间，并且不会与堆栈冲突
  // （即不会覆盖堆栈的内存），则直接在原地扩展堆。
  // 函数更新 pn 的值，增加堆的大小，并返回新分配的内存块的指针。
  if (n < SIZE_MAX - brk && !traverses_stack_p(brk, brk + n) &&
      __syscall(SYS_brk, brk + n) == brk + n) {
    *pn = n;

    // 更新堆长度
    brk += n;
    return (void *)(brk - n);
  }

  // 无法进行原地扩展堆，则会使用mmap分配新的内存区域
  size_t min = (size_t)PAGE_SIZE << mmap_step / 2;
  if (n < min)
    n = min;
  void *area =
      __mmap(0, n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (area == MAP_FAILED)
    return 0;
  *pn = n;
  mmap_step++;
  return area;
}
