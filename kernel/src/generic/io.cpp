#include <generic/io.h>

void outb(uint16_t port,uint8_t val)
{
    asm volatile ("outb %b0,%w1" : : "a"(val), "Nd"(port) : "memory");
}

uint64_t readmsr(uint32_t msr)
{
    uint32_t lo, hi;

    asm volatile (
        "rdmsr"
        : "=a"(lo), "=d"(hi)
        : "c"(msr)
    );

    return ((uint64_t)hi << 32) | lo;
}

void cpuid(uint32_t eaxrequest,
           uint32_t* o_eax,
           uint32_t* o_ebx,
           uint32_t* o_ecx,
           uint32_t* o_edx)
{
    uint32_t eax, ebx, ecx, edx;

    // Inline assembly for CPUID
    asm volatile(
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)   
        : "a"(eaxrequest)                               
    );

    // Store results in output pointers
    if (o_eax) *o_eax = eax;
    if (o_ebx) *o_ebx = ebx;
    if (o_ecx) *o_ecx = ecx;
    if (o_edx) *o_edx = edx;
}