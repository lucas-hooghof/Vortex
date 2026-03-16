#pragma once

#include <generic/io.h>
#include <generic/stdint.h>
#include <generic/stdio.h>


namespace PCI
{
    #define PCI_CONFIG_ADDRESS 0xCF8
    #define PCI_CONFIG_DATA    0xCFC

    #define PCI_HEADER_TYPE_GENERAL_DEVICE          0x0
    #define PCI_HEADER_TYPE_PCI_TO_PCI_BRIDGE       0x1
    #define PCI_HEADER_TYPE_PCI_TO_CARDBUS          0x2

    struct PCICommonHeader
    {
        uint16_t VendorID;
        uint16_t DeviceID;
        uint16_t Command;
        uint16_t Status;
        uint8_t RevisionID;
        uint8_t ProgramInterface;
        uint8_t SubClass;
        uint8_t ClassCode;
        uint8_t CacheLineSize;
        uint8_t LatencyTimer;
        uint8_t HeaderType;
        uint8_t BIST;
    };

    struct PCIDeviceHeader
    {
        PCICommonHeader CommonHeader;
        uint32_t BAR0;
        uint32_t BAR1;
        uint32_t BAR2;
        uint32_t BAR3;
        uint32_t BAR4;
        uint32_t BAR5;
    };
}