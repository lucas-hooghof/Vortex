#include <generic/stdio.h>

#include <generic/kernelInit.h>

#include <fs/filesystems/VXFS.h>
#include <memory/heap.h>

#include <generic/string.h>

#include <memory/PageTableManager.h>

extern void (*__init_array_start[])();
extern void (*__init_array_end[])();

void call_constructors()
{
    for (size_t i = 0; &__init_array_start[i] < __init_array_end; i++)
        __init_array_start[i]();
}

#define PAGE_SIZE 4096

struct ELF64_EHDR
{
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
} __attribute__((packed));

struct ELF64_PHDR
{
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} __attribute__((packed));

#define PT_LOAD 1

void* LoadElf(void* buffer)
{
    ELF64_EHDR* ehdr = (ELF64_EHDR*)buffer;

    if (memcmp(ehdr->e_ident, "\x7F""ELF", 4) != 0)
        return nullptr;

    if (ehdr->e_machine != 0x3E)
        return nullptr;

    ELF64_PHDR* phdrs = (ELF64_PHDR*)((uint64_t)buffer + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        ELF64_PHDR* ph = &phdrs[i];

        if (ph->p_type != PT_LOAD)
            continue;

        uint64_t page_start = ph->p_vaddr & ~(PAGE_SIZE - 1);
        uint64_t page_offset = ph->p_vaddr - page_start;

        uint64_t mem_size = ph->p_memsz + page_offset;
        uint64_t pages = (mem_size + PAGE_SIZE - 1) / PAGE_SIZE;

        for (uint64_t p = 0; p < pages; p++)
        {
            void* virt = (void*)(page_start + (p * PAGE_SIZE));

            void* phys = PageAllocater::GetInstance()->RequestPage();

            PageTableManager::GetInstance()->MapMemory(
                virt,
                phys,
                PAGE_PRESENT | PAGE_RW 
            );
        }

        // Copy file data
        memcpy(
            (void*)ph->p_vaddr,
            (void*)((uint64_t)buffer + ph->p_offset),
            ph->p_filesz
        );

        // Zero BSS
        if (ph->p_memsz > ph->p_filesz)
        {
            memset(
                (void*)(ph->p_vaddr + ph->p_filesz),
                0,
                ph->p_memsz - ph->p_filesz
            );
        }
    }

    return (void*)ehdr->e_entry;
}



extern "C" void __attribute__((noreturn)) kernel_main(bootinfo_t* info)
{
    call_constructors();
    Logger::Initilize(info);
    if (!PrepareMemory(info))
    {
        Logger::Log("Failed to initilize memory",LOG_LEVEL::ERROR);
        while(1) {}
    }
    Logger::Log("Memory Management Initilized\n", LOG_LEVEL::INFO);

    if (!PrepareInterrupts())
    {
        Logger::Log("Failed to initilize interrupts",LOG_LEVEL::ERROR);
        while(1) {}
    }
    Logger::Log("Interrupts Initilized\n",LOG_LEVEL::INFO);

    if (!PrepareHardware(info))
    {
        Logger::Log("Failed to initilize hardware",LOG_LEVEL::ERROR);
        while(1) {}
    }

    Logger::Log("Hardware Intilized",LOG_LEVEL::INFO);

    fs::VXFS vxfs("/dev/sda1");

    size_t size = 0;
    vxfs.readfile("/bin/DE",nullptr,&size);

    void* DE = malloc(size);
    if (!vxfs.readfile("/bin/DE",DE,&size))
    {
        Logger::Log("Failed to load DE",LOG_LEVEL::INFO);
        while (1) {}
    }

    void* entrypoint = LoadElf(DE);

    using entry_t = void (*)();

    entry_t entry = (entry_t)entrypoint;
    entry();
    

    while (1) {}
}