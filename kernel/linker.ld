ENTRY(kernel_main)
OUTPUT_FORMAT(elf64-x86-64)

PHDRS
{
    text PT_LOAD FLAGS(5);  /* R + X */
    data PT_LOAD FLAGS(6);  /* R + W */
    rodata PT_LOAD FLAGS(4);
}



SECTIONS
{
    /* Start at fixed address 0x100000 */
    . = 0x100000;

    .text : 
    {
        *(.text*)
    } :text

    .rodata :
    {
        *(.rodata*)
    } :rodata

    .init_array :
    {
        __init_array_start = .;
        KEEP(*(.init_array*))
        __init_array_end = .;
    } :data

    .fini_array :
    {
        __fini_array_start = .;
        KEEP(*(.fini_array*))
        __fini_array_end = .;
    } :data

    .data : 
    {
        *(.data*)
    } :data

    .bss : 
    {
        *(.bss*)
        *(COMMON)
    } :data
}