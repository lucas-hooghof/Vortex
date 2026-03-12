#pragma once

#include <generic/stdint.h>


struct PageDirectoryEntry
{
    uint64_t Present : 1;
    uint64_t ReadWrite : 1;
    uint64_t UserSuper : 1;
    uint64_t WriteThrough : 1;
    uint64_t CacheDisabled : 1;
    uint64_t Accessed : 1;
    uint64_t Dirty : 1;
    uint64_t LargePage : 1;
    uint64_t Global : 1;

    uint64_t Available1 : 3;

    uint64_t Address : 40;

    uint64_t Available2 : 11;
    uint64_t ExecuteDisable : 1;
};

struct PageTable { 
    PageDirectoryEntry entries [512];
}__attribute__((aligned(0x1000)));

class PageTableManager {
    public:
    PageTableManager() = default;
    void Initilize(PageTable* PML4Address);
    PageTable* PML4;
    void MapMemory(void* virtualMemory, void* physicalMemory);
    static PageTableManager* GetInstance() {return s_Instance;}
    private:
        static PageTableManager* s_Instance;
};