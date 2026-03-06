#include <generic/KernelInit.h>

#include <generic/string.h>

bool InitilizeKernel(bootinfo_t bootinfo)
{
    call_constructers();
    Initilize(bootinfo);
    printf("Framebuffer %x\n",(uint64_t)bootinfo.framebuffer.BaseAddress);
    PageAllocater pageallocater = PageAllocater(bootinfo.mMap,bootinfo.MapSize,bootinfo.DescriptorSize);
    GlobalAllocator = pageallocater;

    while(1){}

    uint64_t fbBase = (uint64_t)bootinfo.framebuffer.BaseAddress;
    uint64_t fbSize = (uint64_t)bootinfo.framebuffer.BufferSize + 4096;

    uint32_t* fb = (uint32_t*)fbBase;
    
    for (uint64_t y = 0; y < bootinfo.framebuffer.Height; y++) {
        for (uint64_t x = 0; x < bootinfo.framebuffer.PixelsPerScanLine; x++) {
            fb[y * bootinfo.framebuffer.PixelsPerScanLine + x] = 0x000000;
        }
    }

    for (size_t t = 0; t < fbSize/0x1000 + 1; t++)
    {
        GlobalAllocator.LockPage((void*)(fbBase + t * 4096));
    }
    PageTable* PML4 = (PageTable*)GlobalAllocator.AllocatePage();
    memset(PML4, 0, 0x1000); // zero it out first!
    PageTableManager Pagetablemanager = PageTableManager(PML4);
    GlobalPageTableManager = Pagetablemanager;

    for (size_t t = 0; t < GetMemorySize(bootinfo.mMap,bootinfo.MapSize,bootinfo.DescriptorSize); t+=4096)
    {
        GlobalPageTableManager.MapMemory((void*)t,(void*)t);
    }
    


    asm volatile ("mov %0, %%cr3" : : "r" (PML4));

    return true;
}