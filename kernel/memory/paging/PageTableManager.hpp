#pragma once
#include "paging.hpp"

class PageTableManager {
    public:
    PageTableManager();
    void Initilize(PageTable* PML4Address);
    PageTable* PML4;
    void MapMemory(void* virtualMemory, void* physicalMemory);

    static PageTableManager* GetInstance() { return s_Instance; }
    private:
        static PageTableManager* s_Instance;
};