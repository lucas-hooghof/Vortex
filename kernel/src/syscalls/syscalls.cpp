#include <syscalls/syscalls.h>

#include <fs/VFS.h>
#include <generic/stdio.h>

ISRHandler handlers[256] =
{
    syscall_0_open,
    syscall_1_write
};

void syscall_handler(ISR_INTERRUPT_FRAME* frame)
{
    if (handlers[frame->rax] != nullptr)
    {
        handlers[frame->rax](frame);
    }

    else 
    {
        panic(frame,"Unknown syscall");
    }
}

void syscall_0_open(ISR_INTERRUPT_FRAME* frame)
{
    const char* addr = (const char*)frame->rbx;
    int flags = frame->rcx;

    fid_t fid = fs::VFS::Open(addr,flags);

    frame->rax = fid;
}
void syscall_1_write(ISR_INTERRUPT_FRAME* frame)
{
    fid_t fid = frame->rbx;
    size_t size = frame->rcx;
    void* mem = (void*)frame->rdx;

    fs::Device* device = fs::VFS::GetInterface(fid);

    if (device != nullptr)
    {
        Logger::Log("Got here ig\n",LOG_LEVEL::INFO);
        frame->rax = device->Write(size,mem);   
    }

    frame->rax = 0;
}