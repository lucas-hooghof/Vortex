#pragma once

#include <generic/stdint.h>

#define FD_READ 1
#define FD_WRITE 2
#define FD_FILE 4



typedef int fid_t;

namespace fs
{
    class Device
    {
    public:
        Device(const char* address,int flags)
            : address(address),allowedflags(flags)
        {}

        virtual size_t Write(size_t buffersize,void* buffer) {(void)buffersize;(void)buffer; return 0;}
        virtual size_t Read(size_t buffersize,void* outbuffer) {(void)buffersize; (void)outbuffer; return 0;};
        virtual void Seek(size_t offset) {(void)offset;}

        const char* address;
        const int allowedflags;
    private:
    };

    class VFS
    {
    public:
        static bool Initilize();
        static void Destroy();

        static bool RegisterVirtualDevice(Device* device);

        static fid_t Open(const char* address,int flags);
        static void Close(fid_t fid);

        static Device* GetInterface(fid_t fid) { return OpendDevices[fid]; }
    private:
        static Device** Devices;
        static Device** OpendDevices;
        static size_t devicearrayentrycount;
        static size_t devicearraynextindex;
        static fid_t nextfid;
        static fid_t OpendArraySize;
    };

}