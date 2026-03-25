#include <fs/devices/BlockDevices/SataDevice.h>

#include <generic/string.h>

namespace fs
{
    SataDevice::SataDevice(const char* address,size_t drivelbastart,PCI::AHCI* driver,PCI::HBA_PORT* port)
        : Device(address,FD_READ | FD_WRITE)
    {
        ahci = driver;
        dport = port;
        currentlba = 0;
        lbastart = drivelbastart;

        ahci->IdentityDrive(dport,&sectorsize,&lba);
    }

    size_t SataDevice::Read(size_t buffersize,void* outbuffer)
    {
        void* pages = PageAllocater::GetInstance()->RequestPages((buffersize + 511) / 512);

        ahci->Read(dport,lbastart + currentlba,(buffersize + 511) / 512,pages);

        memcpy(outbuffer,pages,buffersize);


        return buffersize;
    }

    void SataDevice::Seek(size_t offset)
    {
        currentlba = (offset + 511) / 512;
    }

    size_t SataDevice::Write(size_t buffersize,void* buffer)
    {
        (void)buffer;
        (void)buffersize;   
        return 0;
    }


}