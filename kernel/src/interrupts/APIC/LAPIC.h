#pragma once

#include <generic/stdint.h>
#include <generic/io.h>

#include <interrupts/ISR.h>

#define MSR_LAPIC_ADDRESS_BASE 0x1B

class LAPIC
{
public:
    LAPIC() = default;

    bool Initilize();

    bool InitilizeLAPICTimer();

    void lapic_write(uint16_t lapicreg,uint32_t value);
    uint32_t lapic_read(uint16_t lapicreg);

    static void lapic_timer_intterupt(ISR_INTERRUPT_FRAME* frame);
private:
    uint64_t LapicAddress;

    bool xAPIC;

    void InitiilizexAPIC();
    void Initilizex2APIC();
    void DisablePIC();
};