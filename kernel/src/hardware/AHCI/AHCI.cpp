#include <hardware/AHCI/AHCI.h>

#include <memory/PageTableManager.h>

#include <generic/string.h>

namespace PCI
{
    // Forward declarations (defined later in this file)
    void stop_cmd(HBA_PORT *port);
    void start_cmd(HBA_PORT *port);
    int find_cmdslot(HBA_MEM* abar, HBA_PORT *port);
    int check_type(HBA_PORT* port);

    #define	SATA_SIG_ATA	0x00000101	// SATA drive
    #define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
    #define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
    #define	SATA_SIG_PM	0x96690101	// Port multiplier

    #define AHCI_DEV_NULL 0
    #define AHCI_DEV_SATA 1
    #define AHCI_DEV_SEMB 2
    #define AHCI_DEV_PM 3
    #define AHCI_DEV_SATAPI 4

    #define HBA_PORT_IPM_ACTIVE 1
    #define HBA_PORT_DET_PRESENT 3


    #define HBA_PxCMD_ST    0x0001
    #define HBA_PxCMD_FRE   0x0010
    #define HBA_PxCMD_FR    0x4000
    #define HBA_PxCMD_CR    0x8000

    #define HBA_PxIS_TFES (1 << 30)
    #define ATA_CMD_READ_DMA_EX 0x25

    void port_rebase(HBA_PORT *port, int portno)
    {
        stop_cmd(port);	// Stop command engine
        uint64_t ahci_base = (uint64_t)PageAllocater::GetInstance()->RequestPages(3);
        PageTableManager::GetInstance()->MapMemory((void*)ahci_base,(void*)ahci_base,PAGE_PRESENT | PAGE_RW | PAGE_PCD);
        PageTableManager::GetInstance()->MapMemory((void*)(ahci_base + 0x1000),(void*)(ahci_base + 0x1000),PAGE_PRESENT | PAGE_RW | PAGE_PCD);
        PageTableManager::GetInstance()->MapMemory((void*)(ahci_base + 0x2000),(void*)(ahci_base + 0x2000),PAGE_PRESENT | PAGE_RW | PAGE_PCD);

        // Command list offset: 1K*portno
        // Command list entry size = 32
        // Command list entry maxim count = 32
        // Command list maxim size = 32*32 = 1K per port
        port->clb = ahci_base + (portno<<10);
        port->clbu = 0;
        // FIX: cast port->clb (uint32_t stored address) via uintptr_t to avoid
        //      "cast from integer of different size" on 64-bit targets.
        memset((void*)(uintptr_t)(port->clb), 0, 1024);

        // FIS offset: 32K+256*portno
        // FIS entry size = 256 bytes per port
        port->fb = ahci_base + (32<<10) + (portno<<8);
        port->fbu = 0;
        memset((void*)(uintptr_t)(port->fb), 0, 256);

        // Command table offset: 40K + 8K*portno
        // Command table size = 256*32 = 8K per port
        // FIX: use uintptr_t when converting the stored 32-bit base address to a pointer.
        HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(uintptr_t)(port->clb);
        for (int i=0; i<32; i++)
        {
            cmdheader[i].prdtl = 8;	// 8 prdt entries per command table
                        // 256 bytes per command table, 64+16+48+16*8
            // Command table offset: 40K + 8K*portno + cmdheader_index*256
            cmdheader[i].ctba = ahci_base + (40<<10) + (portno<<13) + (i<<8);
            cmdheader[i].ctbau = 0;
            memset((void*)(uintptr_t)cmdheader[i].ctba, 0, 256);
        }

        start_cmd(port);	// Start command engine
    }

    // Start command engine
    void start_cmd(HBA_PORT *port)
    {
        // Wait until CR (bit15) is cleared
        while (port->cmd & HBA_PxCMD_CR)
            ;

        // Set FRE (bit4) and ST (bit0)
        port->cmd |= HBA_PxCMD_FRE;
        port->cmd |= HBA_PxCMD_ST; 
    }

    // Stop command engine
    void stop_cmd(HBA_PORT *port)
    {
        // Clear ST (bit0)
        port->cmd &= ~HBA_PxCMD_ST;

        // Clear FRE (bit4)
        port->cmd &= ~HBA_PxCMD_FRE;

        // Wait until FR (bit14), CR (bit15) are cleared
        while(1)
        {
            if (port->cmd & HBA_PxCMD_FR)
                continue;
            if (port->cmd & HBA_PxCMD_CR)
                continue;
            break;
        }

    }

    int check_type(HBA_PORT* port)
    {
        uint32_t ssts = port->ssts;

        uint8_t ipm = (ssts >> 8) & 0x0F;
        uint8_t det = ssts & 0x0F;

        if (det != HBA_PORT_DET_PRESENT)	// Check drive status
            return AHCI_DEV_NULL;
        if (ipm != HBA_PORT_IPM_ACTIVE)
            return AHCI_DEV_NULL;

        switch (port->sig)
        {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        default:
            return AHCI_DEV_SATA;
        }
    }

    AHCI::AHCI(PCIDeviceHeader* device)
    {
        m_device = device;
        device->CommonHeader.Command |= 0b0000000000000110;
        PCI::WriteDevice(*device);

        PageTableManager::GetInstance()->MapMemory((void*)(uint64_t)device->BAR5,(void*)(uint64_t)device->BAR5,PAGE_PRESENT | PAGE_PCD | PAGE_RW);

        abar = (HBA_MEM*)(uint64_t)device->BAR5;
        //BIOS handoffs
        if (abar->bohc & 1)
        {

            abar->bohc |= (1 << 1);
            int timeout = 1000000;
            while ((abar->bohc & 1) && timeout--)
            {

            }

            timeout = 1000000;
            while ((abar->bohc & (1 << 4)) && timeout--)
            {
            }
        }

        for (int i = 0; i < 32; i++)
        {
            Ports[i] = nullptr;
        }

        //Reset HBA
        abar->ghc |= (1 << 0); //HBA RESET
        int timeout = 1000000;
        while ((abar->ghc & (1 << 0)) && timeout--) {}

        abar->ghc |= (1 << 31); //Enable AHCI

        uint32_t pi = abar->pi;
        int i = 0;
        while (i<32)
        {
            if (pi & 1)
            {
                int dt = check_type(&abar->ports[i]);
                if (dt == AHCI_DEV_SATA)
                {
                    Logger::Log("SATA drive found at port %d\n",LOG_LEVEL::INFO, i);
                    Ports[i] = &abar->ports[i];
                    port_rebase(&abar->ports[i],i);
                }
                else if (dt == AHCI_DEV_SATAPI)
                {
                    Logger::Log("SATAPI drive found at port %d\n",LOG_LEVEL::INFO, i);
                }
                else if (dt == AHCI_DEV_SEMB)
                {
                    Logger::Log("SEMB drive found at port %d\n",LOG_LEVEL::INFO, i);
                }
                else if (dt == AHCI_DEV_PM)
                {
                    Logger::Log("PM drive found at port %d\n",LOG_LEVEL::INFO, i);
                }
                else
                {
                    Logger::Log("No drive found at port %d\n",LOG_LEVEL::INFO, i);
                }
            }

            pi >>= 1;
            i ++;
        }

    }

    AHCI::~AHCI()
    {

    }

    #define ATA_DEV_BUSY 0x80
    #define ATA_DEV_DRQ 0x08

    bool AHCI::Read(HBA_PORT *port, uint64_t lba, uint32_t count, void *buf)
    {
        if (lba > 0x0000FFFFFFFFFFFF)
            return false;

        port->is = (uint32_t)-1; // Clear pending interrupt bits

        int spin = 0;
        int slot = find_cmdslot(abar, port);
        if (slot == -1)
            return false;

        HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(uintptr_t)port->clb;
        cmdheader += slot;
        cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
        cmdheader->w = 0; // Read
        cmdheader->prdtl = (uint16_t)(((count - 1) >> 4) + 1); // 16 sectors per entry

        HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(uintptr_t)(cmdheader->ctba);
        memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) +
            (cmdheader->prdtl - 1) * sizeof(HBA_PRDT_ENTRY));

        uint8_t *buffer = (uint8_t*)buf;

        int i = 0;
        for (i = 0; i < cmdheader->prdtl - 1; i++)
        {
            uintptr_t addr = (uintptr_t)buffer;

            cmdtbl->prdt_entry[i].dba  = (uint32_t)(addr & 0xFFFFFFFF);
            cmdtbl->prdt_entry[i].dbau = (uint32_t)(addr >> 32);
            cmdtbl->prdt_entry[i].dbc  = 8 * 1024 - 1; // 8K bytes
            cmdtbl->prdt_entry[i].i    = 1;

            buffer += 8 * 1024; // advance by 8KB
            count  -= 16;       // 16 sectors
        }

        // Last PRDT entry
        uintptr_t addr = (uintptr_t)buffer;
        cmdtbl->prdt_entry[i].dba  = (uint32_t)(addr & 0xFFFFFFFF);
        cmdtbl->prdt_entry[i].dbau = (uint32_t)(addr >> 32);
        cmdtbl->prdt_entry[i].dbc  = (count << 9) - 1; // sectors * 512
        cmdtbl->prdt_entry[i].i    = 1;

        // Setup FIS
        FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);

        cmdfis->fis_type = FIS_TYPE_REG_H2D;
        cmdfis->c        = 1;
        cmdfis->command  = ATA_CMD_READ_DMA_EX;

        cmdfis->lba0 = (uint8_t)(lba);
        cmdfis->lba1 = (uint8_t)(lba >> 8);
        cmdfis->lba2 = (uint8_t)(lba >> 16);
        cmdfis->device = 1 << 6; // LBA mode

        cmdfis->lba3 = (uint8_t)(lba >> 24);
        cmdfis->lba4 = (uint8_t)(lba >> 32);
        cmdfis->lba5 = (uint8_t)(lba >> 40);

        cmdfis->countl = count & 0xFF;
        cmdfis->counth = (count >> 8) & 0xFF;

        // Wait until port is ready
        while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
            spin++;

        if (spin == 1000000)
            return false;

        port->ci = 1 << slot; // Issue command

        // Wait for completion
        while (true)
        {
            if ((port->ci & (1 << slot)) == 0)
                break;

            if (port->is & HBA_PxIS_TFES)
                return false;
        }

        // Final error check
        if (port->is & HBA_PxIS_TFES)
            return false;

        return true;
    }

    // Find a free command list slot
    int find_cmdslot(HBA_MEM* abar,HBA_PORT *port)
    {
        int cmdslots = ((abar->cap >> 8) & 0x1F) + 1;
        // If not set in SACT and CI, the slot is free
        uint32_t slots = (port->sact | port->ci);
        for (int i=0; i<cmdslots; i++)
        {
            if ((slots&1) == 0)
                return i;
            slots >>= 1;
        }
        return -1;
    }

}