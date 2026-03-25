global open

open:
    mov rax,0x0
    mov rbx,rdi
    mov rcx,rsi

    int 0x80

    ret

global write

write:
    mov rax,0x01
    mov rbx,rdi
    mov rcx,rsi

    int 0x80

    ret