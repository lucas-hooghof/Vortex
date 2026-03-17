#pragma once

#include <generic/stdint.h>
#include <generic/stdio.h>

#define PAGE_PRESENT  (1ULL << 0)
#define PAGE_RW       (1ULL << 1)
#define PAGE_US       (1ULL << 2)
#define PAGE_PWT      (1ULL << 3)
#define PAGE_PCD      (1ULL << 4)
#define PAGE_ACCESSED (1ULL << 5)
#define PAGE_DIRTY    (1ULL << 6)
#define PAGE_2MB      (1ULL << 7)
#define PAGE_GLOBAL   (1ULL << 8)
#define PAGE_NX       (1ULL << 63)

#define PML4_INDEX(x) (((uint64_t)(x) >> 39) & 0x1FF)
#define PDPT_INDEX(x) (((uint64_t)(x) >> 30) & 0x1FF)
#define PD_INDEX(x)   (((uint64_t)(x) >> 21) & 0x1FF)
#define PT_INDEX(x)   (((uint64_t)(x) >> 12) & 0x1FF)


class PageTableManager
{
public:
    PageTableManager() = default;

    void Initilize(void* PML4);

    void MapMemory(void* va,void* pa,uint64_t flags);

    static PageTableManager* GetInstance() { return s_Instance; }
private:
    uint64_t* PML4;

    static PageTableManager* s_Instance;
};