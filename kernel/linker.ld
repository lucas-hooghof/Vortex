/* linker.ld - Kernel ELF linker script */

ENTRY(_start)          /* Entry point symbol from assembly */

SECTIONS
{

    . = 0x200000;
    .text : ALIGN(0x1000)
    {
        *(.text*)    
        KEEP(*(.ctors))          
        KEEP(*(.init_array))   
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

    .stack ALIGN(16) : {
        __stack_start__ = .;
        . += 0x10000;  /* 64 KiB stack */
        __stack_end = .;
    }: stack

    /DISCARD/ : { *(.note*) *(.comment*) }
}


PHDRS
{
    text  PT_LOAD FLAGS(5);  /* Executable, readable */
    data  PT_LOAD FLAGS(6);  /* Writable, readable */
    rodata PT_LOAD FLAGS(4); /* Read-only, readable */
    stack PT_NULL  FLAGS(6);
}