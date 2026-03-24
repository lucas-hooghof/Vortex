#include <generic/string.h>

void* memcpy(void* dest,const void* src,size_t count)
{
    asm volatile ("rep movsb" : : "D"(dest),"S"(src),"c"(count) : "memory");

    return dest;
}


void* memset(void* dst,int c,size_t size)
{
    asm volatile ("rep stosb" : : "c"(size),"D"(dst),"a"(c));
    return dst;
}

int64_t memcmp(void *m1, const void *m2, size_t len) {
    uint8_t *p = (uint8_t*)m1;
    const uint8_t *q = (uint8_t*)m2;
    for (size_t i = 0; i < len; i++)
        if (p[i] != q[i]) return (int64_t)(p[i]) - (int64_t)(q[i]);

    return 0;
}

int64_t strlen(const char* str)
{
    int64_t i = 0;
    while(*str)
    {
        i++;
        str++;
    }

    return i;
}