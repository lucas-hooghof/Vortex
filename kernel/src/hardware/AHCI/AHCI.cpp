#include <hardware/AHCI/AHCI.h>

#include <hardware/PCI/PCI.h>
#include <memory/PageTableManager.h>

#include <generic/string.h>

namespace PCI
{
    AHCI::AHCI()
    {
        PCIDeviceHeader pheader = PCI::GetDevice(0x8086,0x2922);
        
        abar = (HBA_MEM*)(pheader.BAR5 & ~0xF);
        PageTableManager::GetInstance()->MapMemory((void*)abar,(void*)abar,PAGE_PRESENT | PAGE_RW | PAGE_PCD);

        pheader.CommonHeader.Command |= (1 << 1); // Enable Memory Space
        pheader.CommonHeader.Command |= (1 << 2); // Enable DMA

        PCI::WriteDevice(pheader);

        volatile uint32_t* bohc = (uint32_t*)((uint64_t)abar + 0x28); // BIOS/OS Handoff
        if (*bohc & 1) { // BIOS owns it
            *bohc |= (1 << 1); // request OS ownership

            while (*bohc & 1); // wait until BIOS releases
        }

        volatile uint32_t* ghc = (uint32_t*)((uint64_t)abar + 0x04); // Global HBA control
        *ghc |= (1 << 0); // HBA reset

        while (*ghc & (1 << 0)); // wait till its done  

        *ghc |= (1 << 31); // AHCI mode

        ProbePorts();
    }

    void AHCI::ProbePorts()
    {
        uint32_t pi  = *(uint32_t*)((uint64_t)abar + 0x0C);
        for (int i = 0; i < 32; i++)
        {
            if (!(pi & (1 << i))) continue;

            HBA_PORT* port = (HBA_PORT*)((uint64_t)abar + 0x100 + i * 0x80);
            void* clb = PageAllocater::GetInstance()->RequestPage();
            void* FISBuffer = PageAllocater::GetInstance()->RequestPage();
            void* cmdtbl = PageAllocater::GetInstance()->RequestPage();
            memset(clb, 0, 4096);
            memset(FISBuffer, 0, 4096);
            memset(cmdtbl, 0, 4096);
            PageTableManager::GetInstance()->MapMemory((void*)clb,(void*)clb,PAGE_PRESENT | PAGE_RW | PAGE_PCD);
            PageTableManager::GetInstance()->MapMemory((void*)FISBuffer,(void*)FISBuffer,PAGE_PRESENT | PAGE_RW | PAGE_PCD);
            PageTableManager::GetInstance()->MapMemory((void*)cmdtbl,(void*)cmdtbl,PAGE_PRESENT | PAGE_RW | PAGE_PCD);

            port->clb = (uint64_t)(clb) & 0x00000000FFFFFFFF;
            port->clbu = ((uint64_t)(clb) & 0xFFFFFFFF00000000) >> 32;

            port->fb = (uint64_t)(FISBuffer) & 0x00000000FFFFFFFF;
            port->fbu = ((uint64_t)(FISBuffer) & 0xFFFFFFFF00000000) >> 32;

            HBA_CMD_HEADER* cmdheader = (HBA_CMD_HEADER*)clb;
            for (int slot = 0; slot < 32; slot++) {
                cmdheader[slot].prdtl = 8; // Number of PRDT entries per command table
                cmdheader[slot].ctba = (uint64_t)cmdtbl + slot * sizeof(HBA_CMD_TBL);
            }

            port->cmd &= ~((1 << 4) | (1 << 0)); // Stop command engine first
            while (port->cmd & (1 << 15));       // Wait until CR (command list running) is cleared

            port->is = 0xFFFFFFFF;  // Clear interrupt status
            port->cmd |= (1 << 4);  
        }
    }
}