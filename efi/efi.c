#include "efi.h"

#define VXFS_HEADER "VXFS"
#define BLOCK_SIZE 512

#define GROUP_SIZE_SECTORS   4096   
#define INODE_RATIO_SECTORS  32     
#define EXTENTS_PER_INODE    4

#define BIT(x) (1 << x)

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
}__attribute((packed))VXFS_DIRENTRY;

typedef enum
{
    DIR = BIT(0),
    SYSTEM = BIT(1),
    SYSLINK = BIT(2),
    HARDLINK = BIT(3)
}VXFS_FLAGS;

#define VXFS_ROOT_GUID \
 {0xa5e8bf06, 0x1238, 0x11f1, 0xb7, 0x4b, {0x00, 0x15, 0x5d, 0x0f, 0x3c, 0xbb}}

EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* cout;

void* memset(void* dest,uint8_t c,size_t n)
{
    uint8_t* dest8 = (uint8_t*)dest;
    for (size_t i = 0; i < n; i++)
    {
        dest8[i] = c;
    }
    
    return dest;
}

INTN memcmp(VOID *m1, VOID *m2, UINTN len) {
    UINT8 *p = m1;
    UINT8 *q = m2;
    for (UINTN i = 0; i < len; i++)
        if (p[i] != q[i]) return (INTN)(p[i]) - (INTN)(q[i]);

    return 0;
}


UINTN strlen_c16(CHAR16 *s) {
    UINTN len = 0;
    while (*s++) len++;
    return len;
}

UINTN strlen(CHAR8 *s) {
    UINTN len = 0;
    while (*s++) len++;
    return len;
}

CHAR16 *strrev_c16(CHAR16 *s) {
    if (!s) return s;

    CHAR16 *start = s, *end = s + strlen_c16(s)-1;
    while (start < end) {
        CHAR16 temp = *end;  // Swap
        *end-- = *start;
        *start++ = temp;
    }

    return s;
}

CHAR16 buffer[24];
CHAR16* itoa(UINTN number,UINT8 base)
{
    memset(buffer,0,24);
    const CHAR16 *digits = u"0123456789ABCDEF";

    int i = 0;

    do 
    {
        buffer[i++] = digits[number % base];
        number /= base;
    } while (number > 0);

    if (base == 16)
    {
        buffer[i++] = u'x';
        buffer[i++] = u'0';
    }

    strrev_c16(buffer);

    return buffer;
}

VOID print_bool(BOOLEAN b) {
    cout->OutputString(cout, b ? u"Y" : u"N");
}

VOID print_uint(UINTN val) {
    cout->OutputString(cout, itoa(val, 10));
}

VOID print_hex_dump(VOID *data, UINTN len) {
    UINT8 *bytes = (UINT8*)data;
    for (UINTN i = 0; i < len; i++) {
        // Print offset at the start of each row
        if (i % 16 == 0) {
            cout->OutputString(cout, itoa(i, 16));
            cout->OutputString(cout, u"  ");
        }

        // Pad single digit hex values with a leading zero
        if (bytes[i] < 0x10)
            cout->OutputString(cout, u"0");

        cout->OutputString(cout, itoa(bytes[i], 16));
        cout->OutputString(cout, u" ");

        // Newline + ASCII column at end of each row
        if (i % 16 == 15 || i == len - 1) {
            // Pad incomplete last row so ASCII column lines up
            UINTN col = i % 16;
            if (col != 15) {
                for (UINTN pad = col + 1; pad < 16; pad++)
                    cout->OutputString(cout, u"   ");
            }

            cout->OutputString(cout, u" |");

            // ASCII column - print dot for non-printable chars
            UINTN row_start = i - (i % 16);
            for (UINTN j = row_start; j <= i; j++) {
                CHAR16 ch[2] = { (bytes[j] >= 0x20 && bytes[j] < 0x7F) ? (CHAR16)bytes[j] : u'.', 0 };
                cout->OutputString(cout, ch);
            }

            cout->OutputString(cout, u"|\r\n");
        }
    }
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle,EFI_SYSTEM_TABLE* Systemtable)
{

    cout = Systemtable->ConOut;
    cout->ClearScreen(cout);
    EFI_STATUS status = EFI_SUCCESS;

    //Locate Block IO handles
    EFI_GUID BlockIOGuid = EFI_BLOCK_IO_PROTOCOL_GUID;
    UINTN HandleCount = 0;
    EFI_HANDLE* HandleBuffer = NULL;
    status = Systemtable->BootServices->LocateHandleBuffer(ByProtocol,&BlockIOGuid,NULL,&HandleCount,&HandleBuffer);
    if (status != EFI_SUCCESS)
    {
        cout->OutputString(cout,u"Failed dih");
    }


    EFI_GUID PartitionInfoGuid = EFI_PARTITION_INFO_PROTOCOL_GUID;
    EFI_GUID VxfsRootGuid = VXFS_ROOT_GUID;

    EFI_BLOCK_IO_PROTOCOL* VxfsBlockIO = NULL;  // Will point to the VxFS partition when found

    EFI_BLOCK_IO_PROTOCOL* blockio;
    for (UINTN index = 0; index < HandleCount; index++) {

        status = Systemtable->BootServices->OpenProtocol(
            HandleBuffer[index],
            &BlockIOGuid,
            (VOID**)&blockio,
            ImageHandle,
            NULL,
            EFI_OPEN_PROTOCOL_GET_PROTOCOL
        );

        if (EFI_ERROR(status) || !blockio || !blockio->Media) {
            cout->OutputString(cout, u"  Invalid BlockIO\r\n");
            continue;
        }

        // --- Partition Info ---
        EFI_PARTITION_INFO_PROTOCOL *partInfo = NULL;
        status = Systemtable->BootServices->OpenProtocol(
            HandleBuffer[index],
            &PartitionInfoGuid,
            (VOID**)&partInfo,
            ImageHandle,
            NULL,
            EFI_OPEN_PROTOCOL_GET_PROTOCOL
        );

        if (EFI_ERROR(status) || !partInfo) {

            continue;
        }

        if (partInfo->Type == PARTITION_TYPE_GPT) {

            // Check if this is the VxFS root partition
            if (memcmp(&partInfo->Info.Gpt.PartitionTypeGUID, &VxfsRootGuid, sizeof(EFI_GUID)) == 0) {
                cout->OutputString(cout, u"  *** VxFS ROOT PARTITION FOUND ***\r\n");
                VxfsBlockIO = blockio;
            }

        }
    }

    if (VxfsBlockIO) {
        VXFS_SUPERBLOCK *superblock = NULL;
        UINTN BlockIOPartitionOffset = 0;
        UINTN CurrentLBA = 0;
        status = Systemtable->BootServices->AllocatePool(EfiLoaderData, sizeof(VXFS_SUPERBLOCK), (VOID**)&superblock);
        if (EFI_ERROR(status) || !superblock) {
            cout->OutputString(cout, u"AllocatePool failed\r\n");
            return status;
        }
        memset(superblock, 0, sizeof(VXFS_SUPERBLOCK));

        status = VxfsBlockIO->ReadBlocks(VxfsBlockIO, VxfsBlockIO->Media->MediaId, CurrentLBA, sizeof(VXFS_SUPERBLOCK), (VOID*)superblock);
        BlockIOPartitionOffset += sizeof(VXFS_SUPERBLOCK);
        CurrentLBA++;
        if (EFI_ERROR(status)) {
            cout->OutputString(cout, u"ReadBlocks failed: ");
            cout->OutputString(cout, itoa(status, 16));
            cout->OutputString(cout, u"\r\n");

            return status;
        }
        if (memcmp(superblock->Header,VXFS_HEADER,4))
        {
            cout->OutputString(cout,u"Couldn't find superblock header");
            Systemtable->BootServices->FreePool(superblock);
            return EFI_NOT_FOUND;
        }

        CurrentLBA = superblock->InodeTablesStart  + (sizeof(VXFS_INODE) * superblock->RootInodeID) / 512;
        VXFS_INODE* inodetable = NULL;
        status = Systemtable->BootServices->AllocatePool(EfiLoaderData,512,(VOID**)&inodetable);
        if (EFI_ERROR(status))
        {
            cout->OutputString(cout,u"AllocatePool failed");

            return status;
        }
        status = VxfsBlockIO->ReadBlocks(VxfsBlockIO,VxfsBlockIO->Media->MediaId,CurrentLBA,512,(VOID*)inodetable);
        if (EFI_ERROR(status))
        {
            cout->OutputString(cout, u"ReadBlocks failed: ");
            cout->OutputString(cout, itoa(status, 16));
            cout->OutputString(cout, u"\r\n");

            return status;
        }
        VXFS_INODE rootinode = inodetable[superblock->RootInodeID];
        cout->OutputString(cout,u"Rootnode extent ID: ");
        cout->OutputString(cout,itoa(rootinode.ExtentID,10));
        cout->OutputString(cout,u"\r\n");

        VXFS_EXTENT rootextent = {0};
        CurrentLBA = superblock->ExtentTableStart;
        VXFS_EXTENT* extents = NULL;
        status = Systemtable->BootServices->AllocatePool(EfiLoaderData,512,(VOID**)&extents);
        if (EFI_ERROR(status))
        {
            cout->OutputString(cout,u"AllocatePool failed");

            return status;
        }
        status = VxfsBlockIO->ReadBlocks(VxfsBlockIO,VxfsBlockIO->Media->MediaId,CurrentLBA,512,(VOID*)extents);
        if (EFI_ERROR(status))
        {
            cout->OutputString(cout, u"ReadBlocks failed: ");
            cout->OutputString(cout, itoa(status, 16));
            cout->OutputString(cout, u"\r\n");

            return status;
        }

        rootextent = extents[rootinode.ExtentID];
        cout->OutputString(cout,u"Root Dir: ");
        print_uint(rootextent.StartSector);
        cout->OutputString(cout,u"\r\n");
        cout->OutputString(cout,u"Root Dir Size: ");
        print_uint(rootextent.SizeInSectors);
        cout->OutputString(cout,u"\r\n");

        VOID* direntries = NULL;
        status = Systemtable->BootServices->AllocatePool(EfiLoaderData,rootextent.SizeInSectors * BLOCK_SIZE,(VOID**)&direntries);
        if (EFI_ERROR(status))
        {
            cout->OutputString(cout,u"AllocatePool failed");

            return status;
        }
        CurrentLBA = rootextent.StartSector;
        status = VxfsBlockIO->ReadBlocks(VxfsBlockIO,VxfsBlockIO->Media->MediaId,CurrentLBA,rootextent.SizeInSectors * BLOCK_SIZE,(VOID*)direntries);
        if (EFI_ERROR(status))
        {
            cout->OutputString(cout, u"ReadBlocks failed: ");
            cout->OutputString(cout, itoa(status, 16));
            cout->OutputString(cout, u"\r\n");

            return status;
        }
        UINTN ptr = 0;
        VXFS_DIRENTRY* entry = NULL;
        while (1)
        {
            VXFS_DIRENTRY* direntry = (VXFS_DIRENTRY*)((UINTN)(direntries) + ptr);
            ptr += sizeof(VXFS_DIRENTRY);
            if (direntry->NameLenght == 0 || direntry->valid != 1) break;
            if (strlen("system") == direntry->NameLenght) {
                cout->OutputString(cout,u"Found system dir");
                entry = direntry;
                break;
            }
            ptr += direntry->NameLenght;
        }

        if (entry == NULL)
        {
            cout->OutputString(cout,u"Failed to find system directory filesystem may be corrupt");
            //Maybe add recovery here
            return EFI_NOT_FOUND;
        }


        Systemtable->BootServices->FreePool(superblock);
    }else {
        cout->OutputString(cout, u"\r\nVxFS partition NOT found.\r\n");
        return EFI_NOT_FOUND;
    }

    while(1) {}

    return EFI_SUCCESS;
}