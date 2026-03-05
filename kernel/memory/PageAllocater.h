#pragma once

#include <generic/stdint.h>
#include <generic/bootinfo.h>

#include <generic/stdio.h>



class PageAllocater
{
public:
    PageAllocater(EFI_MEMORY_DESCRIPTOR* desc,size_t mMapSize, size_t mMapDescSize);
    PageAllocater() = default;

    void* AllocatePage();
    void UnallocatePage(void* Page);

    void LockPage(void* Page);

    void ReservePage(void* Page);
    void ReservePages(void* Page,size_t PageCount);
    void UnreservePage(void* Page);
    void UnreservePages(void* Page,size_t PageCount);

    const uint64_t& GetFreeRam() { return this->FreeRam; }
    const uint64_t& GetReservedRam() { return this->ReservedRam; }
    const uint64_t& GetUsedRam() { return this->UsedRam; }
private:
    void* BitmapAddress;
    size_t bitmapsize;



    bool GetBit(size_t index);
    void SetBit(size_t index,bool value);

    uint64_t FreeRam;
    uint64_t ReservedRam;
    uint64_t UsedRam;
};

extern PageAllocater GlobalAllocator;