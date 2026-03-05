#pragma once

#include <generic/stdint.h>
#include <generic/bootinfo.h>

#include <generic/io.h>

#include <generic/stdarg.h>


void Initilize(bootinfo_t bootinfo);

void putc(char c);

void puts(const char* str);

void printf(const char* fmt,...);