#include <string.h>
#include <stdint.h>
#include <endian.h>

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

#ifdef __GNUC__

// 根据大端、小端定义按位左移操作
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define LS >>
#define RS <<
#else
#define LS <<
#define RS >>
#endif

    // 开启 __may_alias__ 属性，表示 u32 可以指带其他类型
	typedef uint32_t __attribute__((__may_alias__)) u32;
	uint32_t w, x;

    // Todo: src 4字节对齐
    // uintptr_t为无符号整数指针，下面fo循环是为了确保源地址
    // src是4字节对齐的。当src没有4字节对齐时，单个字节复制
    // 直到src字节对齐，退出for循环
	for (; (uintptr_t)s % 4 && n; n--) *d++ = *s++;


    // 若目标地址也是4字节对齐，则进入if条件语句中
	if ((uintptr_t)d % 4 == 0) {

        // 如果剩余字节数大于16,则每次复制4次。
        // uint32是4个字节，4次就是16个字节。
		for (; n>=16; s+=16, d+=16, n-=16) {
			*(u32 *)(d+0) = *(u32 *)(s+0);
			*(u32 *)(d+4) = *(u32 *)(s+4);
			*(u32 *)(d+8) = *(u32 *)(s+8);
			*(u32 *)(d+12) = *(u32 *)(s+12);
		}

        // 如果n大小为15-8，那么与0b01000按位与
        // 后的结果一定为0b1000，会进入条件语句中
        // 完成8个字节的复制
		if (n&8) {
			*(u32 *)(d+0) = *(u32 *)(s+0);
			*(u32 *)(d+4) = *(u32 *)(s+4);
			d += 8; s += 8;
		}

        // 同理，如果n大小为7-4，那么与0b100按位与
        // 的结果一定为0b100，会进入条件语句中
        // 完成4个字节的复制
		if (n&4) {
			*(u32 *)(d+0) = *(u32 *)(s+0);
			d += 4; s += 4;
		}
        // 同理，略
		if (n&2) {
			*d++ = *s++; *d++ = *s++;
		}
		if (n&1) {
			*d = *s;
		}
        // 完成字节拷贝，返回目标源地址
		return dest;
	}

    // dest字节未对齐情况
	if (n >= 32) switch ((uintptr_t)d % 4) {
	case 1:
		w = *(u32 *)s;      // 取出目标源数据 
        *d++ = *s++;        // 先拷贝3个字节，使dest4字节对齐
		*d++ = *s++;
		*d++ = *s++;
		n -= 3;
		for (; n>=17; s+=16, d+=16, n-=16) {
            // 通过对w,x的剪裁拼接，将结果保存到src中.
			x = *(u32 *)(s+1);
			*(u32 *)(d+0) = (w LS 24) | (x RS 8);
			w = *(u32 *)(s+5);
			*(u32 *)(d+4) = (x LS 24) | (w RS 8);
			x = *(u32 *)(s+9);
			*(u32 *)(d+8) = (w LS 24) | (x RS 8);
			w = *(u32 *)(s+13);
			*(u32 *)(d+12) = (x LS 24) | (w RS 8);
		}
		break;
	case 2:
        // 同理。也是先拷贝2个数据，然后
		w = *(u32 *)s;
		*d++ = *s++;
		*d++ = *s++;
		n -= 2;
		for (; n>=18; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+2);
			*(u32 *)(d+0) = (w LS 16) | (x RS 16);
			w = *(u32 *)(s+6);
			*(u32 *)(d+4) = (x LS 16) | (w RS 16);
			x = *(u32 *)(s+10);
			*(u32 *)(d+8) = (w LS 16) | (x RS 16);
			w = *(u32 *)(s+14);
			*(u32 *)(d+12) = (x LS 16) | (w RS 16);
		}
		break;
	case 3:
		w = *(u32 *)s;
		*d++ = *s++;
		n -= 1;
		for (; n>=19; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+3);
			*(u32 *)(d+0) = (w LS 8) | (x RS 24);
			w = *(u32 *)(s+7);
			*(u32 *)(d+4) = (x LS 8) | (w RS 24);
			x = *(u32 *)(s+11);
			*(u32 *)(d+8) = (w LS 8) | (x RS 24);
			w = *(u32 *)(s+15);
			*(u32 *)(d+12) = (x LS 8) | (w RS 24);
		}
		break;
	}

    // 到这里，未拷贝的字节数量已经小于32了，就直接与16、8
    // 4、2、1进行按位与，然后进行字节拷贝即可。
	if (n&16) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&8) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&4) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&2) {
		*d++ = *s++; *d++ = *s++;
	}
	if (n&1) {
		*d = *s;
	}
	return dest;
#endif

	for (; n; n--) *d++ = *s++;
	return dest;
}
