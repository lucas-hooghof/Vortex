#include <fs/VFS.h>

#include <memory/heap.h>
#include <generic/string.h>

namespace fs
{
    Device** VFS::Devices = nullptr;
    Device** VFS::OpendDevices = nullptr;
    size_t VFS::devicearrayentrycount = 4;
    size_t VFS::devicearraynextindex = 0;
    fid_t VFS::nextfid = 0;
    fid_t VFS::OpendArraySize = 0;

    bool init = false;

    bool VFS::Initilize()
    {
        if (init) return true;
        init = true;
        Devices = (Device**)malloc(sizeof(Device*) * devicearrayentrycount);

        if (!Devices)
        {
            return false;
        }

        for (size_t i = 0; i < devicearrayentrycount; i++)
        {
            Devices[i] = nullptr;
        }

        return true;
    }

    void VFS::Destroy()
    {
        free(Devices);
        Devices = nullptr;
    }

    bool VFS::RegisterVirtualDevice(Device* device)
    {
        for (size_t i = 0; i < devicearrayentrycount; i++)
        {
            if (Devices[i] == nullptr)
            {
                Devices[i] = device;
                return true;
            }
        }

        Devices = (Device**)realloc(Devices,(OpendArraySize+4) * sizeof(Device*));
        if (Devices == nullptr) return -1;
        for (size_t i = devicearrayentrycount; i < devicearrayentrycount + 4; i++)
        {
            Devices[i] = nullptr;
        }
        devicearrayentrycount += 4;

        for (size_t i = 0; i < devicearrayentrycount; i++)
        {
            if (Devices[i] == nullptr)
            {
                Devices[i] = device;
                return true;
            }
        }

        return false;
    }

    fid_t VFS::Open(const char* address,int flags)
    {
        for (size_t i = 0; i < devicearrayentrycount; i++)
        {
            if (Devices[i] == nullptr) break;
            if ((Devices[i]->allowedflags & flags) != flags) break;
            if (!memcmp((void*)Devices[i]->address,address,strlen(address)))
            {

                if ((nextfid + 1) > OpendArraySize)
                {
                    OpendDevices = (Device**)realloc(OpendDevices,(OpendArraySize+4) * sizeof(Device*));
                    if (OpendDevices == nullptr) return -1;
                    for (fid_t fid = OpendArraySize; fid < OpendArraySize + 4; fid++)
                    {
                        OpendDevices[fid] = nullptr;
                    }
                    OpendArraySize += 4;
                }
                OpendDevices[nextfid++] = Devices[i];
                if (OpendDevices[nextfid] != nullptr)
                {
                    fid_t temp = nextfid;
                    for (fid_t fid = nextfid; fid < OpendArraySize; fid++)
                    {
                        if (OpendDevices[fid] == nullptr)
                        {
                            nextfid = fid;
                        }
                    }
                    
                    if (temp == nextfid)
                    {
                        nextfid = OpendArraySize - 1;
                    }
                }

                return nextfid-1;
            }
        }

        return -1;
    }
} 
