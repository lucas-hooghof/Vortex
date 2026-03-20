#include <generic/stdio.h>

#include <generic/kernelInit.h>

extern void (*__init_array_start[])();
extern void (*__init_array_end[])();

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
        Logger::Log("Failed to initilize interrupts",LOG_LEVEL::ERROR);
        while(1) {}
    }
    Logger::Log("Interrupts Initilized\n",LOG_LEVEL::INFO);

    if (!PrepareHardware())
    {
        Logger::Log("Failed to initilize hardware",LOG_LEVEL::ERROR);
        while(1) {}
    }

    Logger::Log("Hardware Intilized",LOG_LEVEL::INFO);

    while (1) {}
}