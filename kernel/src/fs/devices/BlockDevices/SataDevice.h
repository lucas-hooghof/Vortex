#pragma once

#include <fs/VFS.h>

#include <hardware/AHCI/AHCI.h>

#define DCTL_REQUEST_SECTOR_SIZE 0x01
#define DCTL_REQUEST_SECTOR_COUNT 0x02

namespace fs
{
    class SataDevice : public Device
    {
        public:
            SataDevice(const char* addr,size_t drivelbastart,PCI::AHCI* device,PCI::HBA_PORT* port);

            virtual size_t Write(size_t buffersize,void* buffer) override;
            virtual size_t Read(size_t buffersize,void* outbuffer) override;
            virtual void Seek(size_t offset) override;
            virtual size_t dctl(size_t request) override
            {
                switch(request)
                {
                    case DCTL_REQUEST_SECTOR_COUNT:
                        return lba;
                    case DCTL_REQUEST_SECTOR_SIZE:
                        return sectorsize;
                    default:
                        return 0;
                }
            }

        private:
            size_t lba;
            size_t lbastart;
            size_t sectorsize;

            size_t currentlba;

            PCI::AHCI* ahci;
            PCI::HBA_PORT* dport;

    };
}