#include <generic/bootinfo.h>

#include <generic/stdio.h>
#include <memory/PageAllocater.h>

extern void (*__init_array_start[])();
extern void (*__init_array_end[])();

void call_constructors()
{
    for (size_t i = 0; &__init_array_start[i] < __init_array_end; i++)
        __init_array_start[i]();
}

extern "C" void __attribute__((noreturn)) kernel_main(bootinfo_t* info)
{
    call_constructors();

    Logger::Initilize(info);
    PageAllocater PA;
    PA.Initilize(info);

    for (uint64_t fbpage = 0; fbpage < info->framebuffer->BufferSize / 4096 + 1; fbpage++)
    {
        PA.LockPage((void*)((uint64_t)info->framebuffer->BaseAddress + fbpage * 4096));
    }

    while(1){}
}