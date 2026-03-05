#include <memory/PageAllocater.h>

#include <generic/stdio.h>

static bool Initilized = false;

PageAllocater GlobalAllocator;

PageAllocater::PageAllocater(EFI_MEMORY_DESCRIPTOR* desc, size_t mMapSize, size_t mMapDescSize)
{
    if (Initilized) return;
    Initilized = true;

    this->FreeRam = 0;
    this->UsedRam = 0;
    this->ReservedRam = 0;

    uint64_t Mapentries = mMapSize / mMapDescSize;

    uint64_t memorySize = 0;
    uint64_t largestSegmentSize = 0;
    void* LargestSegment = nullptr;

    for (uint64_t entry = 0; entry < Mapentries; entry++)
    {
        EFI_MEMORY_DESCRIPTOR* memdesc =
            (EFI_MEMORY_DESCRIPTOR*)((uint64_t)desc + (entry * mMapDescSize));

        uint64_t segmentSize = memdesc->NumberOfPages * 4096;

        if (memdesc->Type == EfiConventionalMemory)
        {
            memorySize += segmentSize;

            if (segmentSize > largestSegmentSize)
            {
                largestSegmentSize = segmentSize;
                LargestSegment = (void*)memdesc->PhysicalStart;
            }
        }
    }

    this->FreeRam = memorySize;
    size_t bitmapsize = memorySize / 4096 / 8 + 1;

    if (LargestSegment == nullptr)
    {
        printf("Cant find memory for bitmap");
        while (true) {}
    }

    uint8_t* bitmap = (uint8_t*)LargestSegment;

    for (size_t i = 0; i < bitmapsize; i++)
        bitmap[i] = 0;

    this->BitmapAddress = bitmap;
    this->bitmapsize = bitmapsize;

    size_t bitmapPages = bitmapsize / 4096 + 1;

    for (size_t page = 0; page < bitmapPages; page++)
    {
        LockPage((void*)((uint64_t)bitmap + page * 4096));
    }

    for (uint64_t entry = 0; entry < Mapentries; entry++)
    {
        EFI_MEMORY_DESCRIPTOR* memdesc =
            (EFI_MEMORY_DESCRIPTOR*)((uint64_t)desc + (entry * mMapDescSize));

        if (memdesc->Type != EfiConventionalMemory)
        {
            ReservePages((void*)memdesc->PhysicalStart, memdesc->NumberOfPages);
        }
    }
}
void* PageAllocater::AllocatePage()
{
    for (uint64_t index = 0; index < this->bitmapsize * 8; index++)
    {
        if (GetBit(index)) continue;

        void* addr = (void*)(index * 4096);
        LockPage(addr);
        return addr;
    }
    printf("Failed to find a page\n\r");
    return nullptr;
}
void PageAllocater::UnallocatePage(void* Page)
{
    uint64_t index = (uint64_t)Page / 4096;
    if (this->GetBit(index) == false) return;
    this->SetBit(index, false);
    this->FreeRam += 4096;
    this->UsedRam -= 4096;
}

void PageAllocater::LockPage(void* Page)
{
    uint64_t index = (uint64_t)Page / 4096;
    if (this->GetBit(index) == true) return;
    this->SetBit(index, true);
    this->FreeRam -= 4096;
    this->UsedRam += 4096;
}

void PageAllocater::ReservePage(void* Page)
{
    uint64_t index = (uint64_t)Page / 4096;
    if (this->GetBit(index) == true) return;
    this->SetBit(index, true);
    this->FreeRam -= 4096;
    this->ReservedRam += 4096;
}
void PageAllocater::UnreservePage(void* Page)
{
    uint64_t index = (uint64_t)Page / 4096;
    if (this->GetBit(index) == false) return;
    this->SetBit(index, false);
    this->FreeRam += 4096;
    this->ReservedRam -= 4096;
}

void PageAllocater::ReservePages(void* Page,size_t PageCount)
{
    for (size_t page = 0; page < PageCount; page++)
    {
        this->ReservePage((void*)((uint64_t)Page + page * 4096));
    }
}

void PageAllocater::UnreservePages(void* Page,size_t PageCount)
{
    for (size_t page = 0; page < PageCount; page++)
    {
        this->UnreservePage((void*)((uint64_t)Page + page * 4096));
    }
}


bool PageAllocater::GetBit(size_t index)
{
    uint32_t byteIndex = index / 8;
    uint8_t bitIndex = index % 8;
    uint8_t bitIndexer = 0b10000000 >> bitIndex;
    
    uint8_t* bitmap = (uint8_t*)this->BitmapAddress;
    if ((bitmap[byteIndex] & bitIndexer) > 0)
    {
        return true;
    }

    return false;
}
void PageAllocater::SetBit(size_t index,bool value)
{
    uint64_t byteIndex = index / 8;
    uint8_t bitIndex = index % 8;
    uint8_t bitIndexer = 0b10000000 >> bitIndex;

    uint8_t* bitmap = (uint8_t*)this->BitmapAddress;
    bitmap[byteIndex] &= ~bitIndexer;
    if (value){
        bitmap[byteIndex] |= bitIndexer;
    }
}
