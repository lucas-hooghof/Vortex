global open

open:
    mov rax,0x0

    int 0x80

    ret

global write

write:
    mov rax,0x01

    int 0x80
    jmp $

    ret