#include <memory/PageAllocater.h>


#include <generic/stdio.h>
PageAllocater* PageAllocater::s_Instance = nullptr;

void PageAllocater::Initilize(bootinfo_t* info)
{
    s_Instance = this;
    uint64_t MapEntries = info->MapSize / info->DescriptorSize;

    void* LargestSegement = nullptr;
    size_t LargestSegmentSize = 0;

    size_t memorysize = 0;

    for (size_t entry = 0; entry < MapEntries; entry++)
    {
        EFI_MEMORY_DESCRIPTOR* mapentry = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)info->mMap + (entry * info->DescriptorSize));

        if (mapentry->Type == EfiConventionalMemory)
        {
            if (LargestSegmentSize < mapentry->NumberOfPages * 4096)
            {
                LargestSegmentSize = mapentry->NumberOfPages * 4096;
                LargestSegement = (void*)mapentry->PhysicalStart;
            }
        }

        memorysize += mapentry->NumberOfPages * 4096;
    }

    FreeRam = memorysize;
    TotalMemorySize = memorysize;

    TotalPageCount = memorysize / 4096;

    m_bitmap.bitmap = (uint8_t*)LargestSegement;
    m_bitmap.size = LargestSegmentSize;

    for (size_t entry = 0; entry < MapEntries; entry++)
    {
        EFI_MEMORY_DESCRIPTOR* mapentry = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)info->mMap + (entry * info->DescriptorSize));
        ReservePages((void*)mapentry->PhysicalStart,mapentry->NumberOfPages);
    }

    for (size_t entry = 0; entry < MapEntries; entry++)
    {
        EFI_MEMORY_DESCRIPTOR* mapentry = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)info->mMap + (entry * info->DescriptorSize));
        if (mapentry->Type == EfiBootServicesCode || mapentry->Type == EfiBootServicesData || mapentry->Type == EfiConventionalMemory)
        {
            UnreservePages((void*)mapentry->PhysicalStart,mapentry->NumberOfPages);
        }
    }

    LockPages(0,0x100);
}



void PageAllocater::ReservePage(void* page)
{
    uint64_t PageIndex = (uint64_t)page / 4096;

    if (m_bitmap[PageIndex]) return;

    m_bitmap.Set(PageIndex,true);
    ReservedRam += 4096;
    FreeRam -= 4096;
}

void PageAllocater::ReservePages(void* page,size_t count)
{
    for (uint64_t pagea = (uint64_t)page; pagea < (uint64_t)page + count * 4096; pagea += 0x1000)
    {
        ReservePage((void*)pagea);
    }
}

void PageAllocater::UnreservePage(void* page)
{
    uint64_t PageIndex = (uint64_t)page / 4096;

    if (!m_bitmap[PageIndex]) return;

    m_bitmap.Set(PageIndex,false);

    ReservedRam -= 4096;
    FreeRam += 4096;
}

void PageAllocater::UnreservePages(void* page,size_t count)
{
    NextIndex = (uint64_t)page / 4096;
    for (uint64_t pagea = (uint64_t)page; pagea < (uint64_t)page + count * 4096; pagea += 0x1000)
    {
        UnreservePage((void*)pagea);
    }
}

void PageAllocater::LockPage(void* page)
{
    uint64_t PageIndex = (uint64_t)page / 4096;

    if (m_bitmap[PageIndex]) return;

    m_bitmap.Set(PageIndex,true);
    UsedRam += 4096;
    FreeRam -= 4096;
}

void PageAllocater::LockPages(void* page,size_t count)
{
    for (uint64_t pagea = (uint64_t)page; pagea < (uint64_t)page + count * 4096; pagea += 0x1000)
    {
        LockPage((void*)pagea);
    }
}

void PageAllocater::UnlockPage(void* page)
{
    uint64_t PageIndex = (uint64_t)page / 4096;

    if (!m_bitmap[PageIndex]) return;

    m_bitmap.Set(PageIndex,false);
    UsedRam -= 4096;
    FreeRam += 4096;

    NextIndex = PageIndex;
}

void* PageAllocater::RequestPage()
{
    for (; NextIndex < TotalPageCount; NextIndex++)
    {
        if (!m_bitmap[NextIndex])
        {
            LockPage((void*)(NextIndex * 4096));
            return (void*)(NextIndex*4096);
        }
    }
    NextIndex = 0;
    for (; NextIndex < TotalPageCount; NextIndex++)
    {
        if (!m_bitmap[NextIndex])
        {
            LockPage((void*)(NextIndex * 4096));
            return (void*)(NextIndex*4096);
        }
    }

    return nullptr;
}

void PageAllocater::FreePage(void* page)
{
    UnlockPage(page);
}

void* PageAllocater::RequestPages(size_t pagecount)
{
    size_t concurrentpages = 0;
    size_t start_page = NextIndex;
    for (; NextIndex < TotalPageCount; NextIndex++)
    {
        if (!m_bitmap[NextIndex] && concurrentpages == 0) start_page = NextIndex;
        if (!m_bitmap[NextIndex])
        {
            concurrentpages++;
        }
        if (m_bitmap[NextIndex])
        {
            concurrentpages = 0;
        }

        if (concurrentpages == pagecount)
        {
            LockPages((void*)(start_page * 4096),pagecount);
            Logger::DebugLog("Pages: %x\n",LOG_LEVEL::INFO,start_page * 4096);
            return (void*)(start_page * 4096);
        }
    }
    concurrentpages = 0;
    NextIndex = 0;
    for (; NextIndex < TotalPageCount; NextIndex++)
    {
        if (!m_bitmap[NextIndex] && concurrentpages == 0) start_page = NextIndex;
        if (!m_bitmap[NextIndex])
        {
            concurrentpages++;
        }
        if (m_bitmap[NextIndex])
        {
            concurrentpages = 0;
        }

        if (concurrentpages == pagecount)
        {
            LockPages((void*)(start_page * 4096),pagecount);
            return (void*)(start_page * 4096);
        }
    }
    Logger::DebugLog("Failed to find pages\n",LOG_LEVEL::ERROR);
    return nullptr;
}


void PageAllocater::FreePages(void* page,size_t pagecount)
{
    for (size_t p = 0; p < pagecount; p++)
    {
        FreePage((void*)((uint64_t)page + pagecount * 4096));
    }
}