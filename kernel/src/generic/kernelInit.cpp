#include <generic/kernelInit.h>
#include <generic/string.h>
#include <generic/stdio.h>

#include <memory/PageAllocater.h>
#include <memory/PageTableManager.h>
#include <memory/GDT.h>

#include <interrupts/ISR.h>
#include <interrupts/IDT.h>


extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;
extern uint64_t __stack_start;
extern uint64_t __stack_end;
extern uint64_t __stack_bottom;

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
    PA.LockPages(info->framebuffer->BaseAddress,(info->framebuffer->BufferSize + 4095) / 4096);

    PageTableManager ptm;

    uint64_t* pml4 =(uint64_t*) PA.RequestPage();
    memset((void*)pml4,0,4096); 
    ptm.Initilize((void*)pml4);

    Logger::DebugLog("PML4: %x\n",LOG_LEVEL::INFO,(uint64_t)pml4);

    for (uint64_t t = 0; t < (info->MapSize / info->DescriptorSize); t++)
    {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)info->mMap + t * info->DescriptorSize);

        if (desc->Type != EfiReservedMemoryType)
        {
            for (uint64_t p = 0; p < desc->NumberOfPages; p++)
            {
                ptm.MapMemory((void*)(desc->PhysicalStart + (p * 4096)),(void*)(desc->PhysicalStart + (p * 4096)),PAGE_PRESENT | PAGE_RW);
            }
        }
    }

    for (uint64_t fbpage = 0; fbpage < info->framebuffer->BufferSize / 4096 + 1; fbpage++)
    {
        ptm.MapMemory((void*)((uint64_t)info->framebuffer->BaseAddress + fbpage * 4096),(void*)((uint64_t)info->framebuffer->BaseAddress + fbpage * 4096),PAGE_PRESENT | PAGE_RW | PAGE_PWT | PAGE_PCD);
    }

    #define HIGHER_HALF_BASE 0xFFFFFFFF80000000ULL

    uint64_t kernPhys = (uint64_t)&_KernelStart;
    uint64_t kernVirt = HIGHER_HALF_BASE; // must match your linker script

    for (uint64_t offset = 0; offset < kernelSize; offset += 4096)
    {
        ptm.MapMemory((void*)(kernVirt + offset),
                    (void*)(kernPhys + offset),
                    PAGE_PRESENT | PAGE_RW);
    }

    uint64_t start = (uint64_t)&__stack_bottom;
    uint64_t end   = (uint64_t)&__stack_end;
    for(uint64_t addr = start; addr < end + 4096; addr += 4096)
    {
        ptm.MapMemory((void*)addr, (void*)addr, PAGE_PRESENT | PAGE_RW);
    }
    asm volatile ("mov %0,%%cr3" : : "r"(pml4));

    GDTR gdtr = {0,0};

    gdtr.Size = sizeof(g_GDT) - 1;
    gdtr.Offset = (uint64_t)&g_GDT;

    LoadGDT(&gdtr);

    return true;
}

bool PrepareInterrupts()
{
    IDT_R idtr = {0,0};
    idtr.Size = sizeof(IDT) - 1;
    idtr.Offset = (uint64_t)&IDT;

    asm volatile ("lidt %0" : : "m"(idtr));
    if(!InitilizeISR()) { return false; }
    asm volatile ("sti");

    return true;
}