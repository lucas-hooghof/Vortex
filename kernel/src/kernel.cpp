#include <generic/stdio.h>

#include <generic/kernelInit.h>
#include <interrupts/ISR.h>

#include <interrupts/APIC/LAPIC.h>

extern void (*__init_array_start[])();
extern void (*__init_array_end[])();

extern "C" uint8_t __stack_bottom[];
extern "C" uint8_t __stack_start__[];
extern "C" uint8_t __stack_end[];

void call_constructors()
{
    for (size_t i = 0; &__init_array_start[i] < __init_array_end; i++)
        __init_array_start[i]();
}



extern "C" void __attribute__((noreturn)) kernel_main(bootinfo_t* info)
{
    call_constructors();
    Logger::Initilize(info);

    if (!PrepareMemory(info))
    {
        Logger::Log("Failed to initilize memory",LOG_LEVEL::ERROR);
        while(1) {}
    }
    Logger::Log("Memory Management Initilized\n", LOG_LEVEL::INFO);

    if (!PrepareInterrupts())
    {
        Logger::Log("Failed to initilize Interrupts",LOG_LEVEL::ERROR);
        while (1) {}
    }
    Logger::Log("Interrupts Initilized\n",LOG_LEVEL::INFO);
    LAPIC lapic;
    lapic.Initilize();
    lapic.InitilizeLAPICTimer();
    while (1) {}
}