#include <generic/kernelInit.h>

#include <memory/PageAllocater.h>
#include <memory/GDT.h>
#include <memory/PageTableManager.h>

bool PrepareMemory(bootinfo_t* info)
{
    PageAllocater PA;
    PA.Initilize(info);

    // Lock framebuffer pages
    for (uint64_t fbpage = 0; fbpage < info->framebuffer->BufferSize / 4096 + 1; fbpage++)
    {
        PA.LockPage((void*)((uint64_t)info->framebuffer->BaseAddress + fbpage * 4096));
    }

    PageTableManager PTM;
    PageTable* PML4 = (PageTable*)PA.RequestPage();
    if (!PML4)
    {
        return false;
    }
    PTM.Initilize(PML4);

    // Identity map all EFI memory descriptors
    for (uint64_t t = 0; t < info->MapSize / info->DescriptorSize; t++)
    {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)info->mMap + (t * info->DescriptorSize));
        if (desc->Type != EfiReservedMemoryType)
        {
            for (uint64_t addr = desc->PhysicalStart; addr < desc->PhysicalStart + desc->NumberOfPages * 4096; addr += 4096)
            {
                PTM.MapMemory((void*)addr, (void*)addr);
            }
        }
    }

    // Activate new page table
    asm volatile ("mov %0, %%cr3" : : "r" (PML4));

    GDTR gdtr = {0,0};
    gdtr.Size = sizeof(g_GDT) - 1;
    gdtr.Offset = (uint64_t)&g_GDT;

    LoadGDT(&gdtr);

    return true;
}