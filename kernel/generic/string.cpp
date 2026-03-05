#include <generic/string.h>

void* memset(void* dst,int c,size_t n)
{
    uint8_t* dest = (uint8_t*)dst;

    for (size_t i = 0; i < n; i++)
    {
        dest[i] = c;
    }

    return dst;
}