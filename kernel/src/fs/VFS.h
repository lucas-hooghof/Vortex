#pragma once
#include <generic/stdint.h>

namespace fs {

    class VirtualDevice {
    public:
        const char* address;
        const char* name;
        uint64_t    size;
        uint32_t    flags;

        virtual bool Open (uint32_t flags) { (void)flags; return true; }
        virtual void Close() {}
        virtual void Flush() {}
        virtual int  IOCtl(uint32_t request, void* arg) { (void)request; (void)arg; return -1; }

        virtual uint8_t  Read8 (uint64_t offset) {  (void)offset; return 0; }
        virtual uint16_t Read16(uint64_t offset) {  (void)offset; return 0; }
        virtual uint32_t Read32(uint64_t offset) {  (void)offset; return 0; }
        virtual uint64_t Read64(uint64_t offset) {  (void)offset; return 0; }

        virtual void Write8 (uint8_t  value, uint64_t offset) { (void)value; (void)offset;}
        virtual void Write16(uint16_t value, uint64_t offset) { (void)value; (void)offset;}
        virtual void Write32(uint32_t value, uint64_t offset) { (void)value; (void)offset;}
        virtual void Write64(uint64_t value, uint64_t offset) { (void)value; (void)offset;}

        void* data;
    };

    class VFS {
    public:
        static bool           Initilize();
        static void           Shutdown();
        static void           RegisterVirtualDevice(VirtualDevice* device);
        static void           UnregisterVirtualDevice(VirtualDevice* device);
        static VirtualDevice* GetDevice(const char* address);       // lookup by address
        static VirtualDevice* GetDeviceByName(const char* name);    // lookup by name
        static bool           DeviceExists(const char* address);
        static size_t         GetDeviceCount();

    private:
        static bool            Initilized;
        static VirtualDevice** devices;
        static size_t          count;
        static size_t          Size;

        static bool StrEqual(const char* a, const char* b);
    };

} 