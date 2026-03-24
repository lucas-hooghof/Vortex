#pragma once

#include <generic/stdint.h>
#include <generic/bootinfo.h>
#include <fs/VFS.h>

namespace fs
{
    class FramebufferDevice : public Device
    {
        public:
            FramebufferDevice(const char* addr,FRAMEBUFFER* framebuffer);

            virtual size_t Write(size_t buffersize,void* buffer) override;
            virtual size_t Read(size_t buffersize,void* outbuffer) override;
            virtual void Seek(size_t offset) override;
        private:
            FRAMEBUFFER* framebuffer;
            size_t ptr;
    };
}