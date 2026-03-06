#include <generic/constructers.h>

#include <generic/bootinfo.h>
#include <generic/stdio.h>



extern "C" void kernel_main(bootinfo_t info)
{
    call_constructers();
    Initilize(info);

    uint64_t fbBase = (uint64_t)info.framebuffer.BaseAddress;
    uint64_t fbSize = (uint64_t)info.framebuffer.BufferSize + 4096;

    uint32_t* fb = (uint32_t*)fbBase;
    
    for (uint64_t y = 0; y < info.framebuffer.Height; y++) {
        for (uint64_t x = 0; x < info.framebuffer.PixelsPerScanLine; x++) {
            fb[y * info.framebuffer.PixelsPerScanLine + x] = 0x000000;
        }
    }

    while(1) {}
}