#include <generic/constructers.h>

#include <generic/bootinfo.h>
#include <generic/stdio.h>

#include <memory/PageAllocater.h>
#include <memory/paging/PageTableManager.h>

#include <generic/string.h>

#include <generic/KernelInit.h>

extern "C" void kernel_main(bootinfo_t info)
{

    InitilizeKernel(info);

    while(1) {}
}