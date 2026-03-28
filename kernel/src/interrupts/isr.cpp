#include <interrupts/ISR.h>
#include <memory/PageAllocater.h>

#define ISR_CODE_NO_ERROR_INTERRUPT_NUMBER      6
#define ISR_CODE_NO_ERROR_CALL_HANDLER_OFFSET   54
#define ISR_CODE_ERROR_INTERRUPT_NUMBER         1
#define ISR_CODE_ERROR_CALL_HANDLER_OFFSET      49

#define ISR_WITH_ERROR 10
#define ISR_WITHOUT_ERROR IDT_ENTRY_COUNT - ISR_WITH_ERROR

#include <generic/stdio.h>
#include <generic/string.h>


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

    0x48, 0x83, 0xC4, 0x05, // add rsp, 5 (remove pushed error code)

    0x48, 0xCF              // iretq
};

static uint8_t isr_code_err[] = {
    // cpu push error code
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

    0x48, 0x83, 0xC4, 0x16, // add rsp, 8 (remove pushed error code)

    0x48, 0xCF              // iretq
};

constexpr size_t ISRTableSize = (10 * sizeof(isr_code_err)) + (246 * sizeof(isr_code_no_err));
uint8_t ISRTable[ISRTableSize];

bool InitilizeISR()
{
    for (size_t i = 0; i < IDT_ENTRY_COUNT; i++)
    {
        GenerateISR(i);
    }
    return true;
}

static uint8_t* ISRWritePtr = ISRTable;
static bool ISRAdded[IDT_ENTRY_COUNT] = {0};
static ISRHandler Handles[IDT_ENTRY_COUNT] = {nullptr};

static bool HasErrorCode(uint8_t interrupt)
{
    switch(interrupt)
    {
        case 0x8:
        case 0xA:
        case 0xB:
        case 0xC:
        case 0xD:
        case 0xE:
        case 0x11:
        case 0x15:
        case 0x1D:
        case 0x1E:
            return true;
        default:
            return false;
    }
}

void __attribute__((noreturn)) panic(ISR_INTERRUPT_FRAME* frame, const char* Name)
{
    asm volatile("cli");
    Logger::ClearScreen(0xFFFFFFFF,0xFF0000FF);

    Logger::Log("========== KERNEL PANIC ==========\n", LOG_LEVEL::ERROR);
    Logger::Log("Exception: %s\n", LOG_LEVEL::ERROR, Name);

    Logger::Log("Interrupt: %u  ErrorCode: %u\n",
                LOG_LEVEL::ERROR,
                frame->interrupt, frame->errorcode);

    Logger::Log("RIP=%x  CS=%x  RFLAGS=%x RSP=%x SS=%x\n",
                LOG_LEVEL::ERROR,
                frame->rip, frame->cs, frame->rflags,frame->rsp, frame->ss);


    Logger::Log("RAX=%x RBX=%x RCX=%x RDX=%x\n",
                LOG_LEVEL::ERROR,
                frame->rax, frame->rbx, frame->rcx, frame->rdx);

    Logger::Log("RSI=%x RDI=%x RBP=%x\n",
                LOG_LEVEL::ERROR,
                frame->rsi, frame->rdi, frame->rbp);

    Logger::Log("R8=%x  R9=%x  R10=%x  R11=%x\n",
                LOG_LEVEL::ERROR,
                frame->r8, frame->r9, frame->r10, frame->r11);

    Logger::Log("R12=%x R13=%x R14=%x R15=%x\n",
                LOG_LEVEL::ERROR,
                frame->r12, frame->r13, frame->r14, frame->r15);

    Logger::Log("DS=%x\n", LOG_LEVEL::ERROR, frame->ds);

    Logger::Log("System halted.", LOG_LEVEL::ERROR);

    while (true)
        asm volatile("hlt");
}

void isr_handler(ISR_INTERRUPT_FRAME* frame)
{
    if (Handles[frame->interrupt] != nullptr)
    {
        Handles[frame->interrupt](frame);
    }
    else 
    {
        panic(frame,"Unhandled Interrupt");
    }
}

void GenerateISR(uint8_t interrupt)
{
    uint8_t* dest = ISRWritePtr;
    if (ISRAdded[interrupt]) return;
    idt_set_descriptor(interrupt,0x08,RING0_INTERRUPT_GATE,0,dest);
    ISRAdded[interrupt] = true;
    if (HasErrorCode(interrupt))
    {
        memcpy(dest, isr_code_err, sizeof(isr_code_err));

        dest[ISR_CODE_ERROR_INTERRUPT_NUMBER] = interrupt;

        *(uint64_t*)(dest + ISR_CODE_ERROR_CALL_HANDLER_OFFSET) = (uint64_t)isr_handler;

        ISRWritePtr += sizeof(isr_code_err);
    }
    else
    {
        memcpy(dest, isr_code_no_err, sizeof(isr_code_no_err));

        dest[ISR_CODE_NO_ERROR_INTERRUPT_NUMBER] = interrupt;

        *(uint64_t*)(dest + ISR_CODE_NO_ERROR_CALL_HANDLER_OFFSET) =
            (uint64_t)isr_handler;

        ISRWritePtr += sizeof(isr_code_no_err);
    }
}

void RegisterHandler(uint8_t IntteruptNumber,ISRHandler handler)
{
    Handles[IntteruptNumber] = handler;
}
