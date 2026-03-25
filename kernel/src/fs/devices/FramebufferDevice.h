#pragma once

#include <generic/stdint.h>
#include <generic/bootinfo.h>
#include <fs/VFS.h>

#define DCTL_REQUEST_BUFFER_WIDTH 0x01
#define DCTL_REQUEST_BUFFER_HEIGHT 0x02

namespace fs
{
    class FramebufferDevice : public Device
    {
        public:
            FramebufferDevice(const char* addr,FRAMEBUFFER* framebuffer);

            virtual size_t Write(size_t buffersize,void* buffer) override;
            virtual size_t Read(size_t buffersize,void* outbuffer) override;
            virtual void Seek(size_t offset) override;
            virtual size_t dctl(size_t request) override
            {
                switch(request)
                {
                    case DCTL_REQUEST_BUFFER_WIDTH:
                        return framebuffer->PixelsPerScanLine;
                    case DCTL_REQUEST_BUFFER_HEIGHT:
                        return framebuffer->Height;
                    default:
                        return 0;
                }
            }
        private:
            FRAMEBUFFER* framebuffer;
            size_t ptr;
    };
}