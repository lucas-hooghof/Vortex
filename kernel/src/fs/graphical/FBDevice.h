#pragma once

#include <generic/bootinfo.h>
#include <generic/stdint.h>

#include <fs/VFS.h>

namespace fs
{
    class FBDevice : public VirtualDevice {
    public:
        FBDevice(FRAMEBUFFER* fb,const char* addr) {
            address   = addr;
            name      = "framebuffer0";
            size      = fb->BufferSize;
            flags     = 0;
            data      = fb->BaseAddress;
            _fb       = fb;
        }

        // Pixel read/write using X,Y coords
        uint32_t ReadPixel(uint32_t x, uint32_t y) {
            if (x >= _fb->Width || y >= _fb->Height) return 0;
            uint32_t* buf = (uint32_t*)_fb->BaseAddress;
            return buf[y * _fb->PixelsPerScanLine + x];
        }

        void WritePixel(uint32_t x, uint32_t y, uint32_t color) {
            if (x >= _fb->Width || y >= _fb->Height) return;
            uint32_t* buf = (uint32_t*)_fb->BaseAddress;
            buf[y * _fb->PixelsPerScanLine + x] = color;
        }

        // Fill a rectangle
        void FillRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
            for (uint32_t row = y; row < y + h; row++)
                for (uint32_t col = x; col < x + w; col++)
                    WritePixel(col, row, color);
        }

        // Clear the entire screen
        void Clear(uint32_t color = 0x00000000) {
            uint32_t* buf = (uint32_t*)_fb->BaseAddress;
            for (uint64_t i = 0; i < _fb->PixelsPerScanLine * _fb->Height; i++)
                buf[i] = color;
        }

        // IOCtl for querying framebuffer info
        int IOCtl(uint32_t request, void* arg) override {
            switch (request) {
                case 0x01: // GET_WIDTH
                    *(uint32_t*)arg = _fb->Width;       return 0;
                case 0x02: // GET_HEIGHT
                    *(uint32_t*)arg = _fb->Height;      return 0;
                case 0x03: // GET_STRIDE
                    *(uint32_t*)arg = _fb->PixelsPerScanLine; return 0;
                case 0x04: // GET_SIZE
                    *(uint64_t*)arg = _fb->BufferSize;  return 0;
                default:   return -1;
            }
        }

        // Raw byte access (inherited from VirtualDevice)
        uint8_t  Read8 (uint64_t offset) override { return ((uint8_t*) _fb->BaseAddress)[offset]; }
        uint16_t Read16(uint64_t offset) override { return ((uint16_t*)_fb->BaseAddress)[offset]; }
        uint32_t Read32(uint64_t offset) override { return ((uint32_t*)_fb->BaseAddress)[offset]; }

        void Write8 (uint8_t  v, uint64_t offset) override { ((uint8_t*) _fb->BaseAddress)[offset] = v; }
        void Write16(uint16_t v, uint64_t offset) override { ((uint16_t*)_fb->BaseAddress)[offset] = v; }
        void Write32(uint32_t v, uint64_t offset) override { ((uint32_t*)_fb->BaseAddress)[offset] = v; }

        // Getters
        uint32_t GetWidth()  const { return _fb->Width;  }
        uint32_t GetHeight() const { return _fb->Height; }
        uint32_t GetStride() const { return _fb->PixelsPerScanLine; }
        FRAMEBUFFER* GetFB() const { return _fb; }

    private:
        FRAMEBUFFER* _fb;
    };
}