#include <fs/devices/FramebufferDevice.h>

#include <generic/string.h>

namespace fs
{
    FramebufferDevice::FramebufferDevice(const char* addr,FRAMEBUFFER* fb)
        :Device(addr,FD_READ | FD_WRITE)
    {
        framebuffer = fb;
        ptr = 0;
    }

    size_t FramebufferDevice::Write(size_t buffersize, void* buffer)
    {
        if (!(allowedflags & FD_WRITE))
            return 0;

        size_t ptrstart = ptr;

        if (buffersize == 0)
            return 0;

        if ((buffersize % 4) != 0)
            return 0;

        if (ptr >= framebuffer->BufferSize)
            return 0;

        size_t maxWrite = framebuffer->BufferSize - ptr;
        if (buffersize > maxWrite)
            buffersize = maxWrite;

        memcpy((void*)((uint64_t)framebuffer->BaseAddress + ptr), buffer, buffersize);

        ptr += buffersize;

        return ptr - ptrstart;
    }

    size_t FramebufferDevice::Read(size_t buffersize, void* outbuffer)
    {
        if (!(allowedflags & FD_READ))
            return 0;

        if (ptr >= framebuffer->BufferSize)
            return 0;

        size_t maxRead = framebuffer->BufferSize - ptr;
        if (buffersize > maxRead)
            buffersize = maxRead;

        memcpy(outbuffer, (void*)((uint64_t)framebuffer->BaseAddress + ptr), buffersize);

        ptr += buffersize;

        return buffersize;
    }

    void FramebufferDevice::Seek(size_t offset)
    {
        // Clamp to framebuffer size
        if (offset > framebuffer->BufferSize)
            offset = framebuffer->BufferSize;

        ptr = offset;
    }
}