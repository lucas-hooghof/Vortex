#include <generic/bootinfo.h>

#include <generic/stdio.h>

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

    while(1){}
}