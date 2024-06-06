#ifndef MALLOC_IMPL_H
#define MALLOC_IMPL_H

#include <sys/mman.h>

hidden void *__expand_heap(size_t *);

hidden void __malloc_donate(char *, char *);

hidden void *__memalign(size_t, size_t);

struct chunk {
	size_t psize, csize;
	struct chunk *next, *prev;
};

struct bin {
	volatile int lock[2];
	struct chunk *head;
	struct chunk *tail;
};

#define SIZE_ALIGN (4*sizeof(size_t))       // 最小的chunk大小
#define SIZE_MASK (-SIZE_ALIGN)             
#define OVERHEAD (2*sizeof(size_t))         // chunk的前导大小，psize和csize占用的字节

// MMAP门限值，0x1c00*4*sizeof(size_t)，32位系统是112kb，64位是224kb
// 补充：224kb并不是乱取的，其为bin链表中管理的最大的chunk大小。如果超过了
// 224kb，则直接用mmap分配内存，而不是搜索bin表中的free chunk
#define MMAP_THRESHOLD (0x1c00*SIZE_ALIGN) 
#define DONTCARE 16
#define RECLAIM 163840  // 160kb

#define CHUNK_SIZE(c) ((c)->csize & -2)     //   -2的补码为11111110，进行按位与运算会把最低位清零
#define CHUNK_PSIZE(c) ((c)->psize & -2)

// chunk是连续的内存块，因此，偏置CHUNK_PSIZE个字节后，可以得到上一个chunk的地址
#define PREV_CHUNK(c) ((struct chunk *)((char *)(c) - CHUNK_PSIZE(c)))
#define NEXT_CHUNK(c) ((struct chunk *)((char *)(c) + CHUNK_SIZE(c)))

// 用户使用的内存地址，与chunk的地址地址之间相差OVERHEAD个字节
#define MEM_TO_CHUNK(p) (struct chunk *)((char *)(p) - OVERHEAD)
#define CHUNK_TO_MEM(c) (void *)((char *)(c) + OVERHEAD)

// 取head的地址，然后从内存转换到chunk，因为bin是一种特殊的chunk，所以等于获取bin结构的首地址
#define BIN_TO_CHUNK(i) (MEM_TO_CHUNK(&mal.bins[i].head))

// 对于allocated chunk，flag=1表示使用的是小内存，flag=0表示是大内存
// flag位为1，表示申请的内存是小内存
#define C_INUSE  ((size_t)1)
// flag位为0，表示内存是通过mmap分配的大内存。
#define IS_MMAPPED(c) !((c)->csize & (C_INUSE))

hidden void __bin_chunk(struct chunk *);

hidden extern int __malloc_replaced;

#endif
