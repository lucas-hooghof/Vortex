#include <generic/constructers.h>

#include <generic/bootinfo.h>
#include <generic/stdio.h>

#include <memory/PageAllocater.h>
#include <memory/paging/PageTableManager.h>

#include <generic/string.h>

#define STACK_BASE 0x90000
#define STACK_SIZE 0x4000

extern "C" void kernel_main(bootinfo_t info)
{
    call_constructers();
    Initilize(info);
    PageAllocater pageallocater = PageAllocater(info.mMap,info.MapSize,info.DescriptorSize);
    GlobalAllocator = pageallocater;
    PageTable* PML4 = (PageTable*)GlobalAllocator.AllocatePage();
    memset(PML4, 0, 0x1000); // zero it out first!
    PageTableManager Pagetablemanager = PageTableManager(PML4);
    GlobalPageTableManager = Pagetablemanager;


    while(1) {}
}