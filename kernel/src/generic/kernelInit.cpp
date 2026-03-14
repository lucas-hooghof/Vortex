#include <generic/kernelInit.h>

#include <generic/stdio.h>

#include <memory/PageAllocater.h>
#include <memory/GDT.h>
#include <memory/PageTableManager.h>

#include <interrupts/IDT.h>
#include <interrupts/ISR.h>

extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;
extern uint64_t __stack_start;
extern uint64_t __stack_end;

bool PrepareMemory(bootinfo_t* info)
{
    PageAllocater PA;
    PA.Initilize(info);

    // Lock framebuffer pages
    for (uint64_t fbpage = 0; fbpage < info->framebuffer->BufferSize / 4096 + 1; fbpage++)
    {
        PA.LockPage((void*)((uint64_t)info->framebuffer->BaseAddress + fbpage * 4096));
    }


    uint64_t kernelSize = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
    uint64_t kernelPages = (uint64_t)kernelSize / 4096 + 1;

    PA.LockPages(&_KernelStart,kernelPages);

    PageTableManager PTM;
    void* PML4 = PA.RequestPage();
    if (!PML4)
    {
        return false;
    }
    PTM.Initilize(PML4);

    // Identity map all EFI memory descriptors
    for (uint64_t t = 0; t < 0x40000000; t+=0x1000)
    {
        PTM.MapMemory((void*)t, (void*)t,PAGE_PRESENT | PAGE_RW);
    }

    for (uint64_t t = 0; t < info->MapSize / info->DescriptorSize; t++)
    {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)info->mMap + (t * info->DescriptorSize));
        if (desc->Type == EfiConventionalMemory ||
            desc->Type == EfiBootServicesCode ||
            desc->Type == EfiBootServicesData ||
            desc->Type == EfiLoaderCode ||
            desc->Type == EfiLoaderData)
        {
            for (uint64_t addr = desc->PhysicalStart; addr < desc->PhysicalStart + desc->NumberOfPages * 4096; addr += 4096)
            {
                PTM.MapMemory((void*)addr, (void*)addr,PAGE_PRESENT | PAGE_RW);
            }
        }
    }


    for (uint64_t fb = 0; fb < (info->framebuffer->BufferSize + 4095) / 4096; fb++)
    {
        PTM.MapMemory((void*)((uint64_t)info->framebuffer->BaseAddress + fb * 4096),(void*)((uint64_t)info->framebuffer->BaseAddress + fb * 4096),PAGE_PRESENT | PAGE_RW | PAGE_PCD | PAGE_PWT);
    }

    uint64_t StackSize = (uint64_t)&__stack_end - (uint64_t)&__stack_start;
    uint64_t StackPages = (uint64_t)StackSize / 4096 + 1;

    for (uint64_t t = 0; t < StackPages; t++)
    {
        PTM.MapMemory((void*)((uint64_t)&__stack_start + t * 4096),(void*)((uint64_t)&__stack_start + t * 4096),PAGE_PRESENT | PAGE_RW);
    }

    asm volatile ("mov %0, %%cr3" : : "r" (PML4));

    GDTR gdtr = {0,0};
    gdtr.Size = sizeof(g_GDT) - 1;
    gdtr.Offset = (uint64_t)&g_GDT;

    LoadGDT(&gdtr);

    return true;
}

bool PrepareInterrupts()
{
    IDT_R idtr;
    idtr.Size = sizeof(IDT) - 1;
    idtr.Offset = (uint64_t)&IDT;

    asm volatile ("lidt %0" : : "m"(idtr));
    InitilizeISR();
    
    asm volatile ("sti");

    return true;
}