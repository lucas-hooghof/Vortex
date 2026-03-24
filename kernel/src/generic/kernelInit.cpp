#include <generic/kernelInit.h>
#include <generic/string.h>
#include <generic/stdio.h>

#include <memory/PageAllocater.h>
#include <memory/PageTableManager.h>
#include <memory/GDT.h>
#include <memory/heap.h>

#include <interrupts/ISR.h>
#include <interrupts/IDT.h>

#include <hardware/PCI/PCI.h>
#include <hardware/ACPI/ACPI.h>
#include <hardware/AHCI/AHCI.h> 

#include <fs/VFS.h>
#include <fs/devices/FramebufferDevice.h>
#include <fs/partitions.h>


extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;
extern uint64_t __stack_start;
extern uint64_t __stack_end;
extern uint64_t __stack_bottom;

uint64_t HHDM = 0;

PageAllocater pa;
PageTableManager ptm;



bool PrepareMemory(bootinfo_t* info)
{
    pa.Initilize(info);

    // Lock framebuffer pages
    for (uint64_t fbpage = 0; fbpage < info->framebuffer->BufferSize / 4096 + 1; fbpage++)
    {
        pa.LockPage((void*)((uint64_t)info->framebuffer->BaseAddress + fbpage * 4096));
    }


    uint64_t kernelSize = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
    uint64_t kernelPages = (uint64_t)kernelSize / 4096 + 1;

    pa.LockPages(&_KernelStart,kernelPages);
    pa.LockPages(info->framebuffer->BaseAddress,(info->framebuffer->BufferSize + 4095) / 4096);


    uint64_t* pml4 =(uint64_t*) pa.RequestPage();
    memset((void*)pml4,0,4096); 
    ptm.Initilize((void*)pml4);


    for (uint64_t t = 0; t < (info->MapSize / info->DescriptorSize); t++)
    {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)info->mMap + t * info->DescriptorSize);

        if (desc->Type != EfiReservedMemoryType)
        {
            for (uint64_t p = 0; p < desc->NumberOfPages; p++)
            {
                ptm.MapMemory((void*)(desc->PhysicalStart + (p * 4096)),(void*)(desc->PhysicalStart + (p * 4096)),PAGE_PRESENT | PAGE_RW);
            }
        }
    }

    for (uint64_t fbpage = 0; fbpage < info->framebuffer->BufferSize / 4096 + 1; fbpage++)
    {
        ptm.MapMemory((void*)((uint64_t)info->framebuffer->BaseAddress + fbpage * 4096),(void*)((uint64_t)info->framebuffer->BaseAddress + fbpage * 4096),PAGE_PRESENT | PAGE_RW | PAGE_PWT | PAGE_PCD);
    }

    uint64_t Kernelstart = 0xFFFFFFFF80000000ULL;
    for (uint64_t t = 0; t < kernelPages; t++)
    {
        ptm.MapMemory((void*)(Kernelstart + t * 4096),(void*)(info->kernelstart + t * 4096),PAGE_PRESENT | PAGE_RW);
    }
    
    uint64_t stack_virt_start = (uint64_t)&__stack_bottom;
    uint64_t stack_virt_end   = (uint64_t)&__stack_end;

    for(uint64_t addr = stack_virt_start; addr < stack_virt_end; addr += 4096)
    {
        uint64_t phys = info->kernelstart + (addr - Kernelstart);

        ptm.MapMemory(
            (void*)addr,
            (void*)phys,
            PAGE_PRESENT | PAGE_RW
        );
    }
    
    HHDM = 0xFFFFFFFF80000000;

    asm volatile ("mov %0,%%cr3" : : "r"(pml4));

    GDTR gdtr = {0,0};

    gdtr.Size = sizeof(g_GDT) - 1;
    gdtr.Offset = (uint64_t)&g_GDT;

    LoadGDT(&gdtr);

    InitializeHeap((void*)(HHDM + 0x0000100000000000),0x10);


    return true;
}

bool PrepareInterrupts()
{
    IDT_R idtr = {0,0};
    idtr.Size = sizeof(IDT) - 1;
    idtr.Offset = (uint64_t)&IDT;

    asm volatile ("lidt %0" : : "m"(idtr));
    if(!InitilizeISR()) { return false; }
    asm volatile ("sti");

    return true;
}

bool PrepareHardware(bootinfo_t* info)
{
    PCI::PCI::Initilize();
    if (!fs::VFS::Initilize())
    {
        return false;
    }

    fs::FramebufferDevice* fbdevice = new fs::FramebufferDevice("/dev/fb0",info->framebuffer);
    fs::VFS::RegisterVirtualDevice(fbdevice);


    PCI::PCIDevice* devicelist = PCI::PCI::GetDeviceHeaders();
    for (uint32_t i = 0; i < PCI::PCI::GetDeviceCount(); i++)
    {
        PCI::PCIDeviceHeader deviceheader = PCI::PCI::GetDevice(devicelist[i]);

        if (deviceheader.CommonHeader.ClassCode == 0x01 &&
            deviceheader.CommonHeader.SubClass == 0x06 && 
            deviceheader.CommonHeader.ProgramInterface == 0x1)
        {
            PCI::AHCI* driver = new PCI::AHCI(&deviceheader);
            void* readpage = PageAllocater::GetInstance()->RequestPage();
            fs::MBR* mbr = (fs::MBR*)readpage;
            PageTableManager::GetInstance()->MapMemory(readpage, readpage, PAGE_PRESENT | PAGE_RW | PAGE_PCD);
            memset(mbr, 0, sizeof(fs::MBR));
            for (uint8_t port = 0; port < 32; port++)
            {
                if (driver->Ports[port] == nullptr) continue;


                if(!driver->Read(driver->Ports[port],0,1,mbr))
                {
                    Logger::Log("Read fail\n",LOG_LEVEL::INFO);
                }
                if (mbr->Signature == 0xAA55)
                {
                    if (mbr->Partitions[0].Type == 0xEE)
                    {
                        fs::GPTHeader* gpth = (fs::GPTHeader*)((uint64_t)readpage + sizeof(fs::MBR));
                        driver->Read(driver->Ports[port],1,1,gpth);
                        if (!memcmp(gpth->Signature,"EFI PART",8))
                        {
                            fs::GPTPartition* partitiontable = (fs::GPTPartition*)PageAllocater::GetInstance()->RequestPages(4);
                            driver->Read(driver->Ports[port],2,32,partitiontable);
                            for (uint32_t i = 0; i < gpth->NumPartitionEntries; i++)
                            {
                                fs::GPTPartition partition = partitiontable[i];
                                fs::GPTGuid guid = GPT_GUID_VXFS_ROOT;

                                if (partition.StartingLBA == 0) break;

                                if (!memcmp((void*)&partition.PartitionTypeGUID,(void*)&guid,sizeof(fs::GPTGuid)))
                                {
                                    
                                    break;
                                }
                            }
                        }
                    }
                }
                else  {
                    fs::GPTHeader* gpth = (fs::GPTHeader*)((uint64_t)readpage);
                    if (!memcmp(gpth->Signature,"EFI PART",8))
                    {
                        fs::GPTPartition* partitiontable = (fs::GPTPartition*)PageAllocater::GetInstance()->RequestPages(4);
                        driver->Read(driver->Ports[port],1,32,partitiontable);
                        for (uint32_t i = 0; i < gpth->NumPartitionEntries; i++)
                        {
                            fs::GPTPartition partition = partitiontable[i];
                            fs::GPTGuid guid = GPT_GUID_VXFS_ROOT;

                            if (partition.StartingLBA == 0) break;

                            if (!memcmp((void*)&partition.PartitionTypeGUID,(void*)&guid,sizeof(fs::GPTGuid)))
                            {
                                //Parse this drive and load it into the VFS
                            }
                        }
                    }
                }
            }

            delete driver;
        }
    }

    return true;
}