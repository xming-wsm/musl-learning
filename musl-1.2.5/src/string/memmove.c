#include <string.h>
#include <stdint.h>

#ifdef __GNUC__
typedef __attribute__((__may_alias__)) size_t WT;
#define WS (sizeof(WT))
#endif

void *memmove(void *dest, const void *src, size_t n)
{
	char *d = dest;
	const char *s = src;

	if (d==s) return d;     // dest与src指向同一块内存空间
	if ((uintptr_t)s-(uintptr_t)d-n <= -2*n) return memcpy(d, s, n);    // 复制区域未重叠

	if (d<s) {
#ifdef __GNUC__
        // dest地址比src地址低，从低地址段开始复制即可，不会影响
		if ((uintptr_t)s % WS == (uintptr_t)d % WS) {
            // 先判断src、dest的地址对WS取余后的结果是否相同
            // 实际上，src、dest字节对齐才是绝大多数情况。
			while ((uintptr_t)d % WS) {
                // 把余数部分进行单字节拷贝
				if (!n--) return dest;
				*d++ = *s++;
			}
            // 强转为size_t (unsigned long)类型，进行拷贝复制。
            // size_t是最长的数据类型，用该类型能加快复制速度
			for (; n>=WS; n-=WS, d+=WS, s+=WS) *(WT *)d = *(WT *)s;
		}
#endif
		for (; n; n--) *d++ = *s++;
	} else {
#ifdef __GNUC__
        // dest地址比src地址高，则需从dest开始，向低地址处复制拷贝
		if ((uintptr_t)s % WS == (uintptr_t)d % WS) {
			while ((uintptr_t)(d+n) % WS) {
				if (!n--) return dest;
				d[n] = s[n];
			}
			while (n>=WS) n-=WS, *(WT *)(d+n) = *(WT *)(s+n);
		}
#endif
		while (n) n--, d[n] = s[n];
	}

	return dest;
}
