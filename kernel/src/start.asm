global _start

extern __bss_start
extern __bss_end
extern __stack_end

extern kernel_main

_start:
    push rdi
    xor rax, rax
    mov rdi, __bss_start
    mov rcx, __bss_end          ; load end address
    sub rcx, rdi                ; rcx = __bss_end - __bss_start
    rep stosb
    pop rdi

    mov rsp, __stack_end
    mov rbp, __stack_end

    call kernel_main
    ret
