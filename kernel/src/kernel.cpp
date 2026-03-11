#include <generic/bootinfo.hpp>


extern void (*__init_array_start[])();
extern void (*__init_array_end[])();

void call_constructors()
{
    for (size_t i = 0; &__init_array_start[i] < __init_array_end; i++)
        __init_array_start[i]();
}

extern "C" void __attribute__((noreturn)) kernel_main(bootinfo_t* info)
{

    uint32_t* fb = (uint32_t*)info->framebuffer->BaseAddress;

    for (size_t y = 0; y < info->framebuffer->Height; y++)
    {
        for (size_t x = 0; x < info->framebuffer->PixelsPerScanLine; x++)
        {
            fb[y * info->framebuffer->PixelsPerScanLine + x] = 0xFFFFFFFF;
        }
    }
    while(1){}
}