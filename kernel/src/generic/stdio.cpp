#include <generic/stdio.h>

#include <generic/io.h>
#include <generic/string.h>

#define QEMU_PORT 0xE9

FRAMEBUFFER* Logger::s_framebuffer = nullptr;
PSF1_FONT* Logger::s_font = nullptr;
uint32_t Logger::s_x = 0;
uint32_t Logger::s_y = 0;
uint32_t Logger::Foregroundcolor = 0;
uint32_t Logger::BackGroundColor = 0;

 
void Logger::Initilize(bootinfo_t *info)
{
    s_framebuffer = info->framebuffer;
    s_font = info->font;
    memset(s_framebuffer->BaseAddress,0,s_framebuffer->BufferSize);
    Foregroundcolor = 0xFFFFFFFF;
    BackGroundColor = 0x00000000;
    uint32_t* fb = (uint32_t*)s_framebuffer->BaseAddress;
    for (size_t i = 0; i < s_framebuffer->PixelsPerScanLine * s_framebuffer->Height; i++)
        fb[i] = BackGroundColor;
}

void Logger::ClearScreen(uint32_t foregroundColor,uint32_t backgroundColor)
{
    Foregroundcolor = foregroundColor;
    BackGroundColor = backgroundColor;

    uint32_t* fb = (uint32_t*)s_framebuffer->BaseAddress;
    for (size_t i = 0; i < s_framebuffer->PixelsPerScanLine * s_framebuffer->Height; i++)
        fb[i] = BackGroundColor;
}

void Logger::putc(char c)
{
    putc_dbg(c);


    if (c == '\n')
    {
        s_x = 0;
        s_y += s_font->psf1_Header->charsize;

        if (s_y + s_font->psf1_Header->charsize > s_framebuffer->Height)
        {
            Scroll();
            // s_y stays where it is — it's already pointing at the 
            // last line which Scroll() just cleared
        }
        return;
    }
    if (c == '\t')
    {
        s_x += 32;
        return;
    }

    if (s_x + 8 > s_framebuffer->Width) 
    {
        s_x = 0;
        s_y+=16;
    }

    if (s_y + s_font->psf1_Header->charsize > s_framebuffer->Height)
    {
        //Scroll();
        s_y-=16;
    }

    uint8_t* fontptr = (uint8_t*)s_font->glyphBuffer + (c * s_font->psf1_Header->charsize);
    uint32_t* fb = (uint32_t*)s_framebuffer->BaseAddress;

    for (size_t yoff = s_y; yoff < s_y +s_font->psf1_Header->charsize; yoff++)
    {
        if (yoff >= s_framebuffer->Height) break;

        for (size_t xoff = s_x; xoff < s_x + 8; xoff++)
        {
            if (xoff >= s_framebuffer->Width) break;

            if (*fontptr & (0b10000000 >> (xoff - s_x)))
            {
                fb[xoff +  yoff * s_framebuffer->PixelsPerScanLine] = Foregroundcolor;
            }
            else 
            {
                fb[xoff +  yoff * s_framebuffer->PixelsPerScanLine] = BackGroundColor;
            }
        
        }

        fontptr++;
    }  

    s_x += 8;
}

void Logger::Scroll()
{
    uint32_t charsize = s_font->psf1_Header->charsize;
    uint32_t rows_to_move = s_framebuffer->Height - charsize;

    uint32_t* src = (uint32_t*)((uint64_t)s_framebuffer->BaseAddress + charsize * s_framebuffer->PixelsPerScanLine * sizeof(uint32_t) / sizeof(uint32_t));
    uint32_t* dst = (uint32_t*)s_framebuffer->BaseAddress;

    memcpy(dst, src, rows_to_move * s_framebuffer->PixelsPerScanLine * sizeof(uint32_t));

    // clear bottom row
    uint32_t* clear = (uint32_t*)((uint64_t)s_framebuffer->BaseAddress + rows_to_move * s_framebuffer->PixelsPerScanLine);
    memset(clear, 0, charsize * s_framebuffer->PixelsPerScanLine * sizeof(uint32_t));
}

void Logger::putc_dbg(char c)
{
    outb(QEMU_PORT,c);
}

void Logger::puts(const char* str)
{
    while(*str)
    {
        putc(*str);

        str++;
    }
}

void Logger::puts_dbg(const char* str)
{
    while(*str)
    {
        putc_dbg(*str);

        str++;
    }
}

#define PRINTF_STATE_NORMAL         0
#define PRINTF_STATE_LENGTH         1
#define PRINTF_STATE_LENGTH_SHORT   2
#define PRINTF_STATE_LENGTH_LONG    3
#define PRINTF_STATE_SPEC           4

#define PRINTF_LENGTH_DEFAULT       0
#define PRINTF_LENGTH_SHORT_SHORT   1
#define PRINTF_LENGTH_SHORT         2
#define PRINTF_LENGTH_LONG          3
#define PRINTF_LENGTH_LONG_LONG     4

const char g_HexChars[] = "0123456789ABCDEF";

void log_unsigned(uint64_t number,uint16_t radix,void(*putchar)(char c))
{
    char buffer[32];
    int pos = 0;

    do 
    {
        unsigned long long rem = number % radix;
        number /= radix;
        buffer[pos++] = g_HexChars[rem];
    } while (number > 0);


    while (--pos >= 0)
        putchar(buffer[pos]);
        

}

void log_signed(int64_t number,uint16_t radix,void(*putchar)(char c))
{
    if (number < 0)
    {
        putchar('-');
        log_unsigned(-number,radix,putchar);
    }
    else {log_unsigned(number,radix,putchar);}
}


void Logger::PrivLog(const char* fmt,bool dbg,LOG_LEVEL level,va_list list)
{
    void (*putchar)(char c) = dbg ? putc_dbg : putc;
    void (*putstring)(const char* str) = dbg ? puts_dbg : puts;

    switch(level)
    {
        case LOG_LEVEL::INFO:
            putstring("[ INFO ]: ");
            break;
        case LOG_LEVEL::WARN:
            putstring("[ WARNING ]: ");
            break;
        case LOG_LEVEL::ERROR:
            putstring("[ ERROR ]: ");
            break;
        default:
            putstring("[ TRACE ]: ");
            break;
    }


    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;
    int radix = 10;
    bool sign = false;
    bool number = false;

    while (*fmt)
    {
        switch(state)
        {
            case PRINTF_STATE_NORMAL:
                switch (*fmt)
                {
                case '%':
                    state = PRINTF_STATE_SPEC;
                    break;
                
                default:
                    putchar(*fmt);
                    break;
                }
                break;
            case PRINTF_STATE_LENGTH:
                switch (*fmt)
                {
                    case 'h':   length = PRINTF_LENGTH_SHORT;
                                state = PRINTF_STATE_LENGTH_SHORT;
                                break;
                    case 'l':   length = PRINTF_LENGTH_LONG;
                                state = PRINTF_STATE_LENGTH_LONG;
                                break;
                    default:    goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_LENGTH_SHORT:
                if (*fmt == 'h')
                {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;

            case PRINTF_STATE_LENGTH_LONG:
                if (*fmt == 'l')
                {
                    length = PRINTF_LENGTH_LONG_LONG;
                    state = PRINTF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;
            case PRINTF_STATE_SPEC:
            {
                PRINTF_STATE_SPEC_:
                switch(*fmt)
                {
                    case '%':
                        putchar('%');
                        break;
                    case 'c':
                        putchar((char)va_arg(list,int));
                        break;
                    case 's':
                        putstring((const char* )va_arg(list,char*));
                        break;
                    case 'u':
                        sign = false;
                        number = true;
                        radix = 10;
                        break;
                    case 'x':
                    case 'p':
                        sign = false;
                        number = true;
                        radix = 16;
                        break;
                    case 'd':
                    case 'i':
                        sign =  true;
                        number = true;
                        radix = 10;
                        break;
                    default: break;
                }

                if (number)
                {
                    if (sign)
                    {
                        switch (length)
                        {
                        case PRINTF_LENGTH_SHORT_SHORT:
                        case PRINTF_LENGTH_SHORT:
                        case PRINTF_LENGTH_DEFAULT:     log_signed(va_arg(list, int), radix,putchar);
                                                        break;

                        case PRINTF_LENGTH_LONG:        log_signed(va_arg(list, long), radix,putchar);
                                                        break;

                        case PRINTF_LENGTH_LONG_LONG:   log_signed(va_arg(list, long long), radix,putchar);
                                                        break;
                        }
                    }
                    else
                    {
                        switch (length)
                        {
                        case PRINTF_LENGTH_SHORT_SHORT:
                        case PRINTF_LENGTH_SHORT:
                        case PRINTF_LENGTH_DEFAULT:     log_unsigned(va_arg(list, unsigned int), radix,putchar);
                                                        break;
                                                        
                        case PRINTF_LENGTH_LONG:        log_unsigned(va_arg(list, unsigned  long), radix,putchar);
                                                        break;

                        case PRINTF_LENGTH_LONG_LONG:   log_unsigned(va_arg(list, unsigned  long long), radix,putchar);
                                                        break;
                        }
                    }
                }

                // reset state
                state = PRINTF_STATE_NORMAL;
                length = PRINTF_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                number = false;
                break;
            }
        }
        fmt++;
    }

}

void Logger::Log(const char* fmt,LOG_LEVEL level,...)
{
    va_list list;
    va_start(list,level);
    PrivLog(fmt,false,level,list);

    va_end(list);
}

void Logger::DebugLog(const char* fmt,LOG_LEVEL level,...)
{
    //Warnings that cause errors
    (void)fmt;
    (void)level;
#ifdef LOG
    va_list list;
    va_start(list,level);
    PrivLog(fmt,true,level,list);

    va_end(list);
#endif
}

#include <stdarg.h>
#include <stddef.h>

static void reverse(char* str, int len)
{
    for (int i = 0; i < len / 2; i++)
    {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
}

static int itoa_internal(long long value, char* buffer, int base, bool upper)
{
    int i = 0;

    if (value == 0)
    {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return i;
    }

    while (value != 0)
    {
        int rem = value % base;
        if (rem < 10)
            buffer[i++] = '0' + rem;
        else
            buffer[i++] = (upper ? 'A' : 'a') + (rem - 10);

        value /= base;
    }

    buffer[i] = '\0';
    reverse(buffer, i);
    return i;
}

int snprintf(char* out, size_t size, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    size_t pos = 0;      // actual write position (bounded)
    size_t total = 0;    // total chars that would have been written

    auto putc = [&](char c)
    {
        if (pos + 1 < size)
            out[pos] = c;

        pos++;
        total++;
    };

    for (size_t i = 0; fmt[i]; i++)
    {
        if (fmt[i] != '%')
        {
            putc(fmt[i]);
            continue;
        }

        i++; // skip '%'

        if (fmt[i] == '%')
        {
            putc('%');
            continue;
        }

        char tmp[32];

        switch (fmt[i])
        {
            case 'c':
            {
                char c = (char)va_arg(args, int);
                putc(c);
                break;
            }

            case 's':
            {
                const char* s = va_arg(args, const char*);
                if (!s) s = "(null)";

                while (*s)
                    putc(*s++);
                break;
            }

            case 'd':
            case 'i':
            {
                int val = va_arg(args, int);
                long long v = val;

                if (v < 0)
                {
                    putc('-');
                    v = -v;
                }

                int len = itoa_internal(v, tmp, 10, false);
                for (int j = 0; j < len; j++)
                    putc(tmp[j]);

                break;
            }

            case 'u':
            {
                unsigned int val = va_arg(args, unsigned int);
                int len = itoa_internal(val, tmp, 10, false);

                for (int j = 0; j < len; j++)
                    putc(tmp[j]);

                break;
            }

            case 'x':
            case 'X':
            {
                unsigned int val = va_arg(args, unsigned int);
                bool upper = (fmt[i] == 'X');

                int len = itoa_internal(val, tmp, 16, upper);
                for (int j = 0; j < len; j++)
                    putc(tmp[j]);

                break;
            }

            default:
                // unknown specifier, just print it
                putc('%');
                putc(fmt[i]);
                break;
        }
    }

    // Null-terminate
    if (size > 0)
    {
        if (pos < size)
            out[pos] = '\0';
        else
            out[size - 1] = '\0';
    }

    va_end(args);
    return total;
}