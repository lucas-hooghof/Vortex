#include <memory/PageTableManager.h>

#include <memory/PageAllocater.h>
#include <generic/string.h>

PageTableManager* PageTableManager::s_Instance = nullptr;

void PageTableManager::Initilize(void* PML4)
{
    this->PML4 = (uint64_t*)PML4;

    s_Instance = this;
}

void PageTableManager::MapMemory(void* va, void* pa, uint8_t flags)
{
    uint64_t v = (uint64_t)va;
    uint64_t p = (uint64_t)pa & 0x000FFFFFFFFFF000;

    uint64_t pml4_i = PML4_INDEX(v);
    uint64_t pdpt_i = PDPT_INDEX(v);
    uint64_t pd_i   = PD_INDEX(v);
    uint64_t pt_i   = PT_INDEX(v);

    uint64_t* pdpt;
    uint64_t* pd;
    uint64_t* pt;

    if (!(PML4[pml4_i] & PAGE_PRESENT))
    {
        pdpt = (uint64_t*)PageAllocater::GetInstance()->RequestPage();
        memset(pdpt, 0, 4096);

        PML4[pml4_i] = ((uint64_t)pdpt & 0x000FFFFFFFFFF000) | PAGE_PRESENT | PAGE_RW;
    }
    else
        pdpt = (uint64_t*)(PML4[pml4_i] & 0x000FFFFFFFFFF000);

    if (!(pdpt[pdpt_i] & PAGE_PRESENT))
    {
        pd = (uint64_t*)PageAllocater::GetInstance()->RequestPage();
        memset(pd, 0, 4096);

        pdpt[pdpt_i] = ((uint64_t)pd & 0x000FFFFFFFFFF000) | PAGE_PRESENT | PAGE_RW;
    }
    else
        pd = (uint64_t*)(pdpt[pdpt_i] & 0x000FFFFFFFFFF000);

    if (!(pd[pd_i] & PAGE_PRESENT))
    {
        pt = (uint64_t*)PageAllocater::GetInstance()->RequestPage();
        memset(pt, 0, 4096);

        pd[pd_i] = ((uint64_t)pt & 0x000FFFFFFFFFF000) | PAGE_PRESENT | PAGE_RW;
    }
    else
        pt = (uint64_t*)(pd[pd_i] & 0x000FFFFFFFFFF000);

    pt[pt_i] = p | flags;
}
