#include <interrupts/APIC/LAPIC.h>

#include <generic/stdio.h>

bool LAPIC::Initilize()
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);

    if (edx & CPUID_FEAT_EDX_APIC)
        Logger::Log("Local APIC present\n",LOG_LEVEL::INFO);

    if (ecx & CPUID_FEAT_ECX_X2APIC)
        Logger::Log("x2APIC supported\n",LOG_LEVEL::INFO);

    return true;
}