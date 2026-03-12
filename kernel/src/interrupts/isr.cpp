#include <interrupts/ISR.h>
#include <memory/PageAllocater.h>

#define ISR_CODE_NO_ERROR_INTERRUPT_NUMBER      6
#define ISR_CODE_NO_ERROR_CALL_HANDLER_OFFSET   54
#define ISR_CODE_ERROR_INTERRUPT_NUMBER         1
#define ISR_CODE_ERROR_CALL_HANDLER_OFFSET      49

#define ISR_WITH_ERROR 10
#define ISR_WITHOUT_ERROR IDT_ENTRY_COUNT - ISR_WITH_ERROR


static uint8_t isr_code_no_err[] = {
    // push dummy error code
    0x68, 0x00,0x00,0x00,0x00, // push 0
    0x6A, 0x01,                // push 1

    // push general-purpose registers
    0x50,                   // push rax
    0x53,                   // push rbx
    0x51,                   // push rcx
    0x52,                   // push rdx
    0x56,                   // push rsi
    0x57,                   // push rdi
    0x55,                   // push rbp
    0x41, 0x50,             // push r8
    0x41, 0x51,             // push r9
    0x41, 0x52,             // push r10
    0x41, 0x53,             // push r11
    0x41, 0x54,             // push r12
    0x41, 0x55,             // push r13
    0x41, 0x56,             // push r14
    0x41, 0x57,             // push r15

    // save segment registers
    0x66, 0x8C, 0xD8,       // mov ax, ds
    0x66, 0x50,             // push ax

    // load kernel data segments
    0x66, 0xB8, 0x10, 0x00, // mov ax, 0x10
    0x8E, 0xD8,             // mov ds, ax
    0x8E, 0xC0,             // mov es, ax
    0x8E, 0xE0,             // mov fs, ax
    0x8E, 0xE8,             // mov gs, ax
    0x8E, 0xD0,             // mov ss, ax

    // first argument = pointer to saved registers
    0x48, 0x89, 0xE7,       // mov rdi, rsp

    // ---- call absolute handler ----
    0x48, 0xB8,             // mov rax, <handler 64-bit address>
    // placeholder for 8-byte address, little-endian
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00,
    0xFF, 0xD0,             // call rax

    // restore segment registers
    0x66, 0x58,             // pop ax
    0x8E, 0xD8,             // mov ds, ax
    0x8E, 0xC0,             // mov es, ax
    0x8E, 0xE0,             // mov fs, ax
    0x8E, 0xE8,             // mov gs, ax

    // pop general-purpose registers
    0x41, 0x5F,             // pop r15
    0x41, 0x5E,             // pop r14
    0x41, 0x5D,             // pop r13
    0x41, 0x5C,             // pop r12
    0x41, 0x5B,             // pop r11
    0x41, 0x5A,             // pop r10
    0x41, 0x59,             // pop r9
    0x41, 0x58,             // pop r8
    0x5D,                   // pop rbp
    0x5F,                   // pop rdi
    0x5E,                   // pop rsi
    0x5A,                   // pop rdx
    0x59,                   // pop rcx
    0x5B,                   // pop rbx
    0x58,                   // pop rax

    0x48, 0x83, 0xC4, 0x08, // add rsp, 8 (remove pushed error code)

    0x48, 0xCF              // iretq
};

static uint8_t isr_code_err[] = {
    // push dummy error code
    0x6A, 0x01,                // push 1

    // push general-purpose registers
    0x50,                   // push rax
    0x53,                   // push rbx
    0x51,                   // push rcx
    0x52,                   // push rdx
    0x56,                   // push rsi
    0x57,                   // push rdi
    0x55,                   // push rbp
    0x41, 0x50,             // push r8
    0x41, 0x51,             // push r9
    0x41, 0x52,             // push r10
    0x41, 0x53,             // push r11
    0x41, 0x54,             // push r12
    0x41, 0x55,             // push r13
    0x41, 0x56,             // push r14
    0x41, 0x57,             // push r15

    // save segment registers
    0x66, 0x8C, 0xD8,       // mov ax, ds
    0x66, 0x50,             // push ax

    // load kernel data segments
    0x66, 0xB8, 0x10, 0x00, // mov ax, 0x10
    0x8E, 0xD8,             // mov ds, ax
    0x8E, 0xC0,             // mov es, ax
    0x8E, 0xE0,             // mov fs, ax
    0x8E, 0xE8,             // mov gs, ax
    0x8E, 0xD0,             // mov ss, ax

    // first argument = pointer to saved registers
    0x48, 0x89, 0xE7,       // mov rdi, rsp

    // ---- call absolute handler ----
    0x48, 0xB8,             // mov rax, <handler 64-bit address>
    // placeholder for 8-byte address, little-endian
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00,
    0xFF, 0xD0,             // call rax

    // restore segment registers
    0x66, 0x58,             // pop ax
    0x8E, 0xD8,             // mov ds, ax
    0x8E, 0xC0,             // mov es, ax
    0x8E, 0xE0,             // mov fs, ax
    0x8E, 0xE8,             // mov gs, ax

    // pop general-purpose registers
    0x41, 0x5F,             // pop r15
    0x41, 0x5E,             // pop r14
    0x41, 0x5D,             // pop r13
    0x41, 0x5C,             // pop r12
    0x41, 0x5B,             // pop r11
    0x41, 0x5A,             // pop r10
    0x41, 0x59,             // pop r9
    0x41, 0x58,             // pop r8
    0x5D,                   // pop rbp
    0x5F,                   // pop rdi
    0x5E,                   // pop rsi
    0x5A,                   // pop rdx
    0x59,                   // pop rcx
    0x5B,                   // pop rbx
    0x58,                   // pop rax

    0x48, 0x83, 0xC4, 0x08, // add rsp, 8 (remove pushed error code)

    0x48, 0xCF              // iretq
};

uint8_t* ISRTable = nullptr;

bool InitilizeISR()
{
    constexpr size_t ISRTableSize = (ISR_WITH_ERROR * sizeof(isr_code_err)) + (ISR_WITHOUT_ERROR * sizeof(isr_code_no_err));
    ISRTable = (uint8_t*)PageAllocater::GetInstance()->RequestPages((ISRTableSize + 4095) / 4096);
}

void GenerateISR(uint8_t interrupt)
{

}