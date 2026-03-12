#pragma once

#include <generic/bitmap.h>
#include <generic/bootinfo.h>
#include <generic/stdint.h>

class PageAllocater
{
public:
    PageAllocater() = default;

    void Initilize(bootinfo_t* info);

    void* RequestPage();
    void FreePage(void* page);

    void* RequestPages(size_t pagecount);
    void FreePages(void* page,size_t pagecount);

    void ReservePage(void* page);
    void ReservePages(void* page,size_t count);

    void UnreservePage(void* page);
    void UnreservePages(void* page,size_t count);

    void LockPage(void* page);
    void LockPages(void* page,size_t count);

    static PageAllocater* GetInstance() { return s_Instance; }

    size_t GetTotalMemorySize() { return TotalMemorySize; }
    size_t GetTotalPageCount() { return TotalPageCount; }

    uint64_t GetFreeRam() { return FreeRam; }
    uint64_t GetReservedRam() { return ReservedRam; }
    uint64_t GetUsedRam() { return UsedRam; }

private:
    static PageAllocater* s_Instance;

    void UnlockPage(void* page);

    Bitmap m_bitmap;

    size_t TotalPageCount = 0;
    size_t TotalMemorySize = 0;

    uint64_t FreeRam;
    uint64_t ReservedRam;
    uint64_t UsedRam;

    uint64_t NextIndex;
};