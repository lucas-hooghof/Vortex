#pragma once

#include <generic/stdint.h>
#include <generic/io.h>

#define MSR_LAPIC_ADDRESS_BASE 0x1B

class LAPIC
{
public:
    LAPIC() = default;

    bool Initilize();
private:
    uint64_t LapicAddress;
};