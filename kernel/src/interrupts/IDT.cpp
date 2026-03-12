#include <interrupts/IDT.h>

IDTGate IDT[IDT_ENTRY_COUNT];

void idt_set_descriptor(uint8_t interrupt,uint16_t segment,uint8_t Attributes,uint8_t IST,void* ISR)
{
    IDTGate& gate = IDT[interrupt];

    IDT->SegmentSelector = segment;
    IDT->IST = (IST & 0b00000111);
    IDT->Attributes = Attributes;
    
    uint64_t IsrAddress = (uint64_t)ISR;

    IDT->Offset0 = (IsrAddress & 0xFFFF);
    IDT->Offset1 = (IsrAddress >> 16) & 0xFFFF;
    IDT->Offset2 = (IsrAddress >> 32) & 0xFFFFFFFF;
    IDT->Reserved = 0;
}