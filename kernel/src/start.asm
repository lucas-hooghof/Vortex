global _start

extern kernel_main

extern __bss_start
extern __bss_end

extern __stack_end

_start:
    mov rsp,__stack_end
    mov rbp,rsp


    ;BSS clear
    push rdi
    mov rax,__bss_end
    sub rax, __bss_start        ; rcx = size
    mov rcx,rax
    xor rax,rax
    mov rdi,__bss_start
    rep stosb
    pop rdi

    call kernel_main

    cli
    hlt