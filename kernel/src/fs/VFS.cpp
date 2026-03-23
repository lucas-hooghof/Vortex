#include <fs/VFS.h>

#include <memory/heap.h>

namespace fs {
    bool VFS::Initilized = false;
    VirtualDevice** VFS::devices = nullptr;
    size_t VFS::count = 0;
    size_t VFS::Size = 4;

    bool VFS::Initilize()
    {
        if (Initilized) return true;
        devices = (VirtualDevice**)malloc(Size * sizeof(VirtualDevice*));
        if (devices == nullptr)
        {
            return false;
        }
    }

    void VFS::Shutdown()
    {
        free(devices);
        devices = nullptr;
    }

    void VFS::RegisterVirtualDevice(VirtualDevice* device)
    {
        if (count + 1 >= Size) { 
            devices = (VirtualDevice**)realloc(devices,(Size + 4) * sizeof(VirtualDevice*));
            Size += 4;
        }
        devices[count++] = device;
    }
}