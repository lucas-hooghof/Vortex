#include <generic/constructers.h>

#include <generic/bootinfo.h>
#include <generic/stdio.h>

extern "C" void kernel_main(bootinfo_t info)
{
    call_constructers();
    Initilize(info);
    while(1) {}
}