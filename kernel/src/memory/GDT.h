#pragma once

#include <generic/stdint.h>

#define BIT(x) ( 1 << x)

struct GDTR
{
    uint16_t Size;
    uint64_t Offset;
}__attribute__((packed));

struct GDT_SEGMENT_DESCRIPTOR
{
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t AccessByte;
    uint8_t FlagsAndLimit;
    uint8_t base2;
}__attribute__((packed));



struct GDT
{
    GDT_SEGMENT_DESCRIPTOR kernelnull;
    GDT_SEGMENT_DESCRIPTOR kernelcode;
    GDT_SEGMENT_DESCRIPTOR kerneldata;
    GDT_SEGMENT_DESCRIPTOR usernull;
    GDT_SEGMENT_DESCRIPTOR usercode;
    GDT_SEGMENT_DESCRIPTOR userdata;
}__attribute__((packed))__attribute__((aligned(0x1000)));


extern GDT g_GDT;

extern "C" void LoadGDT(GDTR* gdtr);