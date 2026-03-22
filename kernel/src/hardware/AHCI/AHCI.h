#pragma once

#include <generic/stdint.h>
#include <hardware/PCI/PCI.h>

namespace PCI
{
    class AHCI
    {
        public:
            AHCI(PCIDevice* device);
            ~AHCI();
        private:
            PCIDevice* m_device;
    };
}