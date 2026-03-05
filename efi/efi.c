#include "efi.h"

#define VXFS_HEADER "VXFS"
#define BLOCK_SIZE 512

#define GROUP_SIZE_SECTORS   4096   
#define INODE_RATIO_SECTORS  32     
#define EXTENTS_PER_INODE    4

#define BIT(x) (1 << x)

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

    uint64_t FreeInodes;
    uint64_t NextFreeInode;

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

    uint8_t padding[BLOCK_SIZE-135];

} __attribute__((packed)) VXFS_SUPERBLOCK;

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
} __attribute((packed)) VXFS_INODE;

typedef struct
{
    uint16_t InodeID;
    uint16_t InodeTableID;
    uint8_t valid;
    uint8_t paddign;
    uint16_t NameLenght;
} __attribute((packed)) VXFS_DIRENTRY;

typedef enum
{
    DIR      = BIT(0),
    SYSTEM   = BIT(1),
    SYSLINK  = BIT(2),
    HARDLINK = BIT(3)
} VXFS_FLAGS;

#define EI_NIDENT 16

typedef struct {
    unsigned char e_ident[EI_NIDENT];
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
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

typedef struct {
    void*        BaseAddress;
    size_t       BufferSize;
    unsigned int Width;
    unsigned int Height;
    unsigned int PixelsPerScanLine;
} Framebuffer;

typedef struct 
{
    EFI_MEMORY_DESCRIPTOR* mMap;
    UINTN MapSize;
    UINTN DescriptorSize;
    Framebuffer framebuffer;

    PSF1_FONT* bootfont;
} bootinfo_t;

typedef struct {
    UINT8  Type;
    UINT8  Length;
    UINT16 Handle;
} SMBIOS_HEADER;

typedef struct {
    SMBIOS_HEADER Hdr;
    UINT8  Manufacturer;
    UINT8  ProductName;
    UINT8  Version;
    UINT8  SerialNumber;
    UINT8  UUID[16];
    UINT8  WakeUpType;
    UINT8  SKUNumber;
    UINT8  Family;
} SMBIOS_TYPE1;

#define ET_DYN    3
#define PT_LOAD   1
#define PAGE_SIZE 4096

#define VXFS_ROOT_GUID \
    {0xa5e8bf06, 0x1238, 0x11f1, 0xb7, 0x4b, {0x00, 0x15, 0x5d, 0x0f, 0x3c, 0xbb}}

typedef struct {
    EFI_BLOCK_IO_PROTOCOL* bio;
    UINTN                  PartitionStartLBA;
} VxfsScanResult;



EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* cout;

void* memset(void* dest, uint8_t c, size_t n)
{
    uint8_t* d = (uint8_t*)dest;
    for (size_t i = 0; i < n; i++) d[i] = c;
    return dest;
}

INTN memcmp(VOID* m1, VOID* m2, UINTN len)
{
    UINT8 *p = m1, *q = m2;
    for (UINTN i = 0; i < len; i++)
        if (p[i] != q[i]) return (INTN)p[i] - (INTN)q[i];
    return 0;
}

VOID* memcpy(VOID* dst, VOID* src, UINTN len)
{
    UINT8 *p = dst, *q = src;
    while (len--) *p++ = *q++;
    return dst;
}

UINTN strlen_c16(CHAR16* s) { UINTN n = 0; while (*s++) n++; return n; }
UINTN strlen(CHAR8* s)      { UINTN n = 0; while (*s++) n++; return n; }

CHAR8* AsciiStrStr(const CHAR8* haystack, const CHAR8* needle)
{
    if (!*needle) return (CHAR8*)haystack;
    for (; *haystack; haystack++) {
        const CHAR8 *h = haystack, *n = needle;
        while (*h && *n && (*h == *n)) { h++; n++; }
        if (!*n) return (CHAR8*)haystack;
    }
    return NULL;
}

CHAR16* strrev_c16(CHAR16* s)
{
    if (!s) return s;
    CHAR16 *start = s, *end = s + strlen_c16(s) - 1;
    while (start < end) { CHAR16 t = *end; *end-- = *start; *start++ = t; }
    return s;
}

CHAR16 _itoa_buf[24];
CHAR16* itoa(UINTN number, UINT8 base)
{
    memset(_itoa_buf, 0, 24);
    const CHAR16* digits = u"0123456789ABCDEF";
    int i = 0;
    do { _itoa_buf[i++] = digits[number % base]; number /= base; } while (number > 0);
    if (base == 16) { _itoa_buf[i++] = u'x'; _itoa_buf[i++] = u'0'; }
    strrev_c16(_itoa_buf);
    return _itoa_buf;
}

VOID print_bool(BOOLEAN b)  { cout->OutputString(cout, b ? u"Y" : u"N"); }
VOID print_uint(UINTN val)  { cout->OutputString(cout, itoa(val, 10)); }
VOID print_str(CHAR16* s)   { cout->OutputString(cout, s); }
VOID print_nl(VOID)         { cout->OutputString(cout, u"\r\n"); }

VOID PrintHex(UINT64 value)
{
    CHAR16 buf[17]; buf[16] = 0;
    for (int i = 15; i >= 0; i--) {
        UINTN d = value & 0xF;
        buf[i] = (d < 10) ? (u'0' + d) : (u'A' + d - 10);
        value >>= 4;
    }
    cout->OutputString(cout, buf);
}

VOID print_hex_dump(VOID* data, UINTN len)
{
    UINT8* bytes = (UINT8*)data;
    for (UINTN i = 0; i < len; i++) {
        if (i % 16 == 0) { cout->OutputString(cout, itoa(i, 16)); cout->OutputString(cout, u"  "); }
        if (bytes[i] < 0x10) cout->OutputString(cout, u"0");
        cout->OutputString(cout, itoa(bytes[i], 16));
        cout->OutputString(cout, u" ");
        if (i % 16 == 15 || i == len - 1) {
            UINTN col = i % 16;
            if (col != 15) for (UINTN pad = col + 1; pad < 16; pad++) cout->OutputString(cout, u"   ");
            cout->OutputString(cout, u" |");
            UINTN row_start = i - (i % 16);
            for (UINTN j = row_start; j <= i; j++) {
                CHAR16 ch[2] = { (bytes[j] >= 0x20 && bytes[j] < 0x7F) ? (CHAR16)bytes[j] : u'.', 0 };
                cout->OutputString(cout, ch);
            }
            cout->OutputString(cout, u"|\r\n");
        }
    }
}

BOOLEAN CompareGuid(EFI_GUID* a, EFI_GUID* b)
{
    return (a->TimeLow == b->TimeLow) &&
           (a->TimeMid == b->TimeMid) &&
           (a->TimeHighAndVersion == b->TimeHighAndVersion) &&
           (a->ClockSeqHighAndReserved == b->ClockSeqHighAndReserved) &&
           (a->ClockSeqLow == b->ClockSeqLow) &&
           (memcmp(a->Node, b->Node, 6) == 0);
}

static VOID* AllocZero(EFI_BOOT_SERVICES* bs, UINTN size)
{
    VOID* ptr = NULL;
    if (EFI_ERROR(bs->AllocatePool(EfiLoaderData, size, &ptr)) || !ptr) {
        cout->OutputString(cout, u"AllocatePool failed\r\n");
        while (1) {}
    }
    memset(ptr, 0, size);
    return ptr;
}

static VOID ReadSectors(EFI_BLOCK_IO_PROTOCOL* bio, UINTN lba, UINTN bytes, VOID* buf)
{
    EFI_STATUS s = bio->ReadBlocks(bio, bio->Media->MediaId, lba, bytes, buf);
    if (EFI_ERROR(s)) {
        cout->OutputString(cout, u"ReadBlocks failed: ");
        cout->OutputString(cout, itoa(s, 16));
        cout->OutputString(cout, u"\r\n");
        while (1) {}
    }
}


static VxfsScanResult FindVxfsPartition(
    EFI_SYSTEM_TABLE* ST,
    EFI_HANDLE        ImageHandle,
    EFI_HANDLE*       HandleBuffer,
    UINTN             HandleCount)
{
    EFI_GUID BlockIOGuid       = EFI_BLOCK_IO_PROTOCOL_GUID;
    EFI_GUID PartitionInfoGuid = EFI_PARTITION_INFO_PROTOCOL_GUID;
    EFI_GUID VxfsRootGuid      = VXFS_ROOT_GUID;

    for (UINTN i = 0; i < HandleCount; i++) {
        EFI_BLOCK_IO_PROTOCOL* bio = NULL;
        if (EFI_ERROR(ST->BootServices->OpenProtocol(
                HandleBuffer[i], &BlockIOGuid, (VOID**)&bio,
                ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL))
            || !bio || !bio->Media) continue;

        if (bio->Media->LogicalPartition)
        {
            print_str(u"Found Partition\n\r");
            print_uint(bio->Media->BlockSize);
            print_nl();
            print_uint(bio->Media->LastBlock);
            print_nl();
            print_uint(bio->Media->LastBlock * bio->Media->BlockSize);
            print_nl();
            print_bool(bio->Media->RemovableMedia);
            print_nl();

            VOID* sector = AllocZero(ST->BootServices,512);

            ReadSectors(bio,0,512,sector);

            if (!memcmp(sector,VXFS_HEADER,4))
            {
                VxfsScanResult r = {bio,0};
                return r;
            }
        }

        EFI_PARTITION_INFO_PROTOCOL* partInfo = NULL;
        if (EFI_ERROR(ST->BootServices->OpenProtocol(
                HandleBuffer[i], &PartitionInfoGuid, (VOID**)&partInfo,
                ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL))
            || !partInfo) continue;

        if (partInfo->Type == PARTITION_TYPE_GPT &&
            memcmp(&partInfo->Info.Gpt.PartitionTypeGUID, &VxfsRootGuid, sizeof(EFI_GUID)) == 0) {
            cout->OutputString(cout, u"*** VxFS ROOT PARTITION FOUND via GPT ***\r\n");
            VxfsScanResult r = { bio, 0 };
            return r;
        }
    }

    VxfsScanResult empty = { NULL, 0 };
    return empty;
}

static VXFS_SUPERBLOCK* LoadSuperblock(EFI_BOOT_SERVICES* bs, EFI_BLOCK_IO_PROTOCOL* bio, UINTN partStartLBA)
{
    VXFS_SUPERBLOCK* sb = AllocZero(bs, sizeof(VXFS_SUPERBLOCK));
    ReadSectors(bio, partStartLBA, sizeof(VXFS_SUPERBLOCK), sb);

    if (memcmp(sb->Header, VXFS_HEADER, 4)) {
        cout->OutputString(cout, u"Couldn't find superblock header\r\n");
        bs->FreePool(sb);
        while (1) {}
    }
    return sb;
}

static VXFS_INODE ReadInode(
    EFI_BOOT_SERVICES*     bs,
    EFI_BLOCK_IO_PROTOCOL* bio,
    VXFS_SUPERBLOCK*       sb,
    UINTN                  partStartLBA,
    uint16_t               tableID,
    uint16_t               inodeID)
{
    UINTN lba = partStartLBA
              + sb->InodeTablesStart
              + sb->InodeTableSize * tableID
              + inodeID * sizeof(VXFS_INODE) / BLOCK_SIZE;

    VXFS_INODE* buf = AllocZero(bs, BLOCK_SIZE);
    ReadSectors(bio, lba, BLOCK_SIZE, buf);

    VXFS_INODE node = {0};
    for (UINTN i = 0; i < BLOCK_SIZE / sizeof(VXFS_INODE); i++) {
        if (buf[i].InodeID == inodeID) { node = buf[i]; break; }
    }
    bs->FreePool(buf);
    return node;
}

static VXFS_EXTENT ReadExtent(
    EFI_BOOT_SERVICES*     bs,
    EFI_BLOCK_IO_PROTOCOL* bio,
    VXFS_SUPERBLOCK*       sb,
    UINTN                  partStartLBA,
    uint16_t               tableID,
    uint16_t               extentID)
{
    UINTN lba = partStartLBA
              + sb->ExtentTableStart
              + sb->ExtentTableSize * tableID
              + extentID * sizeof(VXFS_EXTENT) / BLOCK_SIZE;

    VXFS_EXTENT* buf = AllocZero(bs, BLOCK_SIZE);
    ReadSectors(bio, lba, BLOCK_SIZE, buf);

    VXFS_EXTENT ext = {0};
    for (UINTN i = 0; i < BLOCK_SIZE / sizeof(VXFS_EXTENT); i++) {
        if (buf[i].ExtentID == extentID) { ext = buf[i]; break; }
    }
    bs->FreePool(buf);
    return ext;
}

static VOID* ReadExtentData(
    EFI_BOOT_SERVICES*     bs,
    EFI_BLOCK_IO_PROTOCOL* bio,
    VXFS_EXTENT*           ext,
    UINTN                  partStartLBA)
{
    UINTN bytes = ext->SizeInSectors * BLOCK_SIZE;
    VOID* buf   = AllocZero(bs, bytes);
    ReadSectors(bio, partStartLBA + ext->StartSector, bytes, buf);
    return buf;
}

static VXFS_DIRENTRY* FindDirEntry(VOID* dirdata, const CHAR8* name)
{
    UINTN name_len = strlen((CHAR8*)name);
    UINTN ptr = 0;
    while (1) {
        VXFS_DIRENTRY* e = (VXFS_DIRENTRY*)((UINTN)dirdata + ptr);
        ptr += sizeof(VXFS_DIRENTRY);
        if (e->NameLenght == 0 || e->valid != 1) break;
        if (strlen((CHAR8*)name) == e->NameLenght && name_len == e->NameLenght) return e;
        ptr += e->NameLenght;
    }
    return NULL;
}

static VXFS_EXTENT InodeToExtent(
    EFI_BOOT_SERVICES*     bs,
    EFI_BLOCK_IO_PROTOCOL* bio,
    VXFS_SUPERBLOCK*       sb,
    UINTN                  partStartLBA,
    VXFS_INODE*            node)
{
    return ReadExtent(bs, bio, sb, partStartLBA, node->ExtentTableID, node->ExtentID);
}

static VOID* LoadElf(EFI_BOOT_SERVICES* bs, VOID* kernel)
{
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)kernel;

    if (ehdr->e_type != ET_DYN) {
        cout->OutputString(cout, u"Kernel is not correct executable type\r\n");
        while (1) {}
    }

    Elf64_Phdr* phdr = (Elf64_Phdr*)((UINT8*)ehdr + ehdr->e_phoff);

    UINTN max_align = PAGE_SIZE;
    UINTN mem_min = UINT64_MAX, mem_max = 0;

    for (UINT16 i = 0; i < ehdr->e_phnum; i++, phdr++) {
        if (phdr->p_type != PT_LOAD) continue;
        if (max_align < phdr->p_align) max_align = phdr->p_align;
        UINTN begin = phdr->p_vaddr & ~(max_align - 1);
        UINTN end   = (phdr->p_vaddr + phdr->p_memsz + max_align - 1) & ~(max_align - 1);
        if (begin < mem_min) mem_min = begin;
        if (end   > mem_max) mem_max = end;
    }

    UINTN mem_size = mem_max - mem_min;
    UINTN pages    = (mem_size + PAGE_SIZE - 1) / PAGE_SIZE;
    EFI_PHYSICAL_ADDRESS prog = 0;

    if (EFI_ERROR(bs->AllocatePages(AllocateAnyPages, EfiLoaderCode, pages, &prog))) {
        cout->OutputString(cout, u"Failed to allocate memory for kernel\r\n");
        while (1) {}
    }
    memset((VOID*)prog, 0, mem_size);

    phdr = (Elf64_Phdr*)((UINT8*)ehdr + ehdr->e_phoff);
    for (UINT16 i = 0; i < ehdr->e_phnum; i++, phdr++) {
        if (phdr->p_type != PT_LOAD) continue;
        memcpy((UINT8*)prog + (phdr->p_vaddr - mem_min),
               (UINT8*)kernel + phdr->p_offset,
               phdr->p_filesz);
    }

    return (VOID*)((UINT8*)prog + ehdr->e_entry - mem_min);
}

static BOOLEAN DetectQEMU(EFI_SYSTEM_TABLE* ST)
{
    // --- Method 1: CPUID hypervisor bit + brand string ---
    UINT32 ecx = 0;
    __asm__ volatile("cpuid" : "=c"(ecx) : "a"(1) : "ebx", "edx");
    if (ecx & (1 << 31)) {
        // Hypervisor present — read brand string
        UINT32 regs[4] = {0};
        __asm__ volatile("cpuid"
            : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
            : "a"(0x40000000));
        // regs[1..3] contain the hypervisor brand (12 chars)
        CHAR8* brand = (CHAR8*)&regs[1];
        if (AsciiStrStr(brand, "KVMKVM") || AsciiStrStr(brand, "TCGTCG"))
            return TRUE;
    }

    return FALSE;
}

EFI_FILE* LoadFile(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable){
	EFI_FILE* LoadedFile;

    EFI_GUID lip = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
	SystemTable->BootServices->HandleProtocol(ImageHandle, &lip, (void**)&LoadedImage);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;
    EFI_GUID SFSP = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &SFSP, (void**)&FileSystem);

	if (Directory == NULL){
		FileSystem->OpenVolume(FileSystem, &Directory);
	}

	EFI_STATUS s = Directory->Open(Directory, &LoadedFile, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (s != EFI_SUCCESS){
		return NULL;
	}
	return LoadedFile;

}

PSF1_FONT* LoadPSF1Font(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_FILE* font = LoadFile(Directory, Path, ImageHandle, SystemTable);
	if (font == NULL) return NULL;

	PSF1_HEADER* fontHeader;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER), (void**)&fontHeader);
	UINTN size = sizeof(PSF1_HEADER);
	font->Read(font, &size, fontHeader);

	if (fontHeader->magic[0] != PSF1_MAGIC0 || fontHeader->magic[1] != PSF1_MAGIC1){
		return NULL;
	}

	UINTN glyphBufferSize = fontHeader->charsize * 256;
	if (fontHeader->mode == 1) { //512 glyph mode
		glyphBufferSize = fontHeader->charsize * 512;
	}

	void* glyphBuffer;
	{
		font->SetPosition(font, sizeof(PSF1_HEADER));
		SystemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize, (void**)&glyphBuffer);
		font->Read(font, &glyphBufferSize, glyphBuffer);
	}

	PSF1_FONT* finishedFont;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void**)&finishedFont);
	finishedFont->psf1_Header = fontHeader;
	finishedFont->glyphBuffer = glyphBuffer;
	return finishedFont;

}



static Framebuffer SetupFramebuffer(EFI_SYSTEM_TABLE* ST)
{
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;

    if (EFI_ERROR(ST->BootServices->LocateProtocol(&gopGuid, NULL, (VOID**)&gop))) {
        cout->OutputString(cout, u"Failed to find GOP\r\n");
        while (1) {}
    }

    UINT32 bestMode   = 0;
    UINTN  bestPixels = 0;

    for (UINT32 i = 0; i < gop->Mode->MaxMode; i++) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
        UINTN infoSize;
        if (EFI_ERROR(gop->QueryMode(gop, i, &infoSize, &info))) continue;
        if ((info->HorizontalResolution > 1280 || info->VerticalResolution > 720) && DetectQEMU(ST)) continue;
        UINTN pixels = info->HorizontalResolution * info->VerticalResolution;
        if (pixels > bestPixels) { bestPixels = pixels; bestMode = i; }
    }

    gop->SetMode(gop, bestMode);

    Framebuffer fb = {0};
    fb.BaseAddress       = (VOID*)gop->Mode->FrameBufferBase;
    fb.BufferSize        = gop->Mode->FrameBufferSize;
    fb.Width             = gop->Mode->Info->HorizontalResolution;
    fb.Height            = gop->Mode->Info->VerticalResolution;
    fb.PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;
    return fb;
}


EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* ST)
{
    cout = ST->ConOut;
    cout->ClearScreen(cout);

    EFI_GUID    BlockIOGuid  = EFI_BLOCK_IO_PROTOCOL_GUID;
    UINTN       HandleCount  = 0;
    EFI_HANDLE* HandleBuffer = NULL;

    if (EFI_ERROR(ST->BootServices->LocateHandleBuffer(
            ByProtocol, &BlockIOGuid, NULL, &HandleCount, &HandleBuffer))) {
        cout->OutputString(cout, u"Failed LocateHandleBuffer\r\n");
    }

    cout->OutputString(cout, u"Handles found: ");
    PrintHex(HandleCount);
    print_nl();

    VxfsScanResult result = FindVxfsPartition(ST, ImageHandle, HandleBuffer, HandleCount);
    if (!result.bio) {
        cout->OutputString(cout, u"\r\nVxFS partition NOT found on any drive.\r\n");
        while (1) {}
    }

    EFI_BLOCK_IO_PROTOCOL* bio      = result.bio;
    UINTN                  partStart = result.PartitionStartLBA;

    cout->OutputString(cout, u"Partition start LBA: ");
    print_uint(partStart);
    print_nl();

    VXFS_SUPERBLOCK* sb = LoadSuperblock(ST->BootServices, bio, partStart);

    VXFS_INODE  root_inode = ReadInode(ST->BootServices, bio, sb, partStart, 0, (uint16_t)sb->RootInodeID);
    VXFS_EXTENT root_ext   = InodeToExtent(ST->BootServices, bio, sb, partStart, &root_inode);

    cout->OutputString(cout, u"Root Dir: ");  print_uint(root_ext.StartSector);   print_nl();
    cout->OutputString(cout, u"Root Size: "); print_uint(root_ext.SizeInSectors);  print_nl();

    VOID* root_dir = ReadExtentData(ST->BootServices, bio, &root_ext, partStart);

    VXFS_DIRENTRY* sys_entry = FindDirEntry(root_dir, "system");
    if (!sys_entry) {
        cout->OutputString(cout, u"Failed to find system directory\r\n");
        while (1) {}
    }
    cout->OutputString(cout, u"Found system dir\r\n");

    VXFS_INODE  sys_inode = ReadInode(ST->BootServices, bio, sb, partStart,
                                      sys_entry->InodeTableID, sys_entry->InodeID);
    VXFS_EXTENT sys_ext   = InodeToExtent(ST->BootServices, bio, sb, partStart, &sys_inode);

    cout->OutputString(cout, u"System Dir: ");  print_uint(sys_ext.StartSector);   print_nl();
    cout->OutputString(cout, u"System Size: "); print_uint(sys_ext.SizeInSectors);  print_nl();

    ST->BootServices->FreePool(root_dir);
    VOID* sys_dir = ReadExtentData(ST->BootServices, bio, &sys_ext, partStart);

    VXFS_DIRENTRY* kern_entry = FindDirEntry(sys_dir, "kernel");
    if (!kern_entry) {
        cout->OutputString(cout, u"Failed to find kernel\r\n");
        while (1) {}
    }
    cout->OutputString(cout, u"Found kernel\r\n");

    VXFS_INODE  kern_inode = ReadInode(ST->BootServices, bio, sb, partStart,
                                       kern_entry->InodeTableID, kern_entry->InodeID);
    VXFS_EXTENT kern_ext   = InodeToExtent(ST->BootServices, bio, sb, partStart, &kern_inode);

    cout->OutputString(cout, u"Kernel LBA: ");  print_uint(kern_ext.StartSector);   print_nl();
    cout->OutputString(cout, u"Kernel Size: "); print_uint(kern_ext.SizeInSectors);  print_nl();

    ST->BootServices->FreePool(sys_dir);

    VOID* kernel_buf = ReadExtentData(ST->BootServices, bio, &kern_ext, partStart);
    if (!memcmp(kernel_buf, (UINT8[4]){0x7f, 0x45, 0x4c, 0x46}, 4))
        cout->OutputString(cout, u"Kernel loaded into memory\r\n");

    VOID* entrypoint = LoadElf(ST->BootServices, kernel_buf);

    Framebuffer fb = SetupFramebuffer(ST);

    EFI_MEMORY_DESCRIPTOR* Map = NULL;
    UINTN MapSize = 0, MapKey = 0, DescriptorSize = 0;
    UINT32 DescriptorVersion = 0;

    ST->BootServices->FreePool(kernel_buf);
    ST->BootServices->FreePool(sb);

    ST->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
    ST->BootServices->AllocatePool(EfiLoaderData, MapSize, (VOID**)&Map);
    ST->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);

    typedef void (*kernel_fn_t)(bootinfo_t);
    kernel_fn_t __attribute__((sysv_abi)) kernel_main =
        (kernel_fn_t __attribute__((sysv_abi)))(uintptr_t)entrypoint;

    PSF1_FONT* font = LoadPSF1Font(NULL,u"zap-light16.psf",ImageHandle,ST);

    bootinfo_t info = {0};
    info.mMap           = Map;
    info.MapSize        = MapSize;
    info.DescriptorSize = DescriptorSize;
    info.framebuffer    = fb;
    info.bootfont = font;


    ST->BootServices->ExitBootServices(ImageHandle, MapKey);
    kernel_main(info);

    while (1) {}
    return EFI_SUCCESS;
}