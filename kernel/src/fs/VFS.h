#pragma once

#include <generic/stdint.h>

namespace fs {
    class VirtualDevice
    {
    public:
        void Write8(uint8_t value,uint64_t offset);
        void Write16(uint16_t value,uint64_t offset);
        void Write32(uint32_t value,uint64_t offset);

        void* data;

        const char* address; // /proc/fb /dev/sda /dev/ida /dev/uda 
    };

    class VFS
    {
        public:
            static bool Initilize();
            static void Shutdown();

            static void RegisterVirtualDevice(VirtualDevice* device);
        private:
            static bool Initilized;

            static VirtualDevice** devices;
            static size_t count;
            static size_t Size;
    };
}