#include "PageTableManager.h"
#include <generic/stdint.h>
#include <generic/string.h>
#include <memory/PageAllocater.h>

PageTableManager* PageTableManager::s_Instance = nullptr;

void PageTableManager::Initilize(void* PML4Address){
    s_Instance = this;
    this->PML4 = PML4Address;
}
void PageTableManager::MapMemory(void* virtualMemory, void* physicalMemory, uint8_t flags)
{
    uint64_t vaddr = (uint64_t)virtualMemory;

    uint64_t pml4_i = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_i = (vaddr >> 30) & 0x1FF;
    uint64_t pd_i   = (vaddr >> 21) & 0x1FF;
    uint64_t pt_i   = (vaddr >> 12) & 0x1FF;

    uint64_t* PML4 = (uint64_t*)this->PML4;

    // ---------- PML4 → PDPT ----------
    if (!(PML4[pml4_i] & 1))
    {
        uint64_t* new_pdpt = (uint64_t*)PageAllocater::GetInstance()->RequestPage();
        memset(new_pdpt, 0, 0x1000);

        PML4[pml4_i] = ((uint64_t)new_pdpt) | 0b11;
    }
    else
    {
        PML4[pml4_i] |= 0b10; // ensure RW
    }

    uint64_t* PDPT = (uint64_t*)(PML4[pml4_i] & 0xFFFFFFFFFFFFF000);

    // ---------- PDPT → PD ----------
    if (!(PDPT[pdpt_i] & 1))
    {
        uint64_t* new_pd = (uint64_t*)PageAllocater::GetInstance()->RequestPage();
        memset(new_pd, 0, 0x1000);

        PDPT[pdpt_i] = ((uint64_t)new_pd) | 0b11;
    }
    else
    {
        PDPT[pdpt_i] |= 0b10;
    }

    uint64_t* PD = (uint64_t*)(PDPT[pdpt_i] & 0xFFFFFFFFFFFFF000);

    // ---------- PD → PT ----------
    if (!(PD[pd_i] & 1))
    {
        uint64_t* new_pt = (uint64_t*)PageAllocater::GetInstance()->RequestPage();
        memset(new_pt, 0, 0x1000);

        PD[pd_i] = ((uint64_t)new_pt) | 0b11;
    }
    else
    {
        PD[pd_i] |= 0b10;
    }

    uint64_t* PT = (uint64_t*)(PD[pd_i] & 0xFFFFFFFFFFFFF000);

    // ---------- PT → PAGE ----------
    PT[pt_i] = ((uint64_t)physicalMemory & 0xFFFFFFFFFFFFF000) | flags;
}