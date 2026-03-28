#pragma once

#include <generic/stdint.h>

#include <interrupts/IDT.h>

struct ISR_INTERRUPT_FRAME
{
    uint16_t ds;

    uint64_t r15,r14,r13,r12,r11,r10,r9,r8;
    uint64_t rbp,rdi,rsi,rdx,rcx,rbx,rax;

    uint64_t interrupt,errorcode;

    uint64_t rip,cs,rflags,rsp,ss;
}__attribute__((packed));

typedef void (*ISRHandler)(ISR_INTERRUPT_FRAME* frame);

bool InitilizeISR();
void GenerateISR(uint8_t IntteruptNumber,uint16_t segment,uint8_t flags);

void RegisterHandler(uint8_t IntteruptNumber,ISRHandler handler);

void __attribute__((noreturn)) panic(ISR_INTERRUPT_FRAME* frame,const char* Name);
void __attribute__((noreturn)) SystemErrorInterrupt(ISR_INTERRUPT_FRAME* frame,const char* reason);
void isr_handler(ISR_INTERRUPT_FRAME* frame);
