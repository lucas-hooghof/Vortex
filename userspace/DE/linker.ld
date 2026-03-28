OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS
{
    
    . = 0xFFFF8000;

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



    /DISCARD/ : { *(.note*) *(.comment*) }
}


PHDRS
{
    text  PT_LOAD FLAGS(5);  /* Executable, readable */
    data  PT_LOAD FLAGS(6);  /* Writable, readable */
    rodata PT_LOAD FLAGS(4); /* Read-only, readable */
    stack PT_LOAD  FLAGS(6);
}