#include "stdio.hpp"

Framebuffer framebuffer;
PSF1_FONT* font;

uint32_t x,y = 0;

void Initilize(bootinfo_t bootinfo){
    framebuffer = bootinfo.framebuffer;
    font = bootinfo.bootfont;
}

void putc(char c)
{
    uint32_t *fb = (uint32_t*)framebuffer.BaseAddress;
    char* FontPtr = (char*)font->glyphBuffer + (c * font->psf1_Header->charsize);
    switch (c)
    {
        case '\n':
            y += 16;
            x = 0;
            return;
        case '\t':
            x += 4*8;
            return;
        default: 
            break;
    }
    for (uint32_t yoff = y; yoff < y+16; yoff++)
    {
        for (uint32_t xoff = x; xoff < x+8; xoff++)
        {
                if ((*FontPtr & (0b10000000 >> (xoff-x))) > 0){
                    *(unsigned int*)(fb + xoff + (yoff * framebuffer.PixelsPerScanLine)) = 0xFFFFFFFF;
                }
        }
        FontPtr++;
    }

    if (x + 8 >= framebuffer.PixelsPerScanLine) {y+= 16; x=0;}
    else {x+= 8;}
}

void puts(const char* str)
{
    while (*str)
    {
        putc(*str);
        str++;
    }
}

static const char hexchars[] = "0123456789ABCDEF";

static void print_unsigned(uint64_t value, uint32_t base)
{
    char buffer[32];
    int pos = 0;

    if (base < 2 || base > 16)
        base = 10;

    do {
        buffer[pos++] = hexchars[value % base];
        value /= base;
    } while (value && pos < 32);

    while (pos--)
        putc(buffer[pos]);
}

static void print_signed(int64_t value)
{
    if (value < 0) {
        putc('-');
        print_unsigned((uint64_t)(-value), 10);
    } else {
        print_unsigned((uint64_t)value, 10);
    }
}

void printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            putc(*fmt++);
            continue;
        }

        fmt++;

        switch (*fmt) {

        case 'd':
        case 'i':
            print_signed(va_arg(args, int));
            break;

        case 'u':
            print_unsigned(va_arg(args, unsigned int), 10);
            break;

        case 'x':
            print_unsigned(va_arg(args, unsigned int), 16);
            break;

        case 'p':
            puts("0x");
            print_unsigned((uint64_t)va_arg(args, void*), 16);
            break;

        case 'c':
            putc((char)va_arg(args, int));
            break;

        case 's':
        {
            const char* s = va_arg(args, const char*);
            if (!s) s = "(null)";
            while (*s) putc(*s++);
            break;
        }

        case '%':
            putc('%');
            break;

        default:
            putc('%');
            putc(*fmt);
            break;
        }

        fmt++;
    }

    va_end(args);
}