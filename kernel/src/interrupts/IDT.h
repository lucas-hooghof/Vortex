#pragma once

#include <generic/stdint.h>

#define RING0_INTERRUPT_GATE 0x8E
#define RING0_TRAP_GATE     0x8F
#define RING3_INTERRUPT_GATE 0xEF
#define RING3_TRAP_GATE     0xEE

#define IDT_ENTRY_COUNT 256

struct IDT_R
{
    uint16_t Size;
    uint64_t Offset;
}__attribute__((packed));

struct IDTGate
{
    uint16_t Offset0;
    uint16_t SegmentSelector;
    uint8_t  IST;
    uint8_t Attributes;
    uint16_t Offset1;
    uint32_t Offset2;
    uint32_t Reserved;
}__attribute__((packed));

extern IDTGate IDT[IDT_ENTRY_COUNT];

void idt_set_descriptor(uint8_t interrupt,uint16_t segment,uint8_t Attributes,uint8_t IST,void* ISR);
