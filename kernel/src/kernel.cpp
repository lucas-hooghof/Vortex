#include <generic/bootinfo.h>

#include <generic/stdio.h>
#include <memory/PageTableManager.h>
#include <memory/PageAllocater.h>

extern void (*__init_array_start[])();
extern void (*__init_array_end[])();

void call_constructors()
{
    for (size_t i = 0; &__init_array_start[i] < __init_array_end; i++)
        __init_array_start[i]();
}

extern "C" void __attribute__((noreturn)) kernel_main(bootinfo_t* info)
{
    call_constructors();

    Logger::Initilize(info);
    PageAllocater PA;
    PA.Initilize(info);

    for (uint64_t fbpage = 0; fbpage < info->framebuffer->BufferSize / 4096 + 1; fbpage++)
    {
        PA.LockPage((void*)((uint64_t)info->framebuffer->BaseAddress + fbpage * 4096));
    }

    PageTableManager PTM;
    PageTable* PML4 = (PageTable*)PA.RequestPage();

    for (uint64_t t = 0; t < info->MapSize / info->DescriptorSize; t++)
    {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)info->mMap + (t * info->DescriptorSize));

        if (desc->Type != EfiReservedMemoryType)
        {
            for (uint64_t addr = desc->PhysicalStart; addr < desc->PhysicalStart + desc->NumberOfPages * 4096; addr+=4096)
            {
                PTM.MapMemory((void*)addr,(void*)addr);
            }
        }
    }

    asm volatile ("mov %0,%%cr3" : : "r"(PML4));

    Logger::DebugLog("Hello World After paging",LOG_LEVEL::INFO);
    while(1){}
}