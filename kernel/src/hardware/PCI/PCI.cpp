#include <hardware/PCI/PCI.h>

#include <memory/PageTableManager.h>
#include <generic/kernelInit.h>

namespace PCI
{

    PCIDevice* PCI::m_deviceHeaders = nullptr;
    uint32_t   PCI::m_DeviceHeaderCount = 0;
    uint32_t   PCI::m_deviceHeaderPageCount = 0;

    void PCI::Initilize()
    {
        m_deviceHeaderPageCount = 4;
        m_DeviceHeaderCount = 0;
        m_deviceHeaders =  (PCIDevice*)PageAllocater::GetInstance()->RequestPages(m_deviceHeaderPageCount);
        if (m_deviceHeaders == nullptr)
        {
            PanicPCI("Failed to allocate page for PCI\n");
        }
        PageTableManager::GetInstance()->MapMemory((void*)m_deviceHeaders,(void*)m_deviceHeaders,PAGE_PRESENT | PAGE_RW);
        for (uint8_t bus = 0; bus < 255; bus++)
        {
            CheckBus(bus);
        }
    }

    void PCI::Deinit()
    {
        PageAllocater::GetInstance()->FreePages((void*)m_deviceHeaders,m_deviceHeaderPageCount);
    }

    void PCI::CheckBus(uint8_t bus)
    {
        for (uint8_t device = 0; device < 32; device++)
        {
            CheckDevice(bus,device);
        }
    }
    void PCI::CheckDevice(uint8_t bus,uint8_t device)
    {
        for (uint8_t function = 0; function < 8; function++)
        {
            CheckFunction(bus,device,function);
        }
    }
    void PCI::CheckFunction(uint8_t bus,uint8_t device,uint8_t function)
    {
        uint16_t VendorID = ReadPCIConfigWord(bus, device, function, 0);
        uint16_t DeviceID = ReadPCIConfigWord(bus, device, function, 0x2);
        if (VendorID == 0xFFFF || VendorID == 0x0) return;

        uint64_t needed = ((m_DeviceHeaderCount + 1) * sizeof(PCIDevice) + 4095) / 4096;
        if (needed > m_deviceHeaderPageCount) {
            PanicPCI("Not enough space for PCI devices");
            return;
        }

        PCIDevice& dev = m_deviceHeaders[m_DeviceHeaderCount++];
        
        dev.bus      = bus;
        dev.device   = device;
        dev.function = function;
        dev.VendorID = VendorID;
        dev.DeviceID = DeviceID;

        Logger::Log("%x:%x:%x %s / %x\n",LOG_LEVEL::INFO,bus,device,function,GetVendorID(VendorID),DeviceID);
    }

    void PCI::PanicPCI(const char* reason)
    {
        Logger::Log(reason,LOG_LEVEL::INFO);

        asm volatile ("cli; hlt");
    }

    uint16_t PCI::ReadPCIConfigWord(uint8_t bus,uint8_t device,uint8_t function,uint8_t offset)
    {
        uint32_t address;
        uint32_t lbus  = (uint32_t)bus;
        uint32_t ldevice = (uint32_t)device;
        uint32_t lfunction = (uint32_t)function;
        uint16_t tmp = 0;
    
        // Create configuration address as per Figure 1
        address = (uint32_t)((lbus << 16) | (ldevice << 11) |
                (lfunction << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

        outl(0xCF8, address);

        tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
        return tmp;
    }

    PCIDeviceHeader PCI::GetDevice(uint16_t VendorID,uint16_t DeviceID)
    {
        PCIDeviceHeader header = {};

        for (size_t entry = 0; entry < m_DeviceHeaderCount; entry++)
        {
            if (m_deviceHeaders[entry].VendorID == VendorID && m_deviceHeaders[entry].DeviceID == DeviceID)
            {
                uint8_t bus = m_deviceHeaders[entry].bus;
                uint8_t device = m_deviceHeaders[entry].device;
                uint8_t function = m_deviceHeaders[entry].function;
                header.CommonHeader.VendorID                = ReadPCIConfigWord(bus, device, function, 0x00);
                header.CommonHeader.DeviceID                = ReadPCIConfigWord(bus, device, function, 0x02);
                header.CommonHeader.Command                 = ReadPCIConfigWord(bus, device, function, 0x04);
                header.CommonHeader.Status                  = ReadPCIConfigWord(bus, device, function, 0x06);

                uint16_t w08                                = ReadPCIConfigWord(bus, device, function, 0x08);
                header.CommonHeader.RevisionID              = (uint8_t)(w08 & 0xFF);
                header.CommonHeader.ProgramInterface                  = (uint8_t)(w08 >> 8);

                uint16_t w0A                                = ReadPCIConfigWord(bus, device, function, 0x0A);
                header.CommonHeader.SubClass                = (uint8_t)(w0A & 0xFF);
                header.CommonHeader.ClassCode               = (uint8_t)(w0A >> 8);

                uint16_t w0C                                = ReadPCIConfigWord(bus, device, function, 0x0C);
                header.CommonHeader.CacheLineSize           = (uint8_t)(w0C & 0xFF);
                header.CommonHeader.LatencyTimer            = (uint8_t)(w0C >> 8);

                uint16_t w0E                                = ReadPCIConfigWord(bus, device, function, 0x0E);
                header.CommonHeader.HeaderType              = (uint8_t)(w0E & 0xFF);
                header.CommonHeader.BIST                    = (uint8_t)(w0E >> 8);

                // BARs (each is a dword, read as two words)
                header.BAR0  = (uint32_t)ReadPCIConfigWord(bus, device, function, 0x10) |
                            ((uint32_t)ReadPCIConfigWord(bus, device, function, 0x12) << 16);
                header.BAR1  = (uint32_t)ReadPCIConfigWord(bus, device, function, 0x14) |
                            ((uint32_t)ReadPCIConfigWord(bus, device, function, 0x16) << 16);
                header.BAR2  = (uint32_t)ReadPCIConfigWord(bus, device, function, 0x18) |
                            ((uint32_t)ReadPCIConfigWord(bus, device, function, 0x1A) << 16);
                header.BAR3  = (uint32_t)ReadPCIConfigWord(bus, device, function, 0x1C) |
                            ((uint32_t)ReadPCIConfigWord(bus, device, function, 0x1E) << 16);
                header.BAR4  = (uint32_t)ReadPCIConfigWord(bus, device, function, 0x20) |
                            ((uint32_t)ReadPCIConfigWord(bus, device, function, 0x22) << 16);
                header.BAR5  = (uint32_t)ReadPCIConfigWord(bus, device, function, 0x24) |
                            ((uint32_t)ReadPCIConfigWord(bus, device, function, 0x26) << 16);

                header.CardbusCISPointer =
                            (uint32_t)ReadPCIConfigWord(bus, device, function, 0x28) |
                            ((uint32_t)ReadPCIConfigWord(bus, device, function, 0x2A) << 16);

                header.SubsystemVendorID                    = ReadPCIConfigWord(bus, device, function, 0x2C);
                header.SubsystemID                          = ReadPCIConfigWord(bus, device, function, 0x2E);

                header.ExpansionROMBaseAddress =
                            (uint32_t)ReadPCIConfigWord(bus, device, function, 0x30) |
                            ((uint32_t)ReadPCIConfigWord(bus, device, function, 0x32) << 16);

                uint16_t w34                                = ReadPCIConfigWord(bus, device, function, 0x34);
                header.Capabilities                         = (uint8_t)(w34 & 0xFF);
                // reserved0 and reserved1 left as zero

                uint16_t w3C                                = ReadPCIConfigWord(bus, device, function, 0x3C);
                header.InterruptLine                        = (uint8_t)(w3C & 0xFF);
                header.InterruptPin                         = (uint8_t)(w3C >> 8);

                uint16_t w3E                                = ReadPCIConfigWord(bus, device, function, 0x3E);
                header.MinGrant                             = (uint8_t)(w3E & 0xFF);
                header.MaxLatency                           = (uint8_t)(w3E >> 8);

            }
        }
        return header;
    }

    void PCI::WriteDevice(PCIDeviceHeader header)
    {
        // Find matching device
        for (uint32_t i = 0; i < m_DeviceHeaderCount; i++)
        {
            if (m_deviceHeaders[i].VendorID == header.CommonHeader.VendorID &&
                m_deviceHeaders[i].DeviceID == header.CommonHeader.DeviceID)
            {
                uint8_t bus      = m_deviceHeaders[i].bus;
                uint8_t device   = m_deviceHeaders[i].device;
                uint8_t function = m_deviceHeaders[i].function;

                uint16_t cmd = header.CommonHeader.Command;

                // Ensure required bits for AHCI
                cmd |= (1 << 1);   // Memory space
                cmd |= (1 << 2);   // Bus master (DMA)
                cmd &= ~(1 << 10); // Enable interrupts

                WritePCIConfigWord(bus, device, function, 0x04, cmd);

                Logger::Log("PCI CMD set for %x:%x:%x -> %x\n",
                    LOG_LEVEL::INFO, bus, device, function, cmd);

                return;
            }
        }

        PanicPCI("WriteDevice: device not found");
    }

    void PCI::WritePCIConfigWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value)
    {
        uint32_t address = (uint32_t)(
            ((uint32_t)bus << 16) |
            ((uint32_t)device << 11) |
            ((uint32_t)function << 8) |
            (offset & 0xFC) |
            0x80000000
        );

        outl(PCI_CONFIG_ADDRESS, address);

        uint32_t data = inl(PCI_CONFIG_DATA);

        if (offset & 2) {
            data &= 0x0000FFFF;
            data |= ((uint32_t)value << 16);
        } else {
            data &= 0xFFFF0000;
            data |= value;
        }

        outl(PCI_CONFIG_DATA, data);
    }
}