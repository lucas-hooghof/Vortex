#include "bootinfo.hpp"

size_t GetMemorySize(EFI_MEMORY_DESCRIPTOR* mMap,size_t MapSize,size_t MapDescSize)
{
    static uint64_t memorySizeBytes = 0;
    if (memorySizeBytes > 0) return memorySizeBytes;

    size_t mMapEntries = MapSize / MapDescSize;
    for (size_t i = 0; i < mMapEntries; i++){
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)mMap + (i * MapDescSize));
        memorySizeBytes += desc->NumberOfPages * 4096;
    }

    return memorySizeBytes;
}