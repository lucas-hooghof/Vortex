#include <interrupts/APIC/LAPIC.h>

#include <generic/stdio.h>
#include <generic/io.h>
#include <memory/PageTableManager.h>

#define PIC1		0x20		
#define PIC2		0xA0		
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)


#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

#define CASCADE_IRQ 2


bool LAPIC::Initilize()
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);

    if (edx & CPUID_FEAT_EDX_APIC)
    {
        Logger::Log("Local APIC present\n",LOG_LEVEL::INFO);
        xAPIC = true;
        InitiilizexAPIC();
    }
    else if (ecx & CPUID_FEAT_ECX_X2APIC)
    {
        xAPIC = false;
        Logger::Log("x2APIC supported\n",LOG_LEVEL::INFO);
        Initilizex2APIC();
    }
    return true;
}

bool LAPIC::InitilizeLAPICTimer()
{
    RegisterHandler(0x30,&LAPIC::lapic_timer_intterupt);
    lapic_write(0x280, 0);
    lapic_write(0xF0, 0x100 | 0xFF); // enable LAPIC
    lapic_write(0x3E0, 0x3);         // divide by 16
    lapic_write(0x320, 0x30);        // one-shot vector 32

    uint32_t eax, ebx, ecx, edx;
    cpuid(0x15, &eax, &ebx, &ecx, &edx);

    uint64_t tsc_freq = (ecx * ebx) / eax;

    uint64_t wait_cycles = tsc_freq / 100; // 10ms

    uint64_t tsc_start = rdtsc();

    lapic_write(0x380, 0xFFFFFFFF);

    while (rdtsc() - tsc_start < wait_cycles)
    {
    }

    uint32_t current = lapic_read(0x390);

    uint32_t lapic_ticks = 0xFFFFFFFF - current;

    uint32_t ticks_per_ms = lapic_ticks / 10;

    uint32_t initial = ticks_per_ms * 10;

    // periodic timer
    lapic_write(0x320, 0x20000 | 0x20);

    lapic_write(0x380, initial);
    return true;
}

void LAPIC::DisablePIC()
{
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, 0x20);                 // ICW2: Master PIC vector offset
	io_wait();
	outb(PIC2_DATA, 0x28);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(PIC1_DATA, 1 << CASCADE_IRQ);        // ICW3: tell Master PIC that there is a slave PIC at IRQ2
	io_wait();
	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
	
	outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	// Unmask both PICs.
	outb(PIC1_DATA, 0xFF);
	outb(PIC2_DATA, 0xFF);
}

void LAPIC::InitiilizexAPIC()
{
    DisablePIC();
    uint64_t ApicBase = readmsr(MSR_LAPIC_ADDRESS_BASE);

    LapicAddress = ApicBase & 0xFFFFF000;
    Logger::DebugLog("Lapic Base: %x\n",LOG_LEVEL::INFO,LapicAddress);

    ApicBase |= (1 << 11);
    PageTableManager::GetInstance()->MapMemory((void*)0XFEE00000,(void*)0XFEE00000);


    writemsr(MSR_LAPIC_ADDRESS_BASE,ApicBase);

}
void LAPIC::Initilizex2APIC()
{
    DisablePIC();
}

void LAPIC::lapic_write(uint16_t lapicreg,uint32_t value)
{
    *(uint32_t*)(LapicAddress + lapicreg) = value;
}

uint32_t LAPIC::lapic_read(uint16_t lapicreg)
{   
    return *(uint32_t*)(LapicAddress + lapicreg);
}

void LAPIC::lapic_timer_intterupt(ISR_INTERRUPT_FRAME* frame)
{
    (void)frame;
    Logger::DebugLog("Tick\n",LOG_LEVEL::INFO);
}