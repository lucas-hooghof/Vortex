#pragma once

#include <generic/io.h>
#include <generic/stdint.h>
#include <generic/stdio.h>

#include <memory/PageAllocater.h>


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
        uint32_t CardbusCISPointer;
        uint16_t SubsystemVendorID;
        uint16_t SubsystemID;   
        uint32_t ExpansionROMBaseAddress;
        uint8_t Capabilities;
        uint8_t reserved0[3];
        uint32_t reserved1;

        uint8_t InterruptLine;
        uint8_t InterruptPin;
        uint8_t MinGrant;
        uint8_t MaxLatency;
    };
    struct PCIDevice
    {
        uint16_t VendorID;
        uint16_t DeviceID;

        uint8_t bus;
        uint8_t device;
        uint8_t function;

        uint8_t ProgramInterface;
        uint8_t SubClass;
        uint8_t ClassCode;

        uint8_t padding[6];
    };

    struct PCIDeviceIntilized
    {
        PCIDevice device;
        void* Driver;
    };

    class PCI
    {
    public:
        static void Initilize();
        static void Deinit();

        static PCIDeviceHeader GetDevice(PCIDevice pcidevice);
        static void WriteDevice(PCIDeviceHeader header);

        static PCIDevice* GetDeviceHeaders()  { return m_deviceHeaders; }
        static uint32_t GetDeviceCount() { return m_DeviceHeaderCount; }

        static const char* GetVendorID(uint16_t VendorID)
        {
            switch(VendorID)
            {
                case 0x8086:
                    return "Intel Corp";
                case 0x1234:
                    return "Qemu";
                case 0x10EC:
                    return "Realtek Semiconductor Co";
                default:
                    return "Unkown";
            }
        }

    private: 
        static void CheckBus(uint8_t bus);
        static void CheckDevice(uint8_t bus,uint8_t device);
        static void CheckFunction(uint8_t bus,uint8_t device,uint8_t function);

        static void PanicPCI(const char* reason);
        static uint16_t ReadPCIConfigWord(uint8_t bus,uint8_t device,uint8_t function,uint8_t offset);
        static void WritePCIConfigWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value);
    private:
        static PCIDevice* m_deviceHeaders;
        static uint32_t m_DeviceHeaderCount;
        static uint32_t m_deviceHeaderPageCount;

    };
}