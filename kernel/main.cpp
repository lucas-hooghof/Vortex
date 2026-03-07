#include <generic/constructers.hpp>

#include <generic/bootinfo.hpp>
#include <generic/stdio.hpp>

#include <memory/PageAllocater.hpp>
#include <memory/paging/PageTableManager.hpp>

#include <generic/string.hpp>


extern "C" void kernel_main(bootinfo_t info)
{
    call_constructers();
    Initilize(info);

    uint64_t fbBase = (uint64_t)info.framebuffer.BaseAddress;
    uint64_t fbSize = (uint64_t)info.framebuffer.BufferSize + 0x1000;

    PageAllocater allocater = PageAllocater();
    allocater.Initilize(info.mMap,info.MapSize,info.DescriptorSize);
    
    allocater.LockPages((void*)fbBase,fbSize / 4096);

    PageTableManager PTM = PageTableManager();
    PageTable* PML4 = (PageTable*)allocater.RequestPage();
    if (PML4 == nullptr)
    {
        printf("Failed to allocate PageTable!\n");
        while(1) {}; // Maybe make reboot happen
    }
    memset(PML4,0,4096);
    PTM.Initilize(PML4);

    for (uint64_t t = 0; t < GetMemorySize(info.mMap, info.MapSize, info.DescriptorSize); t+= 0x1000){
        PTM.MapMemory((void*)t, (void*)t);
    }

    for (uint64_t t = fbBase; t < fbBase + fbSize; t += 4096){
        PTM.MapMemory((void*)t, (void*)t);
    }
    asm volatile ("mov %0, %%cr3" : : "r" (PML4));
    printf("\n\n\n\n\n\n\n\nHello Test %c\n",'!');
    while(1) {}
}