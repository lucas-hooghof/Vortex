#pragma once

#include <generic/stdint.hpp>
#include <generic/bootinfo.hpp>

#include <generic/io.hpp>

#include <generic/stdarg.hpp>


void Initilize(bootinfo_t bootinfo);

void putc(char c);

void puts(const char* str);

void printf(const char* fmt,...);