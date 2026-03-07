#pragma once

#include <generic/stdint.hpp>
#include <generic/bootinfo.hpp>

#include <generic/Bitmap.hpp>

class PageAllocater
{
public:
    PageAllocater() = default;
    bool Initilize(EFI_MEMORY_DESCRIPTOR* mMap,size_t MapSize,size_t MapDescSize);

    void* RequestPage();
    void FreePage(void* page);

    void ReservePage(void* page);
    void ReservePages(void* page,size_t pagecount);

    void UnReservePage(void* page);
    void UnReservePages(void* page,size_t pagecount);

    void LockPage(void* page);
    void LockPages(void* page,size_t pagecount);

    static PageAllocater* GetInstance()  { return m_Instance; }
private:
    Bitmap m_bitmap;
    static PageAllocater* m_Instance;

    size_t UsedRam;
    size_t FreeRam;
    size_t ReservedRam;
};
