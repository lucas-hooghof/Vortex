#include "efi.h"
#include "efi_lib.h"

#define VXFS_HEADER "VXFS"
#define BLOCK_SIZE 512

#define GROUP_SIZE_SECTORS   4096   // 2 MiB
#define INODE_RATIO_SECTORS  32     // 16 KiB per inode
#define EXTENTS_PER_INODE    4

#define BIT(x) (1 << x)

EFI_STATUS status = EFI_SUCCESS;

typedef struct 
{
    char Header[4];
    uint64_t BytesPerSector;
    uint64_t TotalSectors;

    uint32_t InodeSize;
    uint32_t InodeTableSize;
    uint8_t InodeTables;


    uint32_t ExtentSize;
    uint32_t ExtentTableSize;
    uint32_t ExtentTables;

    uint16_t NextExtentID;
    uint16_t NextInodeID;
    uint16_t NextExtentTableID;
    uint16_t NextInodeTableID;


    uint64_t RootInodeID;

    uint64_t InodeTablesStart;
    uint64_t ExtentTableStart;
    uint64_t DataBitmapStart;
    uint64_t DataRegionStart;

    uint16_t InodesPerTable;
    uint16_t ExtentsPerTable;
    uint64_t DataBitmapSize;

    char Label[16];

    uint16_t unused;

    uint8_t padding[BLOCK_SIZE-119];

}__attribute__((packed))VXFS_SUPERBLOCK;

typedef struct
{
    uint64_t StartSector;
    uint64_t SizeInSectors;

    uint16_t NextExtentID;
    uint16_t NextExtentTableID;

    uint16_t ExtentID;
    uint16_t ExtentTableID;
    uint8_t Availible;
    uint8_t NextExtentUsed;

    uint8_t padding[6];
} __attribute__((packed)) VXFS_EXTENT;

typedef struct 
{
    uint16_t InodeID;
    uint16_t InodeTableID;
    uint16_t Flags;
    uint16_t Permissions;
    uint16_t ExtentID;
    uint16_t ExtentTableID;

    uint8_t free;
    uint64_t SizeInBytes;
    uint16_t NextFreeByte;
}__attribute((packed))VXFS_INODE;

typedef struct
{
    uint16_t InodeID;
    uint16_t InodeTableID;
    uint8_t valid;
    uint8_t paddign;
    uint16_t NameLenght;
    //Follows Name length but not in struct because it can vary
}__attribute((packed))VXFS_DIRENTRY;

typedef enum
{
    DIR = BIT(0),
    SYSTEM = BIT(1),
    SYSLINK = BIT(2),
    HARDLINK = BIT(3)
}VXFS_FLAGS;

// Basic 64-bit ELF types
typedef uint16_t Elf64_Half;   // Unsigned half int (2 bytes)
typedef uint32_t Elf64_Word;   // Unsigned int (4 bytes)
typedef int32_t  Elf64_Sword;  // Signed int (4 bytes)
typedef uint64_t Elf64_Xword;  // Unsigned long int (8 bytes)
typedef int64_t  Elf64_Sxword; // Signed long int (8 bytes)
typedef uint64_t Elf64_Addr;   // Unsigned program address (8 bytes)
typedef uint64_t Elf64_Off;    // Unsigned file offset (8 bytes)
typedef uint16_t Elf64_Section;// Section index (2 bytes)
typedef uint16_t Elf64_Versym; // Version symbol index (2 bytes)

typedef struct {
    unsigned char e_ident[16]; // ELF identification
    Elf64_Half    e_type;      // Object file type
    Elf64_Half    e_machine;   // Architecture
    Elf64_Word    e_version;   // Object file version
    Elf64_Addr    e_entry;     // Entry point virtual address
    Elf64_Off     e_phoff;     // Program header table file offset
    Elf64_Off     e_shoff;     // Section header table file offset
    Elf64_Word    e_flags;     // Processor-specific flags
    Elf64_Half    e_ehsize;    // ELF header size in bytes
    Elf64_Half    e_phentsize; // Program header table entry size
    Elf64_Half    e_phnum;     // Program header table entry count
    Elf64_Half    e_shentsize; // Section header table entry size
    Elf64_Half    e_shnum;     // Section header table entry count
    Elf64_Half    e_shstrndx;  // Section header string table index
} Elf64_Ehdr;

typedef struct {
    Elf64_Word  p_type;   // Segment type
    Elf64_Word  p_flags;  // Segment flags
    Elf64_Off   p_offset; // Offset in file
    Elf64_Addr  p_vaddr;  // Virtual address in memory
    Elf64_Addr  p_paddr;  // Physical address (unused on many OS)
    Elf64_Xword p_filesz; // Size of segment in file
    Elf64_Xword p_memsz;  // Size of segment in memory
    Elf64_Xword p_align;  // Alignment of segment
} Elf64_Phdr;

typedef struct {
    Elf64_Word  sh_name;      // Section name (string tbl index)
    Elf64_Word  sh_type;      // Section type
    Elf64_Xword sh_flags;     // Section flags
    Elf64_Addr  sh_addr;      // Address in memory
    Elf64_Off   sh_offset;    // Offset in file
    Elf64_Xword sh_size;      // Section size in bytes
    Elf64_Word  sh_link;      // Link to another section
    Elf64_Word  sh_info;      // Additional section information
    Elf64_Xword sh_addralign; // Section alignment
    Elf64_Xword sh_entsize;   // Entry size if section holds table
} Elf64_Shdr;

typedef struct {
    Elf64_Addr  r_offset; // Address
    Elf64_Xword r_info;   // Relocation type and symbol index
} Elf64_Rel;

typedef struct {
    Elf64_Addr   r_offset; // Address
    Elf64_Xword  r_info;   // Relocation type and symbol index
    Elf64_Sxword r_addend; // Addend
} Elf64_Rela;

typedef struct {
    Elf64_Word      st_name;  // Symbol name (string tbl index)
    unsigned char   st_info;  // Type and binding
    unsigned char   st_other; // No defined meaning, 0
    Elf64_Section   st_shndx; // Section index
    Elf64_Addr      st_value; // Value of the symbol
    Elf64_Xword     st_size;  // Size of the symbol
} Elf64_Sym;

// Macros to extract symbol info
#define ELF64_ST_BIND(i)   ((i) >> 4)
#define ELF64_ST_TYPE(i)   ((i) & 0xF)
#define ELF64_ST_INFO(b,t) (((b)<<4)+((t)&0xF))

// e_type
#define ET_NONE   0  // No file type
#define ET_REL    1  // Relocatable file
#define ET_EXEC   2  // Executable file
#define ET_DYN    3  // Shared object file
#define ET_CORE   4  // Core file

// e_machine
#define EM_X86_64 62 // AMD x86-64 architecture

// p_type
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_TLS     7

// p_flags
#define PF_X 1
#define PF_W 2
#define PF_R 4

// sh_type
#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9
#define SHT_SHLIB    10
#define SHT_DYNSYM   11

// sh_flags
#define SHF_WRITE     0x1
#define SHF_ALLOC     0x2
#define SHF_EXECINSTR 0x4
#define SHF_TLS       0x400

void PrintVXFS_Superblock(VXFS_SUPERBLOCK* sb)
{
    printf_c16(u"=== VXFS Superblock ===\n\r");

    printf_c16(u"Header: %.4c\n\r", sb->Header[0], sb->Header[1], sb->Header[2], sb->Header[3]);
    printf_c16(u"BytesPerSector: %llu\n\r", sb->BytesPerSector);
    printf_c16(u"TotalSectors: %llu\n\r", sb->TotalSectors);

    printf_c16(u"InodeSize: %u\n\r", sb->InodeSize);
    printf_c16(u"InodeTableSize: %u\n\r", sb->InodeTableSize);
    printf_c16(u"InodeTables: %u\n\r", sb->InodeTables);

    printf_c16(u"ExtentSize: %u\n\r", sb->ExtentSize);
    printf_c16(u"ExtentTableSize: %u\n\r", sb->ExtentTableSize);
    printf_c16(u"ExtentTables: %u\n\r", sb->ExtentTables);

    printf_c16(u"NextExtentID: %u\n\r", sb->NextExtentID);
    printf_c16(u"NextInodeID: %u\n\r", sb->NextInodeID);
    printf_c16(u"NextExtentTableID: %u\n\r", sb->NextExtentTableID);
    printf_c16(u"NextInodeTableID: %u\n\r", sb->NextInodeTableID);


    printf_c16(u"RootInodeID: %llu\n\r", sb->RootInodeID);

    printf_c16(u"InodeTablesStart: %llu\n\r", sb->InodeTablesStart);
    printf_c16(u"ExtentTableStart: %llu\n\r", sb->ExtentTableStart);
    printf_c16(u"DataBitmapStart: %llu\n\r", sb->DataBitmapStart);
    printf_c16(u"DataRegionStart: %llu\n\r", sb->DataRegionStart);

    printf_c16(u"InodesPerTable: %u\n\r", sb->InodesPerTable);
    printf_c16(u"ExtentsPerTable: %u\n\r", sb->ExtentsPerTable);
    printf_c16(u"DataBitmapSize: %llu\n\r", sb->DataBitmapSize);

    printf_c16(u"Label: %.16a\n\r", sb->Label);  // print Label as string

    printf_c16(u"Unused: %u\n\r", sb->unused);
    printf_c16(u"Padding bytes: %u\n\r", (UINTN)sizeof(sb->padding));

    printf_c16(u"========================\n\r");
}

DISK_FILE* FindVXFS(EFI_HANDLE ImageHandle,EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_HANDLE* BIOHandles = NULL;
    EFI_GUID BioGuid = EFI_BLOCK_IO_PROTOCOL_GUID;
    UINTN Handles = 0;
    status = SystemTable->BootServices->LocateHandleBuffer(ByProtocol,&BioGuid,NULL,&Handles,&BIOHandles);
    if (EFI_ERROR(status))
    {
        printf_c16(u"Failed to locate BlockIO %s\n\r",GetEFIError(status));
        return NULL;
    }

    CHAR8 buffer[512] = {0};

    for (UINTN handle = 0; handle < Handles; handle++)
    {
        EFI_BLOCK_IO_PROTOCOL* biop = NULL;
        status = SystemTable->BootServices->OpenProtocol(BIOHandles[handle],&BioGuid,(VOID**)&biop,ImageHandle,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        if (EFI_ERROR(status))
        {
            printf_c16(u"Failed to open biop %s\n\r",GetEFIError(status));
            return NULL;
        }

        if (!biop->Media->MediaPresent)
        {
            printf_c16(u"No media present for %d\r\n",biop->Media->MediaId);
            continue;
        }

        status = biop->ReadBlocks(biop,biop->Media->MediaId,0,512,(VOID*)buffer);
        if (EFI_ERROR(status))
        {
            printf_c16(u"Failed to read first sector of biop %s\n\r",GetEFIError(status));
            return NULL;
        }

        if (biop->Media->BlockSize != 512)
        {
            printf_c16(u"Iregular block size: %lu",biop->Media->BlockSize);
                }

        if (!memcmp(buffer,VXFS_HEADER,4))
        {
            VXFS_SUPERBLOCK* superblock = (VXFS_SUPERBLOCK*)buffer;
            DISK_FILE* diskfile = CreateDiskFile(biop);

            return diskfile;

        }
    }

    
    return NULL;
}

VXFS_INODE GetInode(EFI_STATUS* status,VXFS_SUPERBLOCK* superblock,uint64_t InodeID,uint64_t InodeTableID,DISK_FILE* file)
{
    fseek(file,superblock->InodeTablesStart + InodeTableID * superblock->InodeTableSize + (InodeID * superblock->InodeSize) / 512);

    VXFS_INODE* inodes = NULL;
    *status = bs->AllocatePool(EfiLoaderData,512,(VOID**)&inodes);
    if (EFI_ERROR(*status))
    {
        printf_c16(u"Failed to allocate memory %s\r\n",GetEFIError(*status));
        //Recovery code?
        while(1) {}
    }

    *status = fread(inodes,512,file);

    if (EFI_ERROR(*status))
    {
        printf_c16(u"Failed to read Inode sector: %lu %s\r\n",file->CurrentPositionLBA,GetEFIError(*status));
        //Recovery code?
        while(1) {}
    }

    UINTN InodesPerSector = 512 / superblock->InodeSize;


    for (UINTN i = 0; i < InodesPerSector; i++)
    {
        if (inodes[i].InodeID == InodeID && inodes[i].InodeTableID == InodeTableID) return inodes[i];
    }

    *status = EFI_NOT_FOUND;

    return (VXFS_INODE){0};
}   

VXFS_EXTENT GetExtent(EFI_STATUS* status,VXFS_SUPERBLOCK* superblock,uint64_t ExtentID,uint64_t ExtentTableID,DISK_FILE* file)
{
    UINT64 extentOffset = ExtentID * superblock->ExtentSize;
    UINT64 sector = superblock->ExtentTableStart +
                    ExtentTableID * superblock->ExtentTableSize +
                    extentOffset / 512;

    fseek(file, sector);
    VXFS_EXTENT* extents = NULL;
    *status = bs->AllocatePool(EfiLoaderData,512,(VOID**)&extents);
    if (EFI_ERROR(*status))
    {
        printf_c16(u"Failed to allocate memory %s\r\n",GetEFIError(*status));
        //Recovery code?
        while(1) {}
    }

    *status = fread(extents,512,file);
    if (EFI_ERROR(*status))
    {
        printf_c16(u"Failed to read Extent sector: %lu %s\r\n",file->CurrentPositionLBA,GetEFIError(*status));
        //Recovery code?
        while(1) {}
    }

    UINTN ExtentsPerSector = 512 / superblock->ExtentSize;

    for (UINTN i = 0; i < ExtentsPerSector; i++)
    {
        if (extents[i].ExtentID == ExtentID && extents[i].ExtentTableID == ExtentTableID && extents[i].Availible == false) return extents[i];
    }

    *status = EFI_NOT_FOUND;

    return (VXFS_EXTENT){0};
}

VXFS_DIRENTRY GetDirEntry(EFI_STATUS* status, VXFS_EXTENT extent, DISK_FILE* file, const char* Name)
{
    printf_c16(u"Extent: %lu,%lu\r\n",extent.StartSector,extent.SizeInSectors);
    for (UINT64 block = 0; block < extent.SizeInSectors; block++)
    {
    
        // Seek to the block (convert LBA to byte offset)
        fseek(file, (extent.StartSector + block));

        UINT8 Buffer[512];
        *status = fread(Buffer, 512, file);
        if (EFI_ERROR(*status))
        {
            return (VXFS_DIRENTRY){0};
        }

        UINT16 offset = 0;
        while (offset + sizeof(VXFS_DIRENTRY) <= 512)
        {
            VXFS_DIRENTRY* entry = (VXFS_DIRENTRY*)(Buffer + offset);
            offset += sizeof(VXFS_DIRENTRY);

            if (offset + entry->NameLenght > 512)
                break; // name would overflow the block

            if (memcmp(Buffer + offset, Name, entry->NameLenght) == 0)
            {
                CHAR16 buf[32];
                CHAR8toCHAR16((const CHAR8*)(Buffer+offset),buf);
                printf_c16(u"Found smth: %s",buf);
                *status = EFI_SUCCESS;
                return *entry;
            }

            offset += entry->NameLenght;
        }
    }

    *status = EFI_NOT_FOUND;
    return (VXFS_DIRENTRY){0};
}

void PrintSystemDirInfo(
    EFI_STATUS* status,
    VXFS_SUPERBLOCK* superblock,
    VXFS_DIRENTRY systemEntry,
    DISK_FILE* file
) {
    // 1. Get the inode of the system directory
    VXFS_INODE systemInode = GetInode(status, superblock, systemEntry.InodeID, systemEntry.InodeTableID, file);
    if (*status != EFI_SUCCESS) {
        printf_c16(u"Failed to get system inode %u:%u\n\r", systemEntry.InodeID, systemEntry.InodeTableID);
        return;
    }

    // 2. Get the extent of the system inode
    VXFS_EXTENT systemExtent = GetExtent(status, superblock, systemInode.ExtentID, systemInode.ExtentTableID, file);
    if (*status != EFI_SUCCESS) {
        printf_c16(u"Failed to get system extent %u:%u\n\r", systemInode.ExtentID, systemInode.ExtentTableID);
        return;
    }

    // 3. Print directory entry info
    printf_c16(u"=== System Directory ===\n\r");
    printf_c16(u"DirEntry InodeID: %u, InodeTableID: %u\n\r", systemEntry.InodeID, systemEntry.InodeTableID);
    printf_c16(u"Inode Size: %llu bytes\n\r", systemInode.SizeInBytes);
    printf_c16(u"Inode ExtentID: %u, ExtentTableID: %u\n\r", systemInode.ExtentID, systemInode.ExtentTableID);
    
    // 4. Print extent info
    printf_c16(u"Extent StartSector: %llu, SizeInSectors: %llu\n\r", systemExtent.StartSector, systemExtent.SizeInSectors);
    printf_c16(u"========================\n\r");
}

VOID* LoadFileFromVXFS(EFI_STATUS* status,VXFS_EXTENT extent,DISK_FILE* file)
{
    VOID* filedata = NULL;
    *status = bs->AllocatePool(EfiLoaderData,extent.SizeInSectors * 512,(VOID**)&filedata);
    if (EFI_ERROR(status))
    {
        printf_c16(u"Failed to allocate\n\r");
        return NULL;
    }

    fseek(file,extent.StartSector);
    *status = fread(filedata,extent.SizeInSectors * 512,file);
    if (EFI_ERROR(status))
    {
        printf_c16(u"Failed to read file: %s\n\r",GetEFIError(*status));
        return NULL;
    }

    return filedata;
}

//Copied from gcc source
void *
memset (void *dest, register int val, register size_t len)
{
  register unsigned char *ptr = (unsigned char*)dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}

VOID* LoadELF(EFI_STATUS* status,void* elfdata)
{
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elfdata;

    if (memcmp(ehdr->e_ident,(UINT8[4]){0x7F,'E','L','F'},4))
    {
        *status = EFI_COMPROMISED_DATA;
        return NULL;
    }

    if (ehdr->e_type != ET_DYN)
    {
        *status = EFI_UNSUPPORTED;
        return NULL;
    }

    Elf64_Phdr* phdrs = (Elf64_Phdr*)((uint64_t)elfdata + ehdr->e_phoff);

    UINTN max_alignment = 4096; //Page
    UINTN mem_min = UINT64_MAX, mem_max = 0;

    for (uint64_t header = 0; header < ehdr->e_phnum; header++)
    {
        Elf64_Phdr phdr = phdrs[header];

        if(phdr.p_type != PT_LOAD) continue;

        if (max_alignment < phdr.p_align) max_alignment = phdr.p_align;

        UINTN hdr_begin = phdr.p_paddr;
        UINTN hdr_end = phdr.p_paddr + phdr.p_memsz + max_alignment - 1;

        hdr_begin &= ~(max_alignment-1);
        hdr_end &= ~(max_alignment-1);



    }


        UINTN max_memory_needed = mem_max - mem_min;   

    EFI_PHYSICAL_ADDRESS program_buffer = 0;
    UINTN pages_needed = (max_memory_needed + (4096-1)) / 4096;

    *status = bs->AllocatePages(AllocateAnyPages, EfiLoaderCode, pages_needed, &program_buffer);
    if (EFI_ERROR(*status)) {
        printf_c16(u"Failed to allocate\r\n");
        return NULL;
    }

    // Zero init buffer, to ensure 0 padding for all program sections
    memset((VOID *)program_buffer, 0, max_memory_needed);

    // Load program headers into buffer
    Elf64_Phdr* phdr = (Elf64_Phdr *)((UINT8 *)ehdr + ehdr->e_phoff);
    for (UINT16 i = 0; i < ehdr->e_phnum; i++, phdr++) {
        // Only interested in loadable program headers
        if (phdr->p_type != PT_LOAD) continue;

        // Use relative position of program section in file, to ensure it still works correctly.
        //   With PIE executables, this means we can use any entry point or addresses, as long as
        //   we use the same relative addresses.
        UINTN relative_offset = phdr->p_vaddr - mem_min;

        // Read p_filesz amount of data from p_offset in original file buffer,
        //   to the same relative offset of p_vaddr in new buffer
        UINT8 *dst = (UINT8 *)program_buffer + relative_offset; 
        UINT8 *src = (UINT8 *)elfdata     + phdr->p_offset;
        UINT32 len = phdr->p_filesz;
        memcpy(dst, src, len);
    }

    VOID *entry_point = (VOID *)((UINT8 *)program_buffer + (ehdr->e_entry - mem_min));
    return entry_point;
}

typedef struct {
    void* BaseAddress;       // Framebuffer base pointer
    UINT64 BufferSize;       // Total size in bytes
    UINT32 Width;            // Width in pixels
    UINT32 Height;           // Height in pixels
    UINT32 PixelsPerScanLine;// Stride (pixels per row)
} FRAMEBUFFER;

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} PSF1_HEADER;

typedef struct {
	PSF1_HEADER* psf1_Header;
	void* glyphBuffer;
} PSF1_FONT;

PSF1_FONT* LoadFont(void* fontdata)
{
    PSF1_HEADER* fontheader = (PSF1_HEADER*)fontdata;

    
	if (fontheader->magic[0] != PSF1_MAGIC0 || fontheader->magic[1] != PSF1_MAGIC1){
		return NULL;
	}

    return (PSF1_FONT*)fontdata;
}

EFI_STATUS GetFramebuffer(FRAMEBUFFER* fb) {
    EFI_STATUS status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;

    // Locate Graphics Output Protocol
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    status = bs->LocateProtocol(&gopGuid, NULL, (VOID**)&gop);
    if (EFI_ERROR(status)) {
        return status;
    }

    // Find the best mode <= 1280x720
    UINT32 bestMode = gop->Mode->Mode;
    UINT32 bestWidth = 0, bestHeight = 0;
    for (UINT32 i = 0; i < gop->Mode->MaxMode; i++) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
        UINTN size;
        status = gop->QueryMode(gop, i, &size, &info);
        if (EFI_ERROR(status)) continue;

        if (info->HorizontalResolution <= 1280 &&
            info->VerticalResolution <= 720) {
            if (info->HorizontalResolution >= bestWidth &&
                info->VerticalResolution >= bestHeight) {
                bestMode = i;
                bestWidth = info->HorizontalResolution;
                bestHeight = info->VerticalResolution;
            }
        }
    }

    // Set the best mode
    status = gop->SetMode(gop, bestMode);
    if (EFI_ERROR(status)) {
        return status;
    }

    // Fill framebuffer info
    fb->BaseAddress = (void*)gop->Mode->FrameBufferBase;
    fb->BufferSize = gop->Mode->FrameBufferSize;
    fb->Width = gop->Mode->Info->HorizontalResolution;
    fb->Height = gop->Mode->Info->VerticalResolution;
    fb->PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;

    return EFI_SUCCESS;
}

typedef struct 
{
    FRAMEBUFFER* framebuffer;
    PSF1_FONT* font;
    EFI_MEMORY_DESCRIPTOR* mMap;
    UINTN MapSize;
    UINTN DescriptorSize;
}bootinfo_t;


EFI_STATUS efi_main(EFI_HANDLE ImageHandle,EFI_SYSTEM_TABLE* SystemTable)
{
    Initilize(SystemTable);
    //Maybe do some dualbooting stuff hea


    DISK_FILE* vxfspartition = FindVXFS(ImageHandle,SystemTable);
    if (vxfspartition == NULL)
    {
        printf_c16(u"Failed to find VXFS");
        //Recovery code?
        while(1) {}
    }

    //Read Superblock
    VXFS_SUPERBLOCK* superblock = NULL;
    status = SystemTable->BootServices->AllocatePool(EfiLoaderData,sizeof(VXFS_SUPERBLOCK),(VOID**)&superblock);
    if (EFI_ERROR(status))
    {
        printf_c16(u"Failed to allocate memory %s\r\n",GetEFIError(status));
        //Recovery code?
        while(1) {}
    }
    status = fread(superblock,sizeof(VXFS_SUPERBLOCK),vxfspartition);
    if (EFI_ERROR(status))
    {
        printf_c16(u"Failed to read superblock: %s\r\n",GetEFIError(status));

        while(1) {};
    }


    if (memcmp(superblock->Header,VXFS_HEADER,4))
    {
        printf_c16(u"Corrupt superblock %s\r\n",superblock->Header);
        // Recovery code?
        while(1){}
    }

    //Get Root Inode
    VXFS_INODE rootnode = GetInode(&status,superblock,superblock->RootInodeID,0,vxfspartition);
    if (status == EFI_NOT_FOUND)
    {
        printf_c16(u"Failed to find root node\r\n");
        // Recovery code?
        while(1){}
    }


    //Get Root Extent
    VXFS_EXTENT rootextent = GetExtent(&status,superblock,rootnode.ExtentID,rootnode.ExtentTableID,vxfspartition);
    if (status == EFI_NOT_FOUND)
    {
        printf_c16(u"Failed to find root extent\r\n");
        // Recovery code?
        while(1){}
    }

    printf_c16(u"Root extent start=%llu size=%llu\n\r",
    rootextent.StartSector,
    rootextent.SizeInSectors);


    //Find System dir
    VXFS_DIRENTRY SystemDirEntry = GetDirEntry(&status,rootextent,vxfspartition,"system");
    if (EFI_ERROR(status) || SystemDirEntry.NameLenght != strlen("system"))
    {
        printf_c16(u"Failed to find system directory %s\r\n",GetEFIError(status));
        // Recovery code?
        while(1){}
    }


    //Find System Inode
    VXFS_INODE SystemInode = GetInode(&status,superblock,SystemDirEntry.InodeID,SystemDirEntry.InodeTableID,vxfspartition);
    if (status == EFI_NOT_FOUND)
    {
        printf_c16(u"Failed to find system inode\r\n");
        // Recovery code?
        while(1){}
    }

    //Find System Extent
    VXFS_EXTENT SystemExtent = GetExtent(&status,superblock,SystemInode.ExtentID,SystemInode.ExtentTableID,vxfspartition);
    if (status == EFI_NOT_FOUND)
    {
        printf_c16(u"Failed to find system extent\r\n");
        // Recovery code?
        while(1){}
    }


    //Find kernel dir entry in system
    VXFS_DIRENTRY KernelDirEntry = GetDirEntry(&status,SystemExtent,vxfspartition,"kernel");
    if (status == EFI_NOT_FOUND || KernelDirEntry.NameLenght != strlen("kernel"))
    {
        printf_c16(u"Failed to find kernel %s\r\n",GetEFIError(status));
        // Recovery code?
        while(1){}
    }


    VXFS_INODE KernelInode = GetInode(&status,superblock,KernelDirEntry.InodeID,KernelDirEntry.InodeTableID,vxfspartition);
    if (status == EFI_NOT_FOUND)
    {
        printf_c16(u"Failed to find kernel inode\r\n");
        // Recovery code?
        while(1){}
    }


    VXFS_DIRENTRY DatDirEntry = GetDirEntry(&status,SystemExtent,vxfspartition,"dat");
    if (status == EFI_NOT_FOUND || DatDirEntry.NameLenght != strlen("dat"))
    {
        printf_c16(u"Failed to find dat folder: %s\r\n",GetEFIError(status));
        // Recovery code?
        while(1){}
    }


    VXFS_INODE DatInode = GetInode(&status,superblock,DatDirEntry.InodeID,DatDirEntry.InodeTableID,vxfspartition);
    if (status == EFI_NOT_FOUND)
    {
        printf_c16(u"Failed to find dat inode\r\n");
        // Recovery code?
        while(1){}
    }


    VXFS_EXTENT DatExtent = GetExtent(&status,superblock,DatInode.ExtentID,DatInode.ExtentTableID,vxfspartition);
    if (status == EFI_NOT_FOUND)
    {
        printf_c16(u"Failed to find dat extent\r\n");
        // Recovery code?
        while(1){}
    }

    VXFS_DIRENTRY BootFontDirEntry = GetDirEntry(&status,DatExtent,vxfspartition,"bootfont");
    if (status == EFI_NOT_FOUND || DatDirEntry.NameLenght != strlen("dat"))
    {
        printf_c16(u"Failed to find dat folder\r\n");
        // Recovery code?
        while(1){}
    }


    VXFS_INODE BootFontInode = GetInode(&status,superblock,BootFontDirEntry.InodeID,BootFontDirEntry.InodeTableID,vxfspartition);
    if (status == EFI_NOT_FOUND)
    {
        printf_c16(u"Failed to find dat inode\r\n");
        // Recovery code?
        while(1){}
    }

    VXFS_EXTENT BootFontExtent = GetExtent(&status,superblock,BootFontInode.ExtentID,BootFontInode.ExtentTableID,vxfspartition);
    if (status == EFI_NOT_FOUND)
    {
        printf_c16(u"Failed to find dat extent\r\n");
        // Recovery code?
        while(1){}
    }
    
    VOID* fontdata = LoadFileFromVXFS(&status,BootFontExtent,vxfspartition);
    if (fontdata == NULL)
    {
        printf_c16(u"Failed to read font\r\n");
        //Recovery code?
        while(1){}
    }


    PSF1_FONT* font = LoadFont(fontdata);



    VXFS_EXTENT KernelExtent = GetExtent(&status,superblock,KernelInode.ExtentID,KernelInode.ExtentTableID,vxfspartition);
    if (status == EFI_NOT_FOUND)
    {
        printf_c16(u"Failed to find kernel extent\r\n");
        // Recovery code?
        while(1){}
    }

    VOID* kerneldata = LoadFileFromVXFS(&status,KernelExtent,vxfspartition);
    if (kerneldata == NULL)
    {
        printf_c16(u"Failed to read kernel\r\n");
        //Recovery code?
        while(1){}
    }

    
    //Load kernel
    VOID* kernelEntryPoint = LoadELF(&status,kerneldata);
    if (status != EFI_SUCCESS)
    {
        printf_c16(u"Failed to load elf: %s\n\r",GetEFIError(status));

        //reboot
        while(1){}
    }

    //Load Font
    //Get Framebuffer
    //Get MemoryMap
    DestroyDiskFile(vxfspartition);
    bs->FreePool(superblock);

    FRAMEBUFFER* framebuffer = NULL;
    status = bs->AllocatePool(EfiLoaderData,sizeof(FRAMEBUFFER),(VOID**)&framebuffer);
    GetFramebuffer(framebuffer);


    UINTN MapSize = 0,MapKey = 0,DescriptorSize = 0;
    UINT32 DescriptorVersion = 0;

    EFI_MEMORY_DESCRIPTOR* map = NULL;

    bs->GetMemoryMap(&MapSize,NULL,&MapKey,&DescriptorSize,&DescriptorVersion);
    status = bs->AllocatePool(EfiLoaderData,MapSize,(VOID**)&map);
    if (EFI_ERROR(status))
    {
        printf_c16(u"Failed to Get Memory Map: %s\r\n",GetEFIError(status));
        //reboot after some time
        while (1) {}
    }
    bs->GetMemoryMap(&MapSize,map,&MapKey,&DescriptorSize,&DescriptorVersion);

    bootinfo_t info;
    info.DescriptorSize = DescriptorSize;
    info.MapSize = MapSize;
    info.mMap = map;
    info.framebuffer = framebuffer;
    info.font = font;

    printf_c16(u"Gere");
    //Call kernel
    void (__attribute__((sysv_abi)) *kernel_start)(bootinfo_t*) =
    (void (__attribute__((sysv_abi)) *)(bootinfo_t*)) kernelEntryPoint;

    bs->ExitBootServices(ImageHandle,MapKey);
    kernel_start(&info);

    while(1) {}
    return status;
}