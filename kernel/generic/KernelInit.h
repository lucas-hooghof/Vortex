#pragma once

#include <generic/constructers.h>
#include <generic/stdio.h>

#include <memory/PageAllocater.h>
#include <memory/paging/PageTableManager.h>

bool InitilizeKernel(bootinfo_t bootinfo);