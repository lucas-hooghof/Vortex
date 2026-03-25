#pragma once

#include <interrupts/ISR.h>

void syscall_handler(ISR_INTERRUPT_FRAME* frame);

void syscall_0_open(ISR_INTERRUPT_FRAME* frame);
void syscall_1_write(ISR_INTERRUPT_FRAME* frame);