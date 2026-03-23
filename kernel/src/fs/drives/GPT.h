#pragma once

#include <generic/stdint.h>

namespace fs
{
    struct GPT_HEADER
    {
        char Signature[8];
        uint32_t Revision;
        uint32_t HeaderSize;
        uint32_t HeaderCRC32;
        uint32_t Reserved;
        uint64_t MyLBA;
        uint64_t AlternateLBA;
        uint64_t FirstUsableLBA;
        uint64_t LastUseableLBA;
        uint8_t DiskGuid[16];
        uint64_t PartitiontableLBA;
        uint32_t NumberOfPartitions;
        uint32_t SizeOfPartitionEntry;
        uint32_t PartitionTableCRC32;
        uint8_t padding[512-92];
    }__attribute__((packed));
    
}