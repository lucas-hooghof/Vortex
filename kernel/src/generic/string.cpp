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