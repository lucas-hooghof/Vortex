#include "PageTableManager.h"
#include <generic/stdint.h>
#include <generic/string.h>
#include <memory/PageAllocater.h>

PageTableManager* PageTableManager::s_Instance = nullptr;

void PageTableManager::Initilize(PageTable* PML4Address){
    s_Instance = this;
    this->PML4 = PML4Address;
}

void PageTableManager::MapMemory(void* virtualMemory, void* physicalMemory){
    uint64_t virtualAddress = (uint64_t)virtualMemory;
    virtualAddress >>= 12;
    uint64_t P_i = virtualAddress & 0x1ff;
    virtualAddress >>= 9;
    uint64_t PT_i = virtualAddress & 0x1ff;
    virtualAddress >>= 9;
    uint64_t PD_i = virtualAddress & 0x1ff;
    virtualAddress >>= 9;
    uint64_t PDP_i = virtualAddress & 0x1ff;
    PageDirectoryEntry PDE;

    PDE = PML4->entries[PDP_i];
    PageTable* PDP;
    if (!PDE.Present){
        PDP = (PageTable*)PageAllocater::GetInstance()->RequestPage();
        memset(PDP, 0, 0x1000);
        PDE.Address = (uint64_t)PDP >> 12;
        PDE.Present = true;
        PDE.ReadWrite = true;
        PML4->entries[PDP_i] = PDE;
    }
    else
    {
        PDP = (PageTable*)((uint64_t)PDE.Address << 12);
    }
    
    
    PDE = PDP->entries[PD_i];
    PageTable* PD;
    if (!PDE.Present){
        PD = (PageTable*)PageAllocater::GetInstance()->RequestPage();
        memset(PD, 0, 0x1000);
        PDE.Address = (uint64_t)PD >> 12;
        PDE.Present = true;
        PDE.ReadWrite = true;
        PDP->entries[PD_i] = PDE;
    }
    else
    {
        PD = (PageTable*)((uint64_t)PDE.Address << 12);
    }

    PDE = PD->entries[PT_i];
    PageTable* PT;
    if (!PDE.Present){
        PT = (PageTable*)PageAllocater::GetInstance()->RequestPage();
        memset(PT, 0, 0x1000);
        PDE.Address = (uint64_t)PT >> 12;
        PDE.Present = true;
        PDE.ReadWrite = true;
        PD->entries[PT_i] = PDE;
    }
    else
    {
        PT = (PageTable*)((uint64_t)PDE.Address << 12);
    }

    PDE = PT->entries[P_i];
    PDE.Address = (uint64_t)physicalMemory >> 12;
    PDE.Present = true;
    PDE.ReadWrite = true;
    PT->entries[P_i] = PDE;
}