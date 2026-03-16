/* linker.ld - Kernel ELF linker script */
OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)          /* Entry point symbol from assembly */

SECTIONS
{
    
    . = 0xFFFFFFFF80000000;

    _KernelStart = .;
    .text : ALIGN(0x1000)
    {
        *(.text*)      
    } :text

    
    .rodata : ALIGN(0x1000)
    {
        *(.rodata*)     
    } :rodata

    
    .data : ALIGN(0x1000)
    {
        *(.data*)       
    } :data

    
    .bss : ALIGN(0x1000)
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } :data

    .init_array ALIGN(8) :
    {
        __init_array_start = .;
        KEEP(*(.init_array))
        KEEP(*(.init_array.*))
        __init_array_end = .;
    } :rodata

    .stack ALIGN(0x1000) :
    {
        __stack_bottom = .;       /* guard page — 1 page */
        . += 0x1000;
        __stack_start = .;      /* usable stack begins */
        . += 0x10000;             /* 64KiB */
        __stack_end = .;          /* RSP points here */
    } :stack

    _KernelEnd = .;

    /DISCARD/ : { *(.note*) *(.comment*) }
}


PHDRS
{
    text  PT_LOAD FLAGS(5);  /* Executable, readable */
    data  PT_LOAD FLAGS(6);  /* Writable, readable */
    rodata PT_LOAD FLAGS(4); /* Read-only, readable */
    stack PT_LOAD  FLAGS(6);
}