#pragma once

#include <generic/stdint.h>

#include <interrupts/IDT.h>

struct ISR_INTERRUPT_FRAME
{

}__attribute__((packed));

bool InitilizeISR();
void GenerateISR(uint8_t IntteruptNumber);

void isr_handler(ISR_INTERRUPT_FRAME* frame);