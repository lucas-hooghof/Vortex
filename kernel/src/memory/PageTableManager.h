#pragma once

#include <generic/stdint.h>

#define PAGE_PRESENT   (1 << 0)
#define PAGE_RW        (1 << 1)
#define PAGE_USER      (1 << 2)
#define PAGE_PWT       (1 << 3)
#define PAGE_PCD       (1 << 4)
#define PAGE_ACCESSED  (1 << 5)
#define PAGE_DIRTY     (1 << 6)
#define PAGE_PS        (1 << 7)
#define PAGE_GLOBAL    (1 << 8)
#define PAGE_NX        (1ULL << 63)

class PageTableManager {
    public:
    PageTableManager() = default;
    void Initilize(void* PML4Address);
    void* PML4;
    void MapMemory(void* virtualMemory, void* physicalMemory,uint8_t flags);
    static PageTableManager* GetInstance() {return s_Instance;}
    private:
        static PageTableManager* s_Instance;
};