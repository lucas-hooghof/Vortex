#include "PageAllocater.hpp"

#include <generic/stdio.hpp>

PageAllocater* PageAllocater::m_Instance = nullptr;



bool PageAllocater::Initilize(EFI_MEMORY_DESCRIPTOR* mMap,size_t MapSize,size_t MapDescSize)
{
    if (m_Instance != nullptr) return true;
    m_Instance = this;

    size_t MemorySize = GetMemorySize(mMap,MapSize,MapDescSize);

    size_t MapEntries = MapSize / MapDescSize;

    void* LargestSegment = nullptr;
    size_t LargestSegmentSize = 0;

    FreeRam = MemorySize;
    UsedRam = 0;
    ReservedRam = 0;

    for (size_t e = 0; e < MapEntries; e++)
    {
        EFI_MEMORY_DESCRIPTOR* entry = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)(mMap) + e * MapDescSize);
        if (entry->Type == EfiConventionalMemory)
        {
            if (entry->NumberOfPages * 4096 > LargestSegmentSize)
            {
                LargestSegment = (void*)entry->PhysicalStart;
                LargestSegmentSize = entry->NumberOfPages * 4096;
            }
        }
    }

    if (LargestSegment == nullptr)
    {
        printf("Failed to allocate memory for bitmap\n");
        return false;
    }

    m_bitmap.addr = (uint8_t*)LargestSegment;
    m_bitmap.size = ((MemorySize / 4096) + 7) / 8;
    
    for (size_t i = 0; i < MemorySize / 4096; i++)
    {
        m_bitmap.Set(i,false);
    }

    size_t LockSize = (m_bitmap.size + 4095) / 4096;

    LockPages(LargestSegment,LockSize);

    for (size_t e = 0; e < MapEntries; e++)
    {
        EFI_MEMORY_DESCRIPTOR* entry = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)(mMap) + e * MapDescSize);
        if (entry->Type != EfiConventionalMemory)
        {
            ReservePages((void*)entry->PhysicalStart,entry->NumberOfPages);
        }
    }


    return true;
}

void PageAllocater::LockPage(void* page)
{
    size_t index = (uint64_t)page / 4096;
    if (m_bitmap[index] == true) return;
    if (m_bitmap.Set(index,true)) {
        FreeRam -= 4096;
        UsedRam += 4096;
    }
}

void PageAllocater::LockPages(void* page,size_t pagecount)
{
    for (size_t p = 0; p < pagecount; p++)
    {
        LockPage((void*)((uint64_t)(page) + p * 4096));
    }
}

void PageAllocater::ReservePage(void* page)
{
    size_t index = (uint64_t)page / 4096;
    if (m_bitmap[index] == true) return;
    if (m_bitmap.Set(index, true)){
        FreeRam -= 4096;
        ReservedRam += 4096;
    }
}

void PageAllocater::ReservePages(void* page,size_t pagecount)
{  
    for (size_t p = 0; p < pagecount; p++)
    {
        ReservePage((void*)((uint64_t)(page) + p * 4096));
    }
}

void PageAllocater::UnReservePage(void* page)
{
    uint64_t index = (uint64_t)page / 4096;
    if (m_bitmap[index] == false) return;
    if (m_bitmap.Set(index, false)){
        FreeRam += 4096;
        ReservedRam -= 4096;
    }
}

void PageAllocater::UnReservePages(void* page,size_t pagecount)
{
    for (size_t p = 0; p < pagecount; p++)
    {
        UnReservePage((void*)((uint64_t)(page) + p * 4096));
    }
}

void* PageAllocater::RequestPage()
{
    for (size_t p = 0; p < m_bitmap.size * 8; p++){
        if (m_bitmap[p] == true) {continue;}
        LockPage((void*)(p * 4096));
        return (void*)(p * 4096);
    }

    printf("Failed to find page");

    return nullptr;
}

void PageAllocater::FreePage(void* page)
{
    uint64_t index = (uint64_t)page / 4096;
    if (m_bitmap[index] == false) return;
    if (m_bitmap.Set(index, false)){
        FreeRam += 4096;
        UsedRam -= 4096;
    }
}