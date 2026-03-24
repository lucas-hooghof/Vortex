#pragma once

#include <generic/stdint.h>

void UTF16ToChar(const uint16_t* input, char* output, size_t maxChars)
{
    size_t i = 0;

    for (; i < maxChars - 1; i++)
    {
        if (input[i] == 0)
            break;

        output[i] = (char)(input[i] & 0xFF);
    }

    output[i] = '\0';
}

#define GPT_GUID_VXFS_ROOT \
{ \
    0xCE2ADFBE, \
    0x1C54, \
    0x11F1, \
    { 0xA2, 0xED, 0x00, 0x15, 0x5D, 0x80, 0x74, 0xC4 } \
}

#define GUID_EMPTY \
{ \
    0x00000000, \
    0x0000, \
    0x0000, \
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } \
}

namespace fs
{
    struct GPTGuid
    {
        uint32_t Data1;
        uint16_t Data2;
        uint16_t Data3;
        uint8_t  Data4[8];
    } __attribute__((packed));

    struct GPTHeader
    {
        char     Signature[8];        // "EFI PART"
        uint32_t Revision;            // usually 0x00010000
        uint32_t HeaderSize;

        uint32_t HeaderCRC32;

        uint32_t Reserved;

        uint64_t MyLBA;               // LBA of this header
        uint64_t AlternateLBA;        // backup header
        uint64_t FirstUsableLBA;
        uint64_t LastUsableLBA;

        GPTGuid  DiskGUID;

        uint64_t PartitionEntryLBA;   // where partition entries start
        uint32_t NumPartitionEntries;
        uint32_t SizeOfPartitionEntry;

        uint32_t PartitionEntryArrayCRC32;

        uint8_t padding[512-92];
    } __attribute__((packed));

    struct GPTPartition
    {
        GPTGuid PartitionTypeGUID;
        GPTGuid UniquePartitionGUID;

        uint64_t StartingLBA;
        uint64_t EndingLBA;

        uint64_t Attributes;

        uint16_t PartitionName[36]; // UTF-16 (36 chars)

    } __attribute__((packed));

    struct PartitionEntry
    {
        uint8_t  Status;        // 0x80 = bootable, 0x00 = not bootable
        uint8_t  CHSFirst[3];   // CHS address of first sector
        uint8_t  Type;          // Partition type
        uint8_t  CHSLast[3];    // CHS address of last sector
        uint32_t LBAFirst;      // LBA of first sector
        uint32_t SectorCount;   // Number of sectors in partition
    } __attribute__((packed));

    struct MBR
    {
        uint8_t        Bootloader[446];   // Bootstrap code
        PartitionEntry Partitions[4];     // Four primary partition entries
        uint16_t       Signature;         // 0xAA55
    } __attribute__((packed));
}