#include <PCI/PCI.h>

#include <memory/PageTableManager.h>
#include <generic/kernelInit.h>

namespace PCI
{
    PCI* PCI::s_Instance = nullptr;

    PCI::PCI()
    {
        if (s_Instance) return;
        s_Instance = this;
        m_deviceHeaderPageCount = 4;
        void* page = (PCIDevice*)PageAllocater::GetInstance()->RequestPage();
        Logger::DebugLog("%x\n",LOG_LEVEL::INFO,HHDM + (uint64_t)page);
        PageTableManager::GetInstance()->MapMemory((void*)(HHDM + (uint64_t)page),(void*)page,PAGE_PRESENT | PAGE_RW);
        m_deviceHeaders = (PCIDevice*)((uint64_t)page + HHDM);
        Logger::DebugLog("%p\n", LOG_LEVEL::INFO, m_deviceHeaders);
        for (uint8_t bus = 0; bus < 255; bus++)
        {
            CheckBus(bus);
        }
    }

    PCI::~PCI()
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
        uint16_t VendorID = ReadPCIConfigWord(bus,device,function,0);
        uint16_t DeviceID = ReadPCIConfigWord(bus,device,function,0x2);
        if (VendorID == 0xFFFF || VendorID == 0x0) return;

        if (((((++m_DeviceHeaderCount) * sizeof(PCIDevice)) + 4095) / 4096) > m_deviceHeaderPageCount)
        {
            PanicPCI("Not enough space for PCI devices");
        }

        PCIDevice* pcidevice = &m_deviceHeaders[m_DeviceHeaderCount - 1];

        pcidevice->bus = bus;
        pcidevice->device = device;
        pcidevice->function = function;
        pcidevice->VendorID = VendorID;
        pcidevice->DeviceID = DeviceID;
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
}