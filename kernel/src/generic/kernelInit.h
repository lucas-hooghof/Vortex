#pragma once

#include <generic/bootinfo.h>

extern uint64_t HHDM;

bool PrepareMemory(bootinfo_t* info);
bool PrepareInterrupts();