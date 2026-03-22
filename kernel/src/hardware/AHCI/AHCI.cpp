#include <hardware/AHCI/AHCI.h>

namespace PCI
{
    AHCI::AHCI(PCIDevice* device)
    {
        m_device = device;
    }

    AHCI::~AHCI()
    {

    }
}