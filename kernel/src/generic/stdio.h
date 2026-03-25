#pragma once

#include <generic/stdint.h>

#include <generic/bootinfo.h>
#include <generic/stdarg.h>

enum class LOG_LEVEL
{
    ERROR,
    INFO,
    WARN
}; 

class Logger
{
    public:
        static void Initilize(bootinfo_t *info);
        static void ClearScreen(uint32_t ForegroundColor,uint32_t BackgroundColor);

        static void Log(const char* fmt,LOG_LEVEL level,...);

        static void DebugLog(const char* fmt,LOG_LEVEL level,...);

    private:
        static void putc_dbg(char c);
        static void putc(char c);
        static void puts(const char* str);
        static void puts_dbg(const char* str);

        static void PrivLog(const char* fmt,bool dbg,LOG_LEVEL level,va_list list);

        static void Scroll();

        static FRAMEBUFFER* s_framebuffer;
        static PSF1_FONT* s_font;

        static uint32_t s_x;
        static uint32_t s_y;

        static uint32_t Foregroundcolor;
        static uint32_t BackGroundColor;
};


int snprintf(char* out, size_t size, const char* fmt, ...);