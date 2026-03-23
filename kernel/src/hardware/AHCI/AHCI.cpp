#include <hardware/AHCI/AHCI.h>

#include <memory/PageTableManager.h>

namespace PCI
{
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

        PageTableManager::GetInstance()->MapMemory((void*)device,(void*)device,PAGE_PRESENT | PAGE_PCD | PAGE_RW);

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
}