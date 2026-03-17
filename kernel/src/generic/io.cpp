#include <generic/io.h>

void outb(uint16_t port,uint8_t val)
{
    asm volatile ("outb %b0,%w1" : : "a"(val), "Nd"(port) : "memory");
}

void outl(uint16_t port, uint32_t value)
{
    asm volatile ("outl %0, %1"
                  :
                  : "a"(value), "Nd"(port)
                  : "memory");
}

void io_wait()
{
    asm volatile ("outb %%al, $0x80" : : "a"(0));
}

uint32_t inl(uint16_t port)
{
    uint32_t val;
    asm volatile ("inl %1, %0"
                  : "=a"(val)
                  : "Nd"(port));
    return val;
}


uint64_t rdtsc()
{
    uint32_t lo,hi;

    asm volatile (
        "rdtsc"
        : "=a"(lo), "=d"(hi)
    );

    return ((uint64_t)hi << 32) | lo;
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

void writemsr(uint32_t msr,uint64_t val)
{
    uint32_t low  = (uint32_t)val;
    uint32_t high = (uint32_t)(val >> 32);
    asm volatile (
        "wrmsr"
        :
        : "c"(msr), "a"(low), "d"(high)
    );
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