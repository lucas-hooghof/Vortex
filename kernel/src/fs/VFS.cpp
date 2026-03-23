#include <fs/VFS.h>
#include <memory/heap.h>

namespace fs {

    bool           VFS::Initilized = false;
    VirtualDevice** VFS::devices   = nullptr;
    size_t         VFS::count      = 0;
    size_t         VFS::Size       = 4;

    bool VFS::StrEqual(const char* a, const char* b) {
        if (a == nullptr || b == nullptr) return false;
        while (*a && *b) {
            if (*a != *b) return false;
            a++; b++;
        }
        return *a == *b;
    }


    bool VFS::Initilize() {
        if (Initilized) return true;
        devices = (VirtualDevice**)malloc(Size * sizeof(VirtualDevice*));
        if (devices == nullptr) return false;
        Initilized = true;
        return true;
    }

    void VFS::Shutdown() {
        free(devices);
        devices    = nullptr;
        count      = 0;
        Size       = 4;
        Initilized = false;
    }

    void VFS::RegisterVirtualDevice(VirtualDevice* device) {
        if (device == nullptr) return;

        if (count + 1 >= Size) {
            devices = (VirtualDevice**)realloc(devices, (Size + 4) * sizeof(VirtualDevice*));
            Size += 4;
        }
        devices[count++] = device;
    }

    void VFS::UnregisterVirtualDevice(VirtualDevice* device) {
        if (device == nullptr) return;

        for (size_t i = 0; i < count; i++) {
            if (devices[i] == device) {
                // shift everything left over the removed slot
                for (size_t j = i; j < count - 1; j++)
                    devices[j] = devices[j + 1];
                devices[--count] = nullptr;
                return;
            }
        }
    }


    VirtualDevice* VFS::GetDevice(const char* address) {
        if (address == nullptr) return nullptr;

        for (size_t i = 0; i < count; i++)
            if (StrEqual(devices[i]->address, address))
                return devices[i];

        return nullptr;
    }

    VirtualDevice* VFS::GetDeviceByName(const char* name) {
        if (name == nullptr) return nullptr;

        for (size_t i = 0; i < count; i++)
            if (StrEqual(devices[i]->name, name))
                return devices[i];

        return nullptr;
    }

    bool VFS::DeviceExists(const char* address) {
        return GetDevice(address) != nullptr;
    }

    size_t VFS::GetDeviceCount() {
        return count;
    }

} 