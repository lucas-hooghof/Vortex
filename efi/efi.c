#include "efi.h"

void* memset(void* dest,uint8_t c,size_t n)
{
    uint8_t* dest8 = (uint8_t*)dest;
    for (size_t i = 0; i < n; i++)
    {
        dest8[i] = c;
    }
    
    return dest;
}


UINTN strlen_c16(CHAR16 *s) {
    UINTN len = 0;
    while (*s++) len++;
    return len;
}

CHAR16 *strrev_c16(CHAR16 *s) {
    if (!s) return s;

    CHAR16 *start = s, *end = s + strlen_c16(s)-1;
    while (start < end) {
        CHAR16 temp = *end;  // Swap
        *end-- = *start;
        *start++ = temp;
    }

    return s;
}

CHAR16 buffer[24];
CHAR16* itoa(UINTN number,UINT8 base)
{
    memset(buffer,0,24);
    const CHAR16 *digits = u"0123456789ABCDEF";

    int i = 0;

    do 
    {
        buffer[i++] = digits[number % base];
        number /= base;
    } while (number > 0);

    if (base == 16)
    {
        buffer[i++] = u'x';
        buffer[i++] = u'0';
    }

    strrev_c16(buffer);

    return buffer;
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle,EFI_SYSTEM_TABLE* Systemtable)
{
    Systemtable->ConOut->ClearScreen(Systemtable->ConOut);

    while(1) {}

    return EFI_SUCCESS;
}